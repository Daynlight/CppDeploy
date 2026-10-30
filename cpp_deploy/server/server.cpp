// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include "server.h"



// === Constructors
CD::Server::Server(std::atomic<size_t>& update_request_count, std::atomic<bool>& g_running)
  : update_request_count(update_request_count), 
    g_running(g_running),
    acceptor(io_context) {
  CD::Logger::get().print(CD::LS::ServerBold, "=== Default Param Constructor Called ===");
  onStart();
};



CD::Server::~Server() {
  CD::Logger::get().print(CD::LS::ServerBold, "=== Destructor Called ===");
  onEnd();
};



// === Operations
void CD::Server::onStart() {
  CD::Logger::get().print(CD::LS::Server, "onStart Called");
  if (!server_thread.joinable()) {
    server_thread = std::thread([this](){ runServer(); });
    CD::Logger::get().print(CD::LS::Server, "server_handler started successfully");
  } else CD::Logger::get().print(CD::LS::Server, "server_handler is already running");
};



void CD::Server::onEnd() {
  CD::Logger::get().print(CD::LS::Server, "onEnd Called");
  
  server_stop_requested = true;
  
  boost::system::error_code ec;
  acceptor.close(ec);
  if (ec) CD::Logger::get().print(CD::LS::ServerError, "Error closing acceptor: {}", ec.message());
  else CD::Logger::get().print(CD::LS::Server, "Acceptor closed successfully");
  
  try {
    boost::asio::ip::tcp::socket wakeup_socket(io_context);
    wakeup_socket.connect(boost::asio::ip::tcp::endpoint(
      boost::asio::ip::make_address("127.0.0.1"), CD::Config::PORT
    ), ec);
  } catch (...) {};
  
  if(!io_context.stopped()) io_context.stop();
  if(io_context.stopped()) CD::Logger::get().print(CD::LS::Server, "io_context stopped");
  else CD::Logger::get().print(CD::LS::ServerError, "io_context is still working"); 

  if (server_thread.joinable()) server_thread.join();
  if(!server_thread.joinable()) CD::Logger::get().print(CD::LS::Server, "server_thread stopped");
  else CD::Logger::get().print(CD::LS::ServerError, "server_thread is still working"); 

  std::lock_guard<std::mutex> lock(threads_mutex);
  for (auto& el : worker_threads) if (el.joinable()) el.join();
  worker_threads.clear();
  if(worker_threads.empty()) CD::Logger::get().print(CD::LS::Server, "worker_threads cleaned");
  else CD::Logger::get().print(CD::LS::ServerError, "worker_threads still in use"); 
};



// === Helpers
void CD::Server::runServer() {
  CD::Logger::get().print(CD::LS::ServerThread, "runServer Called");
  
  try {
    acceptor.open(boost::asio::ip::tcp::v4());
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(boost::asio::ip::tcp::endpoint(CD::Config::ADDRESS, CD::Config::PORT));
    acceptor.listen();
    
    CD::Logger::get().print(CD::LS::ServerThread, "Listening on http://{}:{}...", acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());

    while (g_running && !server_stop_requested) {
      boost::asio::ip::tcp::socket socket(io_context);
      boost::system::error_code ec;
      
      acceptor.accept(socket, ec);
      if (ec || !g_running) break;
      else CD::Logger::get().print(CD::LS::ServerThread, "Registered Client Call");

      {
        std::lock_guard<std::mutex> lock(threads_mutex);
        worker_threads.emplace_back([this, socket = std::move(socket)]() mutable { handleClient(socket); });
      }

      worker_threads.erase(
        std::remove_if(worker_threads.begin(), worker_threads.end(), [](std::thread& el) {
          if (el.joinable()) return false; 
          return true; 
        }
      ), worker_threads.end());
    };
  } catch (std::exception& e) {
    CD::Logger::get().print(CD::LS::ServerThreadError, "Fatal Error: {}", e.what());
  };

  boost::system::error_code ec;
  acceptor.close(ec);
  if (ec) CD::Logger::get().print(CD::LS::ServerThreadError, "Error closing acceptor: {}", ec.message()); 
  else CD::Logger::get().print(CD::LS::ServerThread, "Acceptor closed successfully");

  CD::Logger::get().print(CD::LS::ServerThread, "runServer Ended");
};



void CD::Server::handleClient(boost::asio::ip::tcp::socket& socket) {
  CD::Logger::get().print(CD::LS::ServerClientHandler, "handleClient Called");
  
  try {
    boost::system::error_code endpoint_ec;
    boost::asio::ip::tcp::endpoint remote_endpoint = socket.remote_endpoint(endpoint_ec);
    boost::asio::ip::address client_ip = endpoint_ec ? boost::asio::ip::address() : remote_endpoint.address();
    if(!endpoint_ec && client_ip.is_loopback()) CD::Logger::get().print(CD::LS::ServerClientHandler, "Called from IP: {}", client_ip.to_string());
    else {
      std::string ip_str = endpoint_ec ? "unknown" : client_ip.to_string();
      CD::Logger::get().print(CD::LS::ServerClientHandlerError, "Called from IP: {} ! Rejecting !", ip_str); 
      sendRespond(CD::SR::Forbidden, socket);
      return;
    };

    boost::asio::streambuf request_buf(CD::Config::CLIENT_MAX_BUFFER);
    boost::asio::read_until(socket, request_buf, "\r\n\r\n");
    std::istream request_stream(&request_buf);
    std::string method, uri, version;
    
    if (!(request_stream >> method >> uri >> version)) {
      CD::Logger::get().print(CD::LS::ServerClientHandlerError, "failed to parse http request line");
      std::string ip_str = endpoint_ec ? "unknown" : client_ip.to_string();
      CD::Logger::get().print(CD::LS::ServerClientHandlerError, "called from ip: {} ! rejecting !", ip_str); 
      sendRespond(CD::SR::Forbidden, socket);
      return;
    };
    
    if (uri.length() > CD::Config::MAX_URI_SIZE) {
      CD::Logger::get().print(CD::LS::ServerClientHandlerError, "uri too long");
      std::string ip_str = endpoint_ec ? "unknown" : client_ip.to_string();
      CD::Logger::get().print(CD::LS::ServerClientHandlerError, "called from ip: {} ! rejecting !", ip_str); 
      sendRespond(CD::SR::Forbidden, socket);
      return;
    };

    std::string header;
    std::size_t content_length = 0;   
    while (std::getline(request_stream, header)) {
      if (header == "\r" || header.empty()) break;
      
      std::smatch match;
      if (std::regex_search(header, match, CD::Config::FIND_CONTENT_LENGTH)) {
        try {
          content_length = std::stoul(match[1].str());
        } catch (...) {
          CD::Logger::get().print(CD::LS::ServerClientHandlerError, "Content-Length numeric overflow");
          sendRespond(CD::SR::Bad_Request, socket);
          return;
        };

        break; 
      };
    };

    if (content_length > CD::Config::CLIENT_MAX_BODY_SIZE) {
      CD::Logger::get().print(CD::LS::ServerClientHandlerError, "Payload too large: {} bytes (Limit: {})", content_length, CD::Config::CLIENT_MAX_BODY_SIZE);      
      sendRespond(CD::SR::Payload_To_Large, socket);
      return;
    };

    std::string body;
    if (content_length > 0) {
      body.reserve(content_length);

      while (body.length() < content_length && request_stream.peek() != EOF)
        body.push_back(static_cast<char>(request_stream.get()));

      if (body.length() < content_length) {
        std::size_t remaining = content_length - body.length();
        std::vector<char> temp_buffer(remaining);
        
        boost::system::error_code read_ec;
        boost::asio::read(socket, boost::asio::buffer(temp_buffer), read_ec);
        
        if (read_ec) {
          CD::Logger::get().print(CD::LS::ServerClientHandlerError, "Socket read error while fetching body: {}", read_ec.message());
          boost::system::error_code write_ec;
          sendRespond(CD::SR::Bad_Request, socket, &write_ec);
          return;
        };
        
        body.append(temp_buffer.begin(), temp_buffer.end());
      };
    };

    CD::Logger::get().print(CD::LS::ServerClientHandler, "Method: {} | URI: {}", method, uri);
    CD::Logger::get().print(CD::LS::ServerClientHandler, "Payload Body: {}", body);

    if (uri == "/api/update" && method == "POST") {
      CD::Logger::get().print(CD::LS::ServerClientHandler, "API Triggered adding update request");
      update_request_count++;
    };

    std::string respond = "Connection: Close\r\n\r\n\"{status\":\"success\",\"message\":\"added update request\"}";
    sendRespond(CD::RC::Ok, CD::CT::Json, respond, socket);

  } catch (std::exception& e) {
    CD::Logger::get().print(CD::LS::ServerClientHandlerError, "Fatal Error: {}", e.what());
  };

  CD::Logger::get().print(CD::LS::ServerClientHandler, "handleClient Ended");
};



void CD::Server::sendRespond(const std::string& code, const std::string& type, const std::string& message, boost::asio::ip::tcp::socket& socket, boost::system::error_code* err){
  std::string type_line = type.empty() ? "" : type + "\r\n";
  std::string respond = fmt::format("{}\r\n{}content-length: {}\r\n{}\r\n\r\n", code, type_line, message.size(), message);

  if(err != nullptr) boost::asio::write(socket, boost::asio::buffer(respond), *err);
  else boost::asio::write(socket, boost::asio::buffer(respond));
  
  CD::Logger::get().print(CD::LS::ServerClientHandler, "respond: {}", respond);
};



void CD::Server::sendRespond(CD::ShortRespond respond, boost::asio::ip::tcp::socket& socket, boost::system::error_code* err) {
  sendRespond(respond.code, respond.type, respond.message, socket, err);
};


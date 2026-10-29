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
  fmt::println(fg(CD::Config::SERVER_LOG) | fmt::emphasis::bold, "[Server] === Default Param Constructor Called ===");
  onStart();
};



CD::Server::~Server() {
  fmt::println(fg(CD::Config::SERVER_LOG) | fmt::emphasis::bold, "[Server] === Destructor Called ===");
  onEnd();
};



// === Operations
void CD::Server::onStart() {
  fmt::println(fg(CD::Config::SERVER_LOG), "[Server] onStart Called");
  if (!server_thread.joinable()) {
    server_thread = std::thread([this](){ runServer(); });
    fmt::println(fg(CD::Config::SERVER_LOG), "[Server] server_handler started successfully");
  }
  else fmt::println(fg(CD::Config::SERVER_LOG), "[Server] server_handler is already running");
};



void CD::Server::onEnd() {
  fmt::println(fg(CD::Config::SERVER_LOG), "[Server] onEnd Called");
  
  server_stop_requested = true;
  
  boost::system::error_code ec;
  acceptor.close(ec);
  if (ec) fmt::println(fg(CD::Config::ERROR_LOG), "[Server] Error closing acceptor: {}", ec.message());
  else fmt::println(fg(CD::Config::SERVER_LOG), "[Server] Acceptor closed successfully");
  
  try {
    boost::asio::ip::tcp::socket wakeup_socket(io_context);
    wakeup_socket.connect(boost::asio::ip::tcp::endpoint(
      boost::asio::ip::make_address("127.0.0.1"), CD::Config::PORT
    ), ec);
  } catch (...) {};
  
  if(!io_context.stopped()) io_context.stop();
  if(io_context.stopped()) fmt::println(fg(CD::Config::SERVER_LOG), "[Server] io_context stopped");
  else fmt::println(fg(CD::Config::ERROR_LOG), "[Server] io_context is still working"); 

  if (server_thread.joinable()) server_thread.join();
  if(!server_thread.joinable()) fmt::println(fg(CD::Config::SERVER_LOG), "[Server] server_thread stopped");
  else fmt::println(fg(CD::Config::ERROR_LOG), "[Server] server_thread is still working"); 

  std::lock_guard<std::mutex> lock(threads_mutex);
  for (auto& el : worker_threads) if (el.joinable()) el.join();
  worker_threads.clear();
  if(worker_threads.empty()) fmt::println(fg(CD::Config::SERVER_LOG), "[Server] worker_threads cleaned");
  else fmt::println(fg(CD::Config::ERROR_LOG), "[Server] worker_threads still in use"); 
};



// === Helpers
void CD::Server::runServer() {
  fmt::println(fg(CD::Config::SERVER_LOG), "[Server Thread] runServer Called");
  
  try {
    acceptor.open(boost::asio::ip::tcp::v4());
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(boost::asio::ip::tcp::endpoint(CD::Config::ADDRESS, CD::Config::PORT));
    acceptor.listen();
    
    fmt::println(fg(CD::Config::SERVER_LOG), "[Server Thread] Listening on http://{}:{}...", acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());

    while (g_running && !server_stop_requested) {
      boost::asio::ip::tcp::socket socket(io_context);
      boost::system::error_code ec;
      
      acceptor.accept(socket, ec);
      if (ec || !g_running) break;
      else fmt::println(fg(CD::Config::SERVER_LOG), "[Server Thread] Registered Client Call");

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
    fmt::println(fg(CD::Config::ERROR_LOG), "[Server Thread] Fatal Error: {}", e.what());
  };

  boost::system::error_code ec;
  acceptor.close(ec);
  if (ec) fmt::println(fg(CD::Config::ERROR_LOG), "[Server Thread] Error closing acceptor: {}", ec.message()); 
  else fmt::println(fg(CD::Config::SERVER_LOG), "[Server Thread] Acceptor closed successfully");

  fmt::println(fg(CD::Config::SERVER_LOG), "[Server Thread] runServer Ended");
};



void CD::Server::handleClient(boost::asio::ip::tcp::socket& socket) {
  fmt::println(fg(CD::Config::SERVER_LOG), "[Server Client Handler] handleClient Called");
  
  try {
    boost::system::error_code endpoint_ec;
    boost::asio::ip::tcp::endpoint remote_endpoint = socket.remote_endpoint(endpoint_ec);
    boost::asio::ip::address client_ip = endpoint_ec ? boost::asio::ip::address() : remote_endpoint.address();
    if(!endpoint_ec && client_ip.is_loopback()) fmt::println(fg(CD::Config::SERVER_LOG), "[Server Client Handler] Called from IP: {}", client_ip.to_string());
    else {
      std::string ip_str = endpoint_ec ? "unknown" : client_ip.to_string();
      fmt::println(fg(CD::Config::ERROR_LOG), "[Server Client Handler] Called from IP: {} ! Rejecting !", ip_str); 
      std::string err_response = "HTTP/1.1 403 Forbidden\r\nContent-Length: 15\r\nConnection: close\r\n\r\nAccess Denied";
      boost::asio::write(socket, boost::asio::buffer(err_response));
      return;
    };

    boost::asio::streambuf request_buf(CD::Config::CLIENT_MAX_BUFFER);
    boost::asio::read_until(socket, request_buf, "\r\n\r\n");
    std::istream request_stream(&request_buf);
    std::string method, uri, version;
    
    if (!(request_stream >> method >> uri >> version)) {
      fmt::println(fg(CD::Config::ERROR_LOG), "[Server Client Handler] failed to parse http request line");
      std::string ip_str = endpoint_ec ? "unknown" : client_ip.to_string();
      fmt::println(fg(CD::Config::ERROR_LOG), "[Server Client Handler] called from ip: {} ! rejecting !", ip_str); 
      std::string err_response = "http/1.1 403 forbidden\r\ncontent-length: 15\r\nconnection: close\r\n\r\naccess denied";
      boost::asio::write(socket, boost::asio::buffer(err_response));
      return;
    };
    
    if (uri.length() > CD::Config::MAX_URI_SIZE) {
      fmt::println(fg(CD::Config::ERROR_LOG), "[Server Client Handler] uri too long");
      std::string ip_str = endpoint_ec ? "unknown" : client_ip.to_string();
      fmt::println(fg(CD::Config::ERROR_LOG), "[Server Client Handler] called from ip: {} ! rejecting !", ip_str); 
      std::string err_response = "http/1.1 403 forbidden\r\ncontent-length: 15\r\nconnection: close\r\n\r\naccess denied";
      boost::asio::write(socket, boost::asio::buffer(err_response));
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
          fmt::println(fg(CD::Config::ERROR_LOG), "[Server Client Handler] Content-Length numeric overflow");
          std::string err_response = "http/1.1 400 Bad Request\r\ncontent-length: 11\r\nconnection: close\r\n\r\nBad Request";
          boost::asio::write(socket, boost::asio::buffer(err_response));
          return;
        };

        break; 
      };
    };

    if (content_length > CD::Config::CLIENT_MAX_BODY_SIZE) {
      fmt::println(fg(CD::Config::ERROR_LOG), "[Server Client Handler] Payload too large: {} bytes (Limit: {})", content_length, CD::Config::CLIENT_MAX_BODY_SIZE);      
      std::string err_response = "http/1.1 413 Payload Too Large\r\ncontent-length: 17\r\nconnection: close\r\n\r\nPayload Too Large";
      boost::asio::write(socket, boost::asio::buffer(err_response));
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
          fmt::println(fg(CD::Config::ERROR_LOG), "[Server Client Handler] Socket read error while fetching body: {}", read_ec.message());
          std::string err_response = "http/1.1 400 Bad Request\r\ncontent-length: 11\r\nconnection: close\r\n\r\nBad Request";
          boost::system::error_code write_ec;
          boost::asio::write(socket, boost::asio::buffer(err_response), write_ec);
          return;
        };
        
        body.append(temp_buffer.begin(), temp_buffer.end());
      };
    };

    fmt::println(fg(CD::Config::SERVER_LOG), "[Server Client Handler] Method: {} | URI: {}", method, uri);
    fmt::println(fg(CD::Config::SERVER_LOG), "[Server Client Handler] Payload Body: {}", body);

    if (uri == "/api/update" && method == "POST") {
      fmt::println(fg(CD::Config::SERVER_LOG), "[Server Client Handler] API Triggered adding update request");
      update_request_count++;
    };

    std::string json_body = "{\"status\":\"success\",\"message\":\"added update request\"}";
    std::string response = 
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: " + std::to_string(json_body.length()) + "\r\n"
      "Connection: close\r\n\r\n" 
      + json_body;

    boost::asio::write(socket, boost::asio::buffer(response));

  } catch (std::exception& e) {
    fmt::println(fg(CD::Config::ERROR_LOG), "[Server Client Handler] {}", e.what());
  };

  fmt::println(fg(CD::Config::SERVER_LOG), "[Server Client Handler] handleClient Ended");
};


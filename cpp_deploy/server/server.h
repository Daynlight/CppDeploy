// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <regex>

#include <boost/asio.hpp>

#include "../logger/logger.h"
#include "../config.h"



namespace CD {
struct ShortRespond {
  std::string code;
  std::string type;
  std::string message;
};



namespace RC {
  inline const std::string Ok               = "HTTP/1.1 200 OK";
  inline const std::string Bad_Request      = "http/1.1 400 Bad Request";
  inline const std::string Forbidden        = "HTTP/1.1 403 Forbidden";
  inline const std::string Payload_To_Large = "http/1.1 413 Payload Too Large";
};



namespace CT {
  inline const std::string None = "";
  inline const std::string Json = "Content-Type: application/json";
};



namespace SR {
  inline const CD::ShortRespond Bad_Request       = { CD::RC::Bad_Request, CD::CT::None,      "Connection: Close\r\n\r\nBad Request" };
  inline const CD::ShortRespond Forbidden         = { CD::RC::Forbidden, CD::CT::None,        "Connection: Close\r\n\r\nAccess Deniede" };
  inline const CD::ShortRespond Payload_To_Large  = { CD::RC::Payload_To_Large, CD::CT::None, "Connection: Close\r\n\r\nPayload Too Large" };
};



class Server {
// =========================== //
// === Data ================== //
// =========================== //
private:
  std::atomic<size_t>& update_request_count;

  std::atomic<bool> is_running = false;
  std::thread server_thread;
  std::vector<std::thread> worker_threads;
  std::mutex threads_mutex;
  
  boost::asio::io_context io_context;
  boost::asio::ip::tcp::acceptor acceptor;



// =========================== //
// === Functional ============ //
// =========================== //
// === Constructors
public:
  Server(std::atomic<size_t>& update_request_count);
  ~Server() noexcept;
  Server(const Server& second) = delete;
  Server& operator=(const Server& second) = delete;
  Server(Server&& second) = delete;
  Server& operator=(Server&& second) = delete;

// === Operations
  void onStart();
  void onEnd();

// === Helpers
private:
  void runServer();
  void handleClient(boost::asio::ip::tcp::socket& socket);
  std::pair<int, size_t> fetchContentSize(std::istream& request_stream, boost::asio::ip::tcp::socket& socket) const;
  std::pair<int, std::string> fetchBody(size_t content_length, std::istream& request_stream, boost::asio::ip::tcp::socket& socket) const;
  void sendRespond(const std::string& code, const std::string& type, const std::string& message, boost::asio::ip::tcp::socket& socket, boost::system::error_code* err = nullptr) const;
  void sendRespond(CD::ShortRespond request, boost::asio::ip::tcp::socket& socket, boost::system::error_code* err = nullptr) const;

// === Endpoint
private:
  void operateEndpoints(const std::string& uri, const std::string& method, boost::asio::ip::tcp::socket& socket);
  void updateEndpoint(boost::asio::ip::tcp::socket& socket);

};
};


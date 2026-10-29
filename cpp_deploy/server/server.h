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
#include <mutex>
#include <regex>

#include <boost/asio.hpp>
#include <fmt/base.h>
#include <fmt/color.h>

#include "../core/core.h"
#include "../config.h"



namespace CD{
class Server{
// =========================== //
// === Data ================== //
// =========================== //
private:
  std::atomic<bool> server_stop_requested{false};
  std::atomic<bool>& g_running;
  std::atomic<size_t>& update_request_count;

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
  Server(std::atomic<size_t>& update_request_count, std::atomic<bool>& g_running);
  ~Server();
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
};
};


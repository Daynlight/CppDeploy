// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include <unistd.h>
#include <csignal>
#include <thread>
#include <mutex>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <fmt/base.h>
#include <fmt/color.h>

#include "core/core.h"
#include "config.h"



bool g_running = true;
std::mutex g_core_mutex;

void handle_signal(int signal) {
  if (signal == SIGTERM || signal == SIGINT) {
    g_running = false;
  };
};



void handle_client(boost::asio::ip::tcp::socket socket, CD::Core& core) {
  try {
    boost::asio::streambuf request_buf;
    
    boost::asio::read_until(socket, request_buf, "\r\n\r\n");

    std::istream request_stream(&request_buf);
    std::string method, uri, version;
    request_stream >> method >> uri >> version;

    std::string header;
    std::size_t content_length = 0;
    while (std::getline(request_stream, header) && header != "\r") {
      if (header.rfind("Content-Length:", 0) == 0 || header.rfind("content-length:", 0) == 0) {
        content_length = std::stoul(header.substr(15));
      };
    };

    std::string body;
    if (content_length > 0) {
      std::string leftover(boost::asio::buffers_begin(request_buf.data()), boost::asio::buffers_end(request_buf.data()));
      
      if (leftover.length() >= content_length) {
        body = leftover.substr(0, content_length);
      } else {
        body = leftover;
        std::size_t remaining = content_length - body.length();
        std::vector<char> temp_buffer(remaining);
        boost::asio::read(socket, boost::asio::buffer(temp_buffer));
        body.append(temp_buffer.begin(), temp_buffer.end());
      };
    };

    fmt::println(fg(fmt::color::yellow), "[Assio] Method: {} | URI: {}", method, uri);
    fmt::println(fg(fmt::color::yellow), "[Assio] Payload Body: {}", body);

    if (uri == "/api/update" && method == "POST") {
      std::lock_guard<std::mutex> lock(g_core_mutex);
      fmt::println(fg(fmt::color::green), "[Assio] API Triggered! Executing core.onUpdate()...");
      core.onUpdate();
    };

    std::string json_body = "{\"status\":\"success\",\"message\":\"ok\"}";

    std::string response = 
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: " + std::to_string(json_body.length()) + "\r\n"
      "Connection: close\r\n\r\n" 
      + json_body;

    boost::asio::write(socket, boost::asio::buffer(response));

  } catch (std::exception& e) {
    fmt::println(fg(fmt::color::red), "[Assio Error] {}", e.what());
  };
};



void run_server(CD::Core& core) {
  try {
    boost::asio::io_context io_context;

    boost::asio::ip::tcp::acceptor acceptor(
      io_context, 
      boost::asio::ip::tcp::endpoint(CD::Config::ADDRESS, CD::Config::PORT)
    );

    fmt::println(fg(fmt::color::green), "Boost.Asio C++ Daemon listening on http://{}:{}...", CD::Config::ADDRESS.to_string(), CD::Config::PORT);

    while (g_running) {
      boost::asio::ip::tcp::socket socket(io_context);
      boost::system::error_code ec;
      
      acceptor.accept(socket, ec);
      if (ec || !g_running) break;

      std::thread(handle_client, std::move(socket), std::ref(core)).detach();
    }
  } catch (std::exception& e) {
    fmt::println(fg(fmt::color::red), "[Server Fatal Error] {}", e.what());
  };
};



int main() {
  setbuf(stdout, NULL);
  std::signal(SIGTERM, handle_signal);
  std::signal(SIGINT, handle_signal);

  CD::Core core;
  core.onStart();

  std::thread server_thread(run_server, std::ref(core));

  while (g_running) {
    core.onHealthCheck();
    std::this_thread::sleep_for(std::chrono::milliseconds(CD::Config::UPDATE_TIME_MS));
  };

  fmt::println(fg(fmt::color::red), "[Daemon] Shutting down...");
  core.onEnd();

  if (server_thread.joinable()) {
    server_thread.detach();
  };

  return 0;
};


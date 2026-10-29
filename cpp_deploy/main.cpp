// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include <unistd.h>
#include <csignal>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <fmt/base.h>
#include <fmt/color.h>

#include "core/core.h"
#include "server/server.h"
#include "config.h"



std::atomic<bool> g_running = true;



void handle_signal(int signal) {
  if (signal == SIGTERM || signal == SIGINT) {
    g_running = false;
  };
};



int main() {
  std::atomic<size_t> update_request_count = 1;
  setbuf(stdout, NULL);
  std::signal(SIGTERM, handle_signal);
  std::signal(SIGINT, handle_signal);

  CD::Core core;
  CD::Server server(update_request_count, g_running);
  server.onStart();
  core.onStart();

  while (g_running) {
    core.onHealthCheck();
    
    size_t pending_updates = update_request_count.exchange(0);
    for (size_t i = 0; i < pending_updates; ++i) core.onUpdate();

    std::this_thread::sleep_for(std::chrono::milliseconds(CD::Config::UPDATE_TIME_MS));
  };

  fmt::println(fg(fmt::color::red), "[Daemon] Shutting down...");
  core.onEnd();

  server.onEnd();

  return 0;
};


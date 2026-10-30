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
#include <chrono>

#include <fmt/base.h>
#include <fmt/color.h>

#include "core/core.h"
#include "server/server.h"
#include "config.h"
#include "logger/logger.h"



std::atomic<bool> g_running = true;



void handle_signal(int signal) {
  if (signal == SIGTERM || signal == SIGINT) {
    CD::Logger::get().print(CD::LS::SginalBold, "SIGTERM/SIGINT detected");
    g_running = false;
  };
};



int main() {
  CD::Logger::get().print(CD::LS::MainBold, "=== Main Starts ===");
  
  std::atomic<size_t> update_request_count = 1;
  setbuf(stdout, NULL);
  std::signal(SIGTERM, handle_signal);
  std::signal(SIGINT, handle_signal);
  CD::Logger::get().print(CD::LS::Main, "Signal handlers setted");
  
  CD::Core core;
  CD::Server server(update_request_count, g_running);
  server.onStart();
  core.onStart();
  CD::Logger::get().print(CD::LS::Main, "Core and Server started");

  while (g_running) {
    auto start_time = std::chrono::steady_clock::now();
    
    CD::Logger::get().print(CD::LS::Main, "Main loop called");
    
    core.onHealthCheck();
    CD::Logger::get().print(CD::LS::Main, "Core health check");
    
    size_t pending_updates = update_request_count.exchange(0);
    CD::Logger::get().print(CD::LS::Main, "Core update requests: {}", pending_updates);
    for (size_t i = 0; i < pending_updates; ++i) {
      core.onUpdate();
      CD::Logger::get().print(CD::LS::Main, "Core update request");
    };

    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<float> duration = end_time - start_time;
    CD::Logger::get().print(CD::LS::Main, "Main Loop Took: {:.3f}s", duration.count());
    
    auto elapsed = end_time - start_time;
    auto target_interval = std::chrono::milliseconds(CD::Config::UPDATE_TIME_MS);
    if (elapsed < target_interval) std::this_thread::sleep_for(target_interval - elapsed);
  };

  CD::Logger::get().print(CD::LS::MainBold, "Shutdown...");
  server.onEnd();
  core.onEnd();
  CD::Logger::get().print(CD::LS::Main, "Core and Server ended");

  return 0;
};


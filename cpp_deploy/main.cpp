// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include <csignal>
#include <atomic>
#include <chrono>

#include "core/core.h"
#include "logger/logger.h"



std::atomic<bool> g_running = true;



void handle_signal(int signal) {
  if (signal == SIGTERM || signal == SIGINT) {
    CD::Logger::get().print("log", CD::LS::SginalBold, "SIGTERM/SIGINT detected");
    g_running = false;
  };
};



int main() {
  auto start_time = std::chrono::steady_clock::now();
  
  {
    CD::Logger::get().print("log", CD::LS::MainBold, "=== Main Starts ===");
    
    setbuf(stdout, NULL);
    std::signal(SIGTERM, handle_signal);
    std::signal(SIGINT, handle_signal);
    CD::Logger::get().print("log", CD::LS::Main, "Signal handlers setted");
    
    CD::Core core(g_running);
    core.onStart();
    CD::Logger::get().print("log", CD::LS::Main, "Core started");

    while(g_running) core.loop();

    CD::Logger::get().print("log", CD::LS::MainBold, "Shutdown...");
    core.onEnd();
    CD::Logger::get().print("log", CD::LS::Main, "Core ended");
  }

  auto end_time = std::chrono::steady_clock::now();
  std::chrono::duration<float> duration = end_time - start_time;
  CD::Logger::get().print("log", CD::LS::Main, "Program Worked: {:%H:%M:%S}", duration);
  
  return 0;
};


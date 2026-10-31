// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#pragma once
#include <unordered_map>
#include <filesystem>
#include <string>
#include <fstream>
#include <regex>
#include <unistd.h>
#include <poll.h>
#include <termios.h>

#include <fmt/base.h>
#include <fmt/color.h>

#include "../service/service.h"
#include "../logger/logger.h"
#include "../server/server.h"
#include "../config.h"



namespace CD {
class Core {
// =========================== //
// === Data ================== //
// =========================== //
private:
  std::unordered_map<std::filesystem::path, CD::Service> services;
  std::filesystem::file_time_type last_time_write = {};
  std::atomic<size_t> update_request_count = 1;
  CD::Server server;
  std::atomic<bool>& g_running;
  std::thread render_thread;
  termios old_termios{};



// =========================== //
// === Functional ============ //
// =========================== //
// === Constructors
public:
  Core(std::atomic<bool>& g_running);
  ~Core();
  Core(const Core& second) = delete;
  Core& operator=(const Core& second) = delete;
  Core(Core&& second) = delete;
  Core& operator=(Core&& second) = delete;
  
// === Loop
public:
  void loop();
  void onStart();
  void onUpdate();
  void onHealthCheck();
  void onEnd();

// === Helpers
private:
  void parseServiceFile(const std::filesystem::path& path);
  void loadServices();
  void removeServices();

// === Windows
  void renderWindow();
  void commands(const std::string& input_buffer);

};
};


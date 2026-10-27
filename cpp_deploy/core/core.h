// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#pragma once
#include <unordered_map>
#include <filesystem>
#include <string>
#include <fstream>
#include <iterator>
#include <regex>

#include <fmt/base.h>
#include <fmt/color.h>

#include "../service/service.h"
#include "../config.h"



namespace CD{
class Core{
// =========================== //
// === Data ================== //
// =========================== //
private:
  std::unordered_map<std::filesystem::path, CD::Service> services;



// =========================== //
// === Functional ============ //
// =========================== //
// === Constructors
public:
  Core();
  ~Core();
  Core(const Core& second);
  Core& operator=(const Core& second);
  Core(Core&& second);
  Core& operator=(Core&& second);
  
// === Loop
public:
  void onStart();
  void onUpdate();
  void onEnd();

// === Helpers
private:
  void parseServiceFile(const std::filesystem::path& path);
  void loadServices();
  void removeServices();

};
};


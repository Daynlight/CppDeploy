// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#pragma once
#include <regex>
#include <string>
#include <filesystem>

#include <fmt/base.h>
#include <fmt/color.h>



namespace CD{
class Service{
// =========================== //
// === Data ================== //
// =========================== //
private:
  std::string name = "";
  std::string description = "";
  std::filesystem::path local_path = "";
  std::string repo_url = "";
  std::string prod_branch = "";
  std::string stage_branch = "";
  std::string docker_run_cmd = "";
  std::filesystem::path prod_env_path = "";
  std::filesystem::path stage_env_path = "";



// =========================== //
// === Functional ============ //
// =========================== //
// === Constructors
public:
  Service();
  ~Service();
  Service(const Service& second);
  Service& operator=(const Service& second);
  Service(Service&& second);
  Service& operator=(Service&& second);
  
// === Loop
public:
  void onStart();
  void onUpdate();
  void onEnd();

// === Getters/Setters
public:
  void loadFromFile(const std::string& config);

// === Helpers
private:
  void printInfo();

};
};


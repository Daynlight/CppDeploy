// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#pragma once
#include <regex>
#include <string>
#include <filesystem>
#include <vector>
#include <array>
#include <cstdio>
#include <unistd.h>
#include <fstream>
#include <iterator>
#include <sys/types.h>
#include <sys/wait.h>

#include <fmt/base.h>
#include <fmt/color.h>

#include "../config.h"



namespace CD{
class Service{
// =========================== //
// === Data ================== //
// =========================== //
private:
  std::filesystem::path config_path = "";
  std::filesystem::file_time_type last_time_write = {};
  std::string name = "";
  std::string description = "";
  std::filesystem::path local_path = "";
  std::filesystem::path env_path = "";
  std::string repo_url = "";
  std::string branch = "";
  std::string github_token = "";
  std::string run_cmd = "";
  std::string stop_cmd = "";
  std::string status_cmd = "";
  std::string test_cmd = "";
  size_t latest_version = -1;
  size_t current_running_version = -1;
  std::string local_latest_commit = "";



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
  void parseServiceFile(const std::filesystem::path& path);

// === Helpers
private:
  void printInfo();
  
  pid_t spawnProcess(const std::vector<std::string>& args);
  bool waitForProcess(pid_t pid);
  bool executeProcess(const std::vector<std::string>& args);
  
  void cloneRepo(size_t version);
  void removeDir(size_t version);
  bool runTests(size_t version);
  bool runContainer(size_t version);
  bool statusContainer(size_t version);
  bool stopContainer(size_t version);
  
  std::string getRemoteCommit();
  std::string getLocalCommit(size_t version);
 
  void findLatestVersions();
  void observeFile();
  void trimeFolders();
};
};


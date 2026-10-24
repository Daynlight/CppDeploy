// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#pragma once
#include <regex>
#include <string>
#include <filesystem>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <fmt/base.h>
#include <fmt/color.h>

#include "../config.h"



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
  std::string github_token = "";
  std::string prod_branch = "";
  std::string stage_branch = "";
  std::string docker_run_cmd = "";
  std::string docker_stop_cmd = "";
  std::string test_cmd = "";
  std::filesystem::path prod_env_path = "";
  std::filesystem::path stage_env_path = "";
  size_t latest_prod_version = 0;
  size_t latest_stage_version = 0;
  size_t current_prod_running_version = -1;



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
  
  pid_t spawnProcess(const std::vector<std::string>& args);
  bool waitForProcess(pid_t pid);
  bool executeProcess(const std::vector<std::string>& args);
  
  void cloneRepo(size_t version, bool prod);
  void removeDir(size_t version, bool prod);
  bool runTests(size_t version, bool prod);
  bool runContainer(size_t version, bool prod);
  bool stopContainer(size_t version, bool prod);
  
  bool getRemoteStageCommit(size_t version, bool pord);
  bool getRemoteProdCommit(size_t version, bool pord);
  bool getLocalStageCommit(size_t version, bool pord);
  bool getLocalProdCommit(size_t version, bool pord);
  
};
};


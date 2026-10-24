// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include "service.h"



// === Constructors
CD::Service::Service(){
  fmt::println(fg(fmt::color::yellow), "CD::Service::Service() Constructor Called");
};



CD::Service::~Service(){
  fmt::println(fg(fmt::color::yellow), "CD::Service::~Service() Destructor Called");
};



CD::Service::Service(const CD::Service& second)
  : name(second.name),
    description(second.description),
    local_path(second.local_path),
    repo_url(second.repo_url),
    prod_branch(second.prod_branch),
    stage_branch(second.stage_branch),
    docker_run_cmd(second.docker_run_cmd),
    prod_env_path(second.prod_env_path),
    stage_env_path(second.stage_env_path) {
  fmt::println(fg(fmt::color::yellow), "CD::Service::Service() Copy constructor Called");
};



CD::Service& CD::Service::operator=(const CD::Service& second){
  fmt::println(fg(fmt::color::yellow), "CD::Service::Service() Copy assign constructor Called");
  if(this == &second) return *this;

  name = second.name;
  description = second.description;
  local_path = second.local_path;
  repo_url = second.repo_url;
  prod_branch = second.prod_branch;
  stage_branch = second.stage_branch;
  docker_run_cmd = second.docker_run_cmd;
  prod_env_path = second.prod_env_path;
  stage_env_path = second.stage_env_path;

  return *this;
};



CD::Service::Service(CD::Service&& second)
  : name(std::move(second.name)),
    description(std::move(second.description)),
    local_path(std::move(second.local_path)),
    repo_url(std::move(second.repo_url)),
    prod_branch(std::move(second.prod_branch)),
    stage_branch(std::move(second.stage_branch)),
    docker_run_cmd(std::move(second.docker_run_cmd)),
    prod_env_path(std::move(second.prod_env_path)),
    stage_env_path(std::move(second.stage_env_path)) {
  fmt::println(fg(fmt::color::yellow), "CD::Service::Service() Move constructor Called");
};



CD::Service& CD::Service::operator=(CD::Service&& second){
  fmt::println(fg(fmt::color::yellow), "CD::Service::Service() Move assign constructor Called");
  if(this == &second) return *this;

  name = std::move(second.name);
  description = std::move(second.description);
  local_path = std::move(second.local_path);
  repo_url = std::move(second.repo_url);
  prod_branch = std::move(second.prod_branch);
  stage_branch = std::move(second.stage_branch);
  docker_run_cmd = std::move(second.docker_run_cmd);
  prod_env_path = std::move(second.prod_env_path);
  stage_env_path = std::move(second.stage_env_path);

  return *this;
};



// === Loop
void CD::Service::onStart(){
  fmt::println(fg(fmt::color::blue), "CD::Service::onStart called");
};



void CD::Service::onUpdate(){
  fmt::println(fg(fmt::color::blue), "CD::Service::onUpdate called");
  printInfo();
};



void CD::Service::onEnd(){
  fmt::println(fg(fmt::color::blue), "CD::Service::onEnd called");
};



// === Getters/Setters
void CD::Service::loadFromFile(const std::string& config){
  std::regex pattern(R"(([A-Za-z0-9_]+)\s*=\s*([^;]+)(?:;|$))");

  auto begin = std::sregex_iterator(config.begin(), config.end(), pattern);
  auto end = std::sregex_iterator();

  for (auto it = begin; it != end; ++it) {
    std::smatch match = *it;
    std::string key = match[1].str();
    std::string rawValue = match[2].str();

    size_t first = rawValue.find_first_not_of(" \t");
    size_t last = rawValue.find_last_not_of(" \t");
    std::string value = (first == std::string::npos) ? "" : rawValue.substr(first, (last - first + 1));
    
    if(key == "NAME") name = value;
    else if(key == "DESCRIPTION") description = value;
    else if(key == "LOCAL_PATH") local_path = std::filesystem::path(value);
    else if(key == "REPO_URL") repo_url = value;
    else if(key == "PROD_BRANCH") prod_branch = value;
    else if(key == "STAGE_BRANCH") stage_branch = value;
    else if(key == "DOCKER_RUN_CMD") docker_run_cmd = value;
    else if(key == "PROD_ENV_PATH") prod_env_path = std::filesystem::path(value);
    else if(key == "STAGE_ENV_PATH") stage_env_path = std::filesystem::path(value);
  };
};



// === Helpers
void CD::Service::printInfo(){
  fmt::println(fg(fmt::color::green), "=== {} Info ===", name);
  fmt::println(fg(fmt::color::green), "Description:\n{}", description);
  fmt::println(fg(fmt::color::green), "Local Path = {}", local_path.string());
  fmt::println(fg(fmt::color::green), "Repo URL = {}", repo_url);
  fmt::println(fg(fmt::color::green), "Prod Branch = {}", prod_branch);
  fmt::println(fg(fmt::color::green), "Stage Branch = {}", stage_branch);
  fmt::println(fg(fmt::color::green), "Docker run cmd = {}", docker_run_cmd);
  fmt::println(fg(fmt::color::green), "Prod Env Path = {}", prod_env_path.string());
  fmt::println(fg(fmt::color::green), "Stage Env Path = {}", stage_env_path.string());
};


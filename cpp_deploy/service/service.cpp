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
    docker_stop_cmd(second.docker_stop_cmd),
    prod_env_path(second.prod_env_path),
    stage_env_path(second.stage_env_path),
    test_cmd(second.test_cmd),
    github_token(second.github_token) {
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
  docker_stop_cmd = second.docker_stop_cmd;
  prod_env_path = second.prod_env_path;
  stage_env_path = second.stage_env_path;
  test_cmd = second.test_cmd;
  github_token = second.github_token;

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
    docker_stop_cmd(std::move(second.docker_stop_cmd)),
    prod_env_path(std::move(second.prod_env_path)),
    stage_env_path(std::move(second.stage_env_path)),
    test_cmd(std::move(second.test_cmd)),
    github_token(std::move(second.github_token)) {
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
  docker_stop_cmd = std::move(second.docker_stop_cmd);
  prod_env_path = std::move(second.prod_env_path);
  stage_env_path = std::move(second.stage_env_path);
  test_cmd = std::move(second.test_cmd);
  github_token = std::move(second.github_token);

  return *this;
};



// === Loop
void CD::Service::onStart(){
  fmt::println(fg(fmt::color::blue), "CD::Service::onStart called");
  printInfo();
  removeDir(0, false);
  cloneRepo(0, false);
  if(runTests(0, false)) {
    fmt::println("Tests Passed");
    runContainer(0, false);
  };
};



void CD::Service::onUpdate(){
  fmt::println(fg(fmt::color::blue), "CD::Service::onUpdate called");
};



void CD::Service::onEnd(){
  fmt::println(fg(fmt::color::blue), "CD::Service::onEnd called");
  stopContainer(0, false);
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
    else if(key == "DOCKER_STOP_CMD") docker_stop_cmd = value;
    else if(key == "PROD_ENV_PATH") prod_env_path = std::filesystem::path(value);
    else if(key == "STAGE_ENV_PATH") stage_env_path = std::filesystem::path(value);
    else if(key == "TEST_CMD") test_cmd = value;
    else if(key == "GITHUB_TOKEN") github_token = value;
  };
};



// === Helpers
void CD::Service::printInfo(){
  fmt::println(fg(fmt::color::purple), "=== {} Info ===", name);
  fmt::println(fg(fmt::color::purple), "Description:\n{}", description);
  fmt::println(fg(fmt::color::purple), "Local Path = {}", local_path.string());
  fmt::println(fg(fmt::color::purple), "Repo URL = {}", repo_url);
  fmt::println(fg(fmt::color::purple), "Prod Branch = {}", prod_branch);
  fmt::println(fg(fmt::color::purple), "Stage Branch = {}", stage_branch);
  fmt::println(fg(fmt::color::purple), "Docker run cmd = {}", docker_run_cmd);
  fmt::println(fg(fmt::color::purple), "Docker stop cmd = {}", docker_stop_cmd);
  fmt::println(fg(fmt::color::purple), "Prod Env Path = {}", prod_env_path.string());
  fmt::println(fg(fmt::color::purple), "Stage Env Path = {}", stage_env_path.string());
  fmt::println(fg(fmt::color::purple), "Test cmd = {}", test_cmd);
};



pid_t CD::Service::spawnProcess(const std::vector<std::string>& args) {
  if (args.empty()) return -1;

  std::vector<char*> c_args;
  for (const std::string& arg : args) {
    c_args.push_back(const_cast<char*>(arg.c_str()));
  };
  c_args.push_back(nullptr);

  pid_t pid = fork();

  if (pid < 0) {
    fmt::println(fg(fmt::color::red), "Failed to fork process for service.");
    return -1;
  } 
  else if (pid == 0) {
    execvp(c_args[0], c_args.data());
    fmt::println(fg(fmt::color::red), "Failed to execute binary: {}", c_args[0]);
    _exit(127);
  } 
  else {
    fmt::println(fg(fmt::color::green), "Spawned child process with PID: {}", pid);
    return pid;
  };
};



bool CD::Service::waitForProcess(pid_t pid) {
  if (pid <= 0) return false;

  int status = 0;
  pid_t result_pid = waitpid(pid, &status, 0);

  if (result_pid < 0) {
    fmt::println(fg(fmt::color::red), "Failed to wait for process with PID: {}", pid);
    return false;
  };

  if (WIFEXITED(status)) {
    int exit_code = WEXITSTATUS(status);
    if (exit_code == 0) {
      fmt::println(fg(fmt::color::green), "Process with PID {} finished successfully.", pid);
      return true;
    } else {
      fmt::println(fg(fmt::color::red), "Process with PID {} exited with error code: {}", pid, exit_code);
      return false;
    };
  } 
  else if (WIFSIGNALED(status)) {
    int signal_number = WTERMSIG(status);
    fmt::println(fg(fmt::color::red), "Process with PID {} was killed by signal: {}", pid, signal_number);
    return false;
  };

  return false;
};



bool CD::Service::executeProcess(const std::vector<std::string>& args){
  return waitForProcess(spawnProcess(args));
};



void CD::Service::cloneRepo(size_t version, bool prod){
  std::filesystem::path target_dir = local_path / ( prod ? CD::Config::SERVICE_PROD_PATH : CD::Config::SERVICE_STAGE_PATH ) / std::filesystem::path(std::to_string(version) + "/");
  std::string branch = prod ? prod_branch : stage_branch;
  
  if (!std::filesystem::exists(target_dir)) {
    std::error_code ec;
    if (std::filesystem::create_directories(target_dir, ec)) 
      fmt::println(fg(fmt::color::yellow), "Created missing service directory: {}", target_dir.string());
    else {
      fmt::println(fg(fmt::color::yellow), "Failed to create directory {} : {}", target_dir.string(), ec.message());
      return;
    };
  }
  else {
    fmt::println(fg(fmt::color::yellow), "Directory {} alredy exists", target_dir.string());
    return;
  };

  std::string authenticated_url = repo_url;
  if (!github_token.empty()) {
    std::string prefix = "https://";
    if (authenticated_url.rfind(prefix, 0) == 0) {
      authenticated_url.insert(prefix.length(), github_token + "@");
    };
  };

  executeProcess({"git", "clone", "-b", branch, authenticated_url, target_dir.string()});
};



void CD::Service::removeDir(size_t version, bool prod){
  std::filesystem::path target_dir = local_path / ( prod ? CD::Config::SERVICE_PROD_PATH : CD::Config::SERVICE_STAGE_PATH ) / std::filesystem::path(std::to_string(version) + "/");

  if (!std::filesystem::exists(target_dir)) {
    fmt::println(fg(fmt::color::yellow), "Directory does not exist, nothing to remove: {}", target_dir.string());
    return;
  };

  std::error_code ec;
  uintmax_t removed_count = std::filesystem::remove_all(target_dir, ec);

  if (ec)
    fmt::println(fg(fmt::color::red), "Failed to remove directory {} : {}", target_dir.string(), ec.message());
  else {
    fmt::println(fg(fmt::color::green), "Successfully removed directory: {} (deleted {} items)", target_dir.string(), removed_count);
  };
};



bool CD::Service::runTests(size_t version, bool prod){
  std::filesystem::path target_dir = local_path / ( prod ? CD::Config::SERVICE_PROD_PATH : CD::Config::SERVICE_STAGE_PATH ) / std::filesystem::path(std::to_string(version) + "/");
  
  if (!std::filesystem::exists(target_dir)) {
    fmt::println(fg(fmt::color::red), "Directory {} didn't exists", target_dir.string());
    return 0;
  };

  std::string full_command = "cd " + target_dir.string() + " && " + test_cmd;
  fmt::println(fg(fmt::color::cyan), "Running tests in version {}: {}", version, full_command);
  return executeProcess({"/bin/sh", "-c", full_command});
};



bool CD::Service::runContainer(size_t version, bool prod) {
  std::filesystem::path target_dir = local_path / ( prod ? CD::Config::SERVICE_PROD_PATH : CD::Config::SERVICE_STAGE_PATH ) / std::filesystem::path(std::to_string(version) + "/");
  
  if (!std::filesystem::exists(target_dir)) {
    fmt::println(fg(fmt::color::red), "Directory {} didn't exists", target_dir.string());
    return false;
  };

  std::string env_path = prod ? prod_env_path : stage_env_path;
  std::string modified_docker_cmd = docker_run_cmd;
  
  std::string target_prefix = "docker compose";
  size_t pos = modified_docker_cmd.find(target_prefix);
  
  if (pos != std::string::npos)
    modified_docker_cmd.insert(pos + target_prefix.length(), " --env-file " + env_path);

  std::string full_command = "cd " + target_dir.string() + " && " + modified_docker_cmd;
  fmt::println(fg(fmt::color::cyan), "Running docker container in version {}: {}", version, full_command); 
  return executeProcess({"/bin/sh", "-c", full_command});
};



bool CD::Service::stopContainer(size_t version, bool prod){
  std::filesystem::path target_dir = local_path / ( prod ? CD::Config::SERVICE_PROD_PATH : CD::Config::SERVICE_STAGE_PATH ) / std::filesystem::path(std::to_string(version) + "/");
  
  if (!std::filesystem::exists(target_dir)) {
    fmt::println(fg(fmt::color::red), "Directory {} didn't exists", target_dir.string());
    return false;
  };

  std::string env_path = prod ? prod_env_path : stage_env_path;
  std::string modified_docker_cmd = docker_stop_cmd;
  
  std::string target_prefix = "docker compose";
  size_t pos = modified_docker_cmd.find(target_prefix);
  
  if (pos != std::string::npos)
    modified_docker_cmd.insert(pos + target_prefix.length(), " --env-file " + env_path);

  std::string full_command = "cd " + target_dir.string() + " && " + modified_docker_cmd;
  fmt::println(fg(fmt::color::cyan), "Running docker container in version {}: {}", version, full_command); 
  return executeProcess({"/bin/sh", "-c", full_command});
};

// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include "service.h"



// === Constructors
CD::Service::Service(){
  color_id = CD::Logger::get().getServiceColor();
  CD::Logger::get().print("log", name, color_id, "CD::Service::Service() Constructor Called");
};



CD::Service::~Service(){
  CD::Logger::get().print("log", name, color_id, "CD::Service::~Service() Destructor Called");
};



CD::Service::Service(const CD::Service& second)
  : config_path(second.config_path),
    last_time_write(second.last_time_write),
    name(second.name),
    description(second.description),
    local_path(second.local_path),
    env_path(second.env_path),
    repo_url(second.repo_url),
    branch(second.branch),
    github_token(second.github_token), 
    run_cmd(second.run_cmd),
    stop_cmd(second.stop_cmd),
    status_cmd(second.status_cmd),
    test_cmd(second.test_cmd),
    latest_version(second.latest_version),
    current_running_version(second.current_running_version),
    local_latest_commit(second.local_latest_commit) {
  CD::Logger::get().print("log", name, color_id, "CD::Service::Service() Copy constructor Called");
  color_id = CD::Logger::get().getServiceColor();
};



CD::Service& CD::Service::operator=(const CD::Service& second){
  CD::Logger::get().print("log", name, color_id, "CD::Service::Service() Copy assign constructor Called"); 
  if(this == &second) return *this;

  config_path = second.config_path;
  last_time_write = second.last_time_write;
  name = second.name;
  description = second.description;
  local_path = second.local_path;
  env_path = second.env_path;
  repo_url = second.repo_url;
  branch = second.branch;
  github_token = second.github_token; 
  run_cmd = second.run_cmd;
  stop_cmd = second.stop_cmd;
  status_cmd = second.status_cmd;
  test_cmd = second.test_cmd;
  latest_version = second.latest_version;
  current_running_version = second.current_running_version;
  local_latest_commit = second.local_latest_commit;
  color_id = CD::Logger::get().getServiceColor();

  return *this;
};



CD::Service::Service(CD::Service&& second) 
  : config_path(std::move(second.config_path)),
    last_time_write(std::move(second.last_time_write)),
    name(std::move(second.name)),
    description(std::move(second.description)),
    local_path(std::move(second.local_path)),
    env_path(std::move(second.env_path)),
    repo_url(std::move(second.repo_url)),
    branch(std::move(second.branch)),
    github_token(std::move(second.github_token)), 
    run_cmd(std::move(second.run_cmd)),
    stop_cmd(std::move(second.stop_cmd)),
    status_cmd(std::move(second.status_cmd)),
    test_cmd(std::move(second.test_cmd)),
    latest_version(std::move(second.latest_version)),
    current_running_version(std::move(second.current_running_version)),
    local_latest_commit(std::move(second.local_latest_commit)),
    color_id(std::move(second.color_id)) {
  CD::Logger::get().print("log", name, color_id, "CD::Service::Service() Move constructor Called");
};



CD::Service& CD::Service::operator=(CD::Service&& second) {
  CD::Logger::get().print("log", name, color_id, "CD::Service::Service() Move assign constructor Called");
  if(this == &second) return *this;

  config_path = std::move(second.config_path);
  last_time_write = std::move(second.last_time_write);
  name = std::move(second.name);
  description = std::move(second.description);
  local_path = std::move(second.local_path);
  env_path = std::move(second.env_path);
  repo_url = std::move(second.repo_url);
  branch = std::move(second.branch);
  github_token = std::move(second.github_token); 
  run_cmd = std::move(second.run_cmd);
  stop_cmd = std::move(second.stop_cmd);
  status_cmd = std::move(second.status_cmd);
  test_cmd = std::move(second.test_cmd);
  latest_version = std::move(second.latest_version);
  current_running_version = std::move(second.current_running_version);
  local_latest_commit = std::move(second.local_latest_commit);
  color_id = std::move(second.color_id);

  return *this;
};



// === Loop
void CD::Service::onStart(){
  CD::Logger::get().print("log", name, color_id, "CD::Service::onStart called");
  printInfo();
  findLatestVersions();

  local_latest_commit = getLocalCommit(latest_version);
  
  if(runTests(latest_version)){
    if(latest_version > 0) stopContainer(latest_version - 1);  
    if(runContainer(latest_version)){
      current_running_version = latest_version;
      CD::Logger::get().print("log", name, color_id, "Latest Container running on version {}", current_running_version);
    };
      
    local_latest_commit = getLocalCommit(latest_version);
  } else {
    CD::Logger::get().print("log", name,  "Tests failed for version {}. Aborting update.", latest_version);
  };
};



void CD::Service::onUpdate(){
  CD::Logger::get().print("log", name, color_id, "CD::Service::onUpdate called");
  
  observeFile();
  std::string remote_latest_commit = getRemoteCommit();

  if(remote_latest_commit != local_latest_commit){
    CD::Logger::get().print("log", name, color_id, "New commit detected! Updating...");
    
    latest_version++;
    cloneRepo(latest_version);

    if(runTests(latest_version)){
      if(latest_version > 0) stopContainer(latest_version - 1);  
      if(runContainer(latest_version)){
        current_running_version = latest_version;
        CD::Logger::get().print("log", name, color_id, "Latest Container running on version {}", current_running_version);
      };
      
      local_latest_commit = getLocalCommit(latest_version);
    } else {
      CD::Logger::get().print("log", name,  "Tests failed for version {}. Aborting update.", latest_version);
    };
  } else {
    onHealthCheck();
  };
};



void CD::Service::onHealthCheck(){
  CD::Logger::get().print("log", name, color_id, "CD::Service::onHealthCheck called");
  
  observeFile();

  if(!statusContainer(latest_version)) {
    CD::Logger::get().print("log", name, color_id, "Containers are down. Attempting to restart version {}...", latest_version);
    if(!runContainer(latest_version)) {
      CD::Logger::get().print("log", name,  "Failed to restart containers on version {}", latest_version);
    } else {
      CD::Logger::get().print("log", name, color_id, "Containers successfully recovered and running!");
    }
  } else {
    CD::Logger::get().print("log", name, color_id,  "Container is up to date and running smoothly.");
  };
};



void CD::Service::onEnd(){
  CD::Logger::get().print("log", name, color_id, "CD::Service::onEnd called");
  stopContainer(current_running_version);
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
    else if(key == "ENV_PATH") env_path = std::filesystem::path(value);
    else if(key == "REPO_URL") repo_url = value;
    else if(key == "BRANCH") branch = value;
    else if(key == "GITHUB_TOKEN") github_token = value;
    else if(key == "RUN_CMD") run_cmd = value;
    else if(key == "STOP_CMD") stop_cmd = value;
    else if(key == "STATUS_CMD") status_cmd = value;
    else if(key == "TEST_CMD") test_cmd = value;
  };
};



void CD::Service::parseServiceFile(const std::filesystem::path& path){
  config_path = path;

  std::ifstream file(path);
  
  if (!file.is_open()) {
    CD::Logger::get().print("log", name,  "Can't open file: {}", path.string());
    return;
  };

  std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  fileContent.erase(std::remove(fileContent.begin(), fileContent.end(), '\n'), fileContent.end());
  fileContent.erase(std::remove(fileContent.begin(), fileContent.end(), '\r'), fileContent.end());
   
  loadFromFile(fileContent);
  last_time_write = std::filesystem::last_write_time(config_path);
};



// === Helpers
void CD::Service::printInfo(){
  CD::Logger::get().print("log", name, color_id,  "=== {} Info ===", name);
  CD::Logger::get().print("log", name, color_id,  "Description:\n{}", description);
  CD::Logger::get().print("log", name, color_id,  "Local Path = {}", local_path.string());
  CD::Logger::get().print("log", name, color_id,  "Env Path = {}", env_path.string());
  CD::Logger::get().print("log", name, color_id,  "Repo URL = {}", repo_url);
  CD::Logger::get().print("log", name, color_id,  "Branch = {}", branch);
  CD::Logger::get().print("log", name, color_id,  "Run cmd = {}", run_cmd);
  CD::Logger::get().print("log", name, color_id,  "Stop cmd = {}", stop_cmd);
  CD::Logger::get().print("log", name, color_id,  "Status cmd = {}", status_cmd);
  CD::Logger::get().print("log", name, color_id,  "Test cmd = {}", test_cmd);
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
    CD::Logger::get().print("log", name,  "Failed to fork process for service.");
    return -1;
  } 
  else if (pid == 0) {
    execvp(c_args[0], c_args.data());
    CD::Logger::get().print("log", name,  "Failed to execute binary: {}", c_args[0]);
    _exit(127);
  } 
  else {
    CD::Logger::get().print("log", name, color_id, "Spawned child process with PID: {}", pid);
    return pid;
  };
};



bool CD::Service::waitForProcess(pid_t pid) {
  if (pid <= 0) return false;

  int status = 0;
  pid_t result_pid = waitpid(pid, &status, 0);

  if (result_pid < 0) {
    CD::Logger::get().print("log", name,  "Failed to wait for process with PID: {}", pid);
    return false;
  };

  if (WIFEXITED(status)) {
    int exit_code = WEXITSTATUS(status);
    if (exit_code == 0) {
      CD::Logger::get().print("log", name, color_id, "Process with PID {} finished successfully.", pid);
      return true;
    } else {
      CD::Logger::get().print("log", name,  "Process with PID {} exited with error code: {}", pid, exit_code);
      return false;
    };
  } 
  else if (WIFSIGNALED(status)) {
    int signal_number = WTERMSIG(status);
    CD::Logger::get().print("log", name,  "Process with PID {} was killed by signal: {}", pid, signal_number);
    return false;
  };

  return false;
};



bool CD::Service::executeProcess(const std::vector<std::string>& args){
  return waitForProcess(spawnProcess(args));
};



void CD::Service::cloneRepo(size_t version){
  if(version == -1) return;
  if(branch == "") return;
  if(repo_url == "") return;
  trimeFolders();
  std::filesystem::path target_dir = local_path / CD::Config::SERVICE_REPOS_DIR_PATH / std::filesystem::path(std::to_string(version) + "/");
  
  if (!std::filesystem::exists(target_dir)) {
    std::error_code ec;
    if (std::filesystem::create_directories(target_dir, ec)) 
      CD::Logger::get().print("log", name, color_id, "Created missing service directory: {}", target_dir.string());
    else {
      CD::Logger::get().print("log", name, color_id, "Failed to create directory {} : {}", target_dir.string(), ec.message());
      return;
    };
  }
  else {
    CD::Logger::get().print("log", name, color_id, "Directory {} alredy exists", target_dir.string());
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



void CD::Service::removeDir(size_t version){
  if(version == -1) return;
  std::filesystem::path target_dir = local_path / CD::Config::SERVICE_REPOS_DIR_PATH / std::filesystem::path(std::to_string(version) + "/");

  if (!std::filesystem::exists(target_dir)) {
    CD::Logger::get().print("log", name, color_id, "Directory does not exist, nothing to remove: {}", target_dir.string());
    return;
  };

  std::error_code ec;
  uintmax_t removed_count = std::filesystem::remove_all(target_dir, ec);

  if (ec)
    CD::Logger::get().print("log", name,  "Failed to remove directory {} : {}", target_dir.string(), ec.message());
  else {
    CD::Logger::get().print("log", name, color_id, "Successfully removed directory: {} (deleted {} items)", target_dir.string(), removed_count);
  };
};



bool CD::Service::runTests(size_t version){
  if(version == -1) return false;
  std::filesystem::path target_dir = local_path / CD::Config::SERVICE_REPOS_DIR_PATH / std::filesystem::path(std::to_string(version) + "/");
  
  if (!std::filesystem::exists(target_dir)) {
    CD::Logger::get().print("log", name,  "Directory {} didn't exists", target_dir.string());
    return false;
  };

  std::string full_command = "cd " + target_dir.string() + " && " + test_cmd;
  CD::Logger::get().print("log", name, color_id, "Running tests in version {}: {}", version, full_command);
  return executeProcess({"/bin/sh", "-c", full_command});
};



bool CD::Service::runContainer(size_t version) {
  if(version == -1) return false;
  if(run_cmd == "") return false;
  std::filesystem::path target_dir = local_path / CD::Config::SERVICE_REPOS_DIR_PATH / std::filesystem::path(std::to_string(version) + "/");
  
  if (!std::filesystem::exists(target_dir)) {
    CD::Logger::get().print("log", name,  "Directory {} didn't exists", target_dir.string());
    return false;
  };

  std::string modified_docker_cmd = run_cmd;
  
  std::string target_prefix = "docker compose";
  size_t pos = modified_docker_cmd.find(target_prefix);
  
  if (pos != std::string::npos)
    modified_docker_cmd.insert(pos + target_prefix.length(), " --env-file " + env_path.string());

  std::string full_command = "cd " + target_dir.string() + " && " + modified_docker_cmd;
  CD::Logger::get().print("log", name, color_id, "Running docker container in version {}: {}", version, full_command); 
  return executeProcess({"/bin/sh", "-c", full_command});
};



bool CD::Service::statusContainer(size_t version){
  if(version == static_cast<size_t>(-1)) return false;
  if(status_cmd == "") return false;
  std::filesystem::path target_dir = local_path / CD::Config::SERVICE_REPOS_DIR_PATH / std::filesystem::path(std::to_string(version) + "/");
  
  if (!std::filesystem::exists(target_dir)) {
    CD::Logger::get().print("log", name,  "Directory {} didn't exist", target_dir.string());
    return false;
  };

  std::string modified_docker_cmd = status_cmd;
  
  // Replace ALL occurrences of "docker compose" with "docker compose --env-file <path>"
  std::string target_prefix = "docker compose";
  std::string replacement = "docker compose --env-file " + env_path.string();
  size_t pos = 0;
  while ((pos = modified_docker_cmd.find(target_prefix, pos)) != std::string::npos) {
    modified_docker_cmd.replace(pos, target_prefix.length(), replacement);
    pos += replacement.length();
  }

  std::string full_command = "cd " + target_dir.string() + " && " + modified_docker_cmd;
  CD::Logger::get().print("log", name, color_id, "Checking container status in version {}: {}", version, full_command); 
  return executeProcess({"/bin/sh", "-c", full_command});
};



bool CD::Service::stopContainer(size_t version){
  if(version == -1) return false;
  if(stop_cmd == "") return false;
  std::filesystem::path target_dir = local_path / CD::Config::SERVICE_REPOS_DIR_PATH / std::filesystem::path(std::to_string(version) + "/");
  
  if (!std::filesystem::exists(target_dir)) {
    CD::Logger::get().print("log", name,  "Directory {} didn't exists", target_dir.string());
    return false;
  };

  std::string modified_docker_cmd = stop_cmd;
  
  std::string target_prefix = "docker compose";
  size_t pos = modified_docker_cmd.find(target_prefix);
  
  if (pos != std::string::npos)
    modified_docker_cmd.insert(pos + target_prefix.length(), " --env-file " + env_path.string());

  std::string full_command = "cd " + target_dir.string() + " && " + modified_docker_cmd;
  CD::Logger::get().print("log", name, color_id, "Running docker container in version {}: {}", version, full_command); 
  return executeProcess({"/bin/sh", "-c", full_command});
};



std::string CD::Service::getRemoteCommit(){ 
  std::string authenticated_url = repo_url;
  if (!github_token.empty()) {
    std::string prefix = "https://";
    if (authenticated_url.rfind(prefix, 0) == 0) {
      authenticated_url.insert(prefix.length(), github_token + "@");
    };
  };

  std::string command = "git ls-remote " + authenticated_url + " " + branch;
  CD::Logger::get().print("log", name, color_id, "Checking remote commit for branch '{}'...", branch);
  std::array<char, 128> buffer;
  std::string result;
  
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
  if (!pipe) {
    CD::Logger::get().print("log", name,  "Failed to run git ls-remote command.");
    return "";
  };

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();

  if (result.empty()) {
    CD::Logger::get().print("log", name,  "Remote branch '{}' not found or failed to fetch.", branch);
    return "";
  };

  size_t space_pos = result.find_first_of(" \t");
  if (space_pos != std::string::npos) {
    std::string commit_hash = result.substr(0, space_pos);
    CD::Logger::get().print("log", name, color_id, "Latest remote commit for {}: {}", branch, commit_hash);
    return commit_hash;
  };

  return "";
};



std::string CD::Service::getLocalCommit(size_t version) {
  if(version == -1) return "";
  std::filesystem::path target_dir = local_path / CD::Config::SERVICE_REPOS_DIR_PATH / std::filesystem::path(std::to_string(version) + "/");

  if (!std::filesystem::exists(target_dir)) {
    CD::Logger::get().print("log", name,  "Directory {} doesn't exist", target_dir.string());
    return "";
  };

  if (!std::filesystem::exists(target_dir / ".git")) {
    CD::Logger::get().print("log", name,  "Directory {} is not a git repository", target_dir.string());
    return "";
  };

  std::string command = "git -C \"" + target_dir.string() + "\" rev-parse HEAD";

  std::array<char, 128> buffer;
  std::string result;

  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
  if (!pipe) {
    CD::Logger::get().print("log", name,  "Failed to run git rev-parse command.");
    return "";
  };

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();

  if (!result.empty()) result.erase(result.find_last_not_of(" \n\r\t") + 1);

  if (result.empty()) {
    CD::Logger::get().print("log", name,  "Failed to retrieve local commit for version {}", version);
    return "";
  };

  CD::Logger::get().print("log", name, color_id, "Local commit for version {}: {}", version, result);
  return result;
};



void CD::Service::findLatestVersions() {
  std::filesystem::path target_dir = local_path / CD::Config::SERVICE_REPOS_DIR_PATH;

  long latest = -1;
  if (std::filesystem::exists(target_dir) && std::filesystem::is_directory(target_dir)) {
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(target_dir)) {
      if (entry.is_directory()) {
        std::string folder_name = entry.path().filename().string();
        try {
          int version_num = std::stoi(folder_name);
          if (version_num > latest) {
            latest = version_num;
          };
        } catch (...) {};
      };
    };
  };

  latest_version = latest;

  CD::Logger::get().print("log", name, color_id, "Latest Version found: {}", latest);
};



void CD::Service::observeFile(){
  if(config_path == "") return;
  std::filesystem::file_time_type new_last_time_write = std::filesystem::last_write_time(config_path);
  if(new_last_time_write == last_time_write) return;
  CD::Logger::get().print("log", name, color_id, "Detected {} file change", config_path.string());
  parseServiceFile(config_path);
  local_latest_commit = "";
  last_time_write = new_last_time_write;
};



void CD::Service::trimeFolders() {
  if (local_path.empty()) return;
    
  std::filesystem::path target_dir = local_path / CD::Config::SERVICE_REPOS_DIR_PATH;
  if (!std::filesystem::exists(target_dir) || !std::filesystem::is_directory(target_dir)) return;

  size_t n = CD::Config::VERSION_TRIM; 
  for (const auto& entry : std::filesystem::directory_iterator(target_dir)) {
    if (entry.is_directory()) {
      std::string folder_name = entry.path().filename().string();
            
      try {
        size_t version = std::stoull(folder_name);
                
        if (version + n < latest_version) {
          CD::Logger::get().print("log", name, color_id, "Trimming old version folder: {}", entry.path().string());
          std::filesystem::remove_all(entry.path());
        };
      } catch (const std::exception& e) {};
    };
  };
};


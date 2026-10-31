// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include "core.h"



// === Constructors
CD::Core::Core()
  : server(update_request_count) {
  CD::Logger::get().print(CD::LS::CoreBold, "CD::Core::Core() Constructor Called");
};



CD::Core::~Core() noexcept {
  CD::Logger::get().print(CD::LS::CoreBold, "CD::Core::~Core() Destructor Called");
  onEnd();
};



// === Loop
void CD::Core::loop() {
  auto start_time = std::chrono::steady_clock::now();
  
  CD::Logger::get().print(CD::LS::Core, "Main loop called");
  
  onHealthCheck();
  CD::Logger::get().print(CD::LS::Core, "Core health check");
  
  size_t pending_updates = update_request_count.exchange(0);
  CD::Logger::get().print(CD::LS::Core, "Core update requests: {}", pending_updates);
  for (size_t i = 0; i < pending_updates; ++i) {
    onUpdate();
    CD::Logger::get().print(CD::LS::Core, "Core update request");
  };

  auto end_time = std::chrono::steady_clock::now();
  std::chrono::duration<float> duration = end_time - start_time;
  CD::Logger::get().print(CD::LS::Core, "Main Loop Took: {:.3f}s", duration.count());
  
  auto elapsed = end_time - start_time;
  auto target_interval = std::chrono::milliseconds(CD::Config::UPDATE_TIME_MS);
  if (elapsed < target_interval) std::this_thread::sleep_for(target_interval - elapsed);
};



void CD::Core::onStart(){
  CD::Logger::get().print(CD::LS::Core, "CD::Core::onStart called");
  
  server.onStart();
 
  loadServices();
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onStart();
};



void CD::Core::onUpdate(){
  CD::Logger::get().print(CD::LS::Core, "CD::Core::onUpdate called");
  
  loadServices();
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onUpdate();
};



void CD::Core::onHealthCheck(){
  CD::Logger::get().print(CD::LS::Core, "CD::Core::onHealthCheck called");
  
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onHealthCheck();
};



void CD::Core::onEnd(){
  CD::Logger::get().print(CD::LS::Core, "CD::Core::onEnd called");
  
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onEnd();
  services.clear();
  
  server.onEnd();
};



// === Helpers
void CD::Core::loadServices(){
  std::filesystem::path serviceDir = CD::Config::CONFIG_FILES_PATH;

  if (!std::filesystem::exists(serviceDir)) {
    std::error_code ec;
    if (std::filesystem::create_directories(serviceDir, ec)) 
      CD::Logger::get().print(CD::LS::Core, "Created missing service directory: {}", serviceDir.string());
    else {
      CD::Logger::get().print(CD::LS::CoreError, "Failed to create directory {} : {}", serviceDir.string(),  ec.message());
      return;
    };
  };

  for (const auto& entry : std::filesystem::directory_iterator(serviceDir)) {
    std::string filename = entry.path().filename().string();
    if (std::regex_search(filename, CD::Config::SERVICE_EXCLUDE_PATTERN)) continue;
    if (!std::regex_search(filename, CD::Config::SERVICE_INCLUDE_PATTERN)) continue;
    if (services.contains(entry.path())) continue;
    if (entry.is_regular_file()) {
      CD::Logger::get().print(CD::LS::Core, "Added Service: {}", entry.path().string());
      CD::Service service;
      service.parseServiceFile(entry.path());
      services.try_emplace(entry.path(), std::move(service));
    };
  };

  removeServices();
};



void CD::Core::removeServices() {
  std::filesystem::path serviceDir = CD::Config::CONFIG_FILES_PATH;

  std::unordered_map<std::filesystem::path, bool> services_to_remove;
  services_to_remove.reserve(services.size());
  for(auto kv : services) services_to_remove.try_emplace(kv.first);
  
  if (!std::filesystem::exists(serviceDir)) {
    std::error_code ec;
    if (std::filesystem::create_directories(serviceDir, ec)) 
      CD::Logger::get().print(CD::LS::Core, "Created missing service directory: {}", serviceDir.string());
    else {
      CD::Logger::get().print(CD::LS::CoreError, "Failed to create directory {} : {}", serviceDir.string(),  ec.message());
      return;
    };
  };

  for (const auto& entry : std::filesystem::directory_iterator(serviceDir)) {
    std::string filename = entry.path().filename().string();
    if (std::regex_search(filename, CD::Config::SERVICE_EXCLUDE_PATTERN)) continue;
    if (!std::regex_search(filename, CD::Config::SERVICE_INCLUDE_PATTERN)) continue;
    if (!services.contains(entry.path())) continue;
    if (entry.is_regular_file()) {
      services_to_remove.erase(entry.path()); 
    };
  };

  for(const std::pair<std::filesystem::path, bool>& el : services_to_remove) services.erase(el.first);
};


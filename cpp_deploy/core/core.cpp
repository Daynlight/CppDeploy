// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include "core.h"



// === Constructors
CD::Core::Core(){
  CD::Logger::get().print(CD::LS::CoreBold, "CD::Core::Core() Constructor Called");
};



CD::Core::~Core(){
  CD::Logger::get().print(CD::LS::CoreBold, "CD::Core::~Core() Destructor Called");
};



CD::Core::Core(const CD::Core& second){
  CD::Logger::get().print(CD::LS::CoreBold, "CD::Core::Core() Copy constructor Called");
};



CD::Core& CD::Core::operator=(const CD::Core& second){
  CD::Logger::get().print(CD::LS::CoreBold, "CD::Core::Core() Copy assign constructor Called");
  if(this == &second) return *this;

  return *this;
};



CD::Core::Core(CD::Core&& second){
  CD::Logger::get().print(CD::LS::CoreBold, "CD::Core::Core() Move constructor Called");
};



CD::Core& CD::Core::operator=(CD::Core&& second){
  CD::Logger::get().print(CD::LS::CoreBold, "CD::Core::Core() Move assign constructor Called");
  if(this == &second) return *this;

  return *this;
};



// === Loop
void CD::Core::onStart(){
  CD::Logger::get().print(CD::LS::Core, "CD::Core::onStart called");
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


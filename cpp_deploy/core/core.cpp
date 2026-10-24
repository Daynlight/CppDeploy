// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include "core.h"



// === Constructors
CD::Core::Core(){
  fmt::println(fg(fmt::color::yellow), "CD::Core::Core() Constructor Called");
};



CD::Core::~Core(){
  fmt::println(fg(fmt::color::yellow), "CD::Core::~Core() Destructor Called");
};



CD::Core::Core(const CD::Core& second){
  fmt::println(fg(fmt::color::yellow), "CD::Core::Core() Copy constructor Called");
};



CD::Core& CD::Core::operator=(const CD::Core& second){
  fmt::println(fg(fmt::color::yellow), "CD::Core::Core() Copy assign constructor Called");
  if(this == &second) return *this;

  return *this;
};



CD::Core::Core(CD::Core&& second){
  fmt::println(fg(fmt::color::yellow), "CD::Core::Core() Move constructor Called");
};



CD::Core& CD::Core::operator=(CD::Core&& second){
  fmt::println(fg(fmt::color::yellow), "CD::Core::Core() Move assign constructor Called");
  if(this == &second) return *this;

  return *this;
};



// === Loop
void CD::Core::onStart(){
  fmt::println(fg(fmt::color::blue), "CD::Core::onStart called");
  loadServices();
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onStart();
};



void CD::Core::onUpdate(){
  fmt::println(fg(fmt::color::blue), "CD::Core::onUpdate called");
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onUpdate();
};



void CD::Core::onEnd(){
  fmt::println(fg(fmt::color::blue), "CD::Core::onEnd called");
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onEnd();
  services.clear();
};


// === Helpers
void CD::Core::parseServiceFile(const std::filesystem::path& path){
  std::ifstream file(path);
  
  if (!file.is_open()) {
    fmt::println(fg(fmt::color::red), "Can't open file: {}", path.string());
    return;
  };

  std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  fileContent.erase(std::remove(fileContent.begin(), fileContent.end(), '\n'), fileContent.end());
  fileContent.erase(std::remove(fileContent.begin(), fileContent.end(), '\r'), fileContent.end());
  
  fmt::println(fg(fmt::color::pink), "File {}\n {}", path.string(), fileContent);
  
  auto [it, inserted] = services.try_emplace(path); 
  it->second.loadFromFile(fileContent);
};



void CD::Core::loadServices(){
  std::filesystem::path serviceDir = CD::Config::CONFIG_FILES_PATH;
  
  if (!std::filesystem::exists(serviceDir)) {
    std::error_code ec;
    if (std::filesystem::create_directories(serviceDir, ec)) 
      fmt::println(fg(fmt::color::yellow), "Created missing service directory: {}", serviceDir.string());
    else {
      fmt::println(fg(fmt::color::yellow), "Failed to create directory {} : {}", serviceDir.string(),  ec.message());
      return;
    };
  };

  for (const auto& entry : std::filesystem::directory_iterator(serviceDir)) {
    std::string filename = entry.path().filename().string();
    if (std::regex_search(filename, CD::Config::SERVICE_EXCLUDE_PATTERN)) continue;
    if (!std::regex_search(filename, CD::Config::SERVICE_INCLUDE_PATTERN)) continue;
    if (services.contains(entry.path())) continue;
    if (entry.is_regular_file()) parseServiceFile(entry.path());
  };
};


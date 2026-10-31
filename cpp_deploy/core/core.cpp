// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include "core.h"



// === Constructors
CD::Core::Core(std::atomic<bool>& g_running)
  : g_running(g_running),
    server(update_request_count) {
  CD::Logger::get().print("log", CD::LS::CoreBold, "CD::Core::Core() Constructor Called");
};



CD::Core::~Core() noexcept {
  CD::Logger::get().print("log", CD::LS::CoreBold, "CD::Core::~Core() Destructor Called");
  onEnd();
};



// === Loop
void CD::Core::loop() {
  auto start_time = std::chrono::steady_clock::now();
  
  CD::Logger::get().print("log", CD::LS::Core, "Main loop called");
  
  onHealthCheck();
  CD::Logger::get().print("log", CD::LS::Core, "Core health check");
  
  size_t pending_updates = update_request_count.exchange(0);
  CD::Logger::get().print("log", CD::LS::Core, "Core update requests: {}", pending_updates);
  for (size_t i = 0; i < pending_updates; ++i) {
    onUpdate();
    CD::Logger::get().print("log", CD::LS::Core, "Core update request");
  };

  auto end_time = std::chrono::steady_clock::now();
  std::chrono::duration<float> duration = end_time - start_time;
  CD::Logger::get().print("log", CD::LS::Core, "Main Loop Took: {:.3f}s", duration.count());
  
  auto elapsed = end_time - start_time;
  auto target_interval = std::chrono::milliseconds(CD::Config::UPDATE_TIME_MS);
  if (elapsed < target_interval) std::this_thread::sleep_for(target_interval - elapsed);
};



void CD::Core::onStart(){
  CD::Logger::get().print("log", CD::LS::Core, "CD::Core::onStart called");
 
  if (!render_thread.joinable()) {
    render_thread = std::thread([this](){ renderWindow(); });
    CD::Logger::get().print("log", CD::LS::Core, "render_handler started successfully");
  } else CD::Logger::get().print("log", CD::LS::Core, "render_handler is already running");

  server.onStart();
 
  loadServices();
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onStart();
};



void CD::Core::onUpdate(){
  CD::Logger::get().print("log", CD::LS::Core, "CD::Core::onUpdate called");
  
  loadServices();
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onUpdate();
};



void CD::Core::onHealthCheck(){
  CD::Logger::get().print("log", CD::LS::Core, "CD::Core::onHealthCheck called");
  
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onHealthCheck();
};



void CD::Core::onEnd(){
  CD::Logger::get().print("log", CD::LS::Core, "CD::Core::onEnd called");
  
  for(std::pair<const std::filesystem::path, CD::Service>& service : services) service.second.onEnd();
  services.clear();
  
  server.onEnd();

  if (render_thread.joinable()) render_thread.join();
  if(!render_thread.joinable()) CD::Logger::get().print("log", CD::LS::Core, "render_thread stopped");
  else CD::Logger::get().print("log", CD::LS::CoreError, "render_thread is still working"); 
};



// === Helpers
void CD::Core::loadServices(){
  std::filesystem::path serviceDir = CD::Config::CONFIG_FILES_PATH;

  if (!std::filesystem::exists(serviceDir)) {
    std::error_code ec;
    if (std::filesystem::create_directories(serviceDir, ec)) 
      CD::Logger::get().print("log", CD::LS::Core, "Created missing service directory: {}", serviceDir.string());
    else {
      CD::Logger::get().print("log", CD::LS::CoreError, "Failed to create directory {} : {}", serviceDir.string(),  ec.message());
      return;
    };
  };

  for (const auto& entry : std::filesystem::directory_iterator(serviceDir)) {
    std::string filename = entry.path().filename().string();
    if (std::regex_search(filename, CD::Config::SERVICE_EXCLUDE_PATTERN)) continue;
    if (!std::regex_search(filename, CD::Config::SERVICE_INCLUDE_PATTERN)) continue;
    if (services.contains(entry.path())) continue;
    if (entry.is_regular_file()) {
      CD::Logger::get().print("log", CD::LS::Core, "Added Service: {}", entry.path().string());
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
      CD::Logger::get().print("log", CD::LS::Core, "Created missing service directory: {}", serviceDir.string());
    else {
      CD::Logger::get().print("log", CD::LS::CoreError, "Failed to create directory {} : {}", serviceDir.string(),  ec.message());
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


// === Windows
void CD::Core::renderWindow() {
  std::string input_buffer;
  bool initialized = false;
  
#ifndef CPP_DEPLOY_INSTALL 
  tcgetattr(STDIN_FILENO, &old_termios);
  termios new_termios = old_termios;
  new_termios.c_lflag &= ~(ICANON | ECHO);
  new_termios.c_cc[VMIN] = 1;
  new_termios.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
#endif
  
  while (g_running) {
#ifndef CPP_DEPLOY_INSTALL 
    if(initialized) fmt::print("\r\033[2K");
    else initialized = true;
#endif

    auto log_buffer = CD::Logger::get().getWindowBuffer("log");
    for (size_t i = 0; i < log_buffer.size(); ++i) fmt::println(log_buffer[i].style.style, "{}", log_buffer[i].text);
    
#ifndef CPP_DEPLOY_INSTALL 
    fmt::print("Cpp Deploy CLI> {}", input_buffer);
    std::fflush(stdout);
    
    pollfd pfd{};
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;

    if (poll(&pfd, 1, 100) > 0 && (pfd.revents & POLLIN)) {
      char ch;
      ssize_t n = read(STDIN_FILENO, &ch, 1);
      if (n <= 0) continue;

      if (ch == '\n' || ch == '\r') {
        fmt::println("");
        commands(input_buffer);
        input_buffer.clear();
      }
      else if (!input_buffer.empty() && (ch == 127 || ch == '\b')) input_buffer.pop_back();
      else input_buffer += ch;
    };

    std::fflush(stdout);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(CD::Config::UPDATE_TIME_MS));
#endif
  };

#ifndef CPP_DEPLOY_INSTALL 
  tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
#endif
};



void CD::Core::commands(const std::string& input_buffer){
  if (input_buffer == "exit" || input_buffer == "quit") {
    CD::Logger::get().print("log", CD::LS::User, "user exit");
    g_running = false;
  }
  else if (input_buffer == "help") {
    CD::Logger::get().print("log", CD::LS::User, "=== Help ===");
    CD::Logger::get().print("log", CD::LS::User, "exit/quit - exit program");
    CD::Logger::get().print("log", CD::LS::User, "list - list services");
  }
  else if(input_buffer == "list") {
    CD::Logger::get().print("log", CD::LS::User, "=== Services ===");
    for(std::pair<const std::filesystem::path, CD::Service>& el : services) el.second.printInfo();
  }
  else CD::Logger::get().print("log", CD::LS::User, "no such as methode like: {}", input_buffer);
};


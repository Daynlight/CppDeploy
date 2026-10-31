// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include "logger.h"



// === Log
template <typename... T>
void CD::Logger::print(const std::string& log_window_name, LSD type, std::string_view fmt_str, T&&... args) {
  std::lock_guard<std::mutex> lock(log_mutex);
  auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    
  std::string header = fmt::format("[{:%Y-%m-%d %H:%M:%S}] ({}) ", now, type.tag);
  std::string content = fmt::format(fmt::runtime(fmt_str), std::forward<T>(args)...); 
  log_windows_buffers[log_window_name].push_back({header + content, type});
};



template <typename... T>
void CD::Logger::print(const std::string& log_window_name, const std::string& service_name, int color_id, std::string_view fmt_str, T&&... args){
  std::lock_guard<std::mutex> lock(log_mutex);
  auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
  
  if (color_id >= CD::LSC::Services.size()) color_id = 0;
  
  std::string header;
  if (!service_name.empty()) header = fmt::format("[{:%Y-%m-%d %H:%M:%S}] ({} {}) ", now, CD::LST::Service, service_name);
  else header = fmt::format("[{:%Y-%m-%d %H:%M:%S}] ({}) ", now, CD::LST::Service);
  
  std::string content = fmt::format(fmt::runtime(fmt_str), std::forward<T>(args)...);
  
  LSD service_lsd{ 
    service_name.empty() ? CD::LST::Service : service_name.c_str(), 
    CD::LSC::Services[color_id] 
  };
  
  log_windows_buffers[log_window_name].push_back({header + content, service_lsd});
};



template <typename... T>
void CD::Logger::print(const std::string& log_window_name, const std::string& service_name, std::string_view fmt_str, T&&... args){
  print(log_window_name, service_name, 0, fmt_str, std::forward<T>(args)...);
};


// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include "logger.h"



// === Log
template <typename... T>
void CD::Logger::print(LSD type, std::string_view fmt_str, T&&... args) {
  std::lock_guard<std::mutex> lock(log_mutex);
  auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
  fmt::print(type.style, "[{:%Y-%m-%d %H:%M:%S}] ({}) ", now, type.tag);
  fmt::println(type.style, fmt::runtime(fmt_str), std::forward<T>(args)...);
};



template <typename... T>
void CD::Logger::print(const std::string& service_name, int color_id, std::string_view fmt_str, T&&... args){
  std::lock_guard<std::mutex> lock(log_mutex);
  auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
  if(color_id >= CD::LSC::Services.size()) color_id = 0;
  if(service_name != "") fmt::print(CD::LSC::Services[color_id], "[{:%Y-%m-%d %H:%M:%S}] ({} {}) ", now, CD::LST::Service, service_name);
  else fmt::print(CD::LSC::Services[color_id], "[{:%Y-%m-%d %H:%M:%S}] ({}) ", now, CD::LST::Service);
  fmt::println(CD::LSC::Services[color_id], fmt::runtime(fmt_str), std::forward<T>(args)...);
};



template <typename... T>
void CD::Logger::print(const std::string& service_name, std::string_view fmt_str, T&&... args){
  std::lock_guard<std::mutex> lock(log_mutex);
  auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
  if(service_name != "") fmt::print(CD::LSC::Error, "[{:%Y-%m-%d %H:%M:%S}] ({} {}) ", now, CD::LST::Service, service_name);
  else fmt::print(CD::LSC::Error, "[{:%Y-%m-%d %H:%M:%S}] ({}) ", now, CD::LST::Service);
  fmt::println(CD::LSC::Error, fmt::runtime(fmt_str), std::forward<T>(args)...);
};


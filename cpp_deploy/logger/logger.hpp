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
  fmt::println(type.style,  fmt::runtime(fmt_str), std::forward<T>(args)...);
};


// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include "logger.h"



// === Constructors
CD::Logger::Logger() {
};



// === Singleton
CD::Logger& CD::Logger::get(){
  static Logger instance;
  return instance;
};



// === Log
size_t CD::Logger::getServiceColor(){
  size_t id = std::distance(std::begin(service_color_usage), std::min_element(std::begin(service_color_usage), std::end(service_color_usage)));
  service_color_usage[id] += 1;
  return id;
};


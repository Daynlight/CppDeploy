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


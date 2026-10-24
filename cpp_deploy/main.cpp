// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include <iostream>
#include <unistd.h>
#include <csignal>

#include "core/core.h"
#include "config.h"



bool g_running = true;

void handle_signal(int signal) {
  if (signal == SIGTERM || signal == SIGINT) {
    g_running = false;
  };
};



int main() {
  setbuf(stdout, NULL);
  std::signal(SIGTERM, handle_signal);
  std::signal(SIGINT, handle_signal);

  CD::Core core;
  core.onStart();

  while (g_running) {
    core.onUpdate();
    sleep(CD::Config::UPDATE_TIME);
  };

  core.onEnd();

  return 0;
};


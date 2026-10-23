// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#include <iostream>
#include <unistd.h>
#include <csignal>



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

  while (g_running) {
    
    sleep(5);
  };

  return 0;
};


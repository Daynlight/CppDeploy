// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#pragma once
#include <filesystem>
#include <regex>
#include <boost/asio.hpp>



namespace CD::Config{
#ifdef CPP_DEPLOY_INSTALL 
  inline const std::filesystem::path ROOT_PROJECT_PATH = std::filesystem::path(SYSCONF_DIR);
  inline const std::filesystem::path CONFIG_FILES_PATH = ROOT_PROJECT_PATH / "services/"; 
  inline const size_t UPDATE_TIME_MS = 1000 * 5;
  inline const size_t VERSION_TRIM = 10;
#else
  inline const std::filesystem::path ROOT_PROJECT_PATH = std::filesystem::path(__FILE__).parent_path();
  inline const std::filesystem::path CONFIG_FILES_PATH = ROOT_PROJECT_PATH / ".." / "config/"; 
  inline const size_t UPDATE_TIME_MS = 100;
  inline const size_t VERSION_TRIM = 100;
#endif
  inline const boost::asio::ip::address ADDRESS = boost::asio::ip::make_address("127.0.0.1");
  inline const size_t PORT = 9923;
  inline const std::regex SERVICE_EXCLUDE_PATTERN(R"(^(\..*|\.(bak|tmp|swo|swp))$)");
  inline const std::regex SERVICE_INCLUDE_PATTERN(R"(^.*\.(txt|json|conf)$)");
  inline const std::filesystem::path SERVICE_REPOS_DIR_PATH = std::filesystem::path("repos/");
};


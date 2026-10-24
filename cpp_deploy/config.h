// CppDeploy
// Copyright 2026 Daynlight
// Licensed under the GNU General.
// See LICENSE file for details.



#pragma once
#include <filesystem>
#include <regex>



namespace CD::Config{
  inline const std::filesystem::path ROOT_PROJECT_PATH = std::filesystem::path(__FILE__).parent_path();
  inline const std::filesystem::path CONFIG_FILES_PATH = ROOT_PROJECT_PATH / ".." / "config";
  inline const std::regex SERVICE_EXCLUDE_PATTERN(R"(^(\..*|\.(bak|tmp|swo|swp))$)");
  inline const std::regex SERVICE_INCLUDE_PATTERN(R"(^.*\.(txt|json|conf)$)");
  inline const size_t UPDATE_TIME = 5;
};


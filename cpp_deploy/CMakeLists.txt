# CppDeploy
# Copyright 2026 Daynlight
# Licensed under the GNU General.
# See LICENSE file for details.



# ==== Dependencies ==== #
find_package(fmt CONFIG REQUIRED)
find_package(boost_asio CONFIG REQUIRED)



# ==== Project ==== #
project(cpp_deploy LANGUAGES CXX)

set(src
  main.cpp
  core/core.cpp
  service/service.cpp
  server/server.cpp
  logger/logger.cpp
)

add_executable(cpp_deploy ${src})
target_link_libraries(cpp_deploy PRIVATE 
  fmt::fmt
  Boost::asio
)


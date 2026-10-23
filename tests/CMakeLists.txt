# CppDeploy
# Copyright 2026 Daynlight
# Licensed under the GNU General.
# See LICENSE file for details.



# ==== Dependencies ==== #
find_package(Threads REQUIRED)
find_package(GTest CONFIG REQUIRED)

# ==== Project ==== #
set(src
  unit/temp.cpp
)

add_executable(unit_tests ${src})

target_link_libraries(unit_tests PRIVATE
  GTest::gtest
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)


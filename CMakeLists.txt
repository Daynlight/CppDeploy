# CppDeploy
# Copyright 2026 Daynlight
# Licensed under the GNU General.
# See LICENSE file for details.



cmake_minimum_required(VERSION 3.15)


# ==== Settings ==== #
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(CPP_DEPLOY_BUILD_EXAMPLES "Build Examples" ON)
option(CPP_DEPLOY_BUILD_TESTS "Build Tests" ON)
option(CPP_DEPLOY_INSTALL "Build Install" OFF)

# ==== Vcpkg ==== #
if(WIN32)
  set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE PATH "")
  set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vendor" CACHE PATH "")
  set(ZLIB_USE_STATIC_LIBS ON CACHE BOOL "Use static zlib") 
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vendor/x64-mingw-static")
endif()

# ==== Subfolders ==== #
add_subdirectory(cpp_deploy)

if(CPP_DEPLOY_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ==== Install ==== #
if(CPP_DEPLOY_INSTALL)
include(GNUInstallDirs)

install(TARGETS cpp_deploy
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=systemdsystemunitdir systemd
    OUTPUT_VARIABLE SYSTEMD_UNIT_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
endif()

if(NOT SYSTEMD_UNIT_DIR)
  set(SYSTEMD_UNIT_DIR "/etc/systemd/system")
endif()

if(UNIX AND NOT APPLE)
  install(FILES "${CMAKE_SOURCE_DIR}/service/cpp_deploy.service"
    DESTINATION ${SYSTEMD_UNIT_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )
endif()
  
target_compile_definitions(cpp_deploy PRIVATE 
  CPP_DEPLOY_INSTALL
  SYSCONF_DIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}/cpp_deploy"
)
endif()


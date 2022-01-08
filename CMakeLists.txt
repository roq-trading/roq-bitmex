cmake_minimum_required(VERSION 3.12)

# cmake

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
                      "${CMAKE_CURRENT_SOURCE_DIR}/autogen/cmake" ${CMAKE_MODULE_PATH})

# version (using git tag)

include(GetGitRepoVersion)

message("Using GIT_REPO_VERSION=${GIT_REPO_VERSION}")

# project

project(roq-bitmex VERSION ${GIT_REPO_VERSION})

# conda

if(DEFINED ENV{CONDA_PREFIX} AND NOT DEFINED ENV{CONDA_BUILD})
  message("Setting CMAKE_INSTALL_PREFIX and CMAKE_FIND_ROOT_PATH")
  set(CMAKE_INSTALL_PREFIX $ENV{CONDA_PREFIX})
  set(CMAKE_FIND_ROOT_PATH $ENV{CONDA_PREFIX})
endif()

# rpath

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# prefer static libraries

set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})

# c++

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message("Setting default visibility to hidden")
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
endif(CMAKE_BUILD_TYPE STREQUAL "Release")

# filesystem

include(GNUInstallDirs)

# dependencies

find_package(Libevent REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(absl REQUIRED)
find_package(fmt REQUIRED)
find_package(roq-api REQUIRED)
find_package(roq-core REQUIRED)
find_package(roq-logging REQUIRED)
find_package(roq-server REQUIRED)
find_package(uuid REQUIRED)

if(UNIX AND NOT APPLE)
  find_package(rt REQUIRED)
endif()

find_program(CLANG_FORMAT clang-format REQUIRED)

# include

include_directories(
  "${CMAKE_SOURCE_DIR}/src"
  # >>> autogen >>>
  "${CMAKE_BINARY_DIR}/src")

# sub-projects

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/roq/bitmex")

# test

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()

if(BUILD_TESTING)
  include(GoogleTest)
  add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/googletest" EXCLUDE_FROM_ALL)
  add_subdirectory("${CMAKE_SOURCE_DIR}/test")
endif(BUILD_TESTING)

# benchmark

option(BUILD_BENCHMARK "Enable benchmark" ON)

if(BUILD_BENCHMARK)
  find_package(benchmark REQUIRED)
  add_subdirectory("${CMAKE_SOURCE_DIR}/benchmark")
endif(BUILD_BENCHMARK)

# install

install(FILES "share/config.toml" DESTINATION "${CMAKE_INSTALL_DATADIR}/roq-bitmex")

install(FILES "${CMAKE_SOURCE_DIR}/CHANGELOG.md"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/${PROJECT_NAME}")

# debian

option(BUILD_DEB "Enable debian packaging?" OFF)

if(BUILD_DEB)
  set(CPACK_GENERATOR "DEB")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "thraneh@gmail.com")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "roq-api, roq-logging, roq-server")
  include(CPack)
endif(BUILD_DEB)

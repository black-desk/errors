cmake_minimum_required(VERSION 3.11.4) # for RHEL 8

# Let's place extra cmake scripts in /cmake directory
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Use this command to update PFL.cmake from GitHub release page: wget -O
# cmake/PFL.cmake
# https://github.com/black-desk/PFL.cmake/releases/latest/download/PFL.cmake
include(PFL)

set(ENABLE_CPM YES)
if(CMAKE_VERSION VERSION_LESS "3.14")
  set(ENABLE_CPM NO)
endif()

if(CPM_LOCAL_PACKAGES_ONLY)
  set(ENABLE_CPM NO)
endif()

project(
  errors
  VERSION 0.2.0
  DESCRIPTION ""
  HOMEPAGE_URL "https://github.com/black-desk/errors"
  LANGUAGES CXX)

# Use this command to update GitSemver.cmake from GitHub release page: wget -O
# cmake/GitSemver.cmake
# https://github.com/black-desk/GitSemver.cmake/releases/latest/download/GitSemver.cmake
include(GitSemver)
gitsemver(PROJECT_VERSION)

option(errors_WITH_NLOHMANN_JSON "Build examples and tests with nlohmann_json"
       ${PROJECT_IS_TOP_LEVEL})
option(errors_WITH_TL_EXPECTED
       "Build examples and tests with TartanLlama/expected"
       ${PROJECT_IS_TOP_LEVEL})

pfl_init()

macro(cpm_add_dependencies)
  if(errors_WITH_NLOHMANN_JSON)
    CPMFindPackage(
      NAME nlohmann_json
      VERSION 3.5.0
      URL "https://github.com/nlohmann/json/archive/refs/tags/v3.5.0.tar.gz"
      EXCLUDE_FROM_ALL ON
      OPTIONS "JSON_BuildTests OFF")
  endif()

  if(errors_WITH_TL_EXPECTED)
    CPMFindPackage(
      NAME tl-expected
      VERSION 1.0.0
      GITHUB_REPOSITORY TartanLlama/expected
      GIT_TAG v1.1.0
      EXCLUDE_FROM_ALL ON
      OPTIONS "EXPECTED_BUILD_TESTS OFF")
    if(NOT TARGET tl::expected)
      add_library(tl::expected ALIAS tl-expected)
    endif()
  endif()

  if(errors_ENABLE_TESTING)
    CPMAddPackage(
      NAME Catch2
      GITHUB_REPOSITORY catchorg/Catch2
      GIT_TAG v3.4.0
      VERSION 3.4.0)
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    include(Catch)
  endif()
endmacro()

# Use this command to update CPM.cmake from GitHub release page: wget -O
# cmake/CPM.cmake
# https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/CPM.cmake
if(NOT ENABLE_CPM)
  message(STATUS "CPM.cmake disabled.")
else()
  include(CPM)
  cpm_add_dependencies()
endif()

pfl_add_library(
  LIBRARY_TYPE
  HEADER_ONLY
  SOURCES
  # find -regex '\./\(src\|include\)/.+\.[ch]\(pp\)?\(\.in\)?' -type f | sort
  ./include/errors/config.hpp.in
  ./include/errors/error.hpp
  ./include/errors/source_location.hpp
  ./include/errors/version.hpp
  ./src/errors/config.cpp
  ./src/errors/error.cpp
  ./src/errors/version.cpp
  EXAMPLES
  advanced-usage
  basic-usage
  customize-format-global
  customize-format-local
  use-with-expected
  TESTS
  errors-unit-tests
  COMPILE_FEATURES
  INTERFACE
  cxx_std_11)

file(READ ${CMAKE_CURRENT_BINARY_DIR}/include/errors/config.hpp
     ERRORS_CONFIG_HEADER_FILE)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/include/errors/version.hpp
     ERRORS_VERSION_HEADER_FILE)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/include/errors/source_location.hpp
     ERRORS_SOURCE_LOCATION_HEADER_FILE)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/include/errors/error.hpp
     ERRORS_ERROR_HEADER_FILE)
configure_file(./misc/errors.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/errors.hpp)

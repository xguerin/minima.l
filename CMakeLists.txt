#
# Project definition
#

project("minima.l" C)
cmake_minimum_required(VERSION 3.9)

include(CheckIPOSupported)

include(cmake/GetGitRevisionDescription.cmake)
include(cmake/Lemon.cmake)
include(cmake/Ragel.cmake)

#
# Uninstall target
#

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake)

#
# Get the current GIT version
#

git_describe(VERSION)

string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION}")

set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
message(STATUS "Build version: " ${VERSION_SHORT})

#
# Options
#

option(MNML_ENABLE_DEBUG  "Enable YAML configuration plugin"  ON)
option(MNML_ENABLE_TESTS  "Build the Google tests suite"      ON)

#
# Global definitions
#

check_ipo_supported(RESULT MNML_HAS_IPO LANGUAGES C)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
  set(MNML_HAS_IPO NO)
endif()

add_definitions(-DMNML_VERSION="${VERSION_SHORT}")
add_definitions(-D_GNU_SOURCE)
add_definitions(-DYYNOERRORRECOVERY)

#
# Process options
#

if(MNML_ENABLE_DEBUG)
  add_definitions(-DLISP_ENABLE_DEBUG)
endif()

#
# Flags preferences
#

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -Wno-unused-parameter -Wno-address-of-packed-member")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.1")

set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3")

#
# Packages
#

find_package(Lemon REQUIRED)
find_package(Ragel REQUIRED)

#
# Subdirectories
#

add_subdirectory(functions)
add_subdirectory(lib)
add_subdirectory(lisp)
add_subdirectory(tools)

if(MNML_ENABLE_DEBUG)
  enable_testing()
  add_subdirectory(tests)
endif()

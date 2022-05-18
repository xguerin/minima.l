cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)

#
# Includes
#

include(CheckIPOSupported)
include(cmake/Lemon.cmake)
include(cmake/Ragel.cmake)

#
# Compiler configuration
#

set(CMAKE_C_COMPILER_NAMES clang gcc)

#
# Project definition
#

project("minima.l" VERSION 2.3.0 LANGUAGES C)

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

message(STATUS "Build version: " ${CMAKE_PROJECT_VERSION})

#
# Get the current build timestamp
#

string(TIMESTAMP BUILD_TIMESTAMP "%Y/%m/%d-%H:%M:%S")
message(STATUS "Build timestamp: " ${BUILD_TIMESTAMP})

#
# Options
#

option(MNML_ENABLE_DEBUG       "Enable debug output"         ON)
option(MNML_ENABLE_ASAN        "Enable address sanitizer"    OFF)
option(MNML_ENABLE_MSAN        "Enable memory sanitizer"     OFF)
option(MNML_ENABLE_TESTS       "Build and run tests"         ON)
option(MNML_ENABLE_LISPY_TESTS "Enable LISPy tests"          ON)

option(MNML_WITH_MODULES "Build modules"            ON)
option(MNML_WITH_TRANSFN "Transpile LISP functions" ON)

message(STATUS "Build modules: " ${MNML_WITH_MODULES})
message(STATUS "Transpile functions: " ${MNML_WITH_TRANSFN})

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

add_definitions(-DMNML_COMPILER_ID="${CMAKE_C_COMPILER_ID}")
add_definitions(-DMNML_COMPILER_VERSION="${CMAKE_C_COMPILER_VERSION}")
add_definitions(-DMNML_BUILD_TIMESTAMP="${BUILD_TIMESTAMP}")
add_definitions(-DMNML_VERSION="${CMAKE_PROJECT_VERSION}")

add_definitions(-D_GNU_SOURCE)
add_definitions(-DYYNOERRORRECOVERY)

#
# Process options
#

if(MNML_ENABLE_DEBUG)
  add_definitions(-DLISP_ENABLE_DEBUG)
endif()

#
# RPath and PIC configuration
#

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
# C flags preferences
#

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -fno-plt")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3")

if(MNML_ENABLE_ASAN)
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-omit-frame-pointer -fsanitize=address")
  set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=address")
  add_link_options(-fno-omit-frame-pointer -fsanitize=address)
endif()

if(MNML_ENABLE_MSAN)
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=memory")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-omit-frame-pointer -fsanitize=memory")
  set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=memory")
  add_link_options(-fno-omit-frame-pointer -fsanitize=memory)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  add_link_options(-fuse-ld=lld)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-register")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-address-of-packed-member")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-stringop-truncation")
  if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-fallthrough")
  endif()
  if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.0)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-address-of-packed-member")
  endif()
endif()

#
# Check for SSE availability.
#

find_package(SSE)

if(SSE_FOUND)
  message(STATUS "SSE optimizations: ON")
  add_definitions(-DLISP_ENABLE_SSE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse${SSE_VERSION}")
else()
  message(STATUS "SSE optimizations: OFF")
endif()

#
# Required packages
#

find_package(Lemon REQUIRED)
find_package(Ragel REQUIRED)

#
# Subdirectories
#

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(bin)
add_subdirectory(lib)
add_subdirectory(share)

if(MNML_WITH_TRANSFN)
  set(MNML_WITH_MODULES ON)
endif()

if(MNML_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

#
# Formatting
#

find_package(ClangFormat)
find_package(ClangTidy)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} include/*.h lib/*.c)

foreach (SOURCE ${SOURCES})
  string(REGEX REPLACE "[/.]" "_" TARGET ${SOURCE})
  string(TOLOWER ${TARGET} TARGET)
  #
  if (ClangFormat_FOUND)
    add_custom_target(${TARGET}_format COMMAND clang-format -i -style=file ${CMAKE_SOURCE_DIR}/${SOURCE})
    list(APPEND FORMAT_TARGETS ${TARGET}_format)
  endif ()
  #
  if (ClangTidy_FOUND)
    add_custom_target(${TARGET}_tidy COMMAND clang-tidy -fix -quiet -format-style=file -p ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/${SOURCE})
    list(APPEND TIDY_TARGETS ${TARGET}_tidy)
  endif ()
endforeach ()

if (ClangFormat_FOUND)
  add_custom_target(format DEPENDS ${FORMAT_TARGETS})
endif ()

if (ClangTidy_FOUND)
  add_custom_target(tidy DEPENDS ${TIDY_TARGETS})
endif ()

#
# Headers
#

set(HEADERS
  include/mnml/closure.h
  include/mnml/compiler.h
  include/mnml/debug.h
  include/mnml/lisp.h
  include/mnml/module.h
  include/mnml/slab.h
  include/mnml/types.h
  include/mnml/utils.h)

foreach(HEADER IN LISTS HEADERS)
  set(TARGET "${CMAKE_BINARY_DIR}/${HEADER}")
  list(APPEND TARGET_HEADERS "${TARGET}")
  add_custom_command(
    OUTPUT "${TARGET}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/${HEADER}" "${TARGET}"
    MAIN_DEPENDENCY "${CMAKE_SOURCE_DIR}/${HEADER}"
    VERBATIM)
endforeach()

add_custom_target(minimal_headers
  ALL
  DEPENDS ${HEADERS}
  SOURCES ${TARGET_HEADERS})

#
# Install headers
#

install(FILES ${HEADERS} DESTINATION include/mnml)

cmake_minimum_required(VERSION 3.15)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(s2_covering_tests LANGUAGES CXX)
  enable_testing()
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
get_filename_component(s2_repo_root "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

if(NOT TARGET gtest_main)
  if(NOT EXISTS "${s2_repo_root}/deps/googletest/CMakeLists.txt")
    message(FATAL_ERROR "Missing deps/googletest. Run 'make deps-s2' in the repository root.")
  endif()
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  set(BUILD_GMOCK OFF)
  set(INSTALL_GTEST OFF)
  add_subdirectory("${s2_repo_root}/deps/googletest" "${CMAKE_CURRENT_BINARY_DIR}/deps/googletest" EXCLUDE_FROM_ALL)
endif()
include("${s2_repo_root}/cmake/s2.cmake")

add_executable(s2_covering_test
  s2_covering_test.cpp
  s2_cell_id_encoding_test.cpp
)
target_compile_options(s2_covering_test PRIVATE
  -Wall -Wextra -Wpedantic
  "$<$<CONFIG:Debug>:-O0;-g3;-fno-omit-frame-pointer>"
)
target_link_libraries(s2_covering_test PRIVATE s2 gtest_main)

include(GoogleTest)
gtest_discover_tests(s2_covering_test TEST_PREFIX "gis.")

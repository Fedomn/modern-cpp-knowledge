cmake_minimum_required(VERSION 3.15)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(gis_tests LANGUAGES CXX)
  enable_testing()
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
get_filename_component(gis_repo_root "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

# GoogleTest first, then S2 and Abseil (cmake/s2.cmake).
if(NOT TARGET gtest_main)
  if(NOT EXISTS "${gis_repo_root}/deps/googletest/CMakeLists.txt")
    message(FATAL_ERROR "Missing deps/googletest. Run 'make deps' in the repository root.")
  endif()
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  set(BUILD_GMOCK OFF)
  set(INSTALL_GTEST OFF)
  add_subdirectory("${gis_repo_root}/deps/googletest" "${CMAKE_CURRENT_BINARY_DIR}/deps/googletest" EXCLUDE_FROM_ALL)
endif()
include("${gis_repo_root}/cmake/s2.cmake")

# mysql_point_mbr_test.cpp reproduces MySQL's sql/gis/mbr_utils.cc on top of
# Boost.Geometry, so the Boost source tree is needed too. Only headers are used,
# no Boost library is compiled or linked.
if(NOT EXISTS "${gis_repo_root}/deps/boost/libs/geometry/include/boost/geometry.hpp")
  message(FATAL_ERROR "Missing deps/boost. Run 'make deps' in the repository root.")
endif()
# Boost keeps most headers in libs/<lib>/include, plus nested ones such as
# libs/numeric/conversion/include.
file(GLOB gis_boost_include_dirs LIST_DIRECTORIES true
  "${gis_repo_root}/deps/boost/libs/*/include"
  "${gis_repo_root}/deps/boost/libs/*/*/include"
)

# One executable for all the tests in this folder. Every .cpp file here is a test
# source, so pick them up by globbing; CONFIGURE_DEPENDS makes the build re-glob
# and re-run CMake when a test file is added or removed.
file(GLOB gis_test_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(gis_test ${gis_test_sources})
# Boost headers are not warning clean, so include them as SYSTEM and keep
# -Wall/-Wextra for our own sources.
target_include_directories(gis_test SYSTEM PRIVATE ${gis_boost_include_dirs})
target_compile_options(gis_test PRIVATE
  -Wall -Wextra -Wpedantic
  "$<$<CONFIG:Debug>:-O0;-g3;-fno-omit-frame-pointer>"
)
target_link_libraries(gis_test PRIVATE s2 gtest_main)

include(GoogleTest)
gtest_discover_tests(gis_test TEST_PREFIX "gis.")

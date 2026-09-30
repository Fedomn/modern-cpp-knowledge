include_guard(GLOBAL)

get_filename_component(s2_deps_dir "${CMAKE_CURRENT_LIST_DIR}/../deps" ABSOLUTE)
foreach(dependency IN ITEMS abseil-cpp s2geometry)
  if(NOT EXISTS "${s2_deps_dir}/${dependency}/CMakeLists.txt")
    message(FATAL_ERROR "Missing deps/${dependency}. Run 'make deps-s2' in the repository root.")
  endif()
endforeach()

# Keep dependency options local instead of changing the parent project's cache.
function(mcpp_add_s2)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(BUILD_SHARED_LIBS OFF)
  set(BUILD_TESTING OFF)
  set(ABSL_ENABLE_INSTALL OFF)
  # Abseil probes for C++ >= 17 at configure time with try_compile(), which only
  # sees CMAKE_CXX_STANDARD on some caller paths, so pass the flag explicitly.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  add_subdirectory("${s2_deps_dir}/abseil-cpp" "${CMAKE_CURRENT_BINARY_DIR}/deps/abseil-cpp" EXCLUDE_FROM_ALL)

  # S2 0.14.0 reuses the absl targets added above instead of calling
  # find_package(absl). BUILD_TESTS would FetchContent benchmark and googletest
  # for S2's own suite; we build only our tests.
  set(BUILD_TESTS OFF)
  set(BUILD_EXAMPLES OFF)
  set(WITH_PYTHON OFF)
  add_subdirectory("${s2_deps_dir}/s2geometry" "${CMAKE_CURRENT_BINARY_DIR}/deps/s2geometry" EXCLUDE_FROM_ALL)

  # Match the debug flags our tests use, so gdb can step into S2 sources.
  target_compile_options(s2 PRIVATE "$<$<CONFIG:Debug>:-O0;-g3;-fno-omit-frame-pointer>")
endfunction()

mcpp_add_s2()

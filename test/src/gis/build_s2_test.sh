#!/usr/bin/env bash
set -euo pipefail

# Usage: ./build_s2_test.sh [GoogleTest options...]
# Run 'make deps' first. CMake uses the system compiler (or CC / CXX).
# Override S2_TEST_BUILD_DIR / S2_TEST_JOBS through environment variables.
test_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${test_dir}/../../.." && pwd)"
build_dir="${S2_TEST_BUILD_DIR:-${repo_root}/build}"

# Update the same compilation database that the workspace's clangd reads.
# S2 tests build together with the other unit tests, so an existing build
# directory keeps its compiler and switches.
cmake -S "${repo_root}" -B "${build_dir}" -DCMAKE_BUILD_TYPE=Debug
cmake --build "${build_dir}" --target s2_covering_test --parallel "${S2_TEST_JOBS:-4}"

exec "${build_dir}/test/src/gis/s2_covering_test" "$@"

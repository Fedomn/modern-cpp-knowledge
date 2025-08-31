init:
	git clone https://github.com/cmu-db/15445-bootcamp.git resources/15445-bootcamp
	git clone https://github.com/AnthonyCalandra/modern-cpp-features.git resources/modern-cpp-featureses
	git clone https://github.com/changkun/modern-cpp-tutorial.git resources/modern-cpp-tutorial
	git clone https://github.com/wuye9036/CppTemplateTutorial.git resources/CppTemplateTutorial
	git clone https://github.com/abseil/abseil-cpp.git resources/abseil-cpp
	git clone https://github.com/filipdutescu/modern-cpp-template.git resources/modern-cpp-template
	git clone https://github.com/PacktPublishing/Modern-CMake-for-Cpp.git resources/Modern-CMake-for-Cpp
	git clone https://github.com/adah1972/geek_time_cpp.git resources/geek_time_cpp
	git clone https://github.com/sftrabbit/CppPatterns-Patterns.git resources/CppPatterns-Patterns
	git clone https://github.com/downdemo/Cpp-Concurrency-in-Action-2ed.git resources/Cpp-Concurrency-in-Action-2ed
	git clone https://github.com/facebook/folly.git resources/folly
	git clone https://github.com/facebook/rocksdb.git resources/rocksdb

deps:
	git clone https://github.com/google/googletest.git --branch release-1.11.0 deps/googletest
	git clone --recurse-submodules https://github.com/boostorg/boost.git --branch boost-1.85.0 deps/boost
	git clone https://github.com/apache/brpc.git --branch release-1.9 deps/brpc
	git clone https://github.com/google/benchmark.git --branch v1.9.0 benchmark/deps/benchmark
	git clone https://github.com/google/googletest.git --branch release-1.11.0 benchmark/deps/benchmark/googletest

build-gtest:
	git clone https://github.com/google/googletest.git --branch release-1.11.0 deps/googletest
	cd deps/googletest && \
    cmake -Bbuild -Dgtest_disable_pthreads=1 && \
    cmake --build build --config Release && \
    cmake --build build --target install --config Release
	brew install clang-format


.PHONY: install coverage test docs help deps
.DEFAULT_GOAL := help

define PRINT_HELP_PYSCRIPT
import re, sys

for line in sys.stdin:
	match = re.match(r'^([a-zA-Z_-]+):.*?## (.*)$$', line)
	if match:
		target, help = match.groups()
		print("%-20s %s" % (target, help))
endef
export PRINT_HELP_PYSCRIPT

CURRENT_DIR = $(shell pwd)
INSTALL_LOCATION := $(CURRENT_DIR)/install
ENABLE_ASAN=1
ASAN_ENV=ASAN_OPTIONS=detect_leaks=0 LD_PRELOAD=/lib64/libasan.so.6

help:
	@python -c "$$PRINT_HELP_PYSCRIPT" < $(MAKEFILE_LIST)

bld: ## build the project
	rm -rf build/
	cmake -Bbuild -DCMAKE_INSTALL_PREFIX=$(INSTALL_LOCATION) -DCMAKE_BUILD_TYPE="Release" -Dmcpp_BUILD_EXECUTABLE=1
	cmake --build build --config Release -j`nproc`

test: ## run tests quickly with ctest
	rm -rf build/
	cmake -Bbuild -DCMAKE_INSTALL_PREFIX=$(INSTALL_LOCATION) -Dmcpp_ENABLE_UNIT_TESTING=1 -Dmcpp_ENABLE_ASAN=$(ENABLE_ASAN) -DCMAKE_BUILD_TYPE="Debug"
	cmake --build build --config Debug -j`nproc`
	cd build/ && $(ASAN_ENV) ctest -C Debug -VV

coverage: ## check code coverage quickly GCC
	rm -rf build/
	cmake -Bbuild -DCMAKE_INSTALL_PREFIX=$(INSTALL_LOCATION) -Dmcpp_ENABLE_CODE_COVERAGE=1
	cmake --build build --config Release
	cd build/ && ctest -C Release -VV
	cd .. && (bash -c "find . -type f -name '*.gcno' -exec gcov -pb {} +" || true)

install: ## install the package to the `INSTALL_LOCATION`
	rm -rf build/
	cmake -Bbuild -DCMAKE_INSTALL_PREFIX=$(INSTALL_LOCATION)
	cmake --build build --config Release
	cmake --build build --target install --config Release

format: ## format the project sources
	rm -rf build/
	cmake -Bbuild -DCMAKE_INSTALL_PREFIX=$(INSTALL_LOCATION)
	cmake --build build --target clang-format

.PHONY: benchmark
benchmark: ## run the benchmark
	rm -rf ./benchmark/build && mkdir -p ./benchmark/build
	cd ./benchmark/build && cmake ../ -DCMAKE_BUILD_TYPE=Release
	cd ./benchmark/build && make -j`nproc`

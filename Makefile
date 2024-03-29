init:
	gh repo clone cmu-db/15445-bootcamp resources/15445-bootcamp
	gh repo clone AnthonyCalandra/modern-cpp-features resources/modern-cpp-featureses
	gh repo clone changkun/modern-cpp-tutorial resources/modern-cpp-tutorial
	gh repo clone wuye9036/CppTemplateTutorial resources/CppTemplateTutorial
	gh repo clone abseil/abseil-cpp resources/abseil-cpp
	gh repo clone filipdutescu/modern-cpp-template resources/modern-cpp-template
	gh repo clone PacktPublishing/Modern-CMake-for-Cpp resources/Modern-CMake-for-Cpp
	gh repo clone adah1972/geek_time_cpp resources/geek_time_cpp
	gh repo clone sftrabbit/CppPatterns-Patterns resources/CppPatterns-Patterns
	gh repo clone downdemo/Cpp-Concurrency-in-Action-2ed resources/Cpp-Concurrency-in-Action-2ed

deps:
	git clone https://github.com/google/googletest.git --branch release-1.11.0 deps/googletest

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

init:
	gh repo clone AnthonyCalandra/modern-cpp-features resources/modern-cpp-featureses
	gh repo clone changkun/modern-cpp-tutorial resources/modern-cpp-tutorial
	gh repo clone wuye9036/CppTemplateTutorial resources/CppTemplateTutorial
	gh repo clone abseil/abseil-cpp resources/abseil-cpp
	gh repo clone filipdutescu/modern-cpp-template resources/modern-cpp-template
	gh repo clone PacktPublishing/Modern-CMake-for-Cpp resources/Modern-CMake-for-Cpp
	gh repo clone adah1972/geek_time_cpp resources/geek_time_cpp

deps:
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

help:
	@python -c "$$PRINT_HELP_PYSCRIPT" < $(MAKEFILE_LIST)

test: ## run tests quickly with ctest
	rm -rf build/
	cmake -Bbuild -DCMAKE_INSTALL_PREFIX=$(INSTALL_LOCATION) -Dmodern-cpp-knowledge_ENABLE_UNIT_TESTING=1 -DCMAKE_BUILD_TYPE="Release"
	cmake --build build --config Release -j`nproc`
	cd build/ && ctest -C Release -VV

coverage: ## check code coverage quickly GCC
	rm -rf build/
	cmake -Bbuild -DCMAKE_INSTALL_PREFIX=$(INSTALL_LOCATION) -Dmodern-cpp-knowledge_ENABLE_CODE_COVERAGE=1
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
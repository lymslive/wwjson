# Makefile for wwjson project
# Common commands integration for development workflow

.PHONY: help
.PHONY: build test install clean
.PHONY: release build/perf perf clean/perf
.PHONY: test/list perf/list docs

# Default target
help:
	@echo "Available targets:"
	@echo "  build      - Build the project in debug mode (build/ directory)"
	@echo "  test       - Build and run unit tests"
	@echo "  install    - Install the library"
	@echo "  clean      - Clean debug build directory"
	@echo "  clean/perf - Clean release build directory"
	@echo "  release    - Build in release mode (build-release/ directory)"
	@echo "  build/perf - Build performance test in release mode"
	@echo "  perf       - Run performance tests (requires BUILD_PERF_TESTS)"
	@echo "  test/list  - Update utest/cases.md from test program --List output"
	@echo "  perf/list  - Update perf/cases.md from perf program --List output"
	@echo "  docs       - Build docs html web pages"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "make <target> -n  Show the command to execute only"

# All test source files
TEST_SOURCES := $(wildcard utest/*.cpp)

# Mark the last time some phony target executed
.touch:
	mkdir -p .touch

# Standard build
build/Makefile:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
build: build/Makefile
	cd build && make

# Run tests (silent mode)
test: build
	./build/utest/utwwjson --cout=silent

# Install the library
install: build
	cd build && make install

# Clean build directory
clean:
	rm -rf build

# Build in Release mode with performance tests enabled
release: clean/perf
	@echo "Building in Release mode with performance tests enabled..."
	mkdir -p build-release
	cd build-release && cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_PERF_TESTS=ON ..
	cd build-release && make -j4

build/perf:
	cd build-release/perf && make

# Clean release build directory
clean/perf:
	rm -rf build-release

# Run performance tests
perf: build/perf
	@echo "Running performance tests..."
	./build-release/perf/pfwwjson

# Update test cases list
test/list: build
	@echo "Updating utest/cases.md..."
	./build/utest/utwwjson --List | perl .tool/list_case.pl --head="单元测试用例列表" > utest/cases.md

# Update performance test cases list
perf/list: build/perf
	@echo "Updating perf/cases.md..."
	./build-release/perf/pfwwjson --List | perl .tool/list_case.pl --head="性能测试用例列表" > perf/cases.md

# Build docs html web pages
docs:
	docs/build.sh

# Alias for help target
.DEFAULT_GOAL := help

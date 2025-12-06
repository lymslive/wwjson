# Makefile for wwjson project
# Common commands integration for development workflow

.PHONY: build test build/perf install clean clean/perf help release perf

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
	cd build-release && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DBUILD_PERF_TESTS=ON ..
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

# Alias for help target
.DEFAULT_GOAL := help

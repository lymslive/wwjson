# Makefile for wwjson project
# Common commands integration for development workflow

.PHONY: build test build/perf install clean help release perf

# Default target
help:
	@echo "Available targets:"
	@echo "  build      - Build the project using cmake and make"
	@echo "  test       - Build and test"
	@echo "  install    - Install the library"
	@echo "  clean      - Clean build directory"
	@echo "  release    - Build in release mode with performance tests"
	@echo "  build/perf - Build perf_test target"
	@echo "  perf       - Run performance tests (requires BUILD_PERF)"
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

# Ge# Build in Release mode with performance tests enabled
release: clean
	@echo "Building in Release mode with performance tests enabled..."
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DBUILD_PERF=ON ..
	cd build && make -j4

build/perf:
	cd build/perf && make

# Run performance tests
perf: build/perf
	@echo "Running performance tests..."
	./build/perf/pfwwjson

# Alias for help target
.DEFAULT_GOAL := help

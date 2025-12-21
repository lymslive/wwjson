# Makefile for WWJSON Documentation
# Build documentation with proper dependency tracking
# Run from project root with: make -f docs/makefile

# Variables
PANDOC = pandoc
PANDOC_OPTS = --template docs/template/default.html \
              --lua-filter=docs/template/remove-chinese-whitespace.lua \
              --highlight-style=kate \
			  --standalone --verbose

# Directories
BUILD_DIR = docs/build
TEMPLATE_DIR = docs/template
SRC_DIR = docs

# Sources and targets
SRC_FILES = $(SRC_DIR)/index.md $(SRC_DIR)/usage.md
HTML_FILES = $(BUILD_DIR)/index.html $(BUILD_DIR)/usage.html
STATIC_FILES = $(BUILD_DIR)/style.css $(BUILD_DIR)/toc.js
API_FILES = $(BUILD_DIR)/api/index.html

# Default target
all: docs

# Main documentation target
docs: $(HTML_FILES) $(STATIC_FILES) $(API_FILES)
	@echo "Documentation build completed!"

# API documentation with Doxygen - create a marker file to track updates
$(BUILD_DIR)/api/.doxygen-stamp: include/wwjson.hpp Doxyfile | $(BUILD_DIR)/api
	@echo "Generating API documentation with Doxygen..."
	doxygen Doxyfile
	@touch $@

$(BUILD_DIR)/api/index.html: $(BUILD_DIR)/api/.doxygen-stamp

# HTML files from markdown (depends on template file but not static files)
$(BUILD_DIR)/index.html: $(SRC_DIR)/index.md $(TEMPLATE_DIR)/default.html | $(BUILD_DIR)
	@echo "Building index.html..."
	$(PANDOC) $< -o $@ $(PANDOC_OPTS) --metadata title="WWJSON - 高性能C++ JSON构建库"

$(BUILD_DIR)/usage.html: $(SRC_DIR)/usage.md $(TEMPLATE_DIR)/default.html | $(BUILD_DIR)
	@echo "Building usage.html..."
	$(PANDOC) $< -o $@ $(PANDOC_OPTS) --metadata title="WWJSON 用户指南"

# Create build directories
$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/api:
	mkdir -p $@

# Copy static files (CSS and JS)
$(BUILD_DIR)/style.css: $(TEMPLATE_DIR)/style.css | $(BUILD_DIR)
	cp $< $@

$(BUILD_DIR)/toc.js: $(TEMPLATE_DIR)/toc.js | $(BUILD_DIR)
	cp $< $@

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

# Show help
help:
	@echo "Available targets:"
	@echo "  all/docs    - Build complete documentation"
	@echo "  Static files (CSS, JS)"
	@echo "  API documentation (Doxygen)"
	@echo "  clean       - Remove build directory"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Usage: make -f docs/makefile [target]"
	@echo "This makefile should be run from the project root directory."

# Phony targets
.PHONY: all docs clean help

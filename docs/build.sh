#!/bin/bash

# Build complete documentation for WWJSON project

set -e

echo "Building WWJSON documentation..."

# Step 1: Generate API documentation with Doxygen
echo "Step 1: Generating API documentation with Doxygen..."
doxygen Doxyfile

# Step 2: Convert markdown to HTML with Pandoc
echo "Step 2: Converting markdown to HTML..."
./docs/pandoc-gen.sh

# Show result
echo ""
echo "Documentation build completed!"
echo "Generated files in docs/build/:"
ls -la docs/build/

echo ""
echo "Documentation structure:"
echo "  docs/build/index.html    - Homepage"
echo "  docs/build/usage.html     - User guide"
echo "  docs/build/api/           - API reference (Doxygen)"

echo ""
echo "You can view the documentation by opening:"
echo "  docs/build/index.html"
#!/bin/bash

# Build complete documentation for WWJSON project

set -e

echo "Building WWJSON documentation..."

# Step 1: Generate API documentation with Doxygen
echo "Step 1: Generating API documentation with Doxygen..."
doxygen --version
doxygen Doxyfile

# Step 2: Convert markdown to HTML with Pandoc
echo "Step 2: Converting markdown to HTML..."
pandoc --version
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
echo "To start a local HTTP server for testing, run:"
echo "  cd docs/build && python3 -m http.server 8000"
echo "Then open http://localhost:8000 in your browser"

#!/bin/bash

# Build complete documentation for WWJSON project

set -e

# Check if we're in the root directory (by .git presence)
if [ ! -d ".git" ]; then
    echo "Error: This script must be run from the project root directory."
    echo "Please run: ./docs/build.sh"
    exit 1
fi

echo "Check command version..."
echo "doxygen --version: "
doxygen --version
echo ""
echo "pandoc --version:"
pandoc --version
echo ""

echo "Building documentation..."
echo "make -f docs/makefile:"
make -f docs/makefile docs

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
echo "The docs/build directory can depoly to remote web server."
echo ""
echo "To start a local HTTP server for testing, run:"
echo "  cd docs/build && python3 -m http.server 8000"
echo "Then open http://localhost:8000 in your browser"

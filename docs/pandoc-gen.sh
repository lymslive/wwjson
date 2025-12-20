#!/bin/bash

# Convert markdown files to HTML using pandoc with template

set -e

# Ensure docs/build directory exists
mkdir -p docs/build

# Convert index.md to HTML using template
pandoc docs/index.md \
  --template docs/template.html \
  --metadata title="WWJSON - 高性能C++ JSON构建库" \
  --metadata subtitle="仅头文件的高性能JSON字符串生成库" \
  -o docs/build/index.html \
  --standalone

# Convert usage.md to HTML using template
pandoc docs/usage.md \
  --template docs/template.html \
  --metadata title="WWJSON 用户指南" \
  --metadata subtitle="详细的功能介绍和使用示例" \
  -o docs/build/usage.html \
  --standalone

echo "HTML generation completed!"
echo "Generated files:"
echo "  - docs/build/index.html"
echo "  - docs/build/usage.html"
#!/bin/bash

# Convert markdown files to HTML using pandoc with template

set -e

# Ensure docs/build directory exists
mkdir -p docs/build

# Create Lua filter to remove whitespace between Chinese characters
cat > docs/build/remove-chinese-whitespace.lua << 'EOF'
function Para(elem)
  -- Process each paragraph to remove whitespace between Chinese characters
  local content = pandoc.utils.stringify(elem)
  -- Remove whitespace between Chinese characters (Unicode range for common Chinese characters)
  local cleaned = content:gsub('([\228-\233][\128-\191][\128-\191])%s+([\228-\233][\128-\191][\128-\191])', '%1%2')
  if cleaned ~= content then
    return pandoc.Para(pandoc.read(cleaned).blocks[1].content)
  end
  return elem
end
EOF

# Convert index.md to HTML using template
pandoc docs/index.md \
  --template docs/template.html \
  --metadata title="WWJSON - 高性能C++ JSON构建库" \
  --metadata generate-toc="false" \
  --lua-filter=docs/build/remove-chinese-whitespace.lua \
  --highlight-style=kate \
  -o docs/build/index.html \
  --standalone

# Convert usage.md to HTML using template
pandoc docs/usage.md \
  --template docs/template.html \
  --metadata title="WWJSON 用户指南" \
  --metadata generate-toc="true" \
  --lua-filter=docs/build/remove-chinese-whitespace.lua \
  --highlight-style=kate \
  -o docs/build/usage.html \
  --standalone

echo "HTML generation completed!"
echo "Generated files:"
echo "  - docs/build/index.html"
echo "  - docs/build/usage.html"
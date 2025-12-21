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
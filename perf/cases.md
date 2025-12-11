# 性能测试用例列表

本文档列出了 perf/ 目录下所有性能测试用例的名称和描述，便于使用 `--list` 参数查找和运行特定用例。

## p_builder.cpp

JSON 构建器性能测试文件，比较 wwjson 和 yyjson 在不同大小 JSON 构建上的性能。

- `build_0_5k_wwjson` - wwjson 构建器性能测试（约 0.5k JSON，n=6）
- `build_0_5k_yyjson` - yyjson 构建器性能测试（约 0.5k JSON，n=6）
- `build_1k_wwjson` - wwjson 构建器性能测试（约 1k JSON，n=12）
- `build_1k_yyjson` - yyjson 构建器性能测试（约 1k JSON，n=12）
- `build_10k_wwjson` - wwjson 构建器性能测试（约 10k JSON，n=120）
- `build_10k_yyjson` - yyjson 构建器性能测试（约 10k JSON，n=120）
- `build_100k_wwjson` - wwjson 构建器性能测试（约 100k JSON，n=1200）
- `build_100k_yyjson` - yyjson 构建器性能测试（约 100k JSON，n=1200）
- `build_ex_wwjson` - wwjson 构建器性能测试（自定义大小）
- `build_ex_yyjson` - yyjson 构建器性能测试（自定义大小）
- `build_sample*` - 生成不同大小的 JSON 样本用于性能测试
- `build_verify*` - 验证 wwjson 和 yyjson 生成相同的 JSON 结构
- `build_size*` - 测试不同 n 值对应的 JSON 大小

## p_number.cpp

数字序列化性能测试文件，比较不同整数和浮点数类型的序列化性能。

- `number_int8_wwjson` - wwjson int8 数组构建性能测试
- `number_int8_yyjson` - yyjson int8 数组构建性能测试
- `number_int16_wwjson` - wwjson int16 数组构建性能测试
- `number_int16_yyjson` - yyjson int16 数组构建性能测试
- `number_int32_wwjson` - wwjson int32 数组构建性能测试
- `number_int32_yyjson` - yyjson int32 数组构建性能测试
- `number_int64_wwjson` - wwjson int64 数组构建性能测试
- `number_int64_yyjson` - yyjson int64 数组构建性能测试
- `number_float_wwjson` - wwjson float 数组构建性能测试
- `number_float_yyjson` - yyjson float 数组构建性能测试
- `number_double_wwjson` - wwjson double 数组构建性能测试
- `number_double_yyjson` - yyjson double 数组构建性能测试
- `number_int_rel` - 随机整数数组相对性能测试
- `number_double_rel` - 随机 double 数组相对性能测试
- `number_array_compare*` - 比较 wwjson 和 yyjson BuildIntArray 输出

## p_string.cpp

字符串序列化性能测试文件，比较字符串数组、对象和转义内容的构建性能。

- `string_array_wwjson` - wwjson 字符串数组构建性能测试
- `string_array_yyjson` - yyjson 字符串数组构建性能测试
- `string_object_wwjson` - wwjson 字符串对象构建性能测试
- `string_object_yyjson` - yyjson 字符串对象构建性能测试
- `string_escape_wwjson` - wwjson 转义字符串对象构建性能测试
- `string_escape_yyjson` - yyjson 转义字符串对象构建性能测试
- `string_compare*` - 比较 wwjson 和 yyjson 字符串函数输出

## p_design.cpp

设计选择性能测试文件，验证不同优化策略的有效性。

- `design_small_int` - NumberWriter 小整数优化验证
- `design_large_int` - 大整数 WriteUnsigned 与 std::to_chars 性能对比
- `design_small_float` - NumberWriter 小范围浮点数优化验证
- `design_large_division` - 大整数除法策略（10000 vs 100）性能对比


# 性能测试用例列表

## p_api.cpp

- `api_basic_vs_autoclose` - 基本方法 vs 自动关闭方法性能对比
- `api_basic_vs_operator` - 基本方法 vs 操作符方法性能对比
- `api_basic_vs_localobj` - 基本方法 vs 局部对象方法性能对比
- `api_basic_vs_lambda` - 基本方法 vs Lambda方法性能对比
- `api_basic_vs_class` - 基本方法 vs 类方法性能对比
- `api_output_sample*` - 输出各方法构建的JSON示例
- `api_basic_vs_tojson` - 基本方法 vs to_json方法性能对比
- `api_tojson_sample*` - 输出to_json方法构建的JSON示例

## p_builder.cpp

- `build_0_5k_wwjson*` - wwjson 构建器性能测试（约 0.5k JSON，n=6）
- `build_0_5k_yyjson*` - yyjson 构建器性能测试（约 0.5k JSON，n=6）
- `build_1k_wwjson*` - wwjson 构建器性能测试（约 1k JSON，n=12）
- `build_1k_yyjson*` - yyjson 构建器性能测试（约 1k JSON，n=12）
- `build_10k_wwjson*` - wwjson 构建器性能测试（约 10k JSON，n=120）
- `build_10k_yyjson*` - yyjson 构建器性能测试（约 10k JSON，n=120）
- `build_100k_wwjson*` - wwjson 构建器性能测试（约 100k JSON，n=1200）
- `build_100k_yyjson*` - yyjson 构建器性能测试（约 100k JSON，n=1200）
- `build_ex_wwjson*` - wwjson 构建器性能测试（自定义大小）
- `build_ex_yyjson*` - yyjson 构建器性能测试（自定义大小）
- `build_sample*` - 生成不同大小的 JSON 样本用于性能测试
- `build_verify*` - 验证 wwjson 和 yyjson 生成相同的 JSON 结构
- `build_size*` - 测试不同 n 值对应的 JSON 大小
- `build_relative` - JSON 构建相对性能测试

## p_design.cpp

- `design_small_int` - NumberWriter 小整数优化验证
- `design_large_int` - 大整数 WriteUnsigned 与 std::to_chars 性能对比
- `design_small_float` - NumberWriter 小范围浮点数优化验证
- `design_string_literal` - 字符串字面量拷贝优化验证
- `design_string_escape` - 字符串转义优化方案预测试
- `design_fractional_serialization` - 4位小数部分序列化优化验证
- `design_write_unsigned_compare` - 无符号整数序列化方法对比测试

## p_nodom.cpp

- `nodom_raw_vs_snprintf` - wwjson RawBuilder vs snprintf性能对比
- `nodom_raw_vs_append` - wwjson RawBuilder vs string::append性能对比
- `nodom_raw_vs_stream` - wwjson RawBuilder vs stringstream性能对比

## p_number.cpp

- `number_int8_wwjson*` - wwjson int8 数组构建性能测试
- `number_int8_yyjson*` - yyjson int8 数组构建性能测试
- `number_int16_wwjson*` - wwjson int16 数组构建性能测试
- `number_int16_yyjson*` - yyjson int16 数组构建性能测试
- `number_int32_wwjson*` - wwjson int32 数组构建性能测试
- `number_int32_yyjson*` - yyjson int32 数组构建性能测试
- `number_int64_wwjson*` - wwjson int64 数组构建性能测试
- `number_int64_yyjson*` - yyjson int64 数组构建性能测试
- `number_array_compare*` - 比较 wwjson 和 yyjson BuildIntArray 输出
- `number_float_wwjson*` - wwjson float 数组构建性能测试
- `number_float_yyjson*` - yyjson float 数组构建性能测试
- `number_double_wwjson*` - wwjson double 数组构建性能测试
- `number_double_yyjson*` - yyjson double 数组构建性能测试
- `number_int_rel` - 随机整数数组相对性能测试
- `number_double_rel` - 随机 double 数组相对性能测试

## p_string.cpp

- `string_array_wwjson*` - wwjson 字符串数组构建性能测试
- `string_array_yyjson*` - yyjson 字符串数组构建性能测试
- `string_object_wwjson*` - wwjson 字符串对象构建性能测试
- `string_object_yyjson*` - yyjson 字符串对象构建性能测试
- `string_escape_wwjson*` - wwjson 转义字符串对象构建性能测试
- `string_escape_yyjson*` - yyjson 转义字符串对象构建性能测试
- `string_compare*` - 比较 wwjson 和 yyjson 字符串函数输出
- `string_object_relative` - 字符串对象构建相对性能测试（wwjson vs yyjson）
- `string_escape_relative` - 转义字符串对象构建相对性能测试（wwjson vs yyjson）


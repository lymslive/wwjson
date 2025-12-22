# 单元测试用例列表

## t_advance.cpp

- `advance_reopen` - 对象和数组的 Reopen 方法测试
- `advance_merge_instance` - 对象和数组的 Merge 实例方法测试
- `advance_merge_static` - 对象和数组的 Merge 静态方法测试
- `advance_merge_complex` - 复杂合并场景测试
- `advance_putsub` - 添加 JSON 子串的 PutSub 方法测试
- `advance_additemsub` - 将 JSON 子串作为数组项添加的 AddItemSub 方法测试
- `advance_addmembersub` - 将 JSON 子串作为成员值添加的 AddMemberSub 方法测试
- `advance_sub_with_scope` - AddItemSub/AddMemberSub 与 scope 对象结合测试
- `advance_sub_complex` - JSON 子串的复杂场景测试
- `advance_function_lambda` - AddItem 与 lambda 函数测试
- `advance_function_free` - AddItem 与自由函数测试
- `advance_function_class` - AddItem 与类方法测试
- `advance_function_with_addmember` - AddMember 与可调用函数测试
- `advance_function_nested` - 嵌套可调用函数测试
- `advance_function_scope_with_callable` - scope 对象与可调用函数结合测试

## t_basic.cpp

- `basic_builder` - 基础 JSON 构建器测试
- `basic_builder_root` - 使用 BeginRoot 的 JSON 构建器测试
- `basic_builder_nest` - 嵌套 JSON 构建测试（对象数组）
- `basic_wrapper` - M1 字符串接口封装方法测试
- `basic_null_bool_empty` - null、bool、空数组和空对象测试
- `basic_low_level` - 底层方法 PutKey/PutValue/PutNext 使用测试
- `basic_addmember_overloads` - 不同键参数类型的 AddMember 重载测试
- `basic_string_view_support` - std::string_view 对键值的支持测试
- `basic_getresult` - GetResult 移除尾逗号功能测试
- `basic_moveresult` - MoveResult 方法测试
- `basic_null_string` - 字符串边界情况中的空指针安全测试
- `basic_empty_string` - 非空但空字符串的边界情况测试
- `basic_addmember_key` - AddMember 单键名重载测试 - 支持拆分键名设置
- `builder_prefix_constructor` - 测试带前缀的构造函数
- `builder_multiple_json_with_endline` - 测试使用 EndLine 构建多个 JSON

## t_custom.cpp

- `custom_builder` - 自定义字符串的 JSON 构建器测试
- `custom_wrapper` - 自定义字符串的 M1 接口封装方法测试
- `custom_scope` - 使用自定义字符串和自动关闭 scope 方法构建嵌套 JSON
- `custom_number_quoted` - AddItem/AddMember 方法的数字引号行为测试

## t_escape.cpp

- `escape_table_basic` - 转义表基本功能测试
- `escape_table_utf8` - UTF-8 字符不被转义测试
- `escape_table_mapping` - 转义表映射优化测试
- `escape_builder_api` - 构建器中的转义方法测试
- `escape_member_key_only` - AddMemberEscape 单参数键转义测试
- `escape_always_config` - 自定义配置：始终转义测试
- `escape_key_config` - 自定义配置：仅转义键测试
- `escape_value_config` - 自定义配置：仅转义值测试
- `escape_no_config` - 自定义配置：无转义测试
- `escape_scope_objects` - scope 对象的转义功能测试
- `escape_std_ascii` - 标准 ASCII 转义字符测试
- `escape_edge_cases` - 转义功能的边界情况测试

## t_experiment.cpp

- `ext_to_chars*` - test std::to_chars
- `ext_double_view*` - 分析 double 类型的二进制编码
- `ext_float_view*` - 分析 float 类型的二进制编码
- `ext_2decimals*` - 测试两位小数精度
- `ext_4decimals*` - 测试四位小数精度
- `ext_8decimals*` - 测试八位小数精度
- `ext_2decimals_float*` - 使用 float 测试两位小数精度
- `ext_4decimals_float*` - 使用 float 测试四位小数精度
- `ext_8decimals_float*` - 使用 float 测试八位小数精度
- `ext_generic_float*` - 通用定点浮点数测试，支持命令行参数
- `ext_float_place*` - 检测浮点数的小数位数

## t_number.cpp

- `number_integer_member` - 8 种标准整数类型的序列化测试
- `number_integer_item` - 数组中整数的序列化测试
- `number_float_serialization` - 多种浮点数序列化场景测试
- `number_std_support` - 检查当前运行时对 std::to_chars 的支持
- `number_float_basic` - 浮点数优化基本测试
- `number_float_edge` - 浮点数优化的边界情况测试
- `number_float_negative` - 负数浮点数优化测试
- `number_float_special` - 特殊浮点数值优化测试
- `number_small_double` - 测试 double 值是否走 WriteSmall 路径
- `number_miss_double` - 检查 WriteSmall 路径的错过率

## t_operator.cpp

- `operator_string_key` - operator[] 与字符串键赋值测试
- `operator_array_index` - operator[] 与数组索引赋值测试
- `operator_mixed_usage` - operator[] 与传统方法混合使用测试
- `operator_edge_cases` - operator[] 边界情况与特殊字符测试
- `operator_type_safety` - operator[] 与不同数据类型测试
- `operator_nested_structures` - operator[] 与嵌套对象数组测试
- `operator_raii_compatibility` - operator[] 与 RAII scope 类兼容性测试
- `operator_copy_move` - operator[] 与拷贝移动操作测试
- `operator_stream_array` - operator<< 与 GenericArray 链式添加元素测试
- `operator_stream_object` - operator<< 与 GenericObject 交替键值对测试
- `operator_stream_mixed` - operator<< 与传统方法混合使用测试
- `operator_stream_types` - operator<< 与多种数据类型测试
- `operator_stream_complex` - operator<< 与复杂嵌套结构测试

## t_scope.cpp

- `scope_ctor_nest` - RAII 自动关闭的嵌套 JSON 构建
- `scope_auto_nest` - 使用 scope 方法自动关闭的嵌套 JSON 构建
- `scope_vs_constructor` - scope 方法与构造方法对比
- `scope_if_bool_operator` - scope 变量的 if 语句中 operator bool 测试
- `scope_if_bool_vs_constructor` - 构造方法中的 if bool 语法测试
- `scope_addmember_split` - ScopeArray/Object 拆分测试 - AddMember + Scope 的组合用法

## t_usage.cpp

- `usage_2_2_first_example` - example from docs/usage.md
- `usage_3_1_json_concatenation` - example from docs/usage.md
- `usage_3_2_wwjson_encapsulation` - example from docs/usage.md
- `usage_4_1_flat_construction` - example from docs/usage.md
- `usage_4_2_brace_indentation` - example from docs/usage.md
- `usage_4_2_scope_variables` - example from docs/usage.md
- `usage_4_2_named_scope_variables` - example from docs/usage.md
- `usage_4_2_if_statement_scope` - example from docs/usage.md
- `usage_4_3_lambda_substructure` - example from docs/usage.md
- `usage_4_4_operator_overloading` - example from docs/usage.md
- `usage_4_5_step_entrance` - example from docs/usage.md
- `usage_4_5_struct_builder_call` - example from docs/usage.md
- `usage_5_1_add_substring` - example from docs/usage.md
- `usage_5_2_merge_substrings` - example from docs/usage.md
- `usage_6_2_1_safe_config` - example from docs/usage.md
- `usage_6_2_1_quote_numbers` - example from docs/usage.md
- `usage_6_2_2_unsafe_config` - example from docs/usage.md
- `usage_6_3_1_log_line_target` - example from docs/usage.md
- `usage_6_3_2_message_stream_target` - example from docs/usage.md


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

## t_bufferview.cpp

- `bufv_layout` - BufferView 布局大小测试
- `bufv_invariants` - BufferView 不变关系式测试
- `bufv_constructors` - BufferView 构造函数测试
- `bufv_move_constructor` - BufferView 移动构造测试
- `bufv_ends_access` - BufferView 两端指针及元素获取
- `bufv_str_converstion` - BufferView 转换字符串
- `bufv_write_safe` - BufferView 写入方法测试
- `bufv_write_unsafe` - BufferView 不检查边界的写入方法测试
- `bufv_append_string` - BufferView append 字符串重载
- `bufv_borrow_string` - BufferView 借用 std::string 并验证 resize 问题
- `bufv_borrow_vector` - BufferView 借用 std::vector<char> 并验证 resize 问题
- `bufv_extern_write` - BufferView 与外部方法写入集成协作
- `bufv_null_end` - BufferView 空字符封端测试
- `ubuf_constructors` - UnsafeBuffer 继承构造函数测试
- `ubuf_move_constructor` - UnsafeBuffer 移动构造测试
- `ubuf_write_methods` - UnsafeBuffer 写入方法测试

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
- `escape_ident_key` - 自定义配置：键名转义为标识符验证

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

## t_itoa.cpp

- `itoa_unsigned` - IntegerWriter 无符号整数测试
- `itoa_signed` - IntegerWriter 有符号整数测试
- `itoa_edge_cases` - IntegerWriter 边界情况测试

## t_jbuilder.cpp

- `jbuilder_unsafe_level` - unsafe_level 编译期特征萃取测试
- `jbuilder_basic` - Builder 基本功能测试
- `jbuilder_nested` - Builder 嵌套结构测试
- `jbuilder_raii` - Builder RAII 包装器测试
- `jbuilder_fast_basic` - FastBuilder 基本功能测试
- `to_json_scalars` - to_json scalar types and array elements
- `to_json_containers` - to_json containers and nested structs
- `to_json_macro` - TO_JSON macro usage
- `to_json_standalone` - standalone wwjson::to_json(struct)
- `to_json_associative` - to_json associative containers (map)
- `to_json_optional` - to_json std::optional types

## t_jstring.cpp

- `jstr_construct` - JString 基础构造测试
- `jstr_copy_move` - JString 复制和移动语义测试
- `jstr_append_string` - JString 字符串追加测试
- `jstr_push_char` - JString 单字符追加测试
- `jstr_edge_cases` - JString 边界情况测试
- `jstr_capacity_growth` - JString 容量增长测试
- `jstr_reserve` - JString 容量预留测试
- `jstr_unsafe_operations` - JString 不安全操作测试
- `jstr_unsafe_levels` - StringBuffer 不安全级别语义测试
- `jstr_json_patterns` - JString JSON 序列化模式测试
- `jstr_extern_write` - StringBuffer 与外部方法写入集成协作
- `kstr_construct` - KString 基础构造测试
- `kstr_reach_full` - KString 写满对比测试

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


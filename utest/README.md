# wwjson 单元测试

本项目使用 [couttast](https://github.com/lymslive/couttast) 作为测试框架。

## 基本用法：
- `DEF_TAST(name, "description")` 定义一个用例
- `COUT(expr, expect)` 断言语句，可省略 expect 参数，只打印 expr 值观察

## 测试文件规范：
- 以 `t_xxx.cpp` 命名每个测试文件，收集相关用例，以 `xxx_` 前缀命名；
- 非测试功能的辅助文件不以 `t_` 前缀命名；

当前测试文件结构：
- `t_basic.cpp` - 基础构建功能测试
  - `basic_builder` - 测试基础对象构建
  - `basic_builder_array` - 测试数组与对象嵌套
  - `basic_escape` - 测试基本字符串转义
- `t_escape.cpp` - 字符串与转义功能测试
  - `escape_basic` - 测试基本转义功能
  - `escape_always_config` - 测试 always escape 配置
  - `escape_scope` - 测试作用域对象的转义
  - `escape_overloads` - 测试各种字符串参数重载
  - `escape_no_config` - 测试无转义配置
- `t_scope.cpp` - 作用域管理测试
  - `scope_ctor_nest` - 测试构造函数嵌套
  - `scope_auto_nest` - 测试作用域方法自动嵌套
  - `scope_vs_constructor` - 测试作用域方法与构造函数对比
- `t_custom.cpp` - 自定义字符串类型测试
  - `custom_builder` - 测试自定义字符串构建器
  - `custom_scope` - 测试自定义字符串作用域方法
- `t_experiment.cpp` - 实验性功能测试（各种探索性用例）
- `custom_string.cpp` - 自定义字符串类实现（非测试文件）

## 命令行用法

构建成功后在 build/utest 目录运行，或者根目录有软链接 utwwjson。

运行大量用例时建议加 `--cout=silent` ，调试个别用例是不加 `--cout` 参数，默认
输出详细信息。位置参数支持用例名（或子串匹配）与文件名筛选用例。

```bash
# 运行所有测试，或根目录的快速目标 make test
./utwwjson --cout=silent

# 运行特定测试用例，可指定多个
./utwwjson test_name ...

# 运行特定文件内的所有用例，也可指定多个文件
./utwwjson t_filename.cpp --cout=silent

# 运行名字中包含 subname 的用例
./utwwjson subname --cout=silent

# 列出所有测试用例名，--List 还包含文件名位置等详情
./utwwjson --list

# 获取帮助信息
./utwwjson --Help
```

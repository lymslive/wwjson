# 项目开发原始需求管理

本文档由人工维护，**AI受限编辑** ，仅允许在每个 `## TODO` 下加个子标题 `###
DONE` 关联 `task_log.md` 的完成任务ID记录。

需求ID 格式：`TODO:YYYY-MM-DD/n` ，日期/编号。日期表示头脑风暴冒出该需求想法的
日期，若一天中产出多个想法则加编号。仅有初步概念但未细化的需求暂不加日期 id ，
先快速列出 `TODO: 标题`，后面有细化内容时再赋日期 id ，转为正式需求。

---

## 上个版本 v1.0.0 需求摘要

WWJSON v1.0.0 已完成发布，主要成果包括：

### 核心功能
- ✅ 仅头文件 JSON 构建库，支持高性能直接字符串拼接
- ✅ 完整的 RAII 作用域管理（GenericArray/GenericObject）
- ✅ 模板化设计支持自定义字符串类型
- ✅ 配置化转义策略和数字序列化选项
- ✅ 多种 API 风格：传统方法、操作符重载、链式调用、Lambda 嵌套

### 性能优化
- ✅ 小整数缓存表优化，性能提升 10-15 倍
- ✅ 浮点数定点优化，四位小数序列化提升 20%
- ✅ 字符串转义优化，使用临时缓冲区避免频繁 push_back
- ✅ 完整的性能测试框架，与 yyjson 等主流库对比

### 开发工具
- ✅ CMake 构建系统和安装配置
- ✅ 75 个单元测试用例，覆盖所有核心功能
- ✅ 44 个性能测试用例，全面的基准测试
- ✅ CI/CD 流水线，自动化测试和文档部署

### 文档系统
- ✅ 完整的 Doxygen API 文档
- ✅ 详细的用户指南和示例
- ✅ 在线文档自动部署到 GitHub Pages

### 详细记录
- [changelog/v1.0/task_todo.md](changelog/v1.0/task_todo.md)
- [changelog/v1.0/task_log.md](changelog/v1.0/task_log.md)

---

**后续需求记录**

## TODO:2025-12-22/1 v1.0.0 封版发布

基础功能开发完毕，文档补完，可以打个标签了。
后续开发计划：
- 继续性能优化
- 开发专用的字符串类作为写入目标
- 扩展功能可能写在单独的头文件更合适，依然是仅头文件，但未必单头文件

### DONE: 20251222-110430

## TODO:2025-12-22/2 再测试探索整数序列化优化方法

疑问：用 memcpy 拷贝 2 个字符更好，还是写两次单字符赋值？

在 `perf/p_design.cpp` 文件中增加一个相对测试类 `test::perf::WriteUnsignedCompare`：
- 方法 B: 拷贝当前实现 `NumberWriter::WriteUnsigned` 作为测试基准
- 方法 A: 两次 `*(--ptr)` 赋值改为 `::memcpy(ptr -=2, src, 2)`

如果方法 A 反而更慢，再思考下有没更快的写法？

### DONE: 20251222-232501

## TODO:2025-12-22/3 将整数序列化方法改为 memcpy 拷贝缓存表的 2 字符

根据 `test::perf::WriteUnsignedCompare` 的测试结果显示，应该采用 memcpy 2 字符
更好些。据此修改 NumberWriter::WriteUnsigned 与 WriteSmall 的相关实现。

同时为简化代码，不必使用 `const DigitPair &pair` 中间变量了，
直接取 `const char* digit` 指针变量。

修改 wwjson.hpp 前，先备份 ./build-release/perf/pfwwjson 为 pfwwjson.last
修改后，再对比前后版本的性能测试，重点关注以下两个用例：
- `design_large_int` 
- `number_int_rel`

### DONE: 20251223-000925

## TODO:2025-12-23/1 开发更适合 JSON 序列化的字符串类

### 背景问题

标准字符串用于 json 序列化写入目标存在的性能问题：
- 扩容导致的数据复制
- 频繁写入单字符逗号、引号、冒号需要边界检查
- 可能需要时刻保证 '\0' 字符封端
- 与第三方类型格式化转换函数的协作性，不能直接被操作 buffer 内存的话，只能先额
  外保存临时 buffer 再 append 到目标字符串，多一次拷贝，如数字转字符串、字符串
  转义等需求。

其中第一个问题用预估容量的低使用成本就能解决大部分问题，严肃解决方案可能是采用
分段 buffer ，只在最后有需要时再拷成连续的。对于存在被消费的场景适合采用分段免
拷贝方案，但对于构建完整 JSON 的常用需求讲，预申请足够内存比分段申请更简单。

故先尝试优化后面几个问题。设计一种专门的字符类，或者本质上是个 string buffer
类，用以进一步提升 json 串构建性能。

### 初步方案

新增一个 include/jstring.hpp 头文件。

定义一个 struct UnsafeStringConcept : public StringConcept;
C++17 用 struct 模拟 concept 概念说明。

依然是空基类，没有非静态成员数据，额外增加几个方法约定要求：
- 静态常量 kUnsafeLevel，默认 0，`uint8_t` 类型即可
- 方法 `unsafe_push_back` ，不检查边界添加单字符
- 方法 `unsafe_set_end` ，直接设置当前字符串写入末端，相当于设置字符串长度
- 方法 `unsafe_end_cstr` ，在当前字符串末尾添加一个 '\0'
- 方法 `reserve_ex` ，增量式检查预留可写容量，额外申请 kUnsafeLevel 个字节，表
  示写完指定内容后，还允许额外使用 kUnsafeLevel - 1 次 `unsafe_push_back`，-1
  是给 '\0' 字符预留的。

与标准字符串行为的关键区别是，每次 `append` 或 `push_back` 写入主要内容时作边
界检查，但考虑 kUnsafeLevel 余量，使后面可以用几次 `unsafe_push_back` 写入少量
格式字符。并且在字符串增长过程中不保证空字符封端，只需保证最终结果可封端。

再定义一个 StringBuffer 类，带整数型模板参数 kUnsafeLevel ，实现以上 
UnsafeStringConcept。可以 tepydef 定义个 UnsafeLevel 别名表示整数。

StringBuffer 用三指针方案表示内部状态：
- `m_begin` 字符串开始位置
- `m_end` 字符串结束位置，遵循标准左闭开右原则
- `m_cap_end` 当前内存区域结束位置，可以先写入一个 '\0'

考虑将来扩展，再将这三个指针成员单独封装为 struct StringBufferView ，
让 StringBuffer 类私有继承它（再可选继承 UnsafeStringConcept）。
虽然不常用，但应该允许 StringBuffer 类显式转换为其父类 StringBufferView ，将类
的私的成员转为结构体的公有成员。

再定义一个常用别名，JString 类就是 kUnsafeLevel = 4 的 StringBuffer 。因为在
json 串构建中有可能出现连续三个 `":"` 或 `","`，kUnsafeLevel 在 4 以上就安全了。

请实现以上类，也都在 `wwjson::` 命名空间之下。再在 `utest/t_jstring.cpp` 测试
其基本功能。

### DONE:20251223-173539


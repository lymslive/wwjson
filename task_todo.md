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

## TODO:2025-12-24/1 StringBuffer 内存管理优化

首先要澄清 kUnsafeLevel = x 的意义，有两种定义约定：
1. 使用安全 api 写入后，可以再调用 `unsafe_push_back` x-1 次，与一个空字符
2. 使用安全 api 写入后，可以再调用 `unsafe_push_back` x  次，与一个空字符

细微差别在于 kUnsafeLevel 的值要不要包含尾封端预留的 `\0` 字符。
如果按第 1 种定义，那在构造函数传入 capacity = n 时，应该申请 n+x 字节，标准字
符串相当于 kUnsafeLevel = 1；如果按第 2 种定义，构造函数应该申请 n+x+1 字节，
标准字符串相当于 kUnsafeLevel = 0.

我上次的想法是按定义 1 ，JString 的 kUnsafeLevel=4 恰好能满足需求。但再想一下，
按第 2 种定义可能更合适。`-1` 的说明对用户挺拗口，而且若按定义 1 ，就不能实例
化 kUnsafeLevel=0 的类，需要额外加 assert 保证。unsafe 概念应该与标准字符串
就有的空字符封端相互独立，所以按定义 2 明确 kUnsafeLevel 的意义。

那么在构建函数 `StringBuffer(size_t capacity)` 就该申请
`capacity + kUnsafeLevel + 1` 个字节。
且与默认构造时再 `reserve(capacity)` 或 `reserve_ex(capacity)` 应该是一样的行为。

然后在 `allocate(size_t capacity)` 实际申请内存时，还应考虑多申请一点：
- 向上圆整对齐，总不好直接按参数申请奇数个字节；
- 设定个首次最小申请字节，比如 256 ；显然 StringBuffer 不是为小字符串使用的

并且在随后的扩容时应该按常规的 2 倍指数申请新内存，快速扩张到较大内存时如 8M
再线性扩展，每次多申请 8M 。最小申请内存与最大指数内存这两个值定义为编译宏，允
许构建时重定义，实现中给出合理默认值。

容量问题要保证不变式：
- `capacity()` 方法返回的实际容量保证比用户 reserve 的多 kUnsafeLevel ；
- `capacity()` 就该等于 `m_cap_end - m_begin` 两个指针相减；
- 向操作系统申请的内存至少是 `capacity() + 1`;
- `m_cap_end` 指向的位置不属于字符串内容，但属于 StringBuffer 申请管理的内存空
  间，该位置在申请时应该先写入 `\0` ，保证最坏情况不会读越界，中间大段内存考虑
  效率不必全清 0 ，假定要写入覆盖的；
- `unsafe_end_cstr()` 也允许在 `m_cap_end` 处再重复写个 `\0` ，正常情况下是在
  `m_cap_end` 之前提早封端，但为了健壮性用 `<=` 判断，允许最尾端写。

两个 reserve 方法的关系，扩展的 `reserve_ex(n)` 应该恒等于标准 `reserve(size()+n)`，
增补的 `+kUnsafeLevel` 逻辑应放在 `reserve` 。

to AI:
请按以上要求重新 review include/jstring.hpp 的代码与注释，按要求修改。
也再 review utest/t_jstring.cpp 的单元测试，在原来合适的用例中适当增加测试场景，
比较长的用例可用裸 `{}` 划分作用域以便使用相同的变量名。可以修改用例名与描叙，
但不要新加 `DEF_TAST` 用例名了，因为已经比较多了。

### DONE:20251224-121931

## TODO:2025-12-25/1 重构 StringBufferView 继承关系

- 改为 StringBuffer 公有继承 StringBufferView
- StringBufferView 成员改为私有，方法公有
- 将 StringBuffer 的基本只读方法移到 StringBufferView 基类
- 将 StringBuffer 的 unsafe 写入方法移到基类，保留 safe 方法与内存管理方法

### DONE:20251225-114051

## TODO:2025-12-25/2 StringBuffer 功能设计增强

- StringBufferView 增加 begin() end() 方法，返回对应指针成员，一是符合标准迭代
  器惯例，二是允许从 end() 处继续写；也为一致性增加 cap_end() 方法；
- unsafe_set_end 方法增加 char* 指针参数重载，如果易与当前的 size_t 参数混淆，
  保留指针参数；因为想支持的用法是让外部从 end 处开始写，写到新位置再设置回去。
- StringBuffer 增加安全版的 set_end ，检查不溢界
- StringBufferView 增加 unsafe_append(char*, size) 双参数版，StringBuffer 的安
  全版 append 与 push_back 调用基类的 unsafe 版。
- 同样地，基类的 unsafe_end_cstr 不作边界检查，增加 end_cstr 做边界检查。

完善单元测试，t_jstring.cpp 增加一个用例，在 JString 原位 buffer 上调用
std::to_chars 转换整数，先预留足够空间，在从 end 处写入整数，再更新 end 位置。

测试命令用 `./utest/utwwjson t_jstring.cpp --cout=silent`

### DONE:20251225-145239

## TODO:2025-12-26/1 StringBuffer end 状态管理优化

- 待修改文件：include/jstring.hpp，utest/t_jstring.cpp
- 涉及类：StringBufferView, StringBuffer

- `unsafe_set_end(size_t)` 重载方法名改为 unsafe_resize
- `set_end(size_t)` 方法名改为 resize，可扩容
- `clear` 相当于 unsafe_resize(0), 不再隐含修改 end 字符 '\0' ，只让 `c_str`
  修改 end 字符为 '\0'

### DONE:20251226-112337

## TODO: StringBuffer 与标准字符串的互操作

- 待修改文件：include/jstring.hpp，utest/t_jstring.cpp
- StringBufferView 可隐式转换为 std::string_view 
- StringBufferView 需显式转换为 std::string, 因涉及拷贝
- StringBuffer 的 append 方法增加重载，支持参数 std::string 与 std::string_view

## TODO: StringBufferView 增加 fill 方法

- 待修改文件：include/jstring.hpp，utest/t_jstring.cpp
- StringBufferView 增加 fill(ch, count) 方法，类似 memset ，重复填充相同字符
- 不扩容，检查参数 count 不超过容量
- 额外加个参数表示是否移动 end 指针，默认 false
- StringBuffer 增加 append(count, ch)，允许扩容

## TODO: 设计 StringBuffer 默认构造状态

为避免空指针的处理，默认构造提默认容量

StringBufferView 增加 operator bool 判断是不有效内存 m_begin 非空

## TODO: StringBufferView 重命名

- StringBufferView 简化为 BufferView
- 再继承一个 LocalBuffer 类，模板参数 bool UNSAFE
  + 默认 UNSAFE = false, 每次 append push_back 检查边界，kUnsafeLevel = 0
  + UNSAFE = true 时 append push_back 不检查边界，就相当于调用 unsafe 版,
    kUnsafeLevel = 0xFF 表示最大
- BufferView 增加 overflow() 检测，reserve_ex() 空参时检查剩余可用字节

## TODO: 重新设计单元测试

## TODO: 增加 jbuilder.hpp 组合使用 jstring.hpp

- wwjson.hpp 与 jstring.hpp 仍有独立使用意义，互不依赖
- jbuilder.hpp 依赖 wwjson.hpp 与 jstring.hpp 
- wwjson.hpp 增加编译期判断 stringT 的 unsafe level 功能

常用类别名:
- Builder: 使用 JString
- LocalBuffer: 使用 `LoclBuffer<false>`
- UnsafeBuilder / FastBuilder: 使用 `LoclBuffer<true>`

## TODO: wwjson.hpp 根据 unsfe level 重构 GenericBuiler


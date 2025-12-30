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

## TODO:2025-12-26/2 StringBuffer 与标准字符串的互操作

- 待修改文件：include/jstring.hpp，utest/t_jstring.cpp
- StringBufferView 可隐式转换为 std::string_view 
- StringBufferView 需显式转换为 std::string, 因涉及拷贝
- StringBuffer 的 append 方法增加重载，支持参数 std::string 与 std::string_view

### DONE:20251226-121411

## TODO:2025-12-26/3 StringBufferView 增加 fill 方法

- 待修改文件：include/jstring.hpp，utest/t_jstring.cpp
- StringBufferView 增加 fill(ch, count) 方法，类似 memset ，重复填充相同字符
- 不扩容，检查参数 count 不超过容量
- 额外加个参数表示是否移动 end 指针，默认 false
- StringBuffer 增加 append(count, ch)，允许扩容

fill 的参数大约是 (char ch, size_t count = -1, bool end = false);
其设计用意：
- fill(0) 可以将 buffer （剩余）部分全置 0 ，因为默认初始化时只在 cap_end 置 0，
  允许用户按需全填 `\0`
- fill('x', count, true) ，允许填充其他字符内容，且同步 size 增 count
- fill 与 append 的前两个参数位置不同，主要分别参考 memset 与 std::string 的
  append 同名方法的签名

### DONE:20251226-154417

## TODO:2025-12-26/4 设计 StringBuffer 默认构造状态

待修改文件：include/jstring.hpp，utest/t_jstring.cpp

为避免空指针的处理，StringBuffer 缺省构造提默认容量。
- 定义默认容量 1024 - kUnsafeLevel - 1，保证默认初始申请内存 1024.
- JSTRING_MIN_ALLOC_SIZE 最小申请内存宏删除，也允许用户显式指定较小的初始容量
传给构造函数，但仍要向上对齐。
- StringBufferView 增加 operator bool 判断是否申请过内存， m_begin 非空
- StringBufferView 在 back/front 方法与 unsafe 方法中增加仅调试版本的 assert
做安全检查，也再审核其他代码是否需要 assert 。

修改了默认构造函数语义，可能会破坏原有一些单元测试，需要同步修复。
不必再新加单元测试用例，在原有的用例中适合的地方加上 operator bool 判断。

### DONE:20251226-174807

## TODO:2025-12-27/1 StringBufferView 重命名再派生 LocalBuffer 类

- StringBufferView 简化为 BufferView
- 再继承一个 LocalBuffer 类，模板参数 bool UNSAFE
  + 默认 UNSAFE = false, 每次 append push_back 检查边界，kUnsafeLevel = 0
  + UNSAFE = true 时 append push_back 不检查边界，就相当于调用 unsafe 版,
    kUnsafeLevel = 0xFF 表示最大
- BufferView 增加 overflow() 检测是否溢界，reserve_ex() 空参时检查剩余可用字节

LocalBuffer 与 StringBuffer 的作用应该基本相似，但不拥有自己的内存，需要借用其
他内存段，由用户保证在有效的内存区域写入。因此不应该提借默认默认构造，必要提供
有效内存指针构造，支持以下构造：

- `LocalBuffer(char* dst, size_t size)` 指定地址与长度，begin/end 都指向 dst 入
  参，即 size = 0，capacity = size-1，cap_end 处预写 '\0'
- `LocalBuffer(char[N]& dst)`
- `LocalBuffer(std::array<N>& dst)`
- `LocalBuffer(std::string& dst)`
- `LocalBuffer(std:vector<char>& dst)`

以第一个构造函数为主，借用其他对象的内存转调第一个构造函数。正常情况下，不该再
使用传入参数的对象，它只用于自己申请与释放内存。尤其是当通过 LocalBuffer 写入
数据后，注意不能同步 std::string 或 std::vector 的 size ，它仍是 0 。但它们在
传入构造函数之前应该 reserve 内存。

LocalBuffer 构造之后不能扩容，在使用过程中可以按需调用 `reserve_ex()` 检查可写
余量，使用完毕可检查 `overflow()`. 注意在写出边界后 `reserve_ex()` 应该返回负
数，所以返回值不能用 `size_t` ，改用 `int64_t`.

### DONE:20251227-171417
该任务粒度大了，实施较麻烦。

## TODO:2025-12-27/2 优化设计 BufferView::fill 方法的功能

涉及文件：include/jstring.hpp utest/t_jstring.cpp

当前 `BufferView::fill` 一个方法承载了两种功能，又有两个可选参数，设计不良，故
重新设计：
- 单参数 `fill(ch)`, 用 `ch` 填充剩余空间，类似 memset ，不改变当前 size
- 双参数 `fill(ch, count)`, 类似 `append(count, ch)` ，重复填充固定数量的字符
  ，在 `cap_end` 处安全截断，移动 end 指针使用 size 增加 count.
- 双参数的 `unsafe_fill` 版本，不检测 `count` 超出余量
- 然后 StringBuffer 与 LocallBuffer 的 `append(count, ch)` 改为调用基类的
  `unsafe_fill` ，而不是 `fill`

可能会破坏单元测试，需同步修改。

### DONE: 20251227-213549

## TODO:2025-12-27/3 重构 LocalBuffer 上移一些方法至基类

涉及文件：include/jstring.hpp utest/t_jstring.cpp

- LocalBuffer::reserve_ex() 移到基类 BufferView::overflow() 下面
- 现增加 BufferView::full() 方法判断已写满
- 基类增加构造函数 `BufferView(char* dst, size_t size)`，assert 检验参数
- `LocalBuffer(char* dst, size_t size)` 的构造函数转发基类构造函数
- BufferView front/back 两个重载版本存在重复 assert 信息，改用 cast 转发 const 版本

重新设计单元测试用例 `jstring_invariants`:
- 改名为 `bufferview_` 前缀，测试基类方法
- 用局部数组构造 BufferView 进行一些恒等关系的方法测试
- 新加方法 full == !empty; full == (reserve_ex == 0) == (size == capacity)
- 再分析其他有意义的不变式，写入该测试用例

查找之前的涉及 fill 的用例，可能有断言 size() == capacity() ，改为断言 full() 。

### DONE: 20251227-231327

## TODO: 考虑将 reserve_ex() 改名更合适的

空参版 BufferView::reserve_ex() 意义是查询当前剩余可写空间。

最初选这个方法名，是由于 StringBuffer::reserve_ex(n) 表示检查确认还有 n 字节的
剩余空间，不足时扩容。所以当调用 reserve_ex(n) 后，再查询 reserve_ex() 必定不
小于 n 。

而检查扩容的方法名取 `reserve_ex` 是为了与 std::string::reserve 保持对应，
resever 沿用预留绝对空间的意义，reserve_ex 加后缀表示预留相对空间。

请分析 include/jstring.hpp 对该方法的定义，以及在 utest/t_jstring.cpp 的使用，
评估该命名设计的选择是否合适。尤其是空参数查询时，使用 reserved 或 remain 是否
更适合呢。或者有没其他更好的命名方案。

### DONE: 20251227~233530

Deepseek 建议保持 reserve_ex 设计；
```
如果您的目标是最大化代码自解释性（例如面向更广泛的用户群），则建议改为方案 A（remain()），否则保持当前设计即可。
```

## TODO:2025-12-28/1 LocalBuffer 类设计重构

涉及文件：include/jstring.hpp utest/t_jstring.cpp

重构方向：
- 取消 bool 模板参数，将 bool 实例化的两个类功能分别拆分到基类与子类
- 基类 BufferView 作为安全版本，增加 push_back append 等写入方法
- 子类 LocalBuffer 改名 UnsafeBuffer ，覆盖 push_back append 等写入方法直接调
  用 unsafe_ 方法；
- 基类定义常量 kUnsafeLevel = 0 ，派生类 kUnsafeLevel = 0xFF ，都可满足
  UnsafeStringConcept 了。

单元测试用例拆分，localbuffer_ 的 safe mode 段先拆出 bufv_ 缩写前缀的用例名。
unsafe mode 段拆分 ubuf_ 缩写前缀。第二参数描叙写出类名全名。

单元测试按被测试类分为三组，合并写入方法的测试用例。

### DONE: 20251228-111326

## TODO:2025-12-28/2 添加测试用例验证借用容器内存

在 include/jstring.hpp 的 BufferView 类的头注释提到借用标准容器申请的内存写入
后可用 resize 同步。这不一定对的，容器的 resize 可能添加默认字符。

请在 utest/t_jstring.cpp 添加用例验证这种用法。新用例添加在 BufferView 分组后
面。

### DONE: 20251228-122949

## TODO:2025-12-28/3 整理 BufferView 众多方法分类

在 include/jstring.hpp 的 BufferView 类的方法越来越多，许多一开始设计为在其子
类的方法也上移放到基类来了。为方便维护，希望将方法归类，采用 doxygen 的 `@{@}`
注释分组。

目前大概有以下几类方法：
- 基本的构造、析构
- 只读检查方法
- 安全写入方法
- unsafe 写入方法
- 可能还有其他方法

请仔细分析现有的所有方法，提出一种合理的归类分组方案，将同组的方法括在一起，取
个简短标题。对于简单、可自解释的方法，再精简注释，用 `///` 风格写一句话即可。

有个问题需要考虑抉择，对于安全写入方法与其对应的 unsafe 版本，是写在一起更好呢
，还是分在不同组更好。请从可读性与可维护性方面作出决策。

此外，在之前的单元测试中发现，将 BufferView 转为 std::string 需要显示
`static_cast` ，写起来略麻烦。希望再增加一个 `str()` 方法返回 `std::string` ，
这也正好与 `c_str()` 方法返回 C-Style 的字符串相对应。请将这个拟新增方法也一起
考虑进去如何分类。

不要删已有的方法，不该影响单元测试。

### DONE: 20251228-151234

## TODO:2025-12-28/4 重新设计 reserve_ex(n) 返回 bool

修改文件：include/jstring.hpp

重新约定 UnsafeStringConcept 要求 reserve_ex(n) 返回 bool 表示是否有空间再写入
n 字节。

- 剩余空间足够再写入 n 字节时返回 true
- 基类 BufferView::reserve_ex(n) 不扩容，剩余容量不足时返回 false
- UnsafeBuffer::reserve_ex(n) 假定空间够，始终返回 true
- StringBuffer::reserve_ex(n) 扩容失败时返回 false

### DONE: 20251228-213120

## TODO:2025-12-29/1 重构单元测试组织

- 将 `utest/t_usage.cpp` 单独编译为个可执行目标，utdocs，这是同步文档的示例，
  与原来的主单元测试 utwwjson 分开管理；
- ci-unit.yml 流水线增加一个运行步骤，直接执行 `utdocs --cout=silent`, 暂不关联
  手动触发输入的 `$TEST_ARGS` 的参数；
- 将 `utest/t_jstring.cpp` 文件前面几部分关于 BufferView 与 UnsafeBuffer 的测
  试用例，单独拆分到 `t_bufferview.cpp` 文件；该文件想测试的功能是
  `include/jstring.hpp` 文件。
- 更新 `utest/README.md` 文档中关于测试文件列表的说明；而 `utest/cases.md` 文
  档可以用 `make test/list` 快捷工具自动更新；
- utwwjson 目标构建时增加 `JSTRING_MAX_EXP_ALLOC_SIZE` 宏定义，其默认值是 8M
  ，在单元测试中可以设置小一些，如 1024 ，更易测试 jstring 申请内存增长策略；

再优化 `utest/t_jstring.cpp` 文件中剩下的测试用例：
- 用例名前缀 `jstring_` 缩写为 `jstr_`
- 仔细分析现有单元测试用例设计中存在的问题

我觉得这些用例不好的几点：
- StringBuffer 的一些方法在后续开发中上移到基类 BufferView 了，于是一些测试用
  例应该是针对 BufferView 基类的，而不是 StringBuffer 子类；
- 一些用例拆得太细，我想每个 `DEF_TAST` 用例的长度适中，不要太短，也不要太长，
  较长的用例再用 `DESC` 注释加 `{}` 分段也划分作用域。

所以可能需要对现有 `jstring_*` 用例作较大调整，请分析：
- 哪些用例更适配改名迁移到 `t_bufferview.cpp` 文件中；
- 哪些用例是无意义或重复的，可删除；
- 哪些用例是相似的可以合并为一个用例；
- 哪些用例中低质量测试应该修改；
- 是否需要补充新用例，有功能没有被测试覆盖；

但新增用例在本任务中只给出建议，先不实际增加；本任务的目标是精简 `DEF_TAST`
用例数，以及可以优化已有用例的测试代码。

### DONE:20251229-122441 20251229-190504

## TODO:2025-12-29/2 BufferView 基类测试用例完善

utest/t_bufferview.cpp 文件添加了几个空实现的测试用例，请根据注释完成测试代码。

### DONE:20251229-185409

## TODO:2025-12-30/1 扩展 jstring.hpp 中 StringBuffer 最大等级的特化

当 `StringBuffer<255>` 达到最大 unsafe 等级时，它具有如下特征：
- 只在构造函数时申请一次内存，假设用户能预估所需的最大容量，不再需要扩容
- `reserve_ex(n)` 直接返回 true
- 安全写入方法 `push_back` 与 `append` 等不再调用 `reserve_ex(n)`

再仔细分析一下，是否可以只修改 `reserve_ex(n)` ，其他调用者不修改的话，编译优
化 -O2 以上是否也能达到同样效果，相当于空操作。

这个特化类，应该可以实现 `UnsafeBuffer` 一样的功能，但是拥有自己的内存，避免像
后者那样与原内存所有者混乱写入的情况。

为这个特化类取别名 KString 。

在 `utest/t_jstring.cpp` 增加两个测试用例：
- `kstr_construct` 基本测试
  - KString(0) 最少申请 256 字节，255 容量
  - KString 默认申请仍是 1024
  - KString(4k) 再传个较大初始容量的参数
  - 能执行基本的写入操作
- `kstr_reach_full` 对比测试 KString 能写满，不扩容
  - KString 对比另两个类
  - JString
  - `StringBuffer<254>`
  - 逐步写入字符，KString 因为不扩容，应该能写满，full 返回 true，但测试时也应
    避免溢出，使测试程序出问题
  - 另外两个类在写入内容快满时应该会自动扩容，不可能写满，只用安全写入方法时
    full 判断不会 true 。

### DONE:20251230-103330

## TODO:2025-12-30/2 StringBuffer 使用 realloc 尝试原位扩容

大多数情况下 realloc 应该比手动 malloc+memcpy+free 高效

### DONE:20251230~103330
修改单元测试，在首次少量扩容时 8-16 发现是原位扩容；
在较大容量翻倍时不保证原位扩容，扩容前后指针不一样。

## TODO:2025-12-30/3 增加 jbuilder.hpp 组合使用 jstring.hpp

include/jstring.hpp 主要功能开发完毕，下一步的目标是要将它应用到 wwjson.hpp 的
GenericBuilder 模板类中.

- wwjson.hpp 与 jstring.hpp 仍有独立使用意义，互不依赖
- jbuilder.hpp 依赖 wwjson.hpp 与 jstring.hpp 
- wwjson.hpp 增加编译期判断 stringT 的 unsafe level 功能

为此需在 `wwjson.hpp` 文件中设计一个 `unsafe_level<stringT>` 编译期常量模板函数，
其功能为：
- 如果 stringT 有静态常量成员 kUnsafeLevel ，则返回其值
- 否则返回 0

例如如下类的 `unsafe_level` 分别是：
- std::string = 0
- BufferView = 0
- UnsafeView = 255
- JString = 4
- `StringBuffer<N>` = N

新增 `include/jbuilder.hpp` 主要包含 `wwjson.hpp` 与 `jstring.hpp` ，
作为粘合剂，定义一些 `GenericBuilder<stringT>` 的常用别名：
- Builder: 使用 JString (`StringBuffer<4>`) 作为写入目标类
- FastBuilder: 使用 KString (`StringBuffer<255>`) 作为写入目标类

新增 `utest/t_jbuilder.cpp` 测试文件，暂时先测试如下功能：
- 验证 `unsafe_level<stringT>` 萃取函数功能
- 验证 `Builder` 的基本功能，可参考 `basic_builder` 用例，再增加一项
  `AddMemberEscape` 字段，保证覆盖整数、浮点数、字符串与转义字符串。

虽然 `GenericBuilder<stringT>` 暂时未用到 `unsafe_level<stringT>` 特征作优化，
但功能应该已经满足，因为 `JString` 满足 `StringConcept`.

### DONE:20251230-165622
重要设计修改：KString 允许显式 reserve 扩容，否则有协作问题。

## TODO: wwjson.hpp 根据 unsfe level 重构 GenericBuiler

当 `unsafe_level<stringT>` 的值不小于 4 时，写入以下格式字符调用其
`unsafe_push_back` 方法：
- 逗号
- 冒号
- 引号

封装一个 `UnsafePutChar` 方法，根据 `unsafe_level` 选择调用 `push_back` 或
`unsafe_push_back` 。

## TODO: wwjson.hpp 写入浮点数优化

优化 `NumberWriter::Output` 浮点数版的正常分支，
当 `unsafe_level<stringT>` 的值不小于 4 时，直接向 stringT 末尾调用
`std::to_chars` 或 `snprintf` 写入浮点数的字符串格式：
- 先调用 `reserve_ex(n)` 预留空间
- 写完后调用 `unsafe_set_end` 移动 end 指针

这样可以减少临时 buffer 的使用。

再仔细分析一下浮点数序列化预留多少空间是足够的，有必要现在写的 256 字节那么长
吗？如果仅因为考虑 long double 才要这么长，可以拒绝支持 sizeof 大于 8 字节的浮
点数。

## TODO: wwjson.hpp 优化字符串转义方法

优化 `BasicConfig::EscapeString` 方法，
当 `unsafe_level<stringT>` 的值不小于 4 时，可避免临时 buffer 的使用，
直接向 stringT 末尾写入转义的字符，先预留两倍空间。

## TODO: wwjson.hpp 优化整数序列化

优化 `NumberWriter::Output` 整数版，当 `unsafe_level<stringT>` 的值不小于 4 时，
直接从 stringT 末尾正向写入，避免逆向写入临时 buffer 。

需要重新设计一个能正向序列化整数的合适算法。

## TODO: 性能测试

## TODO: 文档优化

## TODO: v1.2.0 封版

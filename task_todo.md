# 项目开发原始需求管理

本文档由人工维护，**AI受限编辑** ，仅允许在每个 `## TODO` 下加个子标题 `###
DONE` 关联 `task_log.md` 的完成任务ID记录。

需求ID 格式：`TODO:YYYY-MM-DD/n` ，日期/编号。日期表示头脑风暴冒出该需求想法的
日期，若一天中产出多个想法则加编号。仅有初步概念但未细化的需求暂不加日期 id ，
先快速列出 `TODO: 标题`，后面有细化内容时再赋日期 id ，转为正式需求。

---

## 上个版本 v1.1.0 需求摘要

WWJSON v1.1.0 已完成发布，主要成果包括：

### 核心功能
- ✅ 新增 JString 高性能字符串缓冲类，专为 JSON 序列化优化
- ✅ BufferView 和 UnsafeBuffer 视图类，支持借用外部内存
- ✅ KString 最大不安全等级特化，单次内存分配零扩容
- ✅ wwjson::to_json 统一转换 API，简化结构体序列化
- ✅ 多头文件架构，jbuilder.hpp 组合使用 wwjson.hpp 和 jstring.hpp

### 性能优化
- ✅ UnsafePutChar 优化格式字符写入（逗号、冒号、引号）
- ✅ UnsafeConfig 优化字符串转义，避免临时 buffer
- ✅ 浮点数序列化优化，支持直接写入 StringBuffer
- ✅ JString 比 std::string 快 6-30%，KString 快 19-30%

### 开发工具
- ✅ 示例程序：struct_to_json、estimate_size、hex_json
- ✅ CI/CD 增强单元测试和性能测试流水线
- ✅ 多头文件安装到 wwjson/ 子目录
- ✅ 完整的文档更新和在线部署

### 详细记录
- [changelog/v1.1/task_todo.md](changelog/v1.1/task_todo.md)
- [changelog/v1.1/task_log.md](changelog/v1.1/task_log.md)

---

**后续需求记录**

## TODO:2026-01-12/1 为相对性能测试增加比值断言

当前 `perf/p_*.cpp` 测试文件的相对性能测试，主要通过打印信息表示结果。

相当性能返回的是 `ratio` 是个时间比，反映比较的方法A/方法B 耗时比。
为这个值增加断言语句，一般形式是 `COUT(ratio < 1.0, true)` ，表示方法A 期望比
方法 B 更快。

把性能断言写进测试用例，避免以后续性能优化时出现较大的失误反转。

但对具体测试用例要具体分析，不一定与 1.0 比较。根据之前的测试结果先给个粗略的
参考值。

### p_nodom.cpp

- nodom_raw_vs_snprintf 理论上 snprintf 一次调用可能更快些，但实测试 RawBuilder
  还更快，先断言 COUT(ratio < 1.2, true)
- nodom_raw_vs_append COUT(ratio < 1.1, true)
- nodom_raw_vs_stream 断言 ratio < 1.0
- nodom_builder_vs_append  断言 ratio < 1.0
- nodom_builder_vs_append  断言 ratio < 1.0
- nodom_fastbuilder_vs_append 断言 ratio < 0.9

这就是说，封装的 RawBuilder 允许比直接使用 std::string 慢一些，
但优化过的 Builder 与 FastBuilder 一定要比 std::string 快。
如果将来反常失败了，那就得分析问题了。

### p_api.cpp

各种 api 的性能都显示差不多，用最基本 api 有时会稍快点，因为简单直接可能少了些
封装调用。

本来应该将 basic 放在方法B作为比较基准的，但实现中将 basic 写在方法 A 了。
所以应该取 ratio 的倒数进行断言：COUT(1.0/ratio < 1.05, true)。

相对测试框架判断 ratio 在 [0.95, 1.05] 区间时打印性能相近信息。

`api_basic_vs_localobj` 比较的 "Local Operator Method" ，链式 << 操作符涉及状态
判断，可能稍慢点。断言值或许可稍放宽些，但之前的大部分测试的误差也在 5% 以内。

### p_builder.cpp

当前，wwjson 在字符串序列化比 yyjson 快，但数字序列化慢一些。
这里是一些综合的测试用例，先断言 ratio(wwjson/yyjson) < 1.2

另外两组测试，JString/std::string 与 KString/std::string 都应该 < 1.0
但注意比较顺序，也要取倒数 1.0/ratio < 1.0

### p_string.cpp

断言 ration(wwjson/yyjson) < 1.0;
ratio(JString/std::string) < 1.0; （取倒数）
ratio(KString/std::string) < 1.0; （取倒数）

### p_number.cpp

先只能断言
ratio(JString/std::string) < 1.0; （取倒数）
ratio(KString/std::string) < 1.0; （取倒数）

wwjson/yyjson 先使用单参数的 COUT 打印： COUT(ratio < 1.0)
但不断言

将来希望能写成 COUT(ratio < 1.0, true) 断言成功

### p_design.cpp

这个文件内是一些研究性质的测试，为设计优化提供依据。
预先并没有太强的倾向性期待哪个更快。
所以不加断言语句，按原来行为只打印信息。

其他用例（多是 `DEF_TOOL` 定义）使用 TIME_TIC/TIME_TOC 测绝对时间的
也不必修改。

### make perf 快捷命令参数调

根目录 makefile 的 `make perf` 与 `make perf/log` 增加 `--cout=fail` 参数。
当这些 ratio 断言成功时不会有打印输出，仅失败时打印。

### DONE:20260112-154233

单次运行偶有浮动误差失败，尤其是默认参数 --items=1000 --loop=1000 较少
增大参数可能使结果更稳定，但运行时间长。

## TODO:2026-01-12/2 整数序列化优化框架初步

增加一个 include/itoa.hpp 文件，设计一个 IntegerWriter 类。
暂时先继承 NumberWriter ，空实现。

修改 include/jbuilder.hpp 的 UnsafeConfig 模板类，
能否限定只能由 `unsafe level > 4` 的 stringT 类才能实例化。
例如 `UnsafeConfig<std::string>` 会导致编译失败。
这样可使该类的方法实现简单点，不必将来每个方法都判断一下 unsafe level 。
现在的 EscapeString 方法希望能取消一层 if constexpre 判断，减少缩进。

utest/t_jbuilder.cpp 的测试用例 jbuilder_unsafe_level 增加一个段落测试场景，
验证 UnsafeConfig 能实例化 JString 但不能实例化 std::string 。先写出来验证有编
译错误，但注释掉。

然后增加两个 UnsafeConfig::NumberString 模板重载方法，将整数与浮点数分开写。
整数版的调用 IntegerWriter::Output，
浮点数版的暂时仍显式调用 NumberWriter::Output
但是在这之前，先调用 stringT::resver_ex 预留容量，使后面的整数/浮点数序列化不
必考虑写入目标 buffer 容量问题。

### DONE:20260112-174938

## TODO:2026-01-13/1 IntegerWriter 小整数正向序列化实现

修改文件：include/itoa.hpp

先尝试两个小整数类型的序列化，`uint8_t` 与 `uint16_t` 。
特化这两个小整数类型的 Output 函数。

uint8 Output 处理方案：
- 最大 255
- 先判断小于 100 的情况，查表写入 1 或 2 位数
- 大于 100 时，除 100 ，高位只会有一位，再写入低两 2 位

uint16 Output 处理方案：
- 最大 65535
- 先以 10000 为界，大于 10000 的已知仅有一位数
- 小于 10000 的，再以 100 为界，分两种情况处理
- 大于 100 的除 100，拆成两个 100 以内的数

提炼辅助函数，分别实现写入 2 位数与 4 位数；
还要考虑它们是高位还是低位，在低位时必定写入 2 位或 4 位数，
在高位时可能写入的位数更少些。

因此共有 4 个逻辑类似的函数，适合模板化。
增加 detail::UnsignedWriter 模板类，除了已有 stringT 目标参数，
再加两个整数模板参数：
- uint8_t DIGIT , 值可能是 2 与 4，将来再考虑 8 与 16 的大整数
- bool HIGH ，是否高位

DIGIT 参数，要求只能是 2^n 的数。

这个内部类，也有个 Output(stringT &dst, uintT value) 方法，
只用考虑正整数类型。
- 当模板参数 DIGIT 大于 2 时，除 10^(DIGIT/2) 拆成两半
- 当 HIGH 为 true 时，拆出的前一半仍为 true ，后一半为 false
- 当 HIGH 为 false 时，拆出的两半都为 false
- 可以先判断大于一半才需要拆分两半
- 当 HIGH 为 false 时，拆出的前一半是 0 也要写入 DIGIT/2 个 0

传入内部 Output 的 value ，必须小于 10^(DIGIT) 。
调试版用 assert 校验。

外层的 wwjson::IntegerWriter::Output 加个偏特化版本，处理负整数，
先写入 `-` 号，再转为整数序列化。

在 `uint32_t` 与 `uint64_t` 暂未特化时，能否调用基类的模板方法。
注意 IntegerWriter 是静态继承，没有多态继承。

如果不行，还是显式特化 `uint32_t` 与 `uint64_t` 版本，先只转调基类的
Output 方法。待后面验证这个算法方案可行时，再实现 32 位 64 大整数。

也先评估一下以上描述的算法能否提升性能：
- if 分支判断是否能减少除法次数
- 似乎隐含递归，编辑器能否实现内联优化抵消调用开销

### DONE:20260113-143559

## TODO:2026-01-13/2 对 itoa.hpp 的正向序列化全面测试与 debug

itoa.hpp 初版实现后，单元测试虽然通过，
但有个性能测试 ./pfwwjson number_int_rel 结果是 nan ，
可能表示某些值序列化结果不对。

先分析代码，看能否发现是哪里有逻辑错误。

再新建个 utest/t_itoa.cpp 单元测试文件，设计用例全面覆盖各种类型及边界情况，
找出问题并解决问题。

再新建 perf/p_itoa.cpp 性能测试文件，
测试相对性能测试派生类，比较 Builder(JString) 与 RawBuilder(std::string) 的实现。
按 int8 int16 int32 int64 写四个测试类。
构造函数随机生成 [0, max 正整数] 范围的整数，当取负不溢出时，将对应的负数也添
加到待序列数组中，否则只添加正数，因此正负比大约 2:1

新测试文件要添加到 cmake 脚本才生效。

### DONE:20260113-172020

本地 WSL 测试，uint8 性能测试显著提升，但 u16/u32 反而下降，可能递归调用的开销。
u64 沿用之前的算法，性能提升可能是 JString 的优化原因，而不是整数优化的原因

在 github CI 流水线上运行 u16/u32 序列化性能有显著提升，u16 提升幅度更大。
应该是能递归优化。

## TODO:2026-01-13/3 完善 itoa.hpp 的整数正向序列化实现

首先，Output2Digits 函数改为用 `unsafe_append` 拷两个字节，代替两次写单字符。

IntegerWriter::WriteUnsigned 方法 `uint32_t` 版，取消第一个小于 10^4 的分支，
直接按 10^8 二分，小于 10^ 时调用 `UnsignedWriter<8,true>` 即可。与 `uint16_t`
版本的入口方法一个，按最大幂划两分支。

按类似思路扩展实现 `uint64_t` 版本。按 10^16 为界二分，大于该值时分成 16+4 两
部分。但是目前 UnsignedWriter 最大只支持 10^8 ，需要扩展支持 10^16 。

另一个方案是在入口方法拆分最大 10^8 的三部分，cast 成 `uint32_t` 再调用
`UnsignedWriter<8>` 与 `UnsignedWriter<4>` 。

请分析这两个方案，在利用 UnsignedWriter 二分递归时，若传入 uint64 比 uint32 是
否有性能劣势，因为拆到后面即使 2 位数也用 uint64 表示。使用哪种方案稍好。

若 uint64 传递小值也没性能损耗，按第1方案更简洁。模板类自动处理二分递归。

### DONE: 20260114-003209

## TODO:2026-01-14/1 itoa.hpp IntegerWriter 零值优化

当前仅在对外对接口 Output 方法优先对 `0` 作特殊处理。
但在 UnsignedWriter 模板类递归中也要处理几种情况：

- 高位 HIGH=true 时，value 不可能为 0，加个调试 assert 大于 0 断言
- 低位 !HIGH 时，value 可能为 0 ，例如 30000，直接写入 DIGIT 个 0
- !HIGH 且 value > 0 时，有可能前半部分为 0，如 30003 ，
  这时也先判断小于 kHalf 可避免一次除法，前半部分写入 DIGIT/2 个 0

可以定义个常量指向 16 个 "0" ，用 usafe_append 写入连续若干个 0 。

还有个小问题，本地编译器较旧，可能没法内联递归，性能反而比 wwjson.hpp
最初的 NumberWriter 的反向写入算法慢。所以加个条件编译宏，默认未定义，
若定义了不递归宏，在 IntegerWriter::Output 入口处统一回滚调用基础版的
NumberWriter::Output 。

### DONE:20260114-093106

今天在 office WSL 环境测试，居然反转结果了，
IntegerWriter 正向的算法如期比 NumberWriter 快了。
可能昨天没有 make release 只 make build/perf ，之前的编译缓存有影响？

## TODO:2026-01-14/2 对比 itoa 优化后的 Builder 与 yyjson 整数序列化性能

首先精简 perf/p_itoa.cpp 测试用例，
itoa_int8/16/32/64 四个用例没必要单独各写一个，在 itoa_all 都覆盖了。

itoa_all 重命名为 itao_forward_write,
中文注释参数 "比较整数正向递归递归序列化与反向临时缓冲性能"
实际比较的 wwjson::Builder 与 RawBuilder 不仅有整数序列化算法的差别，
写入字符串也有差别，不过全是写整数数组，前者占比更关键。

再写一套相对性能测试，比较 wwjson::Builder(方法A) 与 yyjson(方法B) 在序列化
四种整数类型的性能场景。风格模式类似，四个派生类，一个 DEF_TAST 用例入口。

结果 resultA/resultB 都用 std::string，统一公平地从各自底层结构拷一次转回
标准字符串。

用例名建议：itoa_build_vs_yyjson "比较 wwjson::Builder 与 yyjson 整数序列化性能"

### DONE:20260114-104752

Office WSL 测试结果：int8/16 wwjson 更快，int32/64 yyjson 更快
有可能在大整数几层递归时内联优化不足？

## TODO:2026-01-14/3 perf 测试中的绝对时间测试用例分离

`perf/p_*.cpp` 子目录的性能测试文件，最初用是 `TIME_TIC/TIME_TOC` 测试绝对运行
时间的，后来都使用相对测试框架(`perf/relative_perf.h`)了。

现在觉得两套测试风格的用例混在一起编译不是很好，直接删除又过于粗暴。
所以想将它们分离出来，编译为单独的测试可执行程序。

例如，`p_builder.cpp` 若有绝对时间测试，分离出 `tic_builder.cpp` 文件。
再检查一下其他文件，`p_nubmer.cpp` 与 `p_string.cpp` 可能也有。

其他一些为了这种测试用例服务的代码或用例，也一起分离出去。
所有 `tic_*.cpp` 另外编译一个测试目标 ticwwjson。
分离出的用例，确实是 tic/toc 测时的，可以从 DEF_TOOL 改加 DEF_TAST
其他的可保留 DEF_TOOL 定义（默认不运行）。

在主要的 pfwwjson 目标中应该只剩下相对性能测试。
在剩下的相对性能测试中，再检查一些用例的方法A与方法B与测试目的写反了。

方法A 是测试对象，方法B 是参考的基准对象(base)。
在那些使用了类似 `COUT(1.0/ratio)` 语句的就是属于写反了的情况。
尝试过用 `1.0/ratio` 补丁也不太好，阅读分析结果时不友好。

所以要从根本上解决问题，把写反的用例纠正回来。
methodA 与 methodB 要对换，调用处的参数注释名称也要相应调换。

从以前修改记录看，p_api.cpp p_builder.cpp p_string.cpp p_nubmer.cpp 
这几个文件的用例都存在倒转 `1.0/ratio` 的情况。其他的最好也要检查。

这是个重构任务，提交消息类型可用 refactor.

### DONE:20260114-144313

### DONE:20260114-144313

## TODO: 设计几个最小示例研究优化后的汇编码

## TODO: 浮点数序列化算法进一步优化

浮点数序列化：
- 提升 %.g 格式化性能，对比 yyjson 的 22 倍差距
- 研究 Ryū 等快速浮点数序列化算法
- 研究其他著名开源库的实现，如 yyjson fmtlib 等

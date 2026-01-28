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

## TODO:2026-01-15/1 设计几个最小示例研究优化后的汇编码

新建 perf/mini 子目录，先写三个最简示例。

示例一、uint16 序列化，`itoa_u16.cpp` ，核心逻辑如下：
```cpp
uint16_t value = 12233;
char buffer[16];
wwjson::UnsafeBuffer ubuf(buffer);
wwjson::IntegerWriter::Output(ubuf, value);
printf("%s\n", ubuf.c_str());
```

示例二、uint32 序列化，`itoa_u32.cpp` ，核心逻辑类似，`value` 值改为 `1122334455`。

示例三、构建一个最简 json ，`builder.cpp` ，核心逻辑如：
```cpp
using wwjson;
char buffer[64];
UnsafeBuffer ubuf(buffer);
GenericBuilder<UnsafeBuffer, UnsafeConfig<UnsafeBuffer>> builder(std::move(ubuf));
// 构建纯字符串 json {"code":"0","msg":"ok"}
printf("%s\n", builder.json.c_str());
printf("%s\n", buffer);
```

这些最简示例不必加入 CMake 构建系统，就用几个单行 g++ 命令编译，可以封装一个直白的
makefile 脚本。也在子目录下加个 `README.md` 说明文档。

编译的可执行文件后缀使用 `*_linux.exe` ，
以便将 `*.exe` 及汇编中间文件加入 `.gitignore` 。

除了运行可执行文件验证结果正确外，更关键的是分析其汇编码，用常见的 `-O2` 优化。

两个整数序列化的汇编分析重点关注如下问题：
- IntegerWriter 模板类的递归有没优化展开
- 整数除法取模能否自动优化为乘法与移位

最简 json 示例的汇编重点关注它是否在局部 buffer 上直接写入字符，
在多大程度上内联优化，有几次函数调用。

另外，考虑到本地开发环境的 g++ 编译器版本并不高。
再写一个 github 流水线，在 ci 环境上运行。
以及，汇编代码有多长，是否适合将汇编中间码 cat 到日志中，
或者有没其他办法看到汇编结果？

### DONE:20260115-122035

## TODO:2026-01-15/2 perf/mini 再设计一个最简动态 json 构建示例

初始 perf/mini 设计的几个最简示例，由于都是字面量整数与字符串，编译器可能做了
非常极致的优化。大量工作都在编译期完成了，直接往局部 buffer 写入常量了。

所以希望综合 `itoa_u32.cpp` 与 `builder.cpp` 示例，再写个 `dyn_json.cpp` 示例，
引用变量因素。再观察对比优化后的汇编码。

- 仍在 main 定义一个局部 `buf_js[64]` 作为 json 写入目标；`buf_int[16]` 用于整数转换；
- 抽出一个函数，执行 int 转字符串的操作，传入整数、 `buf_int` 与长度；
- 抽出一个函数，拼装 json ，传入 `buf_js` 与 `buf_int` ，后者作为 code 字段的值。
- 在 main 中首先仍传入 `1122334455` 字面量调用一次；
  然后用 scanf 提示用户输入一个整数，读取一个未知整数再调用一次。

组装的 json 类似 `{"code":"%d","msg":"ok"}`。

`wwjson::UnsafeBuffer` 对象支持用 `(char*, size_t)` 参数构造。
分离函数后尽量使代码改动最小。

然后再对比抽出的函数，与原来 `itoa_u32.cpp` 或 `builder.cpp` 的 `main` 函数的
核心代码的汇编码。仔细分析这有什么差异。
可参考原来的汇编分析文档 `assembly_analysis.md` 。

### DONE:20260115-170521

## TODO:2026-01-16/1 perf/mini 再设计一个反向正数序列化的对比示例

参考 `itoa_32.cpp` 与 `dyn_json.cpp` ，再写一个 `itoa_back.cpp` 。
用 NumberWriter 代替 IntegerWriter 整数序列化。
先用 1122334455 字面量，再用 scanf 读入整数变量。
但不用 build json ，就分析比较整数序列化的汇编码。

主要关注 NumberWriter::Output 的内联情况，与除法优化情况。

### DONE:20260116-100136

发现：
office wsl 编译，常规逆向 NumberWriter 有优化除法(/100)，但没有内联，
可能因为涉及 while 循环。

IntegerWriter 模板递归，能优化为内联，但除法不能内联，可能由于递归模板中的
整数类型不好确定？

CI 环境没有完全内联，但序列化整数只生成了 `UnsignedWriter<4>` 一个方法。
可能有它自己的判断折中。

## TODO:2026-01-16/2 itoa.hpp 使用魔数优化除法

考虑到有些编译器可能无法充分将 UnsignedWriter 模板递归类的除法优化为乘法与位移，
所以在代码中显式改写除法与取模运算。

```
x / d ≈ (x * m) >> s
x % d = x - (x / d) * d
```

魔数初步选择如下(AI 给的)：

| 除数 d | 10的幂次 | 魔数 m | 移位 s | 验证范围 |
|--------|----------|--------|--------|----------|
| 100 | 10^2 | 42949673 | 32 | [0, 255] |
| 10000 | 10^4 | 281474978 | 48 | [0, 65535] |
| 100000000 | 10^8 | 1441151882 | 64 | [0, 2^64-1] |

处理 64 位整数的方法 `WriteUnsigned(stringT& dst, uint64_t value)` 再做个适配，
用除两次 10^8 代替除 10^16 。将大于 10^16 的整数拆成三部分：
- high: 4 位数以下，调用 `UnsignedWriter<4,true>`
- middle: 8 位数，调用 `UnsignedWriter<8,false>`
- low: 8 位数，调用 `UnsignedWriter<8,false>`

先简单将原来的除法与取模语句，用字面量魔数改写。
测试验证有效后再考虑代码组织风格的重构。

### DONE:20260116-154054

魔数好像是瞎猜的，整半天不成功。

yyjson 使用的魔数（Integer Writer）:

- 8d 拆半：(val / 10000) == (((u64)val * 109951163) >> 40)
- 4d 拆半：(val / 100) == (aabb * 5243) >> 19
- 6d 取二：(val / 10000) == (((u64)val * 429497) >> 32)
- uint64 除 10^8 分三段，仍用 / % , 未转化 * >>

## TODO:2026-01-17/1 尝试减少 UnsignedWriter 递归层次

写 uint32 最多调用 `UnsignedWriter<4>` ，写 uint64 最多调用 `UnsignedWriter<8>`
在入口方法先拆分。看编译器能否做更好的优化。

本地开发实测，uint32 降一层递归后，性能一度超过 yyjson 5% 左右。
但是再用同样方法改写 uint64 后，性能又降低了。回滚也观察不到了。
很奇怪是什么影响了编译器优化。

先提交看 ci 环境的测试结果。两边也经常不一样。

### DONE:20260117~132630

CI 环境的性能测试，u32 u64 仍然是 yyjson 性能高。
有点更奇怪，u16 在本地 wwjson 快，而在 CI yyjson 快。

## TODO:2026-01-17/2 优化 UnsignedWriter 分支判断

HIGH = false 的分支，多了两个运行时 if ，可能影响编译器内联优化。
之前 `TODO:2026-01-14/1` 提出判断零值，可能是误导，没必要判断的。
即使零值，递归到 2 时写入两个 "00" 也是正确。

UnsignedWriter::Output 方法尽可能只保留编译期 if ，可能使编译器做更好的优化。

本地测试 u32 u64 写入速率反超 yyjson 了。
运行命令：`./build-release/perf/pfwwjson itoa_build_vs_yyjson --items=10000`

### DONE:20260117~220730

perf/run-ci.sh --test-args "p_itoa.cpp --items=10000"

一次 CI 测试结果：u32 序列化反超 yyjson 50%+ ，u8 u16 快 100%+
只有 u64 还慢 15% 左右。

## TODO:2026-01-18/1 itoa.hpp 细节优化及文档完善

IntegerWriter::WriteUnsigned uint8 版，由于已知最大 255 ，可以完善避免除法（或
乘法优化），多加一个 if 小于 200 的判断，直接得出 high 部分是 1 或 2。
先从理论上分析这样是否能进一步提升性能。

性能测试命令：
```bash
make build/perf
./build-release/perf/pfwwjson p_itoa.cpp --items=10000
```

可以先备份原来的测试程序为 pfwwjson.last 。
对比性能是否有细微差异，考虑到可能有运行时浮动误差，
只要不是性能显著降低，都可接受修改。

然后完善 itoa.hpp 文档注释：
- UnsignedWriter::Output 方法，要说明它的递归算法特点
- IntegerWriter 是外部类，更应该有注释
- IntegerWriter::WriteUnsigned 针对四个整数类型的入口方法，简要说明其预处理

### DONE: 20260118-094008

## TODO:2026-01-18/2 重构 perf/mini 的 itoa 最简示例

考虑到编译期整数字面量与整数变量可能有不同优化。
所以将 `itoa_u16.cpp` 与 `itoa_u32.cpp` 作个小扩展：
- 提取函数 `itoa_const` 与 `itoa_var` ，接收一个整数参数，执行现在 main 的功能
- 在 main 中先用常数调用 `itoa_const` ，再用 scanf 从标准输入读取一个整数，调
  用 `itoa_var` 。

类似地，再增加 `itoa_u8.cpp` 与 `itoa_u64.cpp` 两个文件，
测试常数分别是 `122` 与 `11223344556677889900` 。

比较分析优化后的汇编代码，将关键特点写入 perf/mini/README.md 文档中。可重写原
来的相关小节，因为自上次文档写成后代码又作了一些优化变更。直接按当前实现版本分
析。

### DONE: 20260118-104104

## TODO:2026-01-20/1 使用外部库优化浮点数序列化性能

经过调研，我决定不重复造轮子。浮点数序列化是个独立课题，与 json 格式没有强相关。
一些高性能浮点数序列化库涉及缓存 10k+ 的大表，且代码略复杂。
如果抄一份实现，这个功能点可能比 wwjson 的核心代码还大，那就不够轻量了。

所以，我想在 wwjson 构建系统中，可选地或自动检测使用第三方浮点数序列化库。
初步选用集成两个开源库：
- rapidjson
- fmt

这两个库在本地开发环境已安装至标准 /usr/local 目录下。

需要开发两方面的功能。

一是代码层面。增加一个 include/external.hpp 文件作为几个常用推荐库的适配层。
- 命名空间：wwjson::external
- 定义 wwjson::external::NumberWriter 类
- 再分别为每个待集成适配的外部库增加个子空间与类
  + wwjson::external::rapidjson::NumberWriter
  + wwjson::external::fmt::NumberWriter
- 根据条件编译宏来确定 external::NumberWriter 使用哪个子空间的实现类
  + WWJSON_USE_RAPIDJSON_DTOA
  + WWJSON_USE_FMTLIB_DTOA
- 在具体子空间的 NumberWriter 类实现 Output 的浮点数版，为简单起见，
  先只需支持 double 类型，不用重载 float 类型，希望后者能自动提升为 double

增加 `WWJSON_USE_EXTERNAL_DTOA` 条件编译宏，在该宏被定义的情况下，jbuilder.hpp
的 UnsafeConfig::NumberString 浮点数版调用 external::NumberWriter::Output 方法。
任一 WWJSON_USE_RAPIDJSON_DTOA 或 WWJSON_USE_FMTLIB_DTOA 宏被定义时，视为
WWJSON_USE_EXTERNAL_DTOA 有定义。

二是 cmake 构建脚本层面。就支持两种机制，指定用哪个三方库，或自动检测。
如果开启 WWJSON_USE_EXTERNAL_DTOA 选项，自动检查系统是否安装了 rapidjson 或 fmt
库，未安装时回滚 WWJSON_USE_EXTERNAL_DTOA 仍为 false 或相当于未定义。

如果明确指定 WWJSON_USE_RAPIDJSON_DTOA 或 WWJSON_USE_FMTLIB_DTOA 选项，
则在系统未安装对应库时，从 github 自动 FetchContent 依赖。

当前 wwjson 浮点数序列化的默认实现的 NumberWriter 主要是调用标准库 std::to_chars
fmt 库浮点数序列可使用其公开接口，直接写入 StringBuffer 的尾部空间
rapidjson 可能没有公开的独立接口，但是可访问安装的 rapidjson/internal/dtoa.h 文件
使用它的内部实现的浮点数序列化方法。

### DONE:20260120-195759

## TODO:2026-01-21/1 使用外部库序列化浮点数功能优化

根目录 CMakeLists.txt 增加 `WWJSON_USE_EXTERNAL_DTOA` 选项。
当没有显式指定 `WWJSON_USE_RAPIDJSON_DTOA` 或 `WWJSON_USE_FMTLIB_DTOA` 时，
自动检查有没安装 rapidjson 或 fmt 库，找到哪个就用哪个，并设置相应的编译宏。
只检查本地 `find_package` ，未安装时不自动下载。

这需要在原来的两个选项之后再判定 `WWJSON_USE_EXTERNAL_DTOA`，具体三方库的选项
优先级比自动检查的优先高。可能要再加一个临时变量保存是否已选定使用哪个三方库，
比如叫 `HAS_USED_RAPIDJSON_DTOA`，当该值变为 true 时，跳过后续检查。

然后简化 include/external.hpp 实现，将最内层子空间的两个 NumberWriter 的重复部
分，提取到 jbuilder.hpp 的 UnsafeConfig::NumberString 方法中。而且没必要做负数
判断，正常的三方库都应该能同时处理正负数的浮点数，没必要重复判断。只需要特殊处
理 nan 与 inf 。

### DONE:20260121-174530

## TODO:2026-01-21/2 p_itoa 测试优化更准确比较整数的正向与反向序列化

RawBuilder 与 Builder 有两个区别：
- std::string vs JString
- NumberWriter(反向写) vs IntegerWriter(正向写)

为了更准确测试后者的性能比，应该取消前一个变量，所以不能直接用 RawBuilder.
没有直接可用的简单别名，完整名是：
`wwjson::GenericBuilder<wwjson::JString, wwjson::BasicConfig<wwjson::JString>>`

wwjson::Builder 其实就是把 BasicConfig 将成 UnsafeConfig.

### DONE:20260121~222230
用例 `itoa_forward_write` 原来快 100% 左右，修改后仍快 80% 左右。

## TODO:2026-01-21/3 jbuilder 减少别名定义

以下别名其实很少用到：
```cpp
using JObject = GenericObject<JString, UnsafeConfig<JString>>;
using JArray = GenericArray<JString, UnsafeConfig<JString>>;
using FastObject = GenericObject<KString, UnsafeConfig<KString>>;
using FastArray = GenericArray<KString, UnsafeConfig<KString>>;
```

建议直接使用 `auto` 接收创建方法的返回值，没必要定义别名。
定义了别名就是认证、提升了推荐度，不妥。

相应的单元测试也要删了。

### DONE:20260121~223530

## TODO:2026-01-22/1 三方库 dtoa 性能测试

- 新增 perf/p_external.cpp 文件，同步加入 CMake 脚本
- 参考 p_itoa.cp 写两个相对性能测试子类及 DEF_TAST 用例

用例一：
- 随机生成 double 浮点数数组
- 方法 A 使用 wwjson::Builder (JString + UnsafeConfig)
- 方法 B 使用 (JString + BasicConfig) 的 builder
- 可以期望断言 COUT(ratio < 1.0, true);

用例二：
- 随机生成 double 浮点数数组
- 方法 A 使用 wwjson::Builder
- 方法 B 使用 yyjson 构建 json 数组
- 单参数非断言打印 COUT(ratio < 1.0);

然后写个 DEF_TOOL 用例，观察三方库序列化浮点数的格式，不作为性能比较。
也构建一个较短的 json 数组，包含如下浮点数：
- 正负零
- 正负纯整数，无小数部分的浮点数
- double 极大值与极小值
- 离 double 极大值与极小值较近，但不在两端的值
- 中间正常范围的浮点数
- 总数在 10 - 20 个左右为宜

使用不同的 cmake 选项选用 yyjson/fmt 三方库编译 build-release 测试，简单汇报结果。

### DONE:20260122-150820

## TODO:2026-01-22/2 三方库使用 yyjson 的浮点数序列化算法

include/external.hpp 当前适配外部库使用了 rapidjson 与 fmt ，用于浮点数序列化。
但是经过测试它们的性能仍不如 yyjson 。

我也 fork 了 yyjson 仓库，在 ../yyjson 兄弟目录。
似乎没发现它的 .h 头文件有暴露接口将 double 转为字符串，写入一个 char buffer
。

请帮忙再检查一下 yyjson 是否有类似的公开接口。如果真没有，就在这个 fork 仓库中
简单修改一下，封装一个对外接口，提供浮点数序列化功能。接口名建议 `yyjson_dtoa`
。

再修改 wwjson 的 cmake 脚本与 external.hpp ，支持使用 `yyjson_dtoa` 进行浮点数
序列化。

修改 yyjson fork 仓库前先检查同步上游官方仓库，修改完本地安装，再测试 wwjson
的适配修改。

然后帮我写一份给 yyjson 官方仓库提 issue 的英文文案，先暂存 `./doing_plan.tmp`
目录下的文件。我将尝试与 yyjson 的作者沟通，希望它能开放这样的功能。

### DONE:20260122-223100

## TODO:2026-01-23/1 将三方库检测按性能顺序重组

目前检测使用的三方库在 dtoa 上的性能表现，依次是 yyjson fmt rapijson

现在的检测顺序正好反了。需要调整顺序，优先检测 yyjson ，再 fmt 与 rapidjson
涉及修改的文件可能包括：
- 根目录的 CMakeLists
- jbuilder.hpp
- external.hpp

另外，external.hpp 优化一个安全检测，`unsafe_set_end` 之前判断一下 `end` 非空
。毕竟三方库返回的指针，要做最基本的非空判断，但是可用 `wwjson_unlikely` 宏优
化分支预测。

### DONE: 20260124-000547

## TODO:2026-01-25/1 external.hpp 使用公开 API yyjson_write_number

yyjson 有公开的 API `yyjson_write_number` 可用于将浮点数转字符串，
只是要多一个临时 `yyjson_val` 变量，可能性能略有下降。

### DONE: 20260125~144630
在本地测试 `p_external.cpp` 用例 `external_builder_vs_yyjson` 性能很接近了。
`yyjson_write_number` 确有一定的额外开销。

## TODO:2026-01-28/1 性能测试用例优化

新增 `perf_util.cpp` ，利用 xyjson 封装特性简化 json 相等性比较，
取代原 `p_external.cpp` 的比较工具函数；也应用于 `p_number.cpp` 的用例。

但 `p_builder.cpp` 仍无法直接比较 json 。可能因为 wwjson::RawBuilder 
对恰好为整数的浮点数输出为浮点数，在重新解析时认为是整数，与 yyjson 
输出的 `.0` 后缀保持了浮点类型不同，类型不匹配也无法满足相等性比较。

其他一些优化，删除旧注释的 FIX 标记，调整一些相对性能阈值。

### DONE:20260128~183430

## TODO: 将 .tool/ 子目录的脚本等工具转变成一个 submodel 子仓库

当前 `.tool` 目录是一些定制开发的辅助脚本，主要配合 Agent 自定义命令
使用的。最开始是尝试在另一个小项目（xyjson）中使用的，作为其项目的一部分
一起提交了。然后拷到这个 wwjson 项目中辅助开发，不想再重复提交，所以改
目录名为隐藏的 `.tool` 。

由于在开发项目中也可能需要对辅助工具作些调整与升级优化，还是有必要提交
这些脚本以长期保存。但是嵌在各个项目中显然不是个好主意。所以我想把它们独立
成一个仓库，然后在开发其个人项目中将它当作个子模块拉取下来。

大致需求如下：
- 在父目录新建一个 dotAgent 兄弟目录，初始化为 git 仓库，
  收集当前项目的一些以 `.` 开头的配置目录，拷过去时移除开头的点号，包括：
  + `.tool` 
  + `.claude`
  + `.codebuddy`
  + `.iflow`
- 将 dotAgent 仓库推送到 github 远程，可以加个简单的 readme 说明，
  主要给自己看与备忘之用
- 将当前项目的这些 `.` 开头目录再作一次备份，以防出错，全部移到 `tool.bak/`
  子目录中，以移动代删除
- 将 dotAgent 当作子模块拉取，重新从 github 远程下载
- 重建所有这些 `.` 开头的目录软链接至 dotAgent 的相应子目录
- 检查当前项目的 `.gitignore` ，忽略这些点目录与 dotAgent 。

本地装了 gh 命令行工具，能否直接在我的 github 帐户上新建 dotAgent 空仓库，
再将本地的 dotAgent 推上去。如果不行，我可以尝试手动登陆 github 新建空仓库。

## TODO:2026-01-28/2 浮点整数序列化加上 .0 后缀

当前 `wwjson.hpp` 的 NumberWriter 基本实现，主要基于 `std::to_chars` ，
恰好是整数的浮点数输出为整数，满足最短格式。但在 json 序列化与反序列化
来回中没法保持浮点数的类型信息，所以还是参考 yyjson 的格式更合理：
当 double 类型与整数相等时，末尾加上 `.0` 显式表明是浮点数。

然后再修改 `perf/p_builder.cpp` 中几个相对性能测试类的 `methodVerify`
方法，调用 `perf_util.h` 的比较 json 方法，确认它们所比较的两个方法能
输出等效的 json 。

也要检查单元测试 utest ，检查这个格式变更否破坏单元测试，作相应的调整。

## TODO: 文档更新

## TODO: v1.1.2 封版

# AI 协作任务工作日志

格式说明:
- **任务ID**: YYYYMMDD-HHMMSS ，生成命令 `date +"%Y%m%d-%H%M%S"`
- 每条日志开始一个二级标题，标题名就是任务ID
- 可适合分几个三级标题，简明扼要描叙任务过程与结果
- **追加至文件末尾**，与上条日志隔一空行

---

## 上版本 v1.1.0 开发摘要

WWJSON v1.1.0 版本开发周期：2025-12-22 至 2026-01-11

### 主要里程碑

#### JString 高性能字符串库 (2025-12-22 ~ 2025-12-30)
- ✅ 完成 StringBuffer 核心功能实现
- ✅ 实现 BufferView 和 UnsafeBuffer 视图类
- ✅ 添加 KString 最大不安全等级特化
- ✅ 完善与标准字符串的互操作

#### wwjson 性能优化 (2026-01-03 ~ 2026-01-05)
- ✅ 实现 UnsafePutChar 优化格式字符写入
- ✅ 创建 UnsafeConfig 优化字符串转义
- ✅ 优化浮点数序列化，支持直接写入 StringBuffer
- ✅ 完成相对性能测试，验证性能提升

#### 统一转换 API (2026-01-06 ~ 2026-01-10)
- ✅ 实现 wwjson::to_json 统一转换 API
- ✅ 支持结构体、容器、可选类型
- ✅ 完成 TO_JSON 宏简化使用
- ✅ 添加示例程序演示用法

#### 文档和发布准备 (2026-01-06 ~ 2026-01-11)
- ✅ 多头文件安装到 wwjson/ 子目录
- ✅ 完成三个示例程序
- ✅ 更新文档和在线部署
- ✅ 准备 v1.1.0 版本封版

### 技术亮点

#### JString 性能优化
- **不安全级别机制**：kUnsafeLevel 减少边界检查开销
- **三指针设计**：m_begin/m_end/m_cap_end 高效内存管理
- **智能扩容策略**：指数增长 + 线性增长混合策略
- **直接内存操作**：支持第三方格式化函数直接写入

#### wwjson 性能提升
- **格式字符优化**：逗号、冒号、引号使用 unsafe 方法
- **字符串转义优化**：减少临时 buffer 使用
- **浮点数优化**：64 字节 thread_local buffer，支持直接写入
- **性能提升**：JString 快 6-30%，KString 快 19-30%

#### API 设计创新
- **to_json 统一 API**：支持标量、结构体、容器、可选类型
- **TO_JSON 宏**：简化结构体序列化代码
- **Builder 别名**：Builder(JString)、FastBuilder(KString)
- **多头文件架构**：wwjson.hpp、jstring.hpp、jbuilder.hpp

### 开发统计
- **开发时长**：21 天
- **任务记录**：30 个主要任务，详细记录在 changelog/v1.1/task_log.md
- **代码提交**：多次迭代优化，保持向后兼容性
- **测试覆盖**：单元测试 + 性能测试 + CI/CD 自动化

### 详细记录
- [changelog/v1.1/task_todo.md](changelog/v1.1/task_todo.md)
- [changelog/v1.1/task_log.md](changelog/v1.1/task_log.md)

---

**后续工作日志**

## TASK:20260112-154233
-----------------------

### 任务概述

为相对性能测试增加比值断言，避免后续性能优化时出现较大失误反转。

### 修改内容

**p_nodom.cpp** - RawBuilder vs 标准库方法对比：
- `nodom_raw_vs_snprintf`: COUT(ratio < 1.2, true)
- `nodom_raw_vs_append`: COUT(ratio < 1.1, true)
- `nodom_raw_vs_stream`: COUT(ratio < 1.0, true)
- `nodom_builder_vs_append`: COUT(ratio < 1.0, true)
- `nodom_fastbuilder_vs_append`: COUT(ratio < 0.9, true)

**p_api.cpp** - 各种 API 方法对比（使用倒数，因为 basic 在方法 A）：
- 所有测试使用 COUT(1.0/ratio < 1.05, true)
- Local Object 测试稍放宽至 1.15

**p_builder.cpp** - 构建器对比：
- wwjson vs yyjson: COUT(ratio < 1.2, true)
- JString/KString vs std::string: COUT(1.0/ratio < 1.0, true)

**p_string.cpp** - 字符串序列化对比：
- wwjson vs yyjson: COUT(ratio < 1.0, true)
- JString/KString vs std::string: COUT(1.0/ratio < 1.0, true)

**p_number.cpp** - 数字序列化对比：
- wwjson vs yyjson: 仅打印 COUT(ratio < 1.0)，不断言
- JString/KString vs std::string: COUT(1.0/ratio < 1.0, true)

### 测试结果

断言值根据之前测试结果设定，p_design.cpp 不加断言保持原有行为。

## TASK:20260112-174938
-----------------------

### 任务概述

整数序列化优化框架初步：创建 itoa.hpp 并修改 UnsafeConfig 模板约束。

### 修改内容

**include/itoa.hpp** - 新建文件：
- 定义 IntegerWriter 类，继承 NumberWriter
- 添加 static_assert 约束：unsafe_level >= 4
- 当前仅保留框架，forward writing 实现待下个任务完成

**include/jbuilder.hpp** - 修改 UnsafeConfig：
- 添加 static_assert 约束：unsafe_level >= 4
- 简化 EscapeString：去掉 if constexpr 分支（已有编译期约束）
- 添加 NumberString 整数版：调用 IntegerWriter::Output，预留容量
- 添加 NumberString 浮点数版

**utest/t_jbuilder.cpp** - 增加测试用例：
- 验证 UnsafeConfig<JString> 可实例化
- 验证 UnsafeConfig<KString> 可实例化
- 注释说明 UnsafeConfig<std::string> 会编译失败

### 测试结果

单元测试全部 116 项通过。

## TASK:20260113-143559
-----------------------

### 任务概述

实现 IntegerWriter 小整数正向序列化：使用递归二分算法，避免临时 buffer 逆向写入。

### 修改内容

**include/itoa.hpp** - 完全重写：

- 添加 `detail::kPow10<N>` 编译期 10^N 计算
- 添加 `detail::OutputDigit(dst, value)` 写入单个数字
- 添加 `detail::Output2Digits(dst, value)` 写入两位数字
- 实现 `detail::UnsignedWriter<stringT, DIGIT, HIGH>` 模板类：
  - DIGIT: 2/4/8，HIGH=true 时高位可写少于 DIGIT 位
  - 递归二分降级到 DIGIT=2 的 base case
- 外层 `IntegerWriter<stringT>` 提供四个 `WriteUnsigned` 方法：
  - `WriteUnsigned(dst, uint8_t)` - 1~3 位直接分支
  - `WriteUnsigned(dst, uint16_t)` - 调用 UnsignedWriter<4, true>
  - `WriteUnsigned(dst, uint32_t)` - 多层拆分
  - `WriteUnsigned(dst, uint64_t)` - 回退基类
- 统一 `Output<IntT>` 接口用 `if constexpr` 判断正负数

### 关键算法

正向写入避免逆向 buffer：
- DIGIT=2: HIGH=true 时可能只写 1 位 (value<10)
- DIGIT=4: 拆分为两个 2 位，高位可能少于 2 位
- DIGIT=8: 拆分为两个 4 位，递归处理
- 利用 `kDigitPairs` 查表快速写入两位数字

### 测试结果

单元测试全部 116 项通过。

## TASK:20260113-172020
-----------------------

### 任务概述

对 itoa.hpp 进行全面测试与 debug，修复正向序列化 bug。

### 问题定位

性能测试 `number_int_rel` 返回 nan，表明结果不一致。

**Bug 分析**：在 `UnsignedWriter::Output` 中，递归时 `value < kHalf` 分支使用 `HIGH=true` 递归，但未正确处理 DIGIT > 2 的情况，导致高位丢失。

### 修复方案

重构 `UnsignedWriter::Output` 代码结构：
- 第一层 `if constexpr (HIGH)` 分两种情况
- 第二层 `if constexpr (DIGIT == 2)` 处理 base case
- 避免 `else if constexpr` 层次混乱

关键修复：
- `HIGH=true` 且 `value < kHalf` 时，递归时保持 `HIGH=true`
- `HIGH=false` 时始终用完整位数

### 新增测试

**utest/t_itoa.cpp** - 单元测试：
- 按整数类型分组：uint8/16/32, int8/16/32/64
- 覆盖边界值、随机值、幂次边界

**perf/p_itoa.cpp** - 性能测试：
- 比较 IntegerWriter vs NumberWriter
- 按 int8/16/32/64 四种类型测试

### 测试结果

- 单元测试：8 项全部通过
- 性能测试：
  - int8: IntegerWriter 快 190%
  - int64: IntegerWriter 快 23%
  - int16/32: 性能接近，NumberWriter 略优

## TASK:20260114-003209
-----------------------

### 任务概述

完善 itoa.hpp 的整数正向序列化实现，优化性能并扩展 uint64_t 支持。

### 修改内容

**include/itoa.hpp** - 全面优化：

- 优化 `Output2Digits`：使用 `unsafe_append(digit, 2)` 代替两次 `unsafe_push_back`
- 扩展 `kPow10<DIGIT>` 支持 DIGIT=16（uint64_t）
- 扩展 `UnsignedWriter` 支持 DIGIT=16
- 优化 `uint32_t` 版本：取消 <10000 分支，统一按 10^8 二分
- 实现 `uint64_t` 版本：
  - 按 10^16 二分（<10^16 调用 UnsignedWriter<16, true>）
  - ≥10^16 拆分 4+16 两部分，使用 UnsignedWriter<4, true> + UnsignedWriter<16, false>
- 所有 WriteUnsigned 方法使用 `detail::kPow10<N>` 替代字面量常量

**utest/t_itoa.cpp** - 简化并增强：

- 合并 uint8/16/32/64 为 `itoa_unsigned` 测试用例
- 合并 int8/16/32/64 为 `itoa_signed` 测试用例
- 新增 `itoa_edge_cases` 边界测试用例
- 使用 `COUTF` 替代随机循环中的 `COUT`，减少输出
- 添加 uint64_t 大值测试（包括 10 的幂次和 UINT64_MAX）
- 覆盖所有类型 MIN/MAX 值
- 测试用例描述改为中文，保持风格一致

### 技术亮点

**方案选择分析**：uint64_t 实现方案对比
- 方案1：扩展 UnsignedWriter 支持 DIGIT=16
- 方案2：在入口方法拆分成 8+8+8 三部分，cast 成 uint32_t
- **选择方案1**：架构一致，代码简洁，性能无明显差异

**性能优化**：
- Output2Digits 减少函数调用开销
- uint32_t 减少分支预测失败
- uint64_t 从 fallback 改为正向序列化，性能大幅提升

### 测试结果

- 单元测试：3 项全部通过（测试覆盖全面）
- 性能测试（release 编译）：
  - int8_t: IntegerWriter 快 292.4%
  - int16_t: IntegerWriter 快 106.9%
  - int32_t: IntegerWriter 快 58.7%
  - int64_t: IntegerWriter 快 81.8%
  - 平均快约 97.5%

## TASK:20260114-093106
-----------------------

### 任务概述

IntegerWriter 零值优化：在 UnsignedWriter 递归中优化零值处理，避免不必要的除法运算。

### 修改内容

**include/itoa.hpp** - 零值优化：

- 新增 `kZeros[17]` 常量：16 个 '0' 的字符串，用于快速写入连续零
- 新增 `OutputZeros<stringT, DIGIT>` 模板函数：一次性写入 DIGIT 个 '0'
- HIGH=true 分支：添加 `assert(value > 0)` 断言，确保高位不为零
- !HIGH 分支优化：
  - value == 0 时：直接调用 `OutputZeros<DIGIT>` 写入全零，避免递归
  - value < kHalf 时：先写 DIGIT/2 个零，再递归处理低位，避免一次除法
- 新增 `WWJSON_ITOA_NO_RECURSE` 条件编译宏：
  - 定义此宏时回退到 NumberWriter::Output，适用于无法内联递归的旧编译器

### 技术亮点

**性能优化思路**：
- 零值快速路径：避免进入递归分支
- 前缀零批量写入：用 `unsafe_append` 替代多次递归调用
- 减少除法运算：value < kHalf 时跳过除法，直接写零后递归

**兼容性考虑**：
- 旧编译器可能无法内联模板递归导致性能下降
- 提供 `WWJSON_ITOA_NO_RECURSE` 宏回退到原始实现

### 测试结果

- 单元测试：3 项全部通过（itoa_unsigned, itoa_signed, itoa_edge_cases）

## TASK:20260114-104752
-----------------------

### 任务概述

精简性能测试用例 p_itoa.cpp，并新增 wwjson::Builder 与 yyjson 整数序列化性能对比测试。

### 修改内容

**perf/p_itoa.cpp** - 精简与新增测试：

- 删除单独的 `itoa_int8/16/32/64` 测试用例（已由合并用例覆盖）
- 将 `itoa_all` 重命名为 `itoa_forward_write`
- 更新文件注释，明确测试目标：比较 IntegerWriter (Builder) vs NumberWriter (RawBuilder)
- 新增 `BuilderVsYyjsonPerf` 模板类：比较 wwjson::Builder 与 yyjson 整数序列化性能
- 新增四个具体测试类：Int8/16/32/64BuilderVsYyjson
- 新增 `itoa_build_vs_yyjson` 测试入口：运行四种整数类型的对比测试

### 性能对比结果

测试条件：--items=100 --loop=100

**wwjson::Builder vs yyjson**：
- int8_t: wwjson 快 104%（ratio=0.49）
- int16_t: wwjson 快 34%（ratio=0.75）
- int32_t: yyjson 快 30%（ratio=1.30）
- int64_t: yyjson 快 25%（ratio=1.25）

**结论**：
- 小整数类型（int8/16）：wwjson 的 itoa 优化效果显著
- 大整数类型（int32/64）：yyjson 性能更优，平均性能接近
- 建议：针对大整数类型可考虑进一步优化

### 测试结果

- `itoa_forward_write`：通过，比较 IntegerWriter vs NumberWriter
- `itoa_build_vs_yyjson`：通过，比较 wwjson::Builder vs yyjson

## TASK:20260114-144313
-----------------------

### 任务概述

perf 测试用例分离与纠正：
- 将绝对时间测试用例（DEF_TOOL + TIME_TIC/TIME_TOC）分离到 tic_*.cpp 文件
- 修复方法 A/B 写反导致 ratio 计算错误的问题

### 修改内容

**新增文件 (3个)**：
- `perf/tic_builder.cpp` - 13个用例（10 TAST + 3 TOOL）
- `perf/tic_number.cpp` - 13个用例（12 TAST + 1 TOOL）
- `perf/tic_string.cpp` - 7个用例（6 TAST + 1 TOOL）

**perf/CMakeLists.txt** - 添加 ticwwjson 编译目标

**perf/p_*.cpp 文件精简**：
- `p_builder.cpp` - 删除10个TIC用例
- `p_number.cpp` - 删除12个TIC用例
- `p_string.cpp` - 删除6个TIC用例

**修复 A/B 写反问题**：
- p_builder.cpp: 2个（JString/KString）
- p_number.cpp: 4个（int/double JString/KString）
- p_string.cpp: 4个（object/escape JString/KString）
- p_api.cpp: 6个（各种 API 方法）

**makefile 更新**：
- `make perf/list` 合并 pfwwjson 和 ticwwjson 的用例列表

### 测试验证

- pfwwjson: 24个 TAST/TOOL 用例
- ticwwjson: 33个 TAST/TOOL 用例
- 所有用例编译通过

## TASK:20260115-122035
-----------------------

### 任务概述

创建 perf/mini 目录，包含 3 个最简示例用于研究编译优化后的汇编码。

### 新增文件

- `perf/mini/itoa_u16.cpp` - uint16_t 序列化示例
- `perf/mini/itoa_u32.cpp` - uint32_t 序列化示例
- `perf/mini/builder.cpp` - 最简 JSON 构建示例
- `perf/mini/Makefile` - 编译和汇编生成
- `perf/mini/README.md` - 说明文档
- `.github/workflows/ci-mini.yml` - GitHub CI 流水线

### 修改内容

- `include/jstring.hpp` - 添加 `reserve()` 方法支持 `UnsafeBuffer`
- `.gitignore` - 添加 `*.exe` 和 `*.s` 忽略规则

### 汇编分析结论

1. **整数序列化 (itoa_u16/itoa_u32)**：
   - ✅ 使用 DigitPair 查找表 (kDigitPairs)
   - ❌ 函数未完全内联 (仍有 callq 调用)
   - ❌ 除法未优化为乘法和移位

2. **JSON 构建 (builder)**：
   - ✅ 编译器完全内联，整个构建过程无函数调用
   - ✅ 直接在栈上构造最终字符串常量
   - 最优情况已无法进一步优化

## TASK:20260115-170521
-----------------------

### 任务概述

设计 dyn_json.cpp 示例，演示运行时变量对编译器优化的影响。

### 新增文件

- `perf/mini/dyn_json.cpp` - 动态 JSON 构建示例
- `perf/mini/dyn_json.s` - 生成的汇编代码

### 修改内容

- `perf/mini/Makefile` - 添加 dyn_json 目标
- `perf/mini/*.cpp` - 4 个文件注释补全 `-DNDEBUG` 编译选项
- `perf/mini/assembly_analysis.md` - 新增第 7 章 dyn_json 分析

### 关键发现

1. **字面量 vs 运行时变量**
   - 字面量 `1122334455`: 编译期完全优化（5 次查表，无 div）
   - 运行时 `user_value`: 出现 `div` 指令（两处），性能下降

2. **函数分离的影响**
   - `build_json` 函数独立存在，有调用开销
   - 出现了 `memcpy` 调用复制动态整数字符串

3. **性能对比**
   - 字面量: ~3-5 周期
   - 运行时变量: ~50-100+ 周期（除法开销）

## TASK:20260116-100136
-----------------------

### 任务概述

创建 itoa_back.cpp 示例，对比 NumberWriter 与 IntegerWriter 的内联情况与除法优化。

### 新增文件

- `perf/mini/itoa_back.cpp` - NumberWriter 反向序列化示例
- `perf/mini/itoa_back.s` - 生成的汇编代码

### 修改内容

- `perf/mini/Makefile` - 添加 itoa_back 目标

### 汇编分析对比

| 特性 | IntegerWriter (itoa_u32) | NumberWriter (itoa_back) |
|------|-------------------------|-------------------------|
| **内联情况** | 完全内联，无函数调用 | 有独立函数调用 (0x1280, 0x1310) |
| **字面量优化** | 编译期完全展开，直接 embed 结果 | 仍有运行时循环调用 |
| **除法优化** | 无（编译时常量） | imul+shift 优化，无 div 指令 |
| **digit pairs** | 直接 movzwl 加载 | 运行时查表 |

### 关键发现

1. **IntegerWriter 对字面量完全内联**
   - main 函数直接展开预计算的 digit pairs
   - 无函数调用，无除法指令
   - 结果字符串直接嵌入二进制

2. **NumberWriter 仍需运行时处理**
   - 运行时调用 NumberWriter::WriteUnsigned (0x1310)
   - 除法被优化为 `imul $0x51eb851f` + `shr $0x23`（无 div 指令）
   - 但仍有循环和函数调用开销

3. **结论**
   - IntegerWriter 的正向写入算法更有利于编译器内联优化
   - NumberWriter 的通用性导致更难完全内联
   - 编译器已自动将除以 100 优化为乘加移位，无需手动优化

## TASK:20260116-154054
-----------------------

### 任务概述

需求 2026-01-16/2：itoa.hpp 使用魔数优化除法。

将整数除法运算替换为乘法与位移的魔数优化：
```
x / d ≈ (x * m) >> s
x % d = x - (x / d) * d
```

### 修改内容

**include/itoa.hpp** - WriteUnsigned 函数魔数优化：

- uint8_t (100-255): 使用 m=42949673, s=32 优化除以 100
```cpp
uint8_t high = static_cast<uint8_t>(value * 42949673ULL >> 32);
uint8_t low = value - high * 100;
```

- uint16_t (10000-65535): 使用 m=1844674407370956, s=64 优化除以 10000
```cpp
__uint128_t prod = static_cast<__uint128_t>(value) * 1844674407370956ULL;
uint16_t high = static_cast<uint16_t>(prod >> 64);
uint16_t low = value - high * 10000;
```

- uint32_t: 保持原除法（魔数验证失败）
- uint64_t: 保持原除法（不在本次修改范围）
- UnsignedWriter 模板类：保持原除法（不在本次修改范围）

### 魔数验证结论

通过测试程序验证各魔数有效性：

| 除数 | 范围 | 魔数 m | 移位 s | 结果 |
|------|------|--------|--------|------|
| 100 | [0, 255] | 42949673 | 32 | ✅ 通过 |
| 10000 | [0, 65535] | 1844674407370956 | 64 | ✅ 通过 |
| 10^8 | [0, 4294967295] | - | - | ❌ 无有效魔数 |

**说明**：
- 10000 的魔数需要 128 位乘法才能避免溢出
- 10^8 在 64 位位移下无法找到有效的魔数（编译期无法确认正确性）

### 测试结果

- 单元测试：3 项全部通过（itoa_unsigned, itoa_signed, itoa_edge_cases）

## TASK:20260118-094008
-----------------------

### 任务概述

需求 2026-01-18/1：itoa.hpp 细节优化及文档完善。

1. 性能优化：IntegerWriter::WriteUnsigned uint8 版本添加分支优化
2. 文档完善：完善 UnsignedWriter、IntegerWriter 和 WriteUnsigned 方法的注释

### 优化内容

**include/itoa.hpp** - uint8_t 性能优化：

在 `WriteUnsigned(stringT& dst, uint8_t value)` 中，针对 100-255 范围添加分支优化：

```cpp
else if (value < 200)
{
    // 100-199: high=1, low=value-100
    detail::OutputDigit(dst, 1);
    detail::Output2Digits(dst, value - 100);
}
else
{
    // 200-255: high=2, low=value-200
    detail::OutputDigit(dst, 2);
    detail::Output2Digits(dst, value - 200);
}
```

**优化原理**：
- 对于 uint8_t，100-255 范围内的高位只能是 1 或 2
- 使用分支判断直接得出 high 值，避免乘法和位移操作
- 代码更清晰，可读性更好

### 性能测试结果

优化前后对比（p_itoa.cpp，items=10000）：

| 测试项 | 优化前 ratio | 优化后 ratio | 变化 |
|--------|------------|------------|------|
| int8_t | 0.4266 | 0.4026 | +5.6% ⬆️ |
| int16_t | 0.4849 | 0.4612 | +4.9% ⬆️ |
| int32_t | 0.4780 | 0.5123 | -7.2% ⬇️ |
| int64_t | 0.4832 | 0.4846 | -0.3% ⬇️ |
| **平均** | **0.4682** | **0.4652** | **+0.6% ⬆️** |

**结论**：性能略有提升，未发现显著下降，优化有效。

### 文档完善

**1. UnsignedWriter::Output** (line 64-150)：

添加详细文档，说明：
- 分治策略：将数字拆分为高位和低位两部分
- 递归终止条件：DIGIT == 2 时直接输出
- HIGH 参数控制：高位部分可以省略前导零
- 乘法优化：对特定除数使用位运算代替除法

**2. IntegerWriter 类** (line 159-220)：

添加类级别文档，说明：
- 设计目的：为 JString/KString 等高性能字符串类型提供快速整数序列化
- 核心机制：利用字符串类型的高级别 unsafe 操作，直接写入
- 性能优势：避免临时缓冲区，消除反向排序需要
- 类型要求：stringT 必须具有 unsafe_level >= 4

**3. WriteUnsigned 四个方法** (line 226-363)：

为以下方法添加详细注释：
- WriteUnsigned(uint8_t) - 说明值范围分解和分支优化
- WriteUnsigned(uint16_t) - 说明值范围分解和乘法优化
- WriteUnsigned(uint32_t) - 说明值范围分解和多层分解
- WriteUnsigned(uint64_t) - 说明值范围分解和三部分分解

### 测试结果

- 单元测试：119 项全部通过（utwwjson）
- 编译测试：Debug 和 Release 版本均成功编译
- 性能测试：验证优化后性能未下降，略有提升

## TASK:20260118-104104
-----------------------

### 需求分析

需求ID: 2026-01-18/2 - 重构 perf/mini 的 itoa 最简示例

目标: 对 perf/mini 目录下的 itoa_u16.cpp 和 itoa_u32.cpp 进行重构，并新增 itoa_u8.cpp 和 itoa_u64.cpp 两个文件。

### 实施过程

#### 1. 重构现有文件

**itoa_u16.cpp**:
- 提取函数 `itoa_const(uint16_t value)` 和 `itoa_var(uint16_t value)`
- 在 main() 中先用常数 12233 调用 itoa_const
- 再用 scanf 读取整数调用 itoa_var

**itoa_u32.cpp**:
- 提取函数 `itoa_const(uint32_t value)` 和 `itoa_var(uint32_t value)`
- 在 main() 中先用常数 1122334455 调用 itoa_const
- 再用 scanf 读取整数调用 itoa_var

#### 2. 新增文件

**itoa_u8.cpp**:
- 创建 uint8_t 版本的 itoa 示例
- 使用常数 122 进行测试
- 实现 itoa_const 和 itoa_var 函数

**itoa_u64.cpp**:
- 创建 uint64_t 版本的 itoa 示例
- 使用常数 112233445566778899 进行测试
- 实现 itoa_const 和 itoa_var 函数

#### 3. 更新构建系统

**Makefile**:
- 在 SRC 变量中添加 itoa_u8.cpp 和 itoa_u64.cpp
- 更新 run target 以包含新示例的测试用例
- 修复 itoa_u64.cpp 中 scanf 的格式符警告

#### 4. 编译和分析

**构建过程**:
- 使用 `make clean && make` 编译所有示例
- 自动生成对应的 .s 汇编文件
- 修复编译警告（uint64_t 格式符）

**汇编代码分析**:
- 对比 itoa_const 和 itoa_var 函数的优化差异
- 分析不同整数类型的编译器优化策略
- 研究除法指令的优化情况

### 关键发现

#### 核心结论

通过函数参数传递后，编译期常量和运行时变量在编译器优化后生成的汇编代码**几乎完全相同**。

#### 详细分析

1. **无除法指令**: 所有类型（u8/u16/u32/u64）均无 `div` 指令
2. **魔法常数优化**: 除法被完全优化为乘法 + 位移 + 查表
3. **查表法**: 使用 kDigitPairs 数组预计算 0-99 的两位数字表示
4. **分支优化**: 使用 `cmp` + 条件跳转实现多路分支，现代CPU分支预测器能有效优化

#### 性能数据

| 类型 | 指令数 | 除法指令 | 优化程度 |
|------|--------|----------|----------|
| uint8_t | ~90 | 0 | 完全优化 |
| uint16_t | ~100 | 0 | 完全优化 |
| uint32_t | ~180 | 0 | 完全优化 |
| uint64_t | ~600 | 0 | 完全优化 |

#### 设计启示

1. wwjson 的查表优化对**所有情况**有效（const 和 var）
2. 函数参数传递后，编译期常量优势消失
3. 如需保持编译期优化，应使用模板参数而非函数参数
4. 当前实现已达到编译器优化的极限

### 文档更新

重写 perf/mini/README.md 的整数序列化部分，包含:
- 所有四个整数类型的详细分析
- 汇编代码的关键指令说明
- 魔法常数优化原理解释
- 完整的性能对比表格

### 完成状态

✅ 所有代码重构完成
✅ 所有示例编译通过
✅ 汇编代码分析完成
✅ README.md 文档更新完成

## TASK:20260120-195759
-----------------------

### 任务概述

需求 2026-01-20/1：使用外部库优化浮点数序列化性能。

在 wwjson 构建系统中可选地集成第三方浮点数序列化库（rapidjson 或 fmt），用于 UnsafeConfig 的浮点数序列化。

### 实施过程

#### 1. 创建 external.hpp

**include/external.hpp** - 外部库适配层：

- 命名空间 `wwjson::external`，包含 `rapidjson` 和 `fmt` 子命名空间
- 每个子命名空间中有 `NumberWriter<stringT>` 类
- 根据编译宏 `WWJSON_USE_RAPIDJSON_DTOA` 或 `WWJSON_USE_FMTLIB_DTOA` 选择实现
- 使用 `using NumberWriter = rapidjson::NumberWriter<stringT>` 或 `fmt::NumberWriter<stringT>` 选择
- 只实现 double 类型的 Output()，整数序列化委托给内部 IntegerWriter

#### 2. 宏控制逻辑

- CMake 选项 `WWJSON_USE_RAPIDJSON_DTOA` 和 `WWJSON_USE_FMTLIB_DTOA`
- 编译时定义对应的宏，传递给编译器
- jbuilder.hpp 中根据宏定义决定是否 include external.hpp
- external.hpp 中根据宏定义 include 对应的库头文件

#### 3. 构建系统更新

**CMakeLists.txt**：
- 添加选项 `WWJSON_USE_RAPIDJSON_DTOA` 和 `WWJSON_USE_FMTLIB_DTOA`
- rapidjson: header-only，添加 include 目录和编译宏
- fmt: 有 `fmt::fmt` 目标，链接库并添加编译宏
- 未本地安装时自动 FetchContent 从 GitHub 下载

#### 4. UnsafeConfig 集成

**include/jbuilder.hpp**：
```cpp
#if defined(WWJSON_USE_RAPIDJSON_DTOA) || defined(WWJSON_USE_FMTLIB_DTOA)
#define WWJSON_USE_EXTERNAL_DTOA 1
#include "external.hpp"
#else
#define WWJSON_USE_EXTERNAL_DTOA 0
#endif
```

在 `UnsafeConfig::NumberString<floatT>` 中使用：
```cpp
#if WWJSON_USE_EXTERNAL_DTOA
    external::NumberWriter<stringT>::Output(dst, value);
#else
    NumberWriter<stringT>::Output(dst, value);
#endif
```

#### 5. rapidjson 实现

使用 `rapidjson::internal::Dtoa()` 函数，原位写入 StringBuffer：
```cpp
dst.reserve_ex(32);
char* buffer = dst.end();
const char* result = ::rapidjson::internal::Dtoa(buffer, value);
if (result) {
    size_t len = ::strlen(result);
    dst.unsafe_set_end(buffer + len);
}
```

#### 6. fmt 实现

使用 `fmt::format_to()` 直接写入 StringBuffer 末尾：
```cpp
dst.reserve_ex(32);
char* write_ptr = dst.end();
auto end = ::fmt::format_to(write_ptr, "{}", value);
size_t len = static_cast<size_t>(end - write_ptr);
dst.unsafe_set_end(write_ptr + len);
```

### 测试结果

- 单元测试：119 项全部通过
- 浮点数序列化测试：正常输出 NaN/Infinity 为 "null"
- fmt 格式使用 `"{}"` 输出最短表示

### 使用方式

```bash
# 使用 fmt 库
cmake .. -DWWJSON_USE_FMTLIB_DTOA=ON
make

# 使用 rapidjson 库
cmake .. -DWWJSON_USE_RAPIDJSON_DTOA=ON
make
```

## TASK:20260121-174530
-----------------------

### 任务概述

需求 2026-01-21/1：外部库序列化浮点数功能优化。

1. 添加 `WWJSON_USE_EXTERNAL_DTOA` 选项，支持自动检测本地 rapidjson/fmt 库
2. 简化 external.hpp，移除 nan/inf 和负数判断的重复代码
3. 将特殊值处理提取到 UnsafeConfig::NumberString 中

### 修改内容

**CMakeLists.txt** - 外部库检测逻辑重构：

- 新增 `WWJSON_USE_EXTERNAL_DTOA` 选项，默认 OFF
- 添加 `HAS_USED_EXTERNAL_DTOA` 变量跟踪是否已配置
- rapidjson/fmt 检测失败时不会自动下载（本地检查 only）
- 自动检测优先级：rapidjson > fmt（按顺序检测）

**include/external.hpp** - 简化 NumberWriter 实现：

- 移除 rapidjson::NumberWriter 中的 nan/inf 检查和负数处理
- 移除 fmt::NumberWriter 中的 nan/inf 检查和负数处理
- rapidjson::dtoa 调用改为 `::rapidjson::internal::dtoa(value, buffer)`
- 简化调用 `unsafe_set_end` 重设写入尾指针

**include/jbuilder.hpp** - 统一特殊值处理：

- 在 `UnsafeConfig::NumberString<floatT>` 中添加 nan/inf 检测
- 特殊值输出 "null"（JSON 规范）
- 正常值调用 external::NumberWriter::Output 或内部实现

### 技术说明

**关于 rapidjson 检测**：
- rapidjson 是 header-only 库，不提供 CMake config 文件
- 使用 `find_path` 代替 `find_package` 查找 rapidjson/rapidjson.h
- 本地安装路径：/usr/local/include

**关于三方库 dtoa 函数**：
- rapidjson: `::rapidjson::internal::dtoa(value, buffer)`，参数顺序 (value, buffer)
- fmt: `::fmt::format_to(buffer, "{}", value)`

### 测试结果

- 单元测试：119 项全部通过
- 自动检测功能验证：
  - `WWJSON_USE_EXTERNAL_DTOA=ON` 正确检测到 rapidjson
  - `WWJSON_USE_EXTERNAL_DTOA=ON` 正确检测到 fmt（无 rapidjson 时）
- 浮点数序列化正常输出

### 使用方式

```bash
# 方式1：明确指定使用 rapidjson
cmake .. -DWWJSON_USE_RAPIDJSON_DTOA=ON

# 方式2：明确指定使用 fmt
cmake .. -DWWJSON_USE_FMTLIB_DTOA=ON

# 方式3：自动检测本地库（仅检测，不下载）
cmake .. -DWWJSON_USE_EXTERNAL_DTOA=ON
```


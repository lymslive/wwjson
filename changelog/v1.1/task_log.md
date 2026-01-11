# AI 协作任务工作日志

格式说明:
- **任务ID**: YYYYMMDD-HHMMSS ，生成命令 `date +"%Y%m%d-%H%M%S"`
- 每条日志开始一个二级标题，标题名就是任务ID
- 可适合分几个三级标题，简明扼要描叙任务过程与结果
- **追加至文件末尾**，与上条日志隔一空行

---

## 上版本 v1.0.0 开发摘要

WWJSON v1.0.0 版本开发周期：2025-11-25 至 2025-12-22

### 主要里程碑

#### 基础架构阶段 (2025-11-25 ~ 2025-11-27)
- ✅ 完成 CMake 构建系统，支持头文件库安装
- ✅ 建立单元测试框架，集成 couttast 测试库
- ✅ 实现 RAII 作用域管理（ScopeArray/ScopeObject）
- ✅ 完成模板化重构，支持自定义字符串类型

#### 功能完善阶段 (2025-11-27 ~ 2025-12-05)
- ✅ 配置化转义策略，支持键值分离控制
- ✅ 操作符重载支持，实现 [] 和 << 语法
- ✅ 函数参数支持，允许 Lambda 嵌套构建
- ✅ std::string_view 支持，提升字符串处理效率

#### 性能优化阶段 (2025-12-05 ~ 2025-12-15)
- ✅ 小整数缓存表优化，性能提升 10-15 倍
- ✅ 浮点数定点优化，四位小数序列化提升 20%
- ✅ 字符串转义优化，使用临时缓冲区策略
- ✅ 完整的性能测试框架建立

#### 文档和发布阶段 (2025-12-15 ~ 2025-12-22)
- ✅ 完整的 Doxygen API 文档生成
- ✅ 用户指南编写和示例完善
- ✅ GitHub Pages 自动部署系统
- ✅ CI/CD 流水线优化和稳定

### 技术亮点

#### 性能优化成果
- **整数序列化**：通过小整数缓存表和除法策略优化，相比 std::to_chars 提升 10-15 倍
- **浮点数序列化**：定点小数优化和 std::to_chars 回滚机制，四位小数处理提升 20%
- **字符串转义**：临时缓冲区策略避免频繁 push_back，转义性能反超 yyjson

#### API 设计创新
- **多种构建风格**：传统方法、操作符重载、链式调用、Lambda 嵌套、类方法拆分
- **RAII 作用域管理**：自动关闭容器，支持 if 语句语法糖
- **模板化设计**：支持自定义字符串类型，保持接口一致性

#### 测试和文档
- **75 个单元测试**：覆盖所有核心功能，确保代码质量
- **44 个性能测试**：与 yyjson 等主流库全面对比
- **自动化文档**：Doxygen + Pandoc + GitHub Pages 完整链路

### 开发统计
- **开发时长**：28 天
- **任务记录**：47 个主要任务，详细记录在 changelog/v1.0/task_log.md
- **代码提交**：多次迭代优化，保持向后兼容性
- **测试覆盖**：单元测试 + 性能测试 + CI/CD 自动化

### 详细记录
- [changelog/v1.0/task_todo.md](changelog/v1.0/task_todo.md)
- [changelog/v1.0/task_log.md](changelog/v1.0/task_log.md)

---

**后续工作日志**

## TASK:20251222-110430
-----------------------

### 任务概述
完成 WWJSON v1.0.0 版本封版发布，将原始需求文档和任务日志归档到 changelog/v1.0/ 目录，并在根目录创建包含版本摘要的新文档。

### 实现内容

**文档归档**
- 将原始 task_todo.md 移动到 changelog/v1.0/task_todo.md
- 将原始 task_log.md 移动到 changelog/v1.0/task_log.md
- 保留完整的开发历史记录，便于后续参考

**根目录文档重建**
- 创建新的 task_todo.md，包含 v1.0.0 版本摘要和后续开发计划
- 创建新的 task_log.md，包含 v1.0.0 开发历程摘要
- 保持文档结构，但内容精简，便于后续开发使用

**版本摘要内容**
- 核心功能完成情况：RAII、模板化、多种 API 风格
- 性能优化成果：整数、浮点数、字符串转义优化
- 开发工具完善：CMake、测试框架、CI/CD 流水线
- 文档系统建立：API 文档、用户指南、在线部署

### 技术细节

**文档组织策略**
- changelog/v1.0/ 保存完整的 v1.0.0 开发记录
- 根目录文档包含摘要和后续计划，便于日常开发参考
- 保持原有文档格式和结构，确保一致性

**版本管理**
- v1.0.0 作为首个正式发布版本，功能完整且稳定
- 后续开发可基于此版本继续优化和扩展
- 保持单头文件库的核心设计理念

### 完成结果
成功完成 v1.0.0 版本封版工作：
✅ 原始开发文档完整归档
✅ 根目录文档重建，包含版本摘要
✅ 后续开发计划明确
✅ 为 v1.0.0 标签发布做好准备

### 后续开发建议

+ 性能优化方向
  - 专用字符串类作为写入目标，进一步提升性能
  - 更多数值类型的优化，如大整数和高精度浮点数
  - 内存分配策略优化，减少动态分配开销
+ 功能扩展
  - JSON 解析功能（可选，保持单头文件设计）
  - 更多序列化格式支持（如 MessagePack、CBOR）
  - 异步构建支持，适用于大规模数据处理
+ 工具和生态
  - 更多语言的绑定（Python、Rust、Go 等）
  - 与其他 JSON 库的兼容性层
  - 性能分析工具和基准测试套件
+ 文档和示例
  - 更多实际应用场景的示例
  - 最佳实践指南
  - 性能调优手册

## TASK:20251222-232501
-----------------------

### 实现内容
在 `perf/p_design.cpp` 文件中增加了 `test::perf::WriteUnsignedCompare` 测试类，对比两种字符写入方法的性能差异：

- **方法A**: 使用 `::memcpy(ptr -= 2, src, 2)` 一次拷贝两个字符
- **方法B**: 当前实现，使用两次 `*(--ptr)` 单独赋值

### 测试结果
经过多轮测试（10,000-200,000个随机大整数）：

| 测试规模 | 方法A (memcpy) | 方法B (两次赋值) | 性能优势 |
|---------|---------------|----------------|---------|
| 10,000项 | 1.2284ms | 1.3167ms | 7.19% 更快 |
| 100,000项 | 12-14ms | 13-15ms | 3-7% 更快 |
| 200,000项 | 23.961ms | 25.4978ms | 6.41% 更快 |

### 分析结论
1. **memcpy方法明显更快**，稳定优势在3-7%之间，超过预期
2. 性能优势在不同数据规模下都存在，说明memcpy更高效
3. 优势可能来自：
   - memcpy使用优化的指令集（如SSE/AVX）
   - 减少内存操作次数，一次性处理2字节
   - 编译器对标准库函数的特殊优化

### 关于更快写法的探讨
当前实现已接近硬件效率极限，进一步优化可能需要：
- SIMD指令批量处理（增加复杂度）
- 平台特定汇编（降低可移植性）
- CPU微架构优化（过度工程化）

### 建议
**建议采用memcpy方法**，因为：
- 3-7%的性能提升在实际应用中有意义
- 保持代码简洁性，使用标准库函数
- 在性能、可读性和可移植性之间取得良好平衡

## TASK:20251223-000925
-----------------------

完成 2025-12-22/3 需求：将整数序列化方法改为 memcpy 拷贝缓存表的 2 字符

### 实施内容

1. **备份原性能测试二进制**：已将 `./build-release/perf/pfwwjson` 备份为 `pfwwjson.last`

2. **修改 NumberWriter::WriteUnsigned 方法**：
   - 在大整数处理循环中（`while (value >= 100)`），将原来两次单独的字符赋值：
     ```cpp
     const DigitPair &pair = kDigitPairs[chunk];
     *(--ptr) = pair.low;
     *(--ptr) = pair.high;
     ```
   - 改为使用 `memcpy` 一次拷贝两个字符：
     ```cpp
     const char* digit = &kDigitPairs[chunk].high;
     ::memcpy(ptr -= 2, digit, 2);
     ```

3. **修改 WriteSmall 方法的浮点数处理部分**：
   - 同样将字符对的两步赋值改为 `memcpy` 拷贝：
     ```cpp
     const char* digit_q = &kDigitPairs[scaled_int / 100].high;
     const char* digit_r = &kDigitPairs[scaled_int % 100].high;
     ::memcpy(ptr, digit_q, 2);
     ptr += 2;
     ::memcpy(ptr, digit_r, 2);
     ```

4. **简化代码结构**：
   - 移除了 `const DigitPair &pair` 中间变量
   - 直接使用 `const char* digit` 指针变量进行 memcpy 操作

### 性能测试结果

#### design_large_int 测试
- **优化前**：NumberWriter::WriteUnsigned 比 std::to_chars = 0.738
- **优化后**：NumberWriter::WriteUnsigned 比 std::to_chars = 0.725

#### number_int_rel 测试  
- **优化前**：wwjson builder 比 yyjson API = 1.34756
- **优化后**：wwjson builder 比 yyjson API = 1.31824

数值比小说明时间短，性能有所提升，但具体数值可能有浮动误差。

### 结论
根据上次任务的测试结果，完成优化并验证有效。

## TASK:20251223-173539
-----------------------

### 实现内容
完成 2025-12-23/1 需求：开发更适合 JSON 序列化的字符串类

### 实施内容

1. **创建 include/jstring.hpp 头文件**：
   - 实现 `UnsafeStringConcept` 结构体定义不安全字符串操作接口
   - 实现 `StringBufferView` 结构体采用三指针设计（m_begin, m_end, m_cap_end）
   - 实现 `StringBuffer<kUnsafeLevel>` 模板类提供高性能字符串缓冲区功能
   - 定义 `JString` 作为 `StringBuffer<4>` 的别名，适合 JSON 序列化场景

2. **核心功能实现**：
   - **不安全级别机制**：`kUnsafeLevel` 允许在边界检查后进行多次不安全操作
   - **三指针设计**：m_begin(开始)、m_end(当前结束)、m_cap_end(容量结束)
   - **容量管理**：`reserve_ex()` 预留额外安全边界，`reserve()` 精确预留
   - **不安全操作**：`unsafe_push_back()`、`unsafe_set_end()`、`unsafe_end_cstr()`
   - **标准接口**：`append()`、`push_back()`、`clear()`、`size()`、`capacity()` 等

3. **创建单元测试 utest/t_jstring.cpp**：
   - 14个测试用例涵盖基础功能、不安全操作、边界情况、复制移动语义
   - 使用 couttast 框架，遵循项目测试规范
   - 更新 CMakeLists.txt 包含新的测试文件

### 设计特点

1. **性能优化**：
   - 批量边界检查减少单字符写入开销
   - 延迟空字符封端直到最终需要
   - 直接内存操作避免中间拷贝
   - 支持第三方格式化函数直接操作缓冲区

2. **架构设计**：
   - 数据与方法分离：StringBufferView 只存数据，StringBuffer 提供方法
   - 模板参数化：支持不同不安全级别配置
   - RAII 支持：自动内存管理
   - 兼容标准：实现标准字符串接口子集

3. **JSON 优化**：
   - `kUnsafeLevel=4` 适配常见 JSON 模式（`":"`、`","`、`"{"`、`"}"`）
   - 支持连续不安全字符写入提高 JSON 构建效率

### 测试结果

所有 14 个测试用例全部通过：
- 基础构造功能 ✓
- 字符串追加操作 ✓  
- 单字符追加 ✓
- 容量预留管理 ✓
- 不安全操作 ✓
- 多级别支持 ✓
- 复制移动语义 ✓
- 边界情况处理 ✓
- JSON 序列化模式 ✓
- StringBufferView 转换 ✓
- 容量增长 ✓
- 首尾字符访问 ✓
- 清空操作 ✓
- 空字符结尾 ✓

## TASK:20251224-121931
-----------------------

### 需求内容
完成 2025-12-24/1 需求：StringBuffer 内存管理优化

主要内容包括：
1. 澄清 kUnsafeLevel 的语义为定义 2：reserve_ex(n) 后可写入 n 个字符，写入完成后仍可调用 unsafe_push_back kUnsafeLevel 次
2. 构造函数 `StringBuffer(size_t capacity)` 应申请 `capacity + kUnsafeLevel + 1` 个字节
3. 在 `allocate(size_t capacity)` 实际申请内存时考虑向上圆整对齐、首次最小申请字节（256）
4. 扩容时按常规 2 倍指数申请新内存，快速扩张到 8M 后再线性扩展每次 8M
5. 保证容量不变式：capacity() = m_cap_end - m_begin，实际分配 >= capacity() + 1，m_cap_end 指向实际申请内存最后一个字节

### 实施内容

1. **添加编译宏配置**：
   - `JSTRING_MIN_ALLOC_SIZE=256`：最小初始分配大小
   - `JSTRING_MAX_EXP_ALLOC_SIZE=8MB`：最大指数分配大小

2. **新增内存分配计算方法**：
   - `calculate_alloc_size(size_t requested_capacity)`：计算对齐后的分配大小，应用最小分配约束
   - `calculate_growth_size(size_t current_capacity, size_t requested_capacity)`：计算扩容大小，支持指数（2x）和线性（+8MB）增长策略

3. **修改 allocate 方法**：
   - 参数改为 `new_capacity`（包括 kUnsafeLevel）
   - 使用 `calculate_alloc_size` 计算实际分配大小
   - m_cap_end 指向 `m_begin + new_capacity`（最后一个可分配字节）
   - 在 m_cap_end 位置初始化 '\0' 保证安全

4. **修改 reallocate 方法**：
   - 参数改为 `new_capacity`（包括 kUnsafeLevel）
   - 使用 `calculate_growth_size` 计算新分配大小
   - 保持不变式：m_cap_end = m_begin + new_capacity

5. **修改 reserve 方法**：
   - `reserve_ex(size_t additional_capacity)`：调用 `reserve(size() + additional_capacity)`
   - `reserve(size_t new_capacity)`：预留 `new_capacity + kUnsafeLevel` 字节

6. **修改 copy_from 方法**：
   - 添加注释说明分配足够空间容纳其他 StringBuffer 的内容加上自己的 kUnsafeLevel

7. **更新注释和文档**：
   - 澄清 kUnsafeLevel 定义 2 的语义
   - 更新 capacity()、reserve_ex()、reserve() 的文档说明
   - 更新不变式说明

8. **增强单元测试**：
   - 新增 `jstring_kunsafelevel_semantics` 测试 kUnsafeLevel 语义
   - 新增 `jstring_memory_alignment` 测试内存对齐和最小分配
   - 新增 `jstring_invariants` 测试 StringBuffer 不变量

### 修改说明

1. **kUnsafeLevel 语义澄清**：
   - 旧定义：kUnsafeLevel 指不安全操作次数
   - 新定义（定义 2）：reserve_ex(n) 后可写入 n 字符，写入完成后仍可调用 unsafe_push_back kUnsafeLevel 次
   - capacity() = 用户容量 + kUnsafeLevel
   - 实际分配 = capacity() + 1（null terminator）

2. **内存分配策略**：
   - 最小分配：256 字节
   - 对齐：8 字节边界
   - 扩容：指数（2x）直到 8MB，然后线性（+8MB）

3. **不变式保证**：
   - capacity() == m_cap_end - m_begin
   - 实际分配 >= capacity() + 1
   - m_cap_end 指向实际分配内存的最后一个字节
   - m_cap_end 位置初始化为 '\0'

## TASK:20251225-114051
-----------------------

### 任务概述

重构 StringBuffer 继承关系，实现更清晰的类层次结构和职责分离。

### 实施过程

#### 1. StringBufferView 重构

将 `StringBufferView` 从简单的结构体改为具有 protected 成员的类：

- 将 `m_begin`、`m_end`、`m_cap_end` 改为 protected 成员，使派生类可以访问
- 添加公共接口方法：
  - **只读方法**：`size()`, `capacity()`, `empty()`, `data()`, `front()`, `back()`, `c_str()`
  - **unsafe 写入方法**：`unsafe_push_back()`, `unsafe_set_end()`
  - **其他方法**：`clear()`, `unsafe_end_cstr()`

设计理念：StringBufferView 提供读与在不扩容情况下有限范围内的、不检查边界的 unsafe 写操作。

#### 2. StringBuffer 重构

修改 `StringBuffer` 的继承关系和方法分布：

- 继承方式从 `private StringBufferView` 改为 `public StringBufferView`
- 删除已移至基类的重复方法：
  - 只读方法：`size()`, `capacity()`, `empty()`, `data()`, `front()`, `back()`, `c_str()`
  - unsafe 方法：`unsafe_push_back()`, `unsafe_set_end()`, `clear()`, `unsafe_end_cstr()`
- 保留在 StringBuffer 中的方法：
  - **安全写入方法**：`append()`, `push_back()`
  - **内存管理方法**：`reserve()`, `reserve_ex()`, `allocate()`, `deallocate()`, `reallocate()`
  - 所有构造函数和赋值运算符
  - 私有辅助方法：`calculate_alloc_size()`, `calculate_growth_size()`, `copy_from()`, `move_from()`

#### 3. 测试代码更新

更新 `utest/t_jstring.cpp` 中的 `jstring_buffer_view` 测试：

- 将 `reinterpret_cast<const StringBufferView&>(buffer)` 改为 `static_cast<const StringBufferView&>(buffer)`，因为现在是 public 继承
- 将直接访问成员变量改为使用公共接口：
  - `view.m_end - view.m_begin` → `view.size()`
  - `view.m_begin` → `view.data()`
  - `(view.m_end == view.m_begin)` → `view.empty()`
  - `*view.m_begin` → `view.front()`
  - `*(view.m_end - 1)` → `view.back()`

### 测试结果

运行单元测试验证重构正确性：

```bash
cd build && ./utest/utwwjson t_jstring.cpp --cout=silent
```

所有 17 个测试全部通过。

### 设计改进

1. **更好的封装**：StringBufferView 成员改为 protected，外部无法直接访问，必须通过公共接口
2. **清晰的职责分离**：
   - StringBufferView：视图功能（只读和 unsafe 操作）
   - StringBuffer：缓冲区功能（安全操作和内存管理）
3. **更符合 OOP 原则**：public 继承表示 "is-a" 关系，StringBuffer 确实 "is a" StringBufferView
4. **代码复用**：公共方法集中在基类，减少重复代码

### 修改文件

- `include/jstring.hpp`：StringBufferView 和 StringBuffer 类定义重构
- `utest/t_jstring.cpp`：更新测试代码以适配新的公共接口

## TASK:20251225-145239
-----------------------

### 任务概述

完成 2025-12-25/2 需求：StringBuffer 功能设计增强

### 实施内容

#### 1. StringBufferView 增强功能

在 `StringBufferView` 类中添加了迭代器相关方法：

- **begin()**：返回 `char* m_begin` 指针（const 和非 const 版本）
- **end()**：返回 `char* m_end` 指针（const 和非 const 版本）
- **cap_end()**：返回 `char* m_cap_end` 指针（const 和非 const 版本）

这些方法：
- 符合标准容器的迭代器命名惯例
- 允许用户从 `end()` 处继续写入数据
- 保持与三指针设计的一致性

#### 2. unsafe_set_end 指针重载

在 `StringBufferView` 中为 `unsafe_set_end` 添加了 `char*` 参数重载：

```cpp
void unsafe_set_end(char* new_end)
{
    m_end = new_end;
}
```

支持用法：
- 用户可以从 `end()` 处开始写入
- 写到新位置后再设置回去
- 保留原有的 `size_t` 参数版本，两者不冲突

#### 3. unsafe_append 双参数版本

在 `StringBufferView` 中添加了 `unsafe_append(char*, size)` 方法：

```cpp
void unsafe_append(const char* str, size_t len)
{
    ::memcpy(m_end, str, len);
    m_end += len;
}
```

特点：
- 不进行边界检查
- 允许直接在缓冲区末尾追加字符串
- 供 `StringBuffer` 的安全版本 `append()` 调用

#### 4. StringBuffer 方法优化

修改 `StringBuffer` 中的安全方法调用基类的 unsafe 版本：

- **append(const char* str, size_t len)**：改为调用 `StringBufferView::unsafe_append(str, len)`
- **push_back(char c)**：改为调用 `StringBufferView::unsafe_push_back(c)`

这样做的优势：
- 减少代码重复
- 保持 unsafe 操作在基类统一管理
- 安全方法专注于边界检查，实际操作委托给基类

#### 5. safe set_end 方法
#### 6. safe end_cstr 方法
检查边界，不扩容.

#### 7. 单元测试增强

在 `utest/t_jstring.cpp` 中添加了 `jstring_to_chars_integration` 测试用例：

测试场景包括：
- 使用 `std::to_chars` 在 `JString` 原位 buffer 上转换整数
- 先预留足够空间，从 `end()` 处写入整数，再更新 `end` 位置
- 测试正数、负数、无符号整数
- 使用新增的 `begin()`、`end()`、`cap_end()` 方法
- 使用 `unsafe_append(char*, size)` 方法

### 测试结果

运行单元测试验证所有功能正确：

```bash
./build/utest/utwwjson t_jstring.cpp --cout=silent
```

所有 18 个测试用例全部通过，包括新增的 `jstring_to_chars_integration` 测试。

运行全部单元测试确保没有破坏现有功能：

```bash
./build/utest/utwwjson --cout=silent
```

所有 112 个测试用例全部通过。

### 设计改进

1. **更符合标准容器规范**：添加 `begin()`/`end()` 等迭代器方法
2. **更灵活的 buffer 操作**：支持从 `end()` 处继续写入
3. **更安全的接口**：提供带边界检查的 `set_end` 和 `end_cstr` 版本
4. **代码复用**：安全方法调用基类的 unsafe 版本，减少重复
5. **更好的第三方函数集成**：通过 `begin()`/`end()`/`cap_end()` 方便与 `std::to_chars` 等函数集成

### 修改文件

- `include/jstring.hpp`：
  - StringBufferView：添加 `begin()`, `end()`, `cap_end()` 方法
  - StringBufferView：添加 `unsafe_append(char*, size)` 方法
  - StringBufferView：添加 `unsafe_set_end(char*)` 重载
  - StringBuffer：修改 `append()` 和 `push_back()` 调用基类 unsafe 版本
  - StringBuffer：添加 `set_end(size_t)` 和 `set_end(char*)` 方法
  - StringBuffer：添加 `end_cstr()` 方法

- `utest/t_jstring.cpp`：
  - 修复 `jstring_edge_cases` 中的 `unsafe_set_end(0)` 调用，添加显式类型转换
  - 添加 `#include <system_error>` 以支持 `std::make_error_code`
  - 添加 `jstring_to_chars_integration` 测试用例


## TASK:20251226-112337
-----------------------

### 需求

需求 ID：2025-12-26/1

对 StringBuffer 的 end 状态管理进行 API 重构优化：
1. 将 `unsafe_set_end(size_t)` 重命名为 `unsafe_resize(size_t)`
2. 将 `set_end(size_t)` 重命名为 `resize(size_t)`，并支持扩容
3. 修改 `clear()` 方法行为：相当于 `unsafe_resize(0)`，不再隐含添加空字符 '\0'，只有 `c_str()` 才添加空字符

### 实现

#### 1. 修改 StringBufferView

- 将 `unsafe_set_end(size_t)` 重命名为 `unsafe_resize(size_t)`
- 修改 `clear()` 方法，移除 `*m_end = '\0'`，改为调用 `unsafe_resize(0)`
- 保持 `unsafe_set_end(char*)` 方法不变

#### 2. 修改 StringBuffer

- 将 `set_end(size_t)` 重命名为 `resize(size_t)`
- 实现 `resize(size_t)` 支持扩容：若 `new_size > capacity()` 则调用 `reserve(new_size)`
- 保持 `set_end(char*)` 方法不变，用于已知 end 指针的场景

#### 3. 更新测试文件

- 将 `unsafe_set_end(size_t)` 调用改为 `unsafe_resize(size_t)`（3处）
- 将 `unsafe_set_end(static_cast<size_t>(0))` 简化为 `unsafe_resize(0)`
- 保持 `set_end(char*)` 调用不变

### 测试

运行 `make test`，编译成功，所有 112 个测试用例全部通过。

### 设计改进

1. **更清晰的 API 语义**：`resize` 表示调整大小，比 `set_end` 更准确；`resize` 支持扩容，使用更方便
2. **职责分离**：`clear()` 只负责重置大小，不再添加空字符；`c_str()` 统一负责添加空字符
3. **保持灵活性**：保留 `unsafe_set_end(char*)` 和 `set_end(char*)` 用于已知指针的场景

### 修改文件

- `include/jstring.hpp`：StringBufferView 重命名方法并修改 `clear()`；StringBuffer 重命名并实现扩容
- `utest/t_jstring.cpp`：更新方法名调用

## TASK:20251226-121411
-----------------------

### 需求

需求 ID：2025-12-26/2

实现 StringBuffer 与标准字符串的互操作功能：
1. StringBufferView 可隐式转换为 std::string_view
2. StringBufferView 需显式转换为 std::string，因涉及拷贝
3. StringBuffer 的 append 方法增加重载，支持参数 std::string 与 std::string_view

### 实现

1. 在 `include/jstring.hpp` 中添加 `<string>` 和 `<string_view>` 头文件
2. 在 `StringBufferView` 中添加隐式转换到 `std::string_view` 的运算符
3. 在 `StringBufferView` 中添加显式转换到 `std::string` 的运算符
4. 在 `StringBuffer` 的 `append` 方法中增加 `std::string` 和 `std::string_view` 的重载

### 测试

在 `utest/t_jstring.cpp` 中添加了 4 个测试用例：
- jstring_to_string_view：测试隐式转换
- jstring_to_string：测试显式转换
- jstring_append_std_string：测试 append 支持 std::string
- jstring_append_string_view：测试 append 支持 std::string_view

运行单元测试，所有 116 个测试用例全部通过。

### 设计改进

1. 类型安全的互操作：通过隐式转换到 std::string_view，可以方便地将 StringBuffer 传递给接受 string_view 的函数，无需拷贝
2. 显式转换防止意外拷贝：std::string 转换显式化，避免隐式拷贝带来的性能问题
3. 统一的 append 接口：新增重载使 append 方法支持所有常见的字符串类型
4. 更好的标准库集成：通过 std::string_view 转换，可以更方便地与使用标准库的代码集成

### 修改文件

- `include/jstring.hpp`：添加头文件、转换运算符、append 重载
- `utest/t_jstring.cpp`：添加头文件、测试用例

## TASK:20251226-154417
-----------------------

### 需求

需求 ID：2025-12-26/3

为 StringBufferView 和 StringBuffer 新增填充和字符重复追加功能：
1. StringBufferView 增加 fill(ch, count, move_end) 方法，类似 memset
2. StringBuffer 增加 append(count, ch) 方法，支持扩容

### 实现

#### StringBufferView::fill 方法

在 `include/jstring.hpp` 添加 fill 方法：
- 默认 count 为 size_t(-1) 时，自动填充全部剩余容量
- 不扩容，count 超过可用容量时自动截断
- move_end 参数控制是否移动 m_end 指针，默认 false
- 使用 memset 批量填充字符

#### StringBuffer::append(count, ch) 方法

添加 append 重载：
- 直接调用 fill 批量填充，避免循环和多次移动 end 指针
- 支持自动扩容
- 参数顺序遵循 std::string::append(count, ch) 约定

### 测试

在 `utest/t_jstring.cpp` 中添加了 2 个测试用例：
- jstring_fill：测试 fill 方法的各种场景
- jstring_append_count_char：测试 append(count, ch) 的各种场景

同时优化了现有测试 jstring_kunsafelevel_semantics，使用新增的 append(count, ch) 方法。

运行单元测试，所有 118 个测试用例全部通过。

### 修改文件

- `include/jstring.hpp`：添加 fill 和 append(count, ch) 方法
- `utest/t_jstring.cpp`：添加测试用例、优化现有测试

## TASK:20251226-174807
-----------------------

### 需求

需求 ID：2025-12-26/4

设计 StringBuffer 默认构造状态，避免空指针处理：
1. StringBuffer 默认构造时预分配 1024 字节内存（kDefaultAllocate）
2. 删除 JSTRING_MIN_ALLOC_SIZE 宏，不再强制最小申请 256 字节
3. StringBufferView 增加 operator bool() 判断是否申请过内存（m_begin 非空）
4. StringBufferView 在 back/front 方法与 unsafe 方法中增加 debug 版本 assert 做安全检查
5. 修改了默认构造函数语义，修复因变更导致的单元测试

### 实现

#### 1. 修改默认构造函数语义

- 添加 `kDefaultAllocate = 1024` 常量，表示默认内存申请量
- 默认构造函数复用 `StringBuffer(size_t capacity)` 构造函数：
  `StringBuffer() : StringBuffer(kDefaultAllocate - kUnsafeLevel - 1)`
- 默认构造后：申请 1024 字节，capacity() = 1023（减 1 给 null terminator）

#### 2. 删除 JSTRING_MIN_ALLOC_SIZE 宏

- 移除 `#define JSTRING_MIN_ALLOC_SIZE 256` 宏定义
- 修改 `calculate_alloc_size()`：size=0 时返回 0，否则直接对齐到 8 字节
- 修改 `calculate_growth_size()`：移除 `cur_size > JSTRING_MIN_ALLOC_SIZE` 判断，改为 `cur_size > 0`

#### 3. 添加 operator bool()

在 `StringBufferView` 中添加：
```cpp
explicit operator bool() const { return m_begin != nullptr; }
```

#### 4. 添加 debug assert

在 `StringBufferView` 的以下方法中添加 assert：
- `front()`：检查 m_begin 非空且 m_end > m_begin
- `back()`：检查 m_begin 非空且 m_end > m_begin
- `unsafe_push_back()`：检查 m_begin 非空
- `unsafe_resize()`：检查 m_begin 非空
- `unsafe_set_end(char*)`：检查 m_begin 非空
- `unsafe_append()`：检查 m_begin 非空
- `unsafe_end_cstr()`：检查 m_begin 非空
- `fill()`：检查 m_begin 非空

#### 5. 添加 `<cassert>` 头文件

在 `include/jstring.hpp` 中添加 `#include <cassert>` 以支持 assert

#### 6. 修复单元测试

更新以下测试用例以适应新的默认构造语义：
- jstring_basic_construct：默认构造后 capacity 为 1023，添加 operator bool() 测试
- jstring_capacity_growth：使用小容量 (50) 构造观察扩容行为
- jstring_memory_alignment：更新容量期望值（capacity = alloc_size - 1）
- jstring_kunsafelevel_semantics：更新容量期望值
- jstring_copy_move：更新移动后源对象的 capacity 期望值，添加 operator bool() 测试
- jstring_to_string_view：更新容量期望值，添加 operator bool() 测试

### 测试

运行 `make test`，所有 118 个测试用例全部通过。

### 设计改进

1. **避免空指针**：默认构造即分配内存，避免对空指针的特殊处理
2. **清晰的常量命名**：kDefaultAllocate 明确表示默认内存申请量，不是 capacity
3. **复用现有逻辑**：默认构造函数复用带容量的构造函数，避免重复逻辑
4. **安全检查**：在关键方法中添加 assert，帮助在调试阶段尽早发现错误
5. **operator bool() 语义**：检查是否有内存，而不是检查是否为空字符串

### 修改文件

- `include/jstring.hpp`：添加 kDefaultAllocate、operator bool()、assert、删除 JSTRING_MIN_ALLOC_SIZE、修改默认构造
- `utest/t_jstring.cpp`：修复单元测试、添加 operator bool() 测试

## TASK:20251227-171417
-----------------------

### 需求

需求 ID：2025-12-27/1

StringBufferView 重命名再派生 LocalBuffer 类，具体要求：

1. **重命名 StringBufferView 为 BufferView**
   - 简化为 BufferView，更通用的名称
   - 更新所有引用和文档

2. **创建 LocalBuffer 类**
   - 模板参数 `bool UNSAFE`，默认 false
   - UNSAFE = false：kUnsafeLevel = 0，append/push_back 每次检查边界
   - UNSAFE = true：kUnsafeLevel = 0xFF，append/push_back 不检查边界

3. **BufferView 增强**
   - 添加 `overflow()` 方法：检测是否溢出（m_end > m_cap_end）
   - `reserve_ex()` 空参版：检查剩余可用字节，返回 int64_t

4. **LocalBuffer 构造函数**
   - 5 种构造函数支持不同容器类型
   - 无默认构造函数
   - Copy constructor 删除，Move constructor 显式实现

5. **LocalBuffer 行为**
   - 不扩容，固定容量
   - 不拥有内存，借用外部内存
   - 写入完成后可检查 overflow() 状态

### 实现

#### 1. 重命名 StringBufferView 为 BufferView
- 在 `include/jstring.hpp` 中将类名改为 BufferView
- 更新 StringBuffer 的基类声明
- 更新所有注释和断言消息

#### 2. BufferView 添加 overflow() 方法
- 返回 m_end > m_cap_end 的比较结果
- 用于检测写入是否超出容量

#### 3. 创建 LocalBuffer 模板类
- 模板参数 bool UNSAFE，默认 false
- UNSAFE=true 时 kUnsafeLevel=0xFF，false 时 kUnsafeLevel=0
- 继承 BufferView
- 删除 copy constructor 和 copy assignment
- 显式实现 move constructor 和 move assignment（将源对象置空）

#### 4. LocalBuffer 构造函数实现
- LocalBuffer(char* dst, size_t size)：主构造函数，设置三个指针，在 cap_end 处写 '\0'
- 其他 4 个构造函数委托给主构造函数

#### 5. LocalBuffer::reserve_ex() 实现
- 返回 static_cast<int64_t>(m_cap_end - m_end)
- 正值：剩余字节数，0：已满，负值：overflow 的字节数

#### 6. LocalBuffer::append(str, len) 实现
- safe 模式：检查 m_end + len > m_cap_end，超出则返回
- unsafe 模式：直接调用 unsafe_append(str, len)

#### 7. LocalBuffer::push_back(char) 实现
- safe 模式：检查 m_end + 1 > m_cap_end，超出则返回
- unsafe 模式：直接调用 unsafe_push_back(c)

#### 8. LocalBuffer::append(count, ch) 实现
- safe 模式：检查边界后，直接使用 ::memset 写入并移动 m_end
- unsafe 模式：直接使用 ::memset 写入并移动 m_end
- 不调用 BufferView::fill()，避免自动截断

### 测试

在 `utest/t_jstring.cpp` 中添加 8 个测试用例：

#### 1. localbuffer_constructors
测试所有 5 种构造函数，验证容量和基本功能

#### 2. localbuffer_append
测试 append 方法，safe/unsafe 两种模式，使用大 buffer 小 size 测试溢出

#### 3. localbuffer_push_back
测试 push_back 方法，在 overflow 断言后加 reserve_ex 断言

#### 4. localbuffer_append_count
测试 append(count, ch) 方法，不调用 BufferView::fill

#### 5. localbuffer_resize
测试 resize 方法，safe/unsafe 两种模式

#### 6. localbuffer_fill
测试 BufferView::fill 方法，safe 模式自动截断，unsafe 模式仍然自动截断（基类行为）

#### 7. localbuffer_overflow
测试 overflow 和 reserve_ex，使用大 buffer 小 size 测试溢出

#### 8. localbuffer_move_constructor
单独测试移动构造，验证源对象置空，目标对象拥有数据

### 测试结果

编译成功，所有 LocalBuffer 相关测试用例实现完成。

### 设计改进

1. **类型安全**：模板参数 UNSAFE 在编译时决定行为，零开销
2. **清晰的语义**：LocalBuffer 是非拥有的内存视图，BufferView 提供通用操作
3. **正确的边界检查**：m_end + len > m_cap_end 确保不覆盖 cap_end 位置的 '\0'
4. **reserve_ex() 统一语义**：返回值直接表示可用空间状态，正/零/负三种情况
5. **移动语义正确**：显式实现 move，移动后源对象置空
6. **溢出测试安全**：使用大 buffer 小 size，避免真正的内存访问错误

### 修改文件

- `include/jstring.hpp`：
  - 重命名 StringBufferView → BufferView
  - 添加 BufferView::overflow() 方法
  - 添加 LocalBuffer 模板类及其所有方法
  - 添加 <vector> 和 <array> 头文件
- `utest/t_jstring.cpp`：
  - 更新所有 StringBufferView 引用为 BufferView
  - 添加 8 个 LocalBuffer 测试用例
  - 使用 std::string 比较代替 strcmp

### 遗留问题

1. **BufferView::fill 的自动截断行为**
   - 当前 safe 和 unsafe 模式下都会自动截断
   - 如果需要 unsafe 模式能真正溢出，需要重写 fill 方法
   - 留到下个任务解决

## TASK:20251227-213549
-----------------------

需求 ID：2025-12-27/2

### 实现

#### 1. 重新设计 BufferView::fill 方法
- 将原来的单方法拆分为三个方法：
  - `fill(char ch)`：填充剩余空间（m_end 到 m_cap_end），不改变 size
  - `fill(char ch, size_t count)`：填充 count 个字符，安全截断，移动 end 指针
  - `unsafe_fill(char ch, size_t count)`：填充 count 个字符，不检查容量，移动 end 指针
- 删除原来的 `move_end` 参数，通过方法签名区分行为

#### 2. 更新 StringBuffer::append(count, ch)
- 将 `fill(ch, count, true)` 改为 `unsafe_fill(ch, count)`
- 因为 StringBuffer 已经通过 `reserve_ex(count)` 确保容量，可以直接使用 unsafe 版本

#### 3. 更新 LocalBuffer::append(count, ch)
- Safe 模式：检查边界后调用 `unsafe_fill(ch, count)`
- Unsafe 模式：直接调用 `unsafe_fill(ch, count)`，不检查边界

#### 4. 修复测试用例
- 更新 `jstring_fill` 测试中的 `fill(ch, count, true)` 为 `fill(ch, count)`
- 更新 `jstring_append_count_char` 测试中的 `fill('-', 5, true)` 为 `fill('-', 5)`
- 更新 `localbuffer_fill` 测试中的 `fill('-', 5, true)` 为 `fill('-', 5)`
- 将 `fill('x', -1, true)` 改为先计算剩余容量再调用 `fill('x', remaining)`

### 设计说明

#### API 清晰度改进
- **之前**：`fill(ch)` 填充剩余，`fill(ch, count)` 填充指定数量但不移动 end，`fill(ch, count, true)` 填充并移动 end，语义不清
- **之后**：
  - `fill(ch)`：单参数，语义明确是填充剩余
  - `fill(ch, count)`：双参数，语义明确是填充并增长
  - `unsafe_fill(ch, count)`：明确表示不安全操作

#### 性能优化
- `StringBuffer::append(count, ch)` 改用 `unsafe_fill`，因为 `reserve_ex` 已保证容量，避免重复检查
- `LocalBuffer` 统一使用 `unsafe_fill`，边界检查在调用前完成

### 测试结果

编译成功，运行所有测试用例，125 个测试全部通过。

### 修改文件

- `include/jstring.hpp`：
  - 删除 `BufferView::fill(char ch, size_t count = -1, bool move_end = false)`
  - 添加 `BufferView::fill(char ch)`：填充剩余空间
  - 添加 `BufferView::fill(char ch, size_t count)`：填充并移动 end
  - 添加 `BufferView::unsafe_fill(char ch, size_t count)`：不安全填充
  - 更新 `StringBuffer::append(count, ch)` 调用 `unsafe_fill`
  - 更新 `LocalBuffer::append(count, ch)` 调用 `unsafe_fill`
- `utest/t_jstring.cpp`：
  - 更新 `jstring_fill` 测试中的 fill 调用
  - 更新 `jstring_append_count_char` 测试中的 fill 调用
  - 更新 `localbuffer_fill` 测试中的 fill 调用

### 总结

成功将 `BufferView::fill` 方法从混合功能设计拆分为三个清晰的 API：
1. `fill(ch)`：填充剩余，不改变 size
2. `fill(ch, count)`：填充并增长 size，安全截断
3. `unsafe_fill(ch, count)`：填充并增长 size，不检查容量

同时更新了 `StringBuffer` 和 `LocalBuffer` 的 `append(count, ch)` 实现，使其使用更高效的 `unsafe_fill`，因为容量检查已经在调用方完成。

所有测试通过，验证了重构的正确性。

## TASK:20251227-231327
-----------------------

### 需求

需求 ID：2025-12-27/3

重构 LocalBuffer，上移一些方法至基类 BufferView：
1. LocalBuffer::reserve_ex() 移到 BufferView::overflow() 下面，改为 const 方法
2. BufferView 增加 full() 方法判断已写满
3. BufferView 增加构造函数 `BufferView(char* dst, size_t size)`，assert 检验参数
4. LocalBuffer 构造函数转发基类构造函数
5. BufferView front/back 方法使用 const_cast 转发 const 版本，消除重复 assert

重新设计单元测试用例 `jstring_invariants`：
- 改名为 `bufferview_` 前缀，测试基类方法
- 用局部数组构造 BufferView 进行恒等关系的方法测试
- 测试 full() 不变式：full == !empty，full == (reserve_ex == 0) == (size == capacity)
- 查找 fill 相关用例中 `size() == capacity()` 断言，改为断言 `full()`

### 实现

1. BufferView 新增 full() 方法，放在 overflow() 下面
2. BufferView 新增 reserve_ex() const 方法，从 LocalBuffer 上移
3. BufferView 新增带参数构造函数，添加 dst 非空和 size > 0 的 assert
4. LocalBuffer 构造函数改为委托基类构造函数，删除重复代码
5. BufferView 的 front/back 非const 版本使用 const_cast 转发，消除重复 assert
6. 单元测试 `jstring_invariants` 改名为 `bufferview_invariants`，使用局部数组构造测试基类方法
7. 更新 fill 测试中的断言：将 `size() == capacity()` 改为 `full()`

### 测试结果

编译成功，运行所有测试用例，32 个测试全部通过。

### 设计改进

1. 职责分离更清晰：BufferView 提供通用视图操作，LocalBuffer 专注于特定场景
2. 代码复用：构造函数委托基类，front/back 消除重复断言
3. API 语义更清晰：full() 明确表示缓冲区已满状态
4. 不变式验证：bufferview_invariants 测试验证 BufferView 核心不变式

### 修改文件

- `include/jstring.hpp`：BufferView 和 LocalBuffer 重构
- `utest/t_jstring.cpp`：重命名测试用例，更新断言

### BufferView 不变式

1. full() == !empty()（在非空状态下）
2. full() == (reserve_ex() == 0)
3. full() == (size() == capacity())

## TASK:20251228-111326
-----------------------

### 任务概述

将 `LocalBuffer<bool>` 模板类重构为基类 `BufferView` 和子类 `UnsafeBuffer`：
- BufferView：安全模式，写入方法带边界检查，kUnsafeLevel=0
- UnsafeBuffer：不安全模式，覆盖写入方法直接调用 unsafe_ 方法，kUnsafeLevel=0xFF

### 实现内容

**1. BufferView 基类改造**
- 添加 `kUnsafeLevel = 0` 常量和 `UnsafeStringConcept` 继承
- 添加容器构造函数（C array、std::array、std::string、std::vector）
- 添加移动构造、移动赋值（非拥有语义）
- 删除复制构造、复制赋值
- 添加安全写入方法：append、push_back、resize（带边界检查）

**2. UnsafeBuffer 子类创建**
- 定义 `kUnsafeLevel = 0xFF`
- 使用 `using BufferView::BufferView` 继承所有构造函数
- 覆盖写入方法：append、push_back、resize 直接调用 unsafe_ 方法（无边界检查）

**3. 代码简化**
- 删除 `LocalBuffer<UNSAFE>` 模板类
- 将模板 `append(const StringBuffer<kOtherLevel>&)` 改为 `append(const BufferView&)`
- 移除 UnsafeBuffer 中重复的构造/赋值定义

**4. 单元测试重组**
- 将 `LocalBuffer` 测试拆分为 `bufv_`（BufferView）和 `ubuf_`（UnsafeBuffer）前缀
- 合并写入方法测试：`bufv_write_methods` 和 `ubuf_write_methods`
- 调整测试位置，BufferView/UnsafeBuffer 测试分组

### 修改文件

- `include/jstring.hpp`：LocalBuffer 重构为 BufferView 和 UnsafeBuffer
- `utest/t_jstring.cpp`：单元测试重组和合并

### 测试结果

编译成功，所有测试用例通过。

### 设计改进

1. **职责分离**：BufferView 提供安全视图操作，UnsafeBuffer 提供不安全操作
2. **代码复用**：UnsafeBuffer 通过 using 继承构造函数，避免重复
3. **API 简化**：append 方法统一接受 BufferView 参数，无需模板
4. **类型安全**：kUnsafeLevel 明确区分安全（0）和不安全（0xFF）模式

## TASK:20251228-122949
-----------------------

### 任务概述

在 `include/jstring.hpp` 的 `BufferView` 类头注释中提到：借用标准容器（`std::string` 或 `std::vector<char>`）申请的内存写入后可用 `resize()` 同步。但这不正确，因为容器的 `resize()` 会在扩展时填充默认字符，覆盖 `BufferView` 写入的内容。

本任务添加单元测试验证这种用法，并修正相关注释。

### 实现内容

**1. 修正 include/jstring.hpp 注释**

修改了三处关于 `resize()` 的错误注释，澄清 BufferView 借用容器时 resize 的影响。

**2. 添加测试用例**

在 `utest/t_jstring.cpp` 中添加 `#include "couttast/couthex.hpp"`，并在 `BufferView` 分组后添加两个测试用例：

- `bufv_borrow_string_resize`：验证 `BufferView` 借用 `std::string` 内存的问题
  - **场景1**：`std::string` 原始 `size()` 为 0，`BufferView` 写入后调用 `str.resize()` 会用 `\0` 覆盖所有内容
  - **场景2**：`std::string` 有初始数据，`BufferView` 从头覆盖写入，`resize()` 会保留前缀（原始 size 长度），后面的内容被 `\0` 覆盖

- `bufv_borrow_vector_resize`：验证 `BufferView` 借用 `std::vector<char>` 内存的问题
  - **场景1**：`std::vector` 原始 `size()` 为 0，`resize()` 会用 `char()`（即 `\0`）填充所有内容
  - **场景2**：`std::vector` 有初始数据，`resize()` 会保留前缀，后面的内容被 `\0` 覆盖

**3. 测试实现细节**

- 使用 `COUT_HEX()` 宏以十六进制格式打印 `std::string` 内容，便于直观查看包含特殊字符的数据
- 在 `BufferView` 写入后调用 `bv.end_cstr()` 添加空字符终止符，确保后续字符串操作正确
- 在调用容器 `resize()` 之前，先将 `BufferView` 的内容保存到临时变量，避免数据丢失

### 修改文件

- `include/jstring.hpp`：修正三处关于容器 `resize()` 的错误注释
- `utest/t_jstring.cpp`：添加 `couthex.hpp` 头文件引用和两个新测试用例

### 测试结果

编译成功，所有新增测试用例通过，验证了文档中提到的潜在问题。

### 测试发现

通过测试确认：当 `BufferView` 借用标准容器的内存写入数据后，直接调用容器的 `resize()` 方法来同步大小是**不安全的**，因为 `resize()` 会在扩展时用默认字符 `\0` 填充，覆盖 `BufferView` 写入的内容。

**正确做法**：如果需要同步容器的 `size()`，应该使用其他方法（如直接操作容器或使用 `BufferView::unsafe_set_end()`），而不是依赖容器的 `resize()` 方法。

---

## TASK:20251228-151234
-----------------------

### 需求

需求 ID：2025-12-28/3

整理 BufferView 类的众多方法，使用 doxygen 的 `@{` 和 `@}` 分组标记进行组织，以提高文档的可读性和可维护性。

### 实现内容

**1. 添加 str() 方法**
在 `BufferView` 类中添加 `str()` 方法，提供从 `BufferView` 到 `std::string` 的便捷转换。此方法返回 `std::string(m_begin, size())`，与现有的 `c_str()` 返回 `const char*` 相比，提供了更直接的字符串转换方式。

**2. Doxygen 分组标记**
使用 doxygen 的 `@{` 和 `@}` 标记将 `BufferView` 类的众多方法分组为以下 6 个类别：

1. **Constructors and assignment operators**
   - Default constructor、参数化构造函数（C array、std::array、std::string、std::vector）
   - Move constructor、Move assignment、Destructor

2. **Capacity and size queries**
   - `size()`, `capacity()`, `empty()`, `data()`, `operator bool()`
   - `overflow()`, `full()`, `reserve_ex()`

3. **Pointer and element access**
   - `begin()`, `end()`, `cap_end()`
   - `c_str()`, `str()`
   - `std::string_view` 和 `std::string` 转换运算符
   - `front()`, `back()`

4. **Unsafe write operations**（放在最后）
   - `unsafe_push_back()`, `unsafe_resize()`, `unsafe_set_end(char*)`
   - `unsafe_append()`, `unsafe_end_cstr()`, `unsafe_fill()`

5. **Safe write operations**（包括 fill 方法）
   - `fill(char)`, `fill(char, size_t)`
   - `end_cstr()`
   - `append()`（所有重载版本）
   - `push_back()`, `append(size_t, char)`, `resize()`

6. **Clear 操作**（放在 Safe write operations 中）
   - `clear()` 使用 `unsafe_resize(0)` 实现

**3. 分组设计说明**
- 将指针访问和元素访问合并为一个组（用户建议）
- 将 fill 方法合并到安全写入操作组（用户建议）
- 将不安全写入操作放在最后，便于快速定位安全和不安全方法
- 保留了 `UnsafeStringConcept` 继承关系和 `kUnsafeLevel` 常量

### 修改文件

- `include/jstring.hpp`：
  - 添加 `str()` 方法
  - 为 BufferView 类方法添加 doxygen `@{` `@}` 分组标记

### 测试结果

编译成功，运行所有测试用例，127 个测试全部通过。

### 设计改进

1. **文档可读性**：通过 doxygen 分组，方法组织更清晰，便于查找和理解
2. **代码结构**：方法按功能分类，注释更精简
3. **安全区分**：safe 和 unsafe 方法明确分组，降低误用风险
4. **字符串转换**：`str()` 方法提供更便捷的字符串转换方式

## TASK:20251228-213120
-----------------------

### 需求

需求 ID：2025-12-28/4

重新设计 reserve_ex(n) 返回 bool，修改文件 include/jstring.hpp。重新约定 UnsafeStringConcept 要求 reserve_ex(n) 返回 bool 表示是否有空间再写入 n 字节：
- 剩余空间足够再写入 n 字节时返回 true
- 基类 BufferView::reserve_ex(n) 不扩容，剩余容量不足时返回 false
- UnsafeBuffer::reserve_ex(n) 假定空间够，始终返回 true
- StringBuffer::reserve_ex(n) 扩容失败时返回 false

### 实现内容

**1. 更新 UnsafeStringConcept 文档**
更新文档中 `reserve_ex(size_t extra)` 的接口说明，从 "Reserve extra capacity with kUnsafeLevel bytes margin" 改为 "Check if buffer has space to write extra bytes, return true if available"。

**2. BufferView 类修改**
- 保留原有 `int64_t reserve_ex() const` 方法（无参数版本，查询剩余字节数）
- 新增 `bool reserve_ex(size_t n) const` 方法（单参数版本，检查是否有 n 字节空间）
  - 实现复用无参数版本：`return reserve_ex() >= static_cast<int64_t>(n)`
  - 正确处理溢出情况（无参数版本返回负数时比较结果为 false）

**3. UnsafeBuffer 类修改**
- 添加 `using BufferView::reserve_ex` 声明，暴露基类的无参数版本
- 新增 `bool reserve_ex(size_t n) const` 方法，始终返回 true（unsafe 模式假定空间足够）

**4. StringBuffer 类修改**
- `void reserve(size_t new_capacity)` 保持原有行为，内存分配失败时抛 std::bad_alloc 异常（兼容 std::string）
- `bool reserve_ex(size_t add_capacity)` 修改为返回 bool：
  - 调用 `reserve()` 进行扩容
  - 捕获 `std::bad_alloc` 异常并返回 false
  - 成功时返回 true

**5. 内存分配失败处理**
修改 `allocate()` 和 `reallocate()` 方法：
- 检查 `std::malloc` 返回值，失败时抛出 `std::bad_alloc` 异常
- 避免空指针运算导致的未定义行为

### 修改文件

- `include/jstring.hpp`：
  - 更新 UnsafeStringConcept 文档中的 reserve_ex 接口说明
  - BufferView 添加 reserve_ex(size_t n) 重载返回 bool
  - UnsafeBuffer 添加 reserve_ex(size_t n) 方法，始终返回 true
  - StringBuffer 修改 reserve_ex(size_t n) 返回 bool，扩容失败返回 false
  - allocate() 和 reallocate() 检查 malloc 返回值，失败时抛 std::bad_alloc

### 测试结果

编译成功，所有现有测试用例通过。

## TASK:20251229-122441
-----------------------

**关联需求**: 2025-12-29/1 - 单元测试重构

### 完成的工作

**1. 创建独立的 utdocs 可执行文件**
- 在 `utest/CMakeLists.txt` 中创建新的可执行目标 `utdocs`
- 从 `utwwjson` 中移除 `t_usage.cpp`
- `utdocs` 只包含 `t_usage.cpp`，不依赖 `test_util.cpp` 或其他库
- 添加 `WWJSON_USE_SIMPLE_FLOAT_FORMAT=1` 编译定义

**2. 更新 CI 流水线**
- 在 `.github/workflows/ci-unit.yml` 中增加 "Run usage docs tests" 步骤
- 执行命令：`./build-release/utest/utdocs --cout=silent`
- 暂不关联手动触发的 `$TEST_ARGS` 参数

**3. 创建 t_bufferview.cpp 并迁移测试**
- 创建新文件 `utest/t_bufferview.cpp`，包含 BufferView 和 UnsafeBuffer 类的测试
- 迁移的 BufferView 基类测试（7个用例）：
  - `bufv_layout` - BufferView 布局大小测试
  - `bufv_invariants` - BufferView 不变关系式测试
  - `bufv_constructors` - BufferView 构造函数测试
  - `bufv_move_constructor` - BufferView 移动构造测试
  - `bufv_write_methods` - BufferView 写入方法测试
  - `bufv_borrow_string_resize` - BufferView 借用 std::string 并验证 resize 问题
  - `bufv_borrow_vector_resize` - BufferView 借用 std::vector<char> 并验证 resize 问题
- 迁移的 UnsafeBuffer 子类测试（3个用例）：
  - `ubuf_constructors` - UnsafeBuffer 继承构造函数测试
  - `ubuf_move_constructor` - UnsafeBuffer 移动构造测试
  - `ubuf_write_methods` - UnsafeBuffer 写入方法测试
- 从 `t_jstring.cpp` 中移除已迁移的测试用例（删除第12-696行）
- 在 `utest/CMakeLists.txt` 中添加 `t_bufferview.cpp` 到 `utwwjson` 目标

**4. 添加 JSTRING_MAX_EXP_ALLOC_SIZE 宏定义**
- 在 `utest/CMakeLists.txt` 中为 `utwwjson` 添加编译定义：
  - `JSTRING_MAX_EXP_ALLOC_SIZE=1024`（默认 8M，设置为 1024 更容易测试内存增长策略）

**5. 优化 t_jstring.cpp 测试用例**
- 将所有测试用例名的前缀从 `jstring_` 缩短为 `jstr_`
- 修改了22个测试用例名称（从 jstring_* 到 jstr_*）
- 测试用例数量保持不变，通过所有测试

**6. 更新文档**
- 更新 `utest/README.md`：
  - 增加 `t_jstring.cpp` - JString 字符串缓冲类测试
  - 增加 `t_bufferview.cpp` - BufferView 和 UnsafeBuffer 基类测试
  - 增加 `utdocs` 可执行文件的说明
  - 移除 `t_usage.cpp`（因为现在单独编译为 utdocs）

### 测试结果

- `utwwjson` 编译成功，108个测试用例全部通过
- `utdocs` 编译成功，19个测试用例全部通过
- 所有迁移的 BufferView 和 UnsafeBuffer 测试用例通过
- 所有重命名的 JString 测试用例通过

### 修改的文件

- `utest/CMakeLists.txt`：
  - 创建 `utdocs` 可执行目标
  - 添加 `JSTRING_MAX_EXP_ALLOC_SIZE=1024` 编译定义
  - 添加 `t_bufferview.cpp` 到 `utwwjson` 目标

- `.github/workflows/ci-unit.yml`：
  - 增加 "Run usage docs tests" 步骤

- `utest/t_bufferview.cpp`（新文件）：
  - 包含所有 BufferView 和 UnsafeBuffer 相关测试

- `utest/t_jstring.cpp`：
  - 移除 BufferView 和 UnsafeBuffer 测试用例
  - 重命名所有 `jstring_` 前缀为 `jstr_`

- `utest/README.md`：
  - 更新测试文件列表
  - 添加可执行文件说明

### 遗留工作（建议作为后续任务）

**1. 进一步优化测试用例**
根据分析，以下测试用例可以合并：
- `jstr_basic_construct` + `jstr_memory_alignment`：构造和容量相关
- `jstr_append_std_string` + `jstr_append_string_view`：不同类型字符串追加
- `jstr_to_string_view` + `jstr_to_string`：类型转换
- `jstr_front_back_access` + `jstr_clear_operation` + `jstr_c_str_termination`：简单 API 测试

**2. 补充测试覆盖**
建议添加以下新测试用例（本次任务只建议，未实际添加）：
- `jstr_string_capacity_overflow`：测试容量溢出时的行为
- `jstr_large_string_performance`：大字符串性能测试
- `jstr_mixed_safe_unsafe_operations`：混合使用安全和不安全操作

## TASK:20251229-185409
-----------------------

### 关联需求

需求 ID：2025-12-29/2 - BufferView 基类测试用例完善

### 任务概述

完善 `utest/t_bufferview.cpp` 文件中添加的几个空实现测试用例，根据注释完成测试代码。

### 完成内容

**1. 实现 bufv_ends_access 测试**
- 测试边缘指针访问：begin()、end()、cap_end()
- 测试元素访问：front()、back()
- 测试 operator bool() 转换
- 测试空缓冲区状态
- 测试元素修改
- 正确处理指针比较：使用 static_cast<void*> 转换避免被当作字符串

**2. 实现 bufv_str_converstion 测试**
- 测试 c_str() 转换（添加 null 终止符）
- 测试 str() 方法转换为 std::string
- 测试隐式 string_view 转换
- 测试显式 std::string 转换
- 测试空缓冲区和 null 缓冲区的 c_str() 行为
- 测试多种转换方式保持一致性

**3. 实现 bufv_write_unsafe 测试**
- 测试 unsafe_push_back：添加单个字符
- 测试 unsafe_append：添加字符串（带长度）
- 测试 unsafe_fill：填充字符
- 测试 unsafe_resize：调整大小
- 测试 unsafe_set_end：设置结束指针
- 测试 unsafe_end_cstr：添加 null 终止符
- 测试溢出场景：使用 buffer[n+m] 保护，构造时只传 n，测试语句可溢出 n 但不溢出 n+m

**4. 实现 bufv_append_string 测试**
- 测试 append(c-string)：C 字符串追加
- 测试 append(c-string with length)：带长度追加
- 测试 append(std::string)：标准字符串追加
- 测试 append(std::string_view)：字符串视图追加
- 测试 append(BufferView)：追加另一个 BufferView
- 测试 append(JString)：追加 JString
- 测试连续追加多种字符串类型
- 测试 append(nullptr) 应被忽略

**5. 实现 bufv_extern_write 测试**
- 测试 std::to_chars 写入数字：直接在 BufferView 内存中写入，然后调用 resize 或 set_end 调整 size
- 测试 snprintf 格式化写入：使用 snprintf 写入格式化字符串，调整 size
- 测试 memcpy 数据拷贝：直接拷贝数据到 BufferView，调整 size
- 测试 strcpy/strncpy：使用标准字符串函数，调整 size
- 测试 set_end 和 resize 的区别：unsafe_set_end 和 resize 的不同使用场景

**6. 实现 bufv_null_end 测试**
- 测试初始化行为：仅 cap_end 置零，其他内容不清零
- 测试 push_back 后不保证 null 终止符
- 测试 append 后不保证 null 终止符
- 测试显式 end_cstr() 添加 null 终止符
- 测试 c_str() 保证 null 终止符
- 测试无空间时 end_cstr() 的行为
- 测试多次写入后需要显式调用 end_cstr()
- 测试 clear 不重置 cap_end null 终止符
- 测试 resize 保留 cap_end null 终止符
- 使用 fill('@') 填充已知字符，使用 COUT_HEX() 观察十六进制内容
- 正确使用 COUT_HEX 单参数形式观察数据，避免写 COUT(*bv.cap_end(), '\0') 打印空字符

**7. 添加头文件**
- 在 `t_bufferview.cpp` 中添加 `#include <charconv>` 以支持 std::to_chars

### 测试结果

- 编译成功
- 所有 13 个 BufferView 测试用例全部通过
- 测试覆盖 BufferView 的核心功能和边界情况

### 设计要点

1. **指针比较**：使用 static_cast<void*> 避免被当作字符串比较
2. **溢出保护**：使用 buffer[n+m] 构造时传 n，允许测试溢出但保护真实内存
3. **内存填充**：使用 fill('@') 填充已知字符便于观察内存内容
4. **十六进制输出**：COUT_HEX 单参数形式观察二进制数据，避免打印空字符
5. **外部写入**：验证 std::to_chars、snprintf、memcpy 等外部函数与 BufferView 的集成
6. **null 终止符**：详细测试各种情况下的 null 终止符行为

### 修改文件

- `utest/t_bufferview.cpp`：
  - 添加 `#include <charconv>` 头文件
  - 实现 6 个空测试用例：bufv_ends_access、bufv_str_converstion、bufv_write_unsafe、bufv_append_string、bufv_extern_write、bufv_null_end


## TASK:20251229-190504
-----------------------

**关联需求**: 2025-12-29/1 - JString 单元测试重构与完善

### 任务概述

继续完成 2025-12-29/1 需求的第二部分工作。第一部分已完成单元测试结构重组（TASK:20251229-122441），本任务重点完成 JString 测试用例的重构、合并和新测试用例实现。

### 完成内容

**1. 实现完整的 jstr_capacity_growth 测试用例**
- 指数增长测试：验证容量从最小值开始按 2 倍增长直到 1024 字节
- 线性增长测试：验证超过 1024 字节后每次增加 1024 字节
- reserve() 扩容测试：验证通过 reserve() 方法触发的扩容行为
- append() 自动扩容测试：验证通过 append() 方法触发的自动扩容

**2. 大规模测试用例重构**
使用 DESC 子测试块重新组织测试，提高可读性和可维护性：

- **jstr_construct**（重命名自 jstr_basic_construct）
  - 拆分为 3 个 DESC 子测试：默认构造、带容量构造、最小容量构造
  - 合并原 jstr_memory_alignment 测试内容
  - 增加对齐和容量的详细验证

- **jstr_push_char**（重命名自 jstr_push_back）
  - 拆分为 3 个 DESC 子测试：push back、append count of char、append count=0
  - 合并原 jstr_append_count_char 测试内容

- **jstr_append_string**
  - 拆分为 3 个 DESC 子测试：C 风格字符串、std::string、std::string_view
  - 合并原 jstr_append_std_string 和 jstr_append_string_view 测试

- **jstr_unsafe_levels**（重命名自 jstr_different_unsafe_levels）
  - 保留原有逻辑，更新测试用例名称
  - 合并原 jstr_kunsafelevel_semantics 测试内容

- **jstr_edge_cases**
  - 合并原 jstr_clear_operation、jstr_front_back_access、jstr_c_str_termination 的测试内容

**3. 删除冗余和已合并的测试用例**
- jstr_front_back_access：简单 API 测试，合并到 jstr_edge_cases
- jstr_clear_operation：清空操作测试，合并到 jstr_edge_cases
- jstr_c_str_termination：空字符结尾测试，合并到相关测试
- jstr_kunsafelevel_semantics：合并到 jstr_unsafe_levels
- jstr_memory_alignment：合并到 jstr_construct
- jstr_to_chars_integration：整合到新的 jstr_extern_write
- jstr_to_string_view：类型转换测试，合并到 jstr_append_string
- jstr_to_string：类型转换测试，合并到 jstr_append_string
- jstr_fill：填充方法测试，合并到 jstr_push_char
- jstr_append_count_char：字符重复追加测试，合并到 jstr_push_char
- jstr_buffer_view：基类视图测试，已迁移到 t_bufferview.cpp

**4. 新增 jstr_extern_write 测试用例**
整合外部方法写入测试：
- DESC 1：使用 std::to_chars 写入数字
- DESC 2：使用 snprintf 格式化写入字符串

**5. 更新现有测试用例**
- jstr_reserve：保留并优化测试逻辑
- jstr_unsafe_operations：保留并保持原有测试结构
- jstr_json_patterns：保留 JSON 序列化模式测试
- jstr_copy_move：保留复制和移动语义测试
- jstr_edge_cases：保留并整合边界情况测试

### 测试结果

编译成功，所有 102 个测试用例全部通过：
- BufferView/UnsafeBuffer 基类测试：10 个用例
- JString 测试用例：92 个用例

### 代码统计

- 删除：549 行
- 新增：430 行
- 净减少：119 行
- 测试用例数量：从 108 个精简到 102 个

### 设计改进

1. **测试组织**：使用 DESC 子测试块提高测试可读性
2. **减少冗余**：合并相似测试用例，消除重复代码
3. **一致性**：统一使用 jstr_ 前缀命名
4. **完整性**：保留所有核心功能测试，确保测试覆盖率

### 修改文件

- `utest/t_jstring.cpp`：
  - 实现 jstr_capacity_growth 测试用例
  - 重构现有测试用例，使用 DESC 子测试块
  - 合并和删除冗余测试用例
  - 新增 jstr_extern_write 测试用例

## TASK:20251230-103330
-----------------------

**关联需求**: 2025-12-30/1 - 扩展 jstring.hpp 中 StringBuffer 最大等级的特化

### 任务概述

为 `StringBuffer<255>` 实现单次内存分配特化，添加类型别名 `KString`。通过 `if constexpr` 条件编译实现最小代码修改。

### 完成内容

**1. 核心特化实现** (`include/jstring.hpp`)
- `reserve_ex(size_t add_capacity)`: LEVEL < 0xFF 时正常扩容，否则直接返回 true
- `reserve(size_t new_capacity)`: LEVEL < 0xFF 时正常扩容，否则为空操作

**2. 新增类型别名**
```cpp
using KString = StringBuffer<255>;
```

**3. 新增测试用例** (`utest/t_jstring.cpp`)
- `kstr_construct`: KString 基础构造测试
  - KString(0) 最少申请 256 字节，容量 255
  - KString 默认构造 1024 字节
  - KString(4k) 传较大初始容量
  - 基本写入操作验证

- `kstr_reach_full`: KString 写满对比测试
  - 相同初始容量，KString 写满后 full()=true 不会扩容
  - JString 和 StringBuffer<254> 写满后会自动扩容
  - KString 多次追加不扩容验证

### 测试结果

所有 104 个测试用例全部通过。

### 修改文件

- `include/jstring.hpp`: 添加 if constexpr 条件分支实现特化
- `utest/t_jstring.cpp`: 新增 kstr_construct 和 kstr_reach_full 测试用例

### TASK:20251230-165622
------------------------

完成 `2025-12-30/3` 需求：增加 `jbuilder.hpp` 组合使用 `jstring.hpp`

**主要变更：**

1. `include/wwjson.hpp`:
   - 新增 `unsafe_level<T>` 编译期特征萃取 trait
   - 新增 `unsafe_level_v<T>` 便捷变量模板
   - `GenericBuilder` 添加 `string_type` 类型别名
   - 优化 `kUnsafeLevel` 注释文档，更准确地描述其含义

2. `include/jstring.hpp`:
   - `BufferView` 添加 `pop_back()` 方法
   - `StringBuffer<255>` 的 `reserve()` 允许显式扩容（方案A）
   - 优化 `UnsafeStringConcept` 和 `StringBuffer` 的注释文档

3. `include/jbuilder.hpp` (新增):
   - 组合 `wwjson.hpp` 与 `jstring.hpp`
   - 定义 `Builder` = `GenericBuilder<JString>`
   - 定义 `FastBuilder` = `GenericBuilder<KString>` 及其 RAII 包装器

4. `utest/t_jbuilder.cpp` (新增):
   - `jbuilder_unsafe_level`: 测试 `unsafe_level` 萃取功能
   - `jbuilder_basic`: 测试 `Builder` 基本功能（含 `AddMemberEscape`）
   - `jbuilder_nested`: 测试嵌套结构
   - `jbuilder_raii`: 测试 `JObject`/`JArray` RAII 包装器
   - `jbuilder_fast_basic`: 测试 `FastBuilder` 多种构造方式

5. `utest/CMakeLists.txt`:
   - 添加 `t_jbuilder.cpp` 测试文件

**测试结果:** 全部 5 个新增测试用例通过。

### TASK:20251230-234228
------------------------

完成 `2025-12-30/4` 需求：重设 KString 最大不安全等级的意义

**主要变更：**

1. `include/jstring.hpp`:
   - `reserve_ex()` 方法移除 `LEVEL < 0xFF` 条件，允许显式调用扩容
   - `reserve()` 方法移除 `LEVEL < 0xFF` 条件，允许显式调用扩容
   - `append()` 方法添加 `LEVEL < 0xFF` 判断，KString 不会隐式扩容
   - `push_back()` 方法添加 `LEVEL < 0xFF` 判断，KString 不会隐式扩容
   - `resize()` 方法添加 `LEVEL < 0xFF` 判断，KString 不会隐式扩容
   - 更新 KString 相关注释说明

2. `utest/t_jstring.cpp`:
   - `kstr_reach_full` 测试新增显式 reserve 扩容测试用例

**测试结果:** 所有测试用例通过。

## TASK:20260104-001348
-----------------------

**关联需求**: 2026-01-03/1 - wwjson.hpp 根据 unsafe level 重构 GenericBuilder

### 任务概述

根据 `unsafe_level<stringT>` 配置，将逗号、冒号、引号的写入操作重构为使用不安全方法（当 unsafe_level >= 4 时），以提升性能。括号 `[]` 和 `{}` 仍必须使用安全的 `PutChar` 方法以保持结构完整性。

### 完成内容

**1. 新增 UnsafePutChar 方法** (`include/wwjson.hpp:808-828`)
- 添加了 `UnsafePutChar(char c)` 方法到 M1 区域
- 使用 `if constexpr (unsafe_level_v<stringT> >= 4)` 编译期条件判断
- unsafe_level >= 4 时调用 `json.unsafe_push_back(c)`
- 否则调用安全的 `json.push_back(c)`
- 添加了详细的文档说明设计理由和使用场景

**2. 替换逗号写入** (`include/wwjson.hpp:870`)
- `PutNext()`: `PutChar(',')` → `UnsafePutChar(',')`
- `SepItem()` 通过 `PutNext()` 间接使用 `UnsafePutChar(',')`

**3. 替换冒号和引号写入** (`include/wwjson.hpp:1084-1094`)
- `PutKey()` 方法:
  - 开引号: `PutChar('"')` → `UnsafePutChar('"')`
  - 冒号: `PutChar(':')` → `UnsafePutChar(':')`
  - 闭引号: `PutChar('"')` → `UnsafePutChar('"')`

**4. 替换字符串值的引号写入** (`include/wwjson.hpp:1045,1054`)
- `PutValue(const char*, size_t)`:
  - 开引号: `PutChar('"')` → `UnsafePutChar('"')`
  - 闭引号: `PutChar('"')` → `UnsafePutChar('"')`

**5. 替换 AddItemEscape 的引号写入** (`include/wwjson.hpp:1282,1284`)
- `AddItemEscape()`:
  - 开引号: `PutChar('"')` → `UnsafePutChar('"')`
  - 闭引号: `PutChar('"')` → `UnsafePutChar('"')`

**6. 替换 AddMemberEscape 的引号和冒号** (`include/wwjson.hpp:1320,1331,1332`)
- `AddMemberEscape()`:
  - 开引号: `PutChar('"')` → `UnsafePutChar('"')`
  - 闭引号: `PutChar('"')` → `UnsafePutChar('"')`
  - 冒号: `PutChar(':')` → `UnsafePutChar(':')`

**7. 替换 AddItem 数字引用的引号写入** (`include/wwjson.hpp:1147,1149,1164,1166`)
- `AddItem(numberT)` (kQuoteNumber 模式):
  - 开引号: `PutChar('"')` → `UnsafePutChar('"')`
  - 闭引号: `PutChar('"')` → `UnsafePutChar('"')`
- `AddItem(numberT, bool)` (强制引用):
  - 开引号: `PutChar('"')` → `UnsafePutChar('"')`
  - 闭引号: `PutChar('"')` → `UnsafePutChar('"')`

**8. 验证括号仍使用安全方法**
确认以下括号操作保持使用 `PutChar()`:
- `BeginArray()`: `PutChar('[')`
- `EndArray()`: `PutChar(']')`
- `BeginObject()`: `PutChar('{')`
- `EndObject()`: `PutChar('}')`
- `BeginRoot()`: `PutChar(bracket)`
- `EndRoot()`: `PutChar(bracket)`

### 技术细节

**unsafe_level 行为:**
- `unsafe_level < 4`: 使用安全的 `push_back()`（如 std::string, BufferView）
- `unsafe_level >= 4`: 使用不安全的 `unsafe_push_back()`（如 StringBuffer<4>, StringBuffer<255>）
- 不安全方法跳过容量检查，前提是已预留足够的不安全边距

**性能优化目标:**
- 对于具有高 unsafe_level 的字符串类型（如 KString），减少频繁的容量检查
- 格式字符（逗号、冒号、引号）写入频率高，优化收益明显
- 结构字符（括号）保持安全写入，确保 JSON 结构正确性

### 测试结果

所有 109 个测试用例全部通过，验证了重构的正确性：
- 基本功能测试：通过
- 数字序列化测试：通过
- 作用域管理测试：通过
- 自定义字符串类型测试：通过
- 字符串转义测试：通过
- 高级特性测试：通过
- 操作符重载测试：通过
- 自定义构建器测试：通过

### 修改文件

- `include/wwjson.hpp`:
  - 新增 `UnsafePutChar()` 方法（约 20 行）
  - 修改 8 处 `PutChar()` 调用为 `UnsafePutChar()`
  - 保持所有括号操作使用安全 `PutChar()`

## TASK:20260104-141057
------------------------

完成 2026-01-04/1 需求：测试 UnsafePutChar 写格式字符提升性能情况

### 实施内容

1. 在 `perf/p_builder.cpp` 新增两个 JSON 构建函数：
   - `BuildJsonJString()` 使用 `wwjson::Builder` (`GenericBuilder<JString>`)
   - `BuildJsonKString()` 使用 `wwjson::FastBuilder` (`GenericBuilder<KString>`)

2. 新增两个相对性能测试类：
   - `BuildJsonJStringRelativeTest`: 比较 std::string (RawBuilder) vs JString (Builder)
   - `BuildJsonKStringRelativeTest`: 比较 std::string (RawBuilder) vs KString (FastBuilder)

3. 合并命名空间，避免重复声明 `namespace test::perf`

4. JString/KString 测试添加显式验证：确认两种方式生成的 JSON 字符串完全相同

### 性能测试结果

- **JString vs std::string**: JString 快 6.6% ~ 28.7%
- **KString vs std::string**: KString 快 19.4% ~ 30.1%

测试全部通过（45 PASS, 0 FAIL）

## TASK:20260104-160928
-----------------------

完成 2026-01-04/2 需求：优化字符串转义方法

### 实施内容

1. **创建 UnsafeConfig 类** (`include/jbuilder.hpp`)
   - 继承自 `BasicConfig<stringT>`
   - 覆盖 `EscapeString` 静态方法
   - 当 `unsafe_level<stringT> >= 4` 时，使用不安全操作优化

2. **UnsafeConfig::EscapeString 优化策略**:
   - 预申请 2 倍空间 `dst.reserve(dst.size() + len * 2)`
   - 直接向目标字符串的内部缓冲区写入（指针算术）
   - 使用 `dst.unsafe_set_end(write_ptr)` 更新结束位置
   - 对于低 unsafe_level 类型，回退到父类安全实现

3. **更新类型别名使用 UnsafeConfig**:
   - `Builder` = `GenericBuilder<JString, UnsafeConfig<JString>>`
   - `FastBuilder` = `GenericBuilder<KString, UnsafeConfig<KString>>`
   - `JObject` / `JArray` / `FastObject` / `FastArray` 相应更新

### 测试结果

- 编译成功
- 所有 109 个测试用例全部通过

### 修改文件

- `include/jbuilder.hpp`: 添加 `UnsafeConfig` 模板类，更新所有类型别名


## TASK:20260104-175151
-----------------------

### 任务内容

扩展 p_string.cpp 性能测试，比较 RawBuilder(std::string) vs Builder(JString) vs FastBuilder(KString)

### 实施过程

1. 添加 JString/KString 构建函数 `BuildStringObjectJString` `BuildStringObjectKString` `BuildEscapeObjectJString` `BuildEscapeObjectKString`

2. 添加相对性能测试类 `StringObjectJStringRelativeTest` `StringObjectKStringRelativeTest` `EscapeObjectJStringRelativeTest` `EscapeObjectKStringRelativeTest`

3. 扩展 `string_object_relative` 和 `string_escape_relative` 测试用例

4. **修复 Bug**: 初始实现传入 `KString` 对象给 FastBuilder 构造函数，导致 StringBuffer 拷贝。拷贝构造函数只分配 `size()` 而非 `capacity()` 空间，加上默认 Reserve(1024) 不足，引发写越界崩溃。改为直接使用 `FastBuilder(capacity)` 构造。

### 测试结果

- wwjson RawBuilder vs yyjson: wwjson 约 7-12% 更快
- std::string vs JString: JString 约 5-18% 更快
- std::string vs KString: KString 约 5-23% 更快

### 修改文件

- `perf/p_string.cpp`: 新增 4 个构建函数、4 个测试类、扩展 2 个测试用例

## TASK:20260104-223107
-----------------------

### 任务内容

优化 `NumberWriter::Output` 浮点数版实现，减少临时 buffer 大小，支持直接写入 StringBuffer

### 实施过程

1. **分析 buffer 大小需求**
   - float: %.9g 格式，最长约 17 字节
   - double: %.17g 格式，最长约 25 字节
   - long double: %.21Lg 格式，最长约 53 字节
   - 结论: buffer[256] 过于保守，改为 buffer[64] 已足够

2. **优化浮点数序列化逻辑**
   - 将 `buffer[256]` 改为 `static thread_local char buffer[64]`
   - 使用 `constexpr size_t kFloatBufferSize = 64` 统一管理 buffer 大小
   - 根据 `unsafe_level_v<stringT>` 智能选择写入目标:
     - unsafe_level < 4: 写入 thread_local buffer，再 append 到目标
     - unsafe_level >= 4: 直接预留空间写入目标字符串末尾
   - 使用 `if constexpr` 编译期判断是否支持 `std::to_chars`
   - 统一格式化逻辑，避免代码重复

3. **关键优化点**
   - 减少栈内存使用: 256 → 64 字节
   - 支持 StringBuffer 直接写入: 避免临时 buffer 拷贝
   - thread_local 保证线程安全
   - 逻辑简化: to_chars/snprintf 逻辑复用，仅前后根据 unsafe level 处理

### 测试结果

运行 `make test` 回归测试，全部 109 个测试用例通过。

### 技术细节

**优化前问题**:
- buffer[256] 过大，浪费栈空间
- 即使使用 StringBuffer，也需要临时 buffer 拷贝

**优化后改进**:
- buffer[64] 更合理，仍留有足够安全余量
- unsafe_level >= 4 时直接写入 StringBuffer，减少拷贝
- 代码更简洁，可维护性更好

### 修改文件

- `include/wwjson.hpp`: 优化 `NumberWriter::Output` 浮点数版实现

## TASK:20260105-142249
-----------------------

### 任务内容

扩展 p_number.cpp 数字序列化相对性能测试，比较 RawBuilder(std::string) vs Builder(JString) vs FastBuilder(KString)

### 实施内容

1. **扩展 `number_int_rel` 测试用例**:
   - 新增 `RandomIntJStringRel` 测试类：比较 std::string vs JString
   - 新增 `RandomIntKStringRel` 测试类：比较 std::string vs KString
   - 原有的 `RandomIntArray` 保留wwjson vs yyjson 比较

2. **扩展 `number_double_rel` 测试用例**:
   - 新增 `RandomDoubleJStringRel` 测试类：比较 std::string vs JString
   - 新增 `RandomDoubleKStringRel` 测试类：比较 std::string vs KString
   - 原有的 `RandomDoubleArray` 保留wwjson vs yyjson 比较

3. **容量预估优化**:
   - 所有相对测试类新增 `estimateCapacity()` 方法
   - 通过一次预构建获取实际输出大小，保存为 `capacity` 成员
   - `methodA` 和 `methodB` 使用精确容量，避免额外分配

### 测试结果

- 整数测试：JString 快 ~5%，KString 快 ~42%
- 浮点数测试：JString 与 std::string 基本持平，KString 也基本持平
- yyjson 浮点序列化性能远优于 wwjson (~22倍差距)

### 修改文件

- `perf/p_number.cpp`: 新增 4 个测试类，扩展 2 个测试用例
- 包含 `jbuilder.hpp` 以使用 Builder/FastBuilder 类型

## TASK:20260105-173242
-----------------------

### 任务内容

性能测试用例管理优化：将绝对性能测试用例从 DEF_TAST 改为 DEF_TOOL

### 分类原则

- **DEF_TAST** (相对性能测试): 使用 `RelativeTimer::runAndPrint()` 方法，比较两种方法的性能比
- **DEF_TOOL** (工具类测试): 使用 `TIME_TIC/TIME_TOC` 计时，仅测量绝对执行时间

### 实施内容

1. **p_number.cpp**: 13 个绝对时间测试改为 DEF_TOOL
   - number_int8/16/32/64_wwjson/yyjson
   - number_float/double_wwjson/yyjson
   - number_array_compare

2. **p_string.cpp**: 6 个绝对时间测试改为 DEF_TOOL
   - string_array/object/escape_wwjson/yyjson
   - string_compare

3. **p_builder.cpp**: 10 个绝对时间测试改为 DEF_TOOL
   - build_0_5k/1k/10k/100k_wwjson/yyjson
   - build_ex_wwjson/yyjson
   - build_sample/verify/size

4. **保留为 DEF_TAST**:
   - p_design.cpp: 7 个相对性能测试
   - p_api.cpp: 5 个相对性能测试
   - p_builder.cpp: build_relative
   - p_string.cpp: string_object_relative, string_escape_relative
   - p_number.cpp: number_int_rel, number_double_rel

5. **更新文档**:
   - perf/README.md: 添加用例分类说明
   - perf/cases.md: 用 `make perf/list` 自动更新

### 测试结果

运行 `./build-release/perf/pfwwjson --all` 验证所有用例正常执行

### 修改文件

- `perf/p_number.cpp`: 13 个 DEF_TAST → DEF_TOOL
- `perf/p_string.cpp`: 6 个 DEF_TAST → DEF_TOOL
- `perf/p_builder.cpp`: 10 个 DEF_TAST → DEF_TOOL
- `perf/README.md`: 添加用例分类说明
- `perf/cases.md`: 自动更新用例列表

## TASK:20260106-010124
-----------------------

### 任务内容

设计几个 example 示例应用：增加 example/ 子目录，设计三个示例 .cpp 程序，不依赖其他三方库，根目录的 CMakeLists.txt 增加编译选项默认可编译。

### 实施内容

1. **创建 example/ 子目录**:
   - 新增目录用于存放示例程序

2. **实现示例一：多层结构体转 json (example/struct_to_json.cpp)**:
   - 设计嵌套结构体：Address → Publisher → Author → Book
   - 每个结构体提供 `toJson(RawBuilder&) const` 方法
   - 顶层 Book 结构体额外提供 `toJson() const` 无参重载，返回 `std::string`
   - 使用 AddMember("key", lambda) 添加嵌套对象和数组
   - 嵌套的 toJson() 方法不调用 BeginObject()/EndObject()（由 lambda 自动处理）
   - main 方法实例化数据并调用 toJson() 转换输出

3. **实现示例二：估算待构建 json 大小的自定义 Builder (example/estimate_size.cpp)**:
   - 设计 `EString` 特殊字符串类，`c_str()` 返回 `nullptr`，append 只累加长度
   - 设计 `EConfig` 继承 `BasicConfig<EString>`，重写 EscapeString 和 NumberString 估算长度
     - EscapeString：按最大 2 倍估算长度
     - NumberString：32位整数估算11字符，64位整数估算21字符，浮点数估算25字符
   - 定义 `EstBuilder` 别名为 `GenericBuilder<EString, EConfig>`
   - 使用 `wwjson::FastBuilder`（基于 KString，不扩容），传入估算的容量
   - 对比估算大小与实际大小，演示容量预分配的意义

4. **实现示例三：十六进制表示的 json (example/hex_json.cpp)**:
   - 设计模板化的 `HexConfig<stringT>` 继承 `BasicConfig<stringT>`
     - `kEscapeValue = true`：启用字符串转义
     - `kQuoteNumber = true`：数字加引号
     - EscapeString：将字符串每个字节转为2个十六进制字符
     - NumberString：整数转为十六进制加 `0x` 前缀，浮点数用 `%g` 格式
   - 使用 `HexConfig<std::string>` 和 `JString` 构建示例 JSON
   - 展示十六进制转换结果

5. **修改 CMakeLists.txt**:
   - 添加 `option(BUILD_EXAMPLES "Build example programs" ON)`
   - 在 `add_subdirectory(example)` 前检查 `BUILD_EXAMPLES`

6. **创建 example/CMakeLists.txt**:
   - 为三个示例分别创建可执行文件
   - 添加编译定义 `WWJSON_USE_SIMPLE_FLOAT_FORMAT=1`
   - 链接 wwjson 库
   - 创建 `examples` 和 `run_examples` 自定义目标

### 测试结果

所有三个示例程序编译通过并运行正常，输出合法的 JSON：

1. **示例1 输出**: 
```json
{"title":"Modern C++ Design","isbn":"978-0-201-70431-0","price":49.99,"publishedYear":2001,"pageCount":352,"genre":"Programming","authors":[{"name":"Andrei Alexandrescu","email":"andrei@example.com","publisher":{"name":"Addison-Wesley Professional","address":{"street":"75 Arlington Street","city":"Boston","country":"USA"}}},{"name":"Scott Meyers","email":"scott@example.com","publisher":{"name":"Addison-Wesley Professional","address":{"street":"75 Arlington Street","city":"Boston","country":"USA"}}}]}
```

2. **示例2 输出**:
   - 估算大小: 234 字节
   - 实际大小: 198 字节
   - 估算比率: 118.182% (保守高估)
   - FastBuilder 使用估算的容量预分配，避免扩容

3. **示例3 输出**:
   - 字符串转十六进制: "Hello" → "48656c6c6f"
   - 整数转十六进制: 10 → "0xa", 255 → "0xff"
   - 浮点数保持 %g 格式: 3.14 → "3.14"

### 修改文件

- 新增: `example/struct_to_json.cpp`
- 新增: `example/estimate_size.cpp`
- 新增: `example/hex_json.cpp`
- 新增: `example/CMakeLists.txt`
- 修改: `CMakeLists.txt` - 添加 BUILD_EXAMPLES 选项和 example 子目录

## TASK:20260106-164348
-----------------------

### 任务内容

多个头文件安装优化：根目录 CMakeLists.txt 修改为安装所有头文件到 wwjson/ 子目录；example 源码改为 `<wwjson/wwjson.hpp>` 引用方式；创建外部集成示例和 CI 工作流验证。

### 实施内容

1. **修改根 CMakeLists.txt 安装规则**:
   - `target_include_directories` 的 INSTALL_INTERFACE 改为 `include/wwjson`
   - 安装所有 `include/*.hpp` 文件到 `wwjson/` 子目录

2. **修改 example 源码引用方式**:
   - `struct_to_json.cpp`: `#include <wwjson.hpp>` → `#include <wwjson/wwjson.hpp>`
   - `estimate_size.cpp`: 同样改为 `wwjson/wwjson.hpp` 和 `wwjson/jbuilder.hpp`
   - `hex_json.cpp`: 同样改为 `wwjson/wwjson.hpp` 和 `wwjson/jstring.hpp`

3. **创建 example/find_package/ 独立 CMakeLists.txt**:
   - 用于演示系统安装后 `find_package(wwjson)` 使用方式
   - 源文件通过 `${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp` 引用

4. **创建 example/fetch_content/ 独立 CMakeLists.txt**:
   - 用于演示不安装情况下 `FetchContent` 集成方式
   - GIT_TAG 暂时设为 `dev`（开发期间）

5. **创建 .github/workflows/ci-example.yml**:
   - **local-build**: 在 build/ 目录本地编译 example
   - **find-package**: 先安装 wwjson 到 `$HOME/include`，再编译
   - **fetch-content**: 使用 FetchContent 自动下载集成
   - 触发条件：监听 `include/`、`example/`、CI 文件变更

6. **解决本地编译的目录结构问题**:
   - example/CMakeLists.txt 使用 CMake 脚本在 build/include 下创建软链接
   - `build/include/wwjson` → `../include`
   - 每个 example target 添加 `${CMAKE_BINARY_DIR}/include` 到 include 路径

7. **删除 WWJSON_USE_SIMPLE_FLOAT_FORMAT**:
   - 从所有 CMakeLists.txt 中移除该编译定义
   - 该宏仅用于单元测试比较浮点数格式，实践中不推荐使用

8. **更新 docs 配置**:
   - docs/makefile: `include/wwjson.hpp` → `include/*.hpp`
   - Doxyfile: INPUT 添加所有头文件

### 测试结果

- 本地编译 example：通过软链接方案成功编译运行
- 单元测试：通过 (109 PASS, 0 FAIL)
- 所有 example 程序正常运行输出正确 JSON

### 修改文件

- 修改: `CMakeLists.txt` - 安装路径和 include 目录配置
- 修改: `example/CMakeLists.txt` - 添加软链接支持，删除 WWJSON_USE_SIMPLE_FLOAT_FORMAT
- 修改: `example/struct_to_json.cpp` - 头文件引用方式
- 修改: `example/estimate_size.cpp` - 头文件引用方式
- 修改: `example/hex_json.cpp` - 头文件引用方式
- 新增: `example/find_package/CMakeLists.txt`
- 新增: `example/fetch_content/CMakeLists.txt`
- 新增: `.github/workflows/ci-example.yml`
- 修改: `docs/makefile`
- 修改: `Doxyfile`

## TASK:20260106-233456
-----------------------

### 任务内容

测试验证定制配置只重载 EscapeString 方法的正确性：分析当 UnsafeConfig 覆盖 EscapeString 时，转义键是否会调用派生类版本；实现测试用例验证该行为，并修复 UnsafeConfig 类以正确支持键转义。

### 实施内容

**问题分析**

1. **C++ 静态方法的多态性问题**:
   - `BasicConfig::EscapeKey` 内部调用 `EscapeString(dst, key, len)`
   - C++ 静态方法不具备运行时多态性
   - 派生类覆盖 `EscapeString` 后，基类的 `EscapeKey` 仍调用 `BasicConfig::EscapeString`
   - 这是一个潜在的陷阱：开发者可能期望覆盖 `EscapeString` 后自动生效

2. **UnsafeConfig 缺失 EscapeKey 覆盖**:
   - `UnsafeConfig` 覆盖了 `EscapeString` 以优化字符串转义
   - 但没有覆盖 `EscapeKey`，导致转义键时调用基类版本
   - 这样键的转义不会使用优化版本

**解决方案**

1. **修改 UnsafeConfig 类** (`include/jbuilder.hpp`):
   - 添加显式覆盖 `EscapeKey` 方法
   - `EscapeKey` 内部调用派生类的 `EscapeString`
   - 添加详细注释说明为什么需要显式覆盖（静态方法无多态性）

2. **优化注释组织**:
   - 类级别注释简化，说明用途和适用场景
   - `EscapeKey` 方法注释解释多态性问题
   - `EscapeString` 方法注释描述具体优化策略

3. **创建测试用例** (`utest/t_escape.cpp`):
   - 定义 `IdentifierEscapeConfig` 配置类
   - `EscapeString` 将非字母数字字符转为下划线，使键名像编程语言标识符
   - 覆盖 `EscapeKey` 方法以确保键转义生效
   - 测试场景：
     * 键包含特殊字符（`"`, `@`, `#`, `-`, `.`）应转为下划线
     * `PutKey` 使用自定义转义
     * `AddMemberEscape` 强制转义值（值包含换行符应被转义）

### 测试结果

所有测试通过：
- `escape_ident_key` 用例 5 个断言全部通过
- 验证键名特殊字符正确转为下划线
- 验证 `PutKey` 使用自定义转义
- 验证 `AddMemberEscape` 强制转义值

### 修改文件

- 修改: `include/jbuilder.hpp` - `UnsafeConfig` 添加 `EscapeKey` 方法，优化注释
- 修改: `utest/t_escape.cpp` - 添加 `IdentifierEscapeConfig` 和 `escape_ident_key` 测试用例

## TASK:20260107-134537
-----------------------

**需求 ID**: 2026-01-07/1

**实现内容**:
- 在 `include/jbuilder.hpp` 添加 `wwjson::to_json` 模板函数系列
- 支持标量（字符串、数字、bool）使用 `AddMember/AddItem`
- 支持结构体自动包装 `BeginObject/EndObject` 并调用 `st.to_json(builder)`
- 支持顺序容器（vector 等）自动构建 JSON 数组
- 提供单参数入口函数 `wwjson::to_json(st)` 返回 `std::string`
- 提供 `TO_JSON(field)` 宏简化字段序列化
- 重构 `example/struct_to_json.cpp` 展示两种使用方式

**关键设计**:
- 使用 `if constexpr` 内部判断类型（标量/容器/结构体）
- 简化模板匹配逻辑，避免之前复杂的 SFINAE 检测
- 用户 struct 的 `to_json` 方法不再需要 `BeginObject/EndObject`，由 helper 自动处理

**测试结果**:
- 单元测试: 110 PASS
- 示例运行正常，输出正确 JSON


## TASK:20260107-163804
-----------------------

**需求 ID**: 2026-01-07/2

**任务内容**:
`wwjson::detail` 内部子空间优化：将内部辅助类型 traits 移入 detail 命名空间，改善代码组织结构。

**实施内容**:

1. **include/wwjson.hpp 修改**:
   - 移动 `is_key` / `is_key_v` 到 `wwjson::detail` 命名空间
   - 移动 `has_float_to_chars_v` / `use_simple_float_format` 到 detail
   - 移动 `unsafe_level` / `unsafe_level_v` 到 detail
   - 保留 `StringConcept` 在主命名空间（作为文档契约）
   - 更新所有引用这些 traits 的代码，改用 `detail::` 前缀

2. **include/jbuilder.hpp 修改**:
   - 移动 `is_sequence_container` / `is_sequence_container_v` 到 detail
   - 移动 `is_scalar_v` 到 detail
   - 更新相关引用

3. **测试文件更新**:
   - `utest/t_number.cpp`: 更新 `has_float_to_chars_v` / `use_simple_float_format` 引用
   - `utest/t_jbuilder.cpp`: 更新 `unsafe_level` / `unsafe_level_v` 引用
   - `utest/t_experiment.cpp`: 更新 `has_float_to_chars_v` 引用

**设计决策**:
- `StringConcept` 和 `UnsafeStringConcept` 保留在主命名空间，因为它们是用户文档契约
- 只有内部类型 traits 才移到 detail 命名空间

**测试结果**:
- 编译: 通过
- 单元测试: 110 PASS, 0 FAIL

**修改文件**:
- 修改: `include/wwjson.hpp` - 内部 traits 移到 detail 命名空间
- 修改: `include/jbuilder.hpp` - 内部 traits 移到 detail 命名空间
- 修改: `utest/t_number.cpp` - 更新 detail:: 前缀引用
- 修改: `utest/t_jbuilder.cpp` - 更新 detail:: 前缀引用
- 修改: `utest/t_experiment.cpp` - 更新 detail:: 前缀引用

## TASK:20260107-221431
-----------------------

**需求 ID**: 2026-01-07/3

**任务内容**:
完善 example 文档：在 example/ 子目录增加 README.md 介绍示例用法，并优化根目录 README.md 反映版本改进。

**实施内容**:

1. **创建 example/README.md**:
   - 为三个示例程序编写详细说明：
     * struct_to_json.cpp - 展示 `wwjson::to_json` API 的两种用法
     * estimate_size.cpp - 展示自定义 Builder 估算 JSON 大小
     * hex_json.cpp - 展示自定义配置实现十六进制 JSON 表示
   - 每个示例包含：
     * 特性展示
     * 编译运行方法
     * 代码片段示例
   - 添加构建所有示例的命令说明
   - 说明三种使用 WWJSON 的方式（源码树、find_package、FetchContent）
   - 提供技术要点和学习建议

2. **优化根目录 README.md（由用户完成）**:
   - 更新"核心特性"部分：
     * 添加模块化设计说明（单头文件提供基本功能，其他头文件拓展高级功能）
     * 添加易用API说明（提供多种风格与不同抽象层次的 API）
   - 更新"使用方式"部分：
     * 更新头文件引用说明
     * 新增关于 wwjson 子目录的说明
     * 在代码示例中展示 `wwjson/jbuilder.hpp` 和 `TO_JSON` 宏的用法
   - 更新 CMake 版本要求（1.0 -> 1.1）
   - 添加构建选项说明（`-DWWJSON_LIB_ONLY=ON`）
   - 在"架构设计"部分新增"头文件结构"小节：
     * 说明三个头文件的用途（wwjson.hpp, jstring.hpp, jbuilder.hpp）
     * 列出各类别名和功能
     * 提及统一安装到 `wwjson/` 子目录
   - 在"性能特点"部分新增"额外边界扩容"说明
   - 在"配置选项"部分后新增"推荐使用方式"小节：
     * 介绍 RawBuilder/Builder/FastBuilder 三个别名
     * 推荐使用 `wwjson::to_json` 统一 API
   - 将"配置选项"改为"配置选项定制"
   - 在"参考文档"部分添加示例程序链接
   - 更新 xyjson 描述

3. **更新英文版 README-en.md**:
   - 完整翻译中文版 README.md 的所有更新内容
   - 保持英文版与中文版内容一致

**设计决策**:
- 不在 README 首页生硬添加"版本更新"章节
- 将新功能说明自然融入现有章节，保持文档连贯性
- 详细的版本变更记录应放在正式的 release note 中

**测试结果**:
- 文档内容完整，覆盖三个示例程序
- README.md 和 README-en.md 内容对等且准确

**新增文件**:
- 新增: `example/README.md` - 示例程序使用指南

**修改文件**:
- 修改: `README-en.md` - 同步更新英文版文档

**说明**:
- `README.md` 由用户自行修改，主要反映模块化设计、易用API、头文件结构等改进
- `example/README.md` 由 AI 创建，详细说明三个示例程序的用法
- `README-en.md` 由 AI 翻译，与中文版保持一致

## TASK:20260108-122950
-----------------------

**需求 ID**: 2026-01-08/1

**任务内容**:
完善对 to_json 的测试：在 `utest/t_jbuilder.cpp` 补充单元测试，验证 `wwjson::to_json` 函数的各种用法。

**实施内容**:

1. **设计 4 个主要测试用例**，用 DESC 分段覆盖多个场景：
   - `to_json_scalars`: 标量类型(int/double/bool/string)与数组元素测试
   - `to_json_containers`: 容器(vector/array)与嵌套结构体测试
   - `to_json_macro`: TO_JSON 宏的简单结构体与嵌套结构体用法测试
   - `to_json_standalone`: 单参数 `wwjson::to_json(struct)` 返回 JSON 字符串测试

2. **测试结构设计**：
   - 局部结构体定义放在各测试用例内部，避免污染全局命名空间
   - 使用 DESC 分段组织多个测试场景
   - 覆盖标量、容器、嵌套结构、宏使用等多种情况

**测试结果**:
- 全部 4 个测试用例通过
- 构建成功，无编译警告

**修改文件**:
- 修改: `utest/t_jbuilder.cpp` - 新增 to_json 单元测试

# AI 协作任务工作日志

格式说明:
- **任务ID**: YYYYMMDD-HHMMSS ，生成命令 `date +"%Y%m%d-%H%M%S"`
- 每条日志开始一个二级标题，标题名就是任务ID
- 可适合分几个三级标题，简明扼要描叙任务过程与结果
- **追加至文件末尾**，与上条日志隔一空行

## TASK:20260108-122950
-----------------------

**需求 ID**: 2026-01-08/1

**任务内容**:
完善对 to_json 的测试：在 `utest/t_jbuilder.cpp` 补充单元测试，验证 `wwjson::to_json` 函数的各种用法。

**实施内容**:

1. **设计 4 个主要测试用例**，用 DESC 分段覆盖多个场景：
   - `to_json_scalars`: 标量类型(int/double/bool/string)与数组元素测试
   - `to_json_containers`: 容器(vector/array)与嵌套结构体测试
   - `to_json_macro`: TO_JSON 宏的简单结构体与嵌套结构体用法测试
   - `to_json_standalone`: 单参数 `wwjson::to_json(struct)` 返回 JSON 字符串测试

2. **测试结构设计**：
   - 局部结构体定义放在各测试用例内部，避免污染全局命名空间
   - 使用 DESC 分段组织多个测试场景
   - 覆盖标量、容器、嵌套结构、宏使用等多种情况

**测试结果**:
- 全部 4 个测试用例通过
- 构建成功，无编译警告

**修改文件**:
- 修改: `utest/t_jbuilder.cpp` - 新增 to_json 单元测试

## TASK:20260109-104752
-----------------------

**需求 ID**: 2026-01-09/1

**任务内容**:
增强 `jbuilder.hpp` 的 `to_json` 功能：支持关联容器(map)和可选类型(optional)，重构 `to_json_impl` 统一处理逻辑。

**实施内容**:

1. **类型 trait 简化与新增**：
   - `is_sequence_container` → `is_vector`，与 `is_optional` 命名风格统一
   - `is_associative_container` → `is_map`
   - `is_map` 要求 `key_type` 必须是字符串类型（`is_key_v`）
   - `is_vector` 排除 map 类型，避免误判
   - 新增 `is_optional` trait 支持 `std::optional`

2. **to_json_impl 重构**：
   - 将两个几乎相同的重载合并为一个统一函数
   - 使用 `detail::is_key_v<keyT>` 判断是否有 key
   - key 参数改为完美转发 `keyT&&` + `std::forward`
   - map 分支使用 `to_json_impl(builder, k, v)` 递归处理键值对
   - null 值使用 `AddMember(key, nullptr)` 和 `AddItem(nullptr)`

3. **新增测试用例**：
   - `to_json_associative`: `std::map` 和 `std::unordered_map` 转 JSON 对象
   - `to_json_optional`: `std::optional` 有值时序列化，无值时输出 null

**测试结果**:
- 全部 116 个测试通过
- 构建成功

**修改文件**:
- `include/jbuilder.hpp` - 类型 trait 简化和 to_json_impl 重构
- `utest/t_jbuilder.cpp` - 新增 map 和 optional 测试用例

## TASK:20260109-170855
-----------------------

**需求 ID**: 2026-01-09/2

**任务内容**:
测试 to_json api 的相对性能：在 `perf/p_api.cpp` 文件中增加相对性能测试，对比使用 jbuilder.hpp 中定义的 `wwjson::to_json` 转换结构体为 json 的方法与使用常规 Begin/End/Add api 的方法。

**实施内容**:

1. **新增结构体定义** (`perf/p_api.cpp`):
   - `ItemData`: 包含 id、value、name、tags 字段，每个字段都有 `to_json(builder)` 方法
   - `MetadataData`: 包含 version、created、author 字段
   - `RootData`: 包含 data(ItemData数组) 和 metadata(MetadataData) 字段

2. **新增 `ApiToJson` 测试类**:
   - 继承自 `RelativeTimer<ApiToJson>`
   - 构造函数接收 items 和 start 参数，生成测试数据
   - `methodA()`: 使用传统 RawBuilder API（Begin/End/AddMember）
   - `methodB()`: 使用 `wwjson::to_json` API
   - 两种方法都使用 `RawBuilder` 以统一比较基准
   - `methodVerify()` 验证两种方法输出完全相同

3. **新增测试用例**:
   - `api_basic_vs_tojson`: 对比基本方法与 to_json 方法的性能比
   - `api_tojson_sample`: 工具用例，输出两种方法构建的 JSON 示例用于验证

**测试结果**:
- 编译成功
- 输出验证：两种方法生成的 JSON 完全相同
- 性能测试：`Basic Method` 略快于 `to_json Method`（约 5-6%）
  - 这符合预期，因为 to_json 增加了模板实例化和递归调用的开销

**修改文件**:
- `perf/p_api.cpp` - 新增 ApiToJson 类和两个测试用例

## TASK:20260109-212430
-----------------------

**需求来源**: TODO:2026-01-09/3 测试几种 Json 拼装方法的相对性能

**任务内容**: 新增 perf/p_nodom.cpp 性能测试文件，对比 wwjson::RawBuilder 与其他原生字符串拼装方法的性能。

**实现方案**:
1. 设计测试数据: RootData（4字段）+ DataItem（50字段，字符串值）
2. 四种拼装方法:
   - A: wwjson::RawBuilder（使用 ScopeObject/AddMember）
   - B1: snprintf 一次性格式化（所有字段名嵌入格式串）
   - B2: std::string::append 逐步添加
   - B3: std::ostringstream << 逐步添加
3. 三个相对测试: RawBuilder vs snprintf/append/streamstream
4. 使用 REFERENCE_JSON 字面量进行验证

**测试结果** (100000次循环):
- RawBuilder vs snprintf: RawBuilder 快 50%（122ms vs 183ms）
- RawBuilder vs append: append 快 62%（79ms vs 128ms）
- RawBuilder vs stream: RawBuilder 快 98%（117ms vs 232ms）

**修改文件**:
- `perf/p_nodom.cpp` - 新增测试文件（850行）
- `perf/CMakeLists.txt` - 添加 p_nodom.cpp
- `perf/README.md` - 更新文件列表

## TASK:20260110-214323
-----------------------

**需求来源**: TODO:2026-01-10/3 p_nodom.cpp 测试分析与补充

**任务内容**: 完善 perf/p_nodom.cpp 性能测试，分析性能差异，增加 Builder 和 FastBuilder 测试用例。

**实现方案**:
1. **重构 BuilderMethod 为模板类**:
   - 创建 `BuilderMethodT<BuilderT>` 模板类
   - 通过类型别名定义 RawBuilderMethod、BuilderMethod、FastBuilderMethod
   - 减少代码重复，提高可维护性

2. **改用 BeginObject/EndObject**:
   - 替换 ScopeObject 为 BeginObject/EndObject 调用
   - 使用最平实的 API，代码更清晰直观

3. **新增两个测试用例**:
   - `BuilderVsAppend`: 对比 Builder (JString+UnsafeConfig) vs string::append
   - `FastBuilderVsAppend`: 对比 FastBuilder (KString+UnsafeConfig) vs string::append

4. **清理冗余打印**:
   - 删除所有测试用例中的 `DESC("Performance ratio: %.3f", ratio);`
   - 避免与 tester.runAndPrint 重复打印

5. **性能分析与验证**:
   - 运行原始测试分析性能差异
   - 使用 JString/KString 优化验证性能提升

**测试结果** (1000次循环):
- RawBuilder vs snprintf: RawBuilder 快 79.36%
- RawBuilder vs append: RawBuilder 快 19.60% (Begin/End 后性能提升)
- RawBuilder vs stream: RawBuilder 快 136.15%
- Builder vs append: Builder 快 116.71% (JString 优化)
- FastBuilder vs append: FastBuilder 快 103.27% (KString 优化)

**关键发现**:
- 使用 BeginObject/EndObject 替代 ScopeObject 后，RawBuilder 性能从比 append 慢 32.98% 变为快 19.60%
- Builder 和 FastBuilder 通过 JString/KString 优化后，性能优势明显
- StringBuilder 仍是最快的，但 Builder 提供了更完善的 JSON 构建功能

**修改文件**:
- `perf/p_nodom.cpp` - 重构模板类，新增测试用例，清理冗余代码

## TASK:20260110-222559
-----------------------

**需求来源**: TODO:2026-01-10/4 优化性能测试输出

**任务内容**:
1. 在根目录的 makefile 增加一个快捷开发命令 make perf/log，功能与已有的 make perf 类似运行性能测试程序，但是将输出结果重定向保存到 perf/report.log/ 目录下，文件名取 local-yyyymmdd-hhmmss.log 格式
2. 分析性能测试程序的输出文本，删除 tester.runAndPrint 调用之后的冗余 DESC 输出

**实现方案**:

**1. 添加 make perf/log 命令**:
- 在 `.PHONY` 中添加 `perf/log` 目标
- 在 help 信息中添加 `perf/log` 的说明
- 创建 `perf/log` 目标，执行以下操作：
  - 自动创建 `perf/report.log/` 目录（如果不存在）
  - 生成日志文件名：`local-yyyymmdd-hhmmss.log`
  - 运行性能测试程序并将输出重定向到日志文件
  - 显示日志文件保存路径

**2. 删除冗余 DESC 输出**:
- 分析所有性能测试文件（perf/p_*.cpp）
- 识别在 `tester.runAndPrint()` 调用之后的冗余 DESC 输出
- 删除以下文件中的冗余输出：
  - `perf/p_api.cpp`: 删除 6 处 `DESC("Performance ratio: %.3f", ratio);`
  - `perf/p_number.cpp`: 删除 6 处 `DESC("xxx ratio: %.3f", ratio);`
  - `perf/p_design.cpp`: 无冗余 DESC，WARNING 信息保留（在 if 条件中，用于错误提示）
  - `perf/p_nodom.cpp`: 无冗余 DESC

**修改文件**:
- `makefile` - 添加 `perf/log` 命令和相关说明
- `perf/p_api.cpp` - 删除 6 处冗余 DESC 输出（第 528, 546, 564, 582, 600, 700 行）
- `perf/p_number.cpp` - 删除 6 处冗余 DESC 输出（number_int_rel 和 number_double_rel 测试）

**测试验证**:
- ✅ 构建性能测试程序成功
- ✅ 运行单个测试用例，验证冗余 DESC 已删除
- ✅ 运行 `make perf/log`，确认日志文件正确生成
- ✅ 检查日志文件内容，确认输出完整且格式正确
- ✅ 确认 `perf/report.log/` 目录自动创建

**效果**:
- 性能测试输出更加简洁，去除了重复的性能比例信息
- `make perf/log` 命令方便地将性能测试结果保存到日志文件
- 日志文件按时间戳命名，便于追踪历史记录

## TASK:20260111-122545
-----------------------

### 需求内容

完成需求 ID：2026-01-11/1 - 开发脚本自动运行性能测试 github 流水线及提取日志

### 实施内容

#### 1. 更新 perf/README.md 文档

在 perf/README.md 文件中添加 "GitHub Actions 自动化测试" 章节，包含：

**使用 gh 命令行工具**
- 触发工作流：基本触发、指定分支、自定义参数
- 查看工作流状态：列出运行记录、查看详细状态、实时监控
- 下载工作流日志：下载完整日志、只查看失败步骤

**自动化脚本**
- run-ci.sh：自动触发 CI、等待完成、下载和提取日志
- extract-perf-log.pl：从全量日志中提取性能测试日志

#### 2. 开发 perf/run-ci.sh 脚本

创建了完整的 bash 自动化脚本，功能包括：

**依赖检查**
- 检查 gh 命令是否安装
- 检查 GitHub CLI 认证状态
- 检查 jq 命令是否可用

**工作流触发和管理**
- 触发 ci-perf.yml 工作流（默认在 dev 分支）
- 获取最新的 run-id
- 等待工作流运行完成（轮询检查，超时 1 小时）
- 检查工作流运行状态和结论

**日志处理**
- 下载完整日志到 perf/report.log/ci-{run-id}-full.log
- 从日志中提取时间戳，转换为本地时间格式（UTC+8）
- 调用 perl 脚本提取性能测试日志

**错误处理和日志输出**
- 使用彩色输出增强可读性（INFO/WARN/ERROR）
- 完整的错误处理机制
- 详细的进度提示

#### 3. 开发 perf/extract-perf-log.pl 脚本

创建了独立的 perl 脚本用于日志提取，具有以下特点：

**功能特性**
- 定位 "Run performance tests" 步骤的起始和结束行
- 去除 GitHub Actions 日志前缀（时间戳、步骤名等）
- 只保留 pfwwjson 程序的实际输出
- 自动提取时间戳并转换为本地时间格式（UTC+8）

**日志格式处理**
- 正确处理 GitHub Actions 日志格式：`performance-tests\tStep Name\tTimestamp\tMessage`
- 去除时间戳前缀：`2026-01-10T16:46:13.3186685Z`
- 过滤掉控制行（shell:、env:、ANSI 转义码等）
- 只保留程序输出内容

**使用方式**
```bash
# 从全量日志提取性能测试日志
perl perf/extract-perf-log.pl ci-20881321775-full.log ci-20260111-004613.log

# 或输出到标准输出
perl perf/extract-perf-log.pl ci-20881321775-full.log > perf.log
```

#### 4. bash 脚本与 perl 脚本集成

修改了 bash 脚本的 `extract_perf_log` 函数，改为调用 perl 脚本：

- 检查 perl 脚本是否存在
- 调用 perl 脚本提取日志
- 处理提取结果和错误

### 测试验证

#### perl 脚本测试

使用实际的全量日志进行测试：
```bash
perl perf/extract-perf-log.pl ci-20260111-005330-20881321775.log /tmp/test-extract.log
```

测试结果：
- 成功定位到 'Run performance tests' 步骤（行 366 到 820）
- 提取了 418 行性能测试日志
- 时间戳转换正确（UTC 转 UTC+8）
- 日志格式与手动提取的精简日志一致

#### 日志格式验证

对比提取的日志与原始精简日志：
- 格式基本一致
- 性能测试输出完整
- 只包含 pfwwjson 程序的实际输出
- 去除了所有 GitHub Actions 日志前缀

### 设计亮点

#### 1. 脚本解耦设计
- perl 脚本独立，可单独调试和使用
- bash 脚本负责自动化流程
- 清晰的职责分离

#### 2. 灵活的配置
- 可配置的分支名（默认 dev）
- 可配置的轮询间隔和超时时间
- 支持自定义 CMake 和测试参数

#### 3. 健壮的错误处理
- 完整的依赖检查
- 工作流状态监控和错误提示
- 日志提取失败处理

#### 4. 时间戳处理
- 自动从日志提取 UTC 时间戳
- 转换为本地时间格式（UTC+8）
- 支持失败时使用当前时间作为备选

### 修改文件

**新增文件**
- `perf/run-ci.sh`：GitHub Actions CI 自动化脚本
- `perf/extract-perf-log.pl`：日志提取 perl 脚本

**修改文件**
- `perf/README.md`：添加 GitHub Actions 使用说明和自动化脚本文档

### 后续改进建议

1. **脚本参数化**：支持命令行参数，如 --branch、--cmake-args 等
2. **并发处理**：支持同时触发多个工作流
3. **日志归档**：自动清理旧的日志文件，保留最近 N 个
4. **邮件/通知**：工作流完成或失败时发送通知
5. **性能优化**：使用 GitHub API 的 watch 功能替代轮询

### 脚本优化和修正

在初始版本完成后，针对实际测试中发现的问题进行了以下重要优化和修正：

#### 1. 添加 --no-trigger 选项
- **功能**：支持不触发新工作流，只处理最近运行的流水线日志
- **使用方式**：`bash perf/run-ci.sh --no-trigger`
- **应用场景**：当只需要下载和提取已有工作流日志时，避免重复触发

#### 2. 修复输出交错问题
- **问题**：perl 脚本的进度输出与 bash 脚本的完成信息交错，导致输出混乱
- **解决方案**：在 bash 脚本中捕获 perl 脚本输出，仅在失败时显示错误信息
- **实现**：在 `extract_perf_log()` 函数中使用 `perl_output=$(perl "$script_dir/extract-perf-log.pl" ...)` 捕获输出

#### 3. 优化全局变量使用
- **改进**：去除函数间的 `echo` 返回值传递，直接使用全局变量
- **关键变量**：RUN_ID、FULL_LOG_FILE、TIMESTAMP、PERF_LOG_FILE 统一使用全局作用域
- **优势**：避免 stdout 输出被污染，提高脚本健壮性

#### 4. 增强错误处理
- **状态检查**：在非触发模式下检查工作流状态，提示用户可能仍在运行
- **依赖验证**：增加对关键命令的完整检查，提供明确的错误指导

### 完成结果

✅ 完成 perf/README.md 的 GitHub Actions 使用说明
✅ 开发 perf/run-ci.sh 自动化脚本
✅ 开发 perf/extract-perf-log.pl 日志提取脚本
✅ 测试验证脚本功能正常
✅ 实现 bash 脚本与 perl 脚本集成
✅ 日志格式正确，时间戳转换准确
✅ 添加 --no-trigger 选项支持不触发工作流
✅ 修复脚本输出交错问题，提高输出清晰度
✅ 优化全局变量使用，增强脚本健壮性
✅ 完善错误处理和状态检查机制

## TASK:20260111-140329
-----------------------

### 任务来源
需求ID: 2026-01-11/2 "性能测试结果分析"

### 实现内容
1. **日志数据提取**: 编写 `parse_perf_logs.pl` 脚本解析所有性能测试日志，提取结构化数据
2. **综合分析**: 编写 `analyze_perf.pl` 脚本生成性能分析报告，重点突出新版本特性
3. **报告重写**: 重写 `perf/report.md`，全面分析CI环境与本地环境的性能差异
4. **新版本特性分析**: 重点分析新引入的 `Builder<JString>` 和 `FastBuilder<KString>` 的性能优势

### 关键发现
1. **新构建器显著提升性能**:
   - `JString` 比 `std::string(RawBuilder)` 快 16-66% (CI环境) 或 16-32% (本地环境)
   - `KString` 比 `std::string(RawBuilder)` 快 21-78% (CI环境) 或 21-33% (本地环境)
   - 优化构建器在各类JSON构建场景中表现稳定，大对象场景优势依然保持

2. **wwjson总体性能优势**:
   - 在JSON构建方面普遍优于yyjson，尤其在本地环境下优势更明显
   - 字符串对象构建性能稳定优势，wwjson比yyjson快0-70%
   - 大JSON对象(100k+)场景下优势最为突出

3. **数字序列化需优化**:
   - yyjson API在数字序列化方面仍有明显优势，尤其在CI环境下
   - 但优化构建器(JString/KString)在数字序列化中比std::string仍有12-23%优势

4. **环境差异影响**:
   - CI环境(标准Ubuntu)编译器优化更好，yyjson在数字序列化方面优势更大
   - 本地环境(WSL1 Ubuntu 20.04) wwjson在JSON构建方面相对优势更大
   - 环境差异对性能对比有显著影响

### 技术分析
1. **构建器优化原理**: `JString` 和 `KString` 通过减少内存分配和字符串复制实现性能提升
2. **架构优势**: wwjson避免了DOM中间结构，在以字符串为主的JSON构建中性能优势明显
3. **API设计**: 提供多种API调用方式适应不同编程风格，性能差异相对较小

### 后续开发建议
1. **性能优化优先级**:
   - 重点优化数字序列化算法，尤其是整数和浮点数处理
   - 研究主流JSON库的数字转字符串实现，考虑引入更高效算法
   - 进一步优化内存分配策略，减少动态分配开销

2. **构建器使用指南**:
   - **高性能场景**: 优先使用 `FastBuilder<KString>`
   - **通用场景**: 使用 `Builder<JString>`
   - **兼容性要求**: 使用 `std::string(RawBuilder)`

3. **测试建议**:
   - 在标准Linux环境下进行基准测试以获得更可靠结果
   - 增加更多真实应用场景的性能测试案例
   - 定期运行CI性能测试，监控性能变化趋势

4. **功能扩展方向**:
   - 考虑JSON解析功能(可选，保持单头文件设计)
   - 更多序列化格式支持(如MessagePack、CBOR)
   - 异步构建支持，适用于大规模数据处理

### 工具和脚本
- `perf/parse_perf_logs.pl`: 日志数据提取脚本，输出结构化JSON
- `perf/analyze_perf.pl`: 性能分析报告生成脚本
- `perf/extract-perf-log.pl`: GitHub Actions全量日志提取工具
- `perf/report.md`: 更新后的性能测试报告

### 数据来源
- **CI环境**: 最近3次GitHub Actions性能测试日志 (ci-20260111-*.log)
- **本地环境**: 最近3次本地性能测试日志 (local-20260111-*.log)
- **测试时间范围**: 2026-01-10 至 2026-01-11

## TASK:20260111-184936
-----------------------

### 任务概述

完成 WWJSON v1.1.0 版本封版发布，将原始需求文档和任务日志归档到 changelog/v1.1/ 目录，并在根目录创建包含版本摘要的新文档。

### 实现内容

**1. 更新版本号**
- CMakeLists.txt: VERSION 1.0.0 → 1.1.0

**2. 文档归档**
- 将原始 task_todo.md 移动到 changelog/v1.1/task_todo.md
- 将原始 task_log.md 移动到 changelog/v1.1/task_log.md
- 保留完整的开发历史记录，便于后续参考

**3. 根目录文档重建**
- 创建新的 task_todo.md，包含 v1.1.0 版本摘要和后续开发计划
- 创建新的 task_log.md，包含 v1.1.0 开发历程摘要
- 保持文档结构，但内容精简，便于后续开发使用

**4. 添加发布任务记录**
- 在 changelog/v1.1/task_log.md 末尾添加本次发布任务记录
- 包含一份简要的 release note

### 版本摘要内容

**核心功能**
- ✅ 新增 JString 高性能字符串缓冲类，专为 JSON 序列化优化
- ✅ BufferView 和 UnsafeBuffer 视图类，支持借用外部内存
- ✅ KString 最大不安全等级特化，单次内存分配零扩容
- ✅ wwjson::to_json 统一转换 API，简化结构体序列化
- ✅ 多头文件架构，jbuilder.hpp 组合使用 wwjson.hpp 和 jstring.hpp

**性能优化**
- ✅ UnsafePutChar 优化格式字符写入（逗号、冒号、引号）
- ✅ UnsafeConfig 优化字符串转义，避免临时 buffer
- ✅ 浮点数序列化优化，支持直接写入 StringBuffer
- ✅ JString 比 std::string 快 6-30%，KString 快 19-30%

**开发工具**
- ✅ 示例程序：struct_to_json、estimate_size、hex_json
- ✅ CI/CD 增强单元测试和性能测试流水线
- ✅ 多头文件安装到 wwjson/ 子目录
- ✅ 完整的文档更新和在线部署

### Release Note

**WWJSON v1.1.0 Release Notes**

WWJSON v1.1.0 是一个重要的性能优化版本，引入了高性能字符串缓冲库 JString，显著提升了 JSON 序列化性能。

**主要更新**

1. **JString 高性能字符串库**
   - 新增 `include/jstring.hpp`，提供 JString (StringBuffer<4>) 和 KString (StringBuffer<255>)
   - BufferView 和 UnsafeBuffer 视图类，支持借用外部内存
   - 不安全级别机制 (kUnsafeLevel)，减少边界检查开销
   - JString 比 std::string 快 6-30%，KString 快 19-30%

2. **性能优化**
   - UnsafePutChar 优化格式字符写入（逗号、冒号、引号）
   - UnsafeConfig 优化字符串转义，避免临时 buffer
   - 浮点数序列化优化，支持直接写入 StringBuffer
   - 浮点数 buffer 大小从 256 字节优化到 64 字节

3. **统一转换 API**
   - 新增 `wwjson::to_json` 统一转换 API
   - 支持标量、结构体、容器（std::vector, std::map）、可选类型（std::optional）
   - 提供 `TO_JSON(field)` 宏简化结构体序列化代码

4. **多头文件架构**
   - `include/wwjson.hpp`: 核心库，保持单头文件设计
   - `include/jstring.hpp`: 高性能字符串缓冲库
   - `include/jbuilder.hpp`: 组合使用，提供 Builder 和 FastBuilder 别名
   - 多头文件安装到 wwjson/ 子目录

5. **示例程序**
   - struct_to_json: 多层结构体转 JSON 示例
   - estimate_size: 容量估算示例
   - hex_json: 十六进制 JSON 示例

**API 变更**

- 新增 `Builder = GenericBuilder<JString, UnsafeConfig<JString>>`
- 新增 `FastBuilder = GenericBuilder<KString, UnsafeConfig<KString>>`
- 新增 `wwjson::to_json()` 统一转换 API
- 新增 `TO_JSON(field)` 宏简化代码

**破坏性变更**

- 多头文件安装到 wwjson/ 子目录，引用路径从 `#include <wwjson.hpp>` 改为 `#include <wwjson/wwjson.hpp>`
- example 程序使用 `find_package` 或 FetchContent 时的引用路径需要相应更新

**性能提升**

根据 CI 环境性能测试结果：
- JString 比 std::string (RawBuilder) 快 16-66%
- KString 比 std::string (RawBuilder) 快 21-78%
- wwjson 在 JSON 构建方面普遍优于 yyjson
- wwjson 字符串对象构建性能稳定优势，比 yyjson 快 0-70%

**后续计划**

- 优化整数序列化算法，探索正向写入方法
- 提升浮点数序列化性能，缩小与 yyjson 的差距
- 考虑 JSON 解析功能（可选）

### 完成结果

成功完成 v1.1.0 版本封版工作：
✅ 版本号更新为 1.1.0
✅ 原始开发文档完整归档到 changelog/v1.1/
✅ 根目录文档重建，包含版本摘要
✅ 后续开发计划明确
✅ 为 v1.1.0 标签发布做好准备
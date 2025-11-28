# AI 协作任务工作日志

格式说明:
- **任务ID**: YYYYMMDD-HHMMSS ，生成命令 `date +"%Y%m%d-%H%M%S"`
- 每条日志开始一个二级标题，标题名就是任务ID
- 可适合分几个三级标题，简明扼要描叙任务过程与结果
- **追加至文件末尾**，与上条日志隔一空行

## TASK:20251125-112235
-----------------------

### 任务概述
完成 cmake 构建系统实现，支持 wwjson 头文件库的编译、安装和单元测试功能。

### 实现内容

**核心文件结构**
- `CMakeLists.txt`: 主构建脚本，定义头文件库和安装配置
- `cmake/wwjsonConfig.cmake.in`: CMake 配置模板文件  
- `utest/CMakeLists.txt`: 单元测试构建脚本

**功能实现**
1. 头文件库配置：使用 INTERFACE 库类型，支持 C++17 标准
2. 安装系统：提供标准 CMake 安装路径和配置文件生成
3. 单元测试集成：依赖管理优先本地安装，备用 GitHub 下载
4. couttast 依赖：正确处理本地 ~/include 和 ~/lib 路径下的小写库名
5. 构建选项：`BUILD_UNIT_TESTS` 选项默认启用
6. 测试目标：生成 `utwwjson` 可执行文件，支持 CTest 和自定义 --List 选项

**验证测试**
- CMake 配置成功：检测到本地 couttast 安装
- 编译测试通过：生成 utwwjson 可执行文件
- 单元测试运行：4个测试全部通过，耗时约 107 微秒
- CTest 集成：在 build/utest 子目录下正常运行

### 技术细节
- 使用 FetchContent 模块处理依赖下载备用方案
- 支持多种 couttast 目标命名方式（couttast::couttast, couttast）
- 手动搜索路径优先级：$HOME/include/lib > /usr/local > /usr
- RAII 模式的 CMake 配置文件生成和版本管理

### 结果
完全满足需求 2025-11-25/1，构建系统符合 CMake 标准规范，其他项目可通过 find_package(wwjson) 集成使用。

## TASK:20251125-162627
-----------------------

### 任务概述
为 RawJsonBuilder 增加 scope 方法创建能自动关闭的数组或对象，优化衍生类结构并完善单元测试。

### 实现内容

**新增 Scope 方法**
- `ScopeArray(bool hasNext = false)`: 创建自动关闭的 RawJsonArray
- `ScopeArray(const char* pszKey, bool hasNext = false)`: 创建带键名的自动关闭 RawJsonArray
- `ScopeObject(bool hasNext = false)`: 创建自动关闭的 RawJsonObject  
- `ScopeObject(const char* pszKey, bool hasNext = false)`: 创建带键名的自动关闭 RawJsonObject

**衍生类优化**
- 将 `m_build` 重命名为 `m_builder` 提高命名一致性
- 将成员变量 `m_builder` 和 `m_next` 改为 private 权限
- 保留 reference 类型实现，经分析比指针更合适（避免空指针检查，性能更优）

**单元测试扩展**
- 创建 `t_scope.cpp` 新测试文件
- 将原 `basic_builder_nest` 移至新文件并改名为 `scope_builder_nest`  
- 新增 `scope_vs_constructor` 测试用例，验证 scope 方法与构造函数方法产生相同结果
- 所有测试通过：6个测试用例全部 PASS，耗时约 119 微秒

### 技术细节
- 使用前向声明解决类间依赖关系
- Scope 方法返回值类型确保 RAII 自动资源管理
- 支持 auto 关键字简化用户代码：`auto obj = builder.ScopeObject();`
- 保持向后兼容：原有构造函数方式仍可正常使用

### 结果
完全满足需求 2025-11-25/3，用户现在只需记住 RawJsonBuilder 一个类名，通过 Scope 方法创建衍生类并用 auto 接收，简化了 API 使用。

## TASK:20251125-172718
-----------------------

### 任务概述
将 RawJsonBuilder 及相关类重构为模板化实现，支持其他库的自定义字符串类型，只要与 std::string 具有相同接口。

### 实现内容

**StringConcept 概念定义**
- 在 `wwjson.hpp` 开头添加 `StringConcept` 结构体
- 详细列出 GenericBuilder 需要用到的 std::string 接口作为文档说明
- 虽然不支持真正的 concept 限定，但提供了接口规范

**核心类模板化重构**
- `RawJsonBuilder` → `template<typename stringT> class GenericBuilder`
- `RawJsonObject` → `template<typename stringT> class GenericObject`  
- `RawJsonArray` → `template<typename stringT> class GenericArray`
- 更新前向声明为模板形式

**类型别名系统**
- `using RawBuilder = GenericBuilder<std::string>;`
- `using RawObject = GenericObject<std::string>;`
- `using RawArray = GenericArray<std::string>;`
- 保持向后兼容的遗留别名：`RawJsonBuilder/RawJsonObject/RawJsonArray`

**单元测试更新**
- 将所有测试文件中的类名更新为使用新的类型别名
- `RawJsonBuilder` → `RawBuilder`
- `RawJsonObject` → `RawObject` 
- `RawJsonArray` → `RawArray`

### 技术细节
- 模板参数设计：使用简单的 `template<typename stringT>` 参数
- 接口兼容性：保持所有现有公共接口不变
- 转义方法更新：`EscapeString` 方法的 `dst` 参数类型改为模板化 `stringT&`
- 合并方法更新：`Merge` 和 `MergeObject` 方法支持模板化字符串类型

### 测试结果
- 编译成功：模板化代码编译无错误
- 功能验证：所有 6 个单元测试全部 PASS
- 向后兼容：使用类型别名的现有代码功能完全不变

### 结果
成功完成模板化重构，满足了需求 2025-11-25/4 的所有要求：
✓ 支持自定义字符串类型的模板化实现
✓ 保持与 std::string 的完全兼容性
✓ 通过类型别名实现向后兼容
✓ 单元测试验证功能正确性
✓ 清晰的接口文档（StringConcept）

## TASK:20251126-114254
-----------------------

### 任务概述
测试使用自定义字符串类型构建 JSON，创建自定义字符串类并验证 wwjson 库的泛型支持。

### 实现内容

**自定义字符串类实现**
- 创建 `utest/custom_string.h/cpp` 实现 `test::string` 类
- 满足 `wwjson::StringConcept` 要求的所有接口：
  - 基本操作：append、push_back、clear、empty、size、c_str、front、back、reserve
  - 构造函数：默认构造和带容量参数构造
  - 赋值操作：拷贝构造、移动构造、拷贝赋值、移动赋值
- 内存管理：动态数组存储，自动扩容机制
- 兼容性：支持与 `std::string` 的比较操作

**测试用例实现**
- 创建 `utest/t_custom.cpp` 测试文件
- 按要求使用 `DEF_TAST` 宏和 `custom_` 前缀命名：
  - `custom_basic_builder`: 测试基本的 JSON 构建功能
  - `custom_scope_auto_nest`: 测试自动关闭的嵌套结构
- 测试覆盖：整数、字符串、字符、数组、对象等数据类型
- 验证输出与使用 `std::string` 时完全一致

**构建系统更新**
- 修改 `utest/CMakeLists.txt` 添加新文件到编译列表
- 添加 `t_custom.cpp` 和 `custom_string.cpp` 到 `utwwjson` 可执行文件

### 技术细节
- 内存管理：使用 `new[]/delete[]` 动态分配，容量不足时自动扩容为 2 倍
- 接口兼容：严格遵循 `std::string` 的接口语义
- 模板实例化：使用 `GenericBuilder<test::string>` 实例化模板类
- 测试验证：通过 COUT 宏输出结果进行比对验证

### 测试结果
- 编译测试：项目成功编译，新增文件无语法错误
- 功能测试：新增 2 个测试用例全部通过
  - `custom_basic_builder`: 8 微秒完成
  - `custom_scope_auto_nest`: 4 微秒完成
- 总体验证：所有 8 个测试用例 PASS，0 个 FAIL

### 结果
成功完成需求 2025-11-26/1 的所有要求：
✓ 实现了符合 StringConcept 的自定义字符串类
✓ 创建了使用自定义字符串的 JSON 构建测试用例
✓ 验证了 wwjson 库对泛型字符串类型的完全支持
✓ 所有编译和测试通过，不影响现有功能
✓ 证明了模板化设计的有效性和可用性

## TASK:20251126-161114
-----------------------

### 任务概述
对 GenericBuilder 进行配置化重构，增加模板参数以支持序列化选项的可配置性，包括转义策略、数字引号、尾逗号等选项。

### 实现内容

**BasicConfig 模板类设计**
- 创建 `template<typename stringT> struct BasicConfig` 配置类
- 定义三个编译期配置常量：
  - `kAlwaysEscape = false`: 是否总是转义字符串值
  - `kQuoteNumber = false`: 是否默认给数字加引号
  - `kTailComma = false`: 是否允许尾逗号
- 实现静态方法：
  - `EscapeKey()`: 对象键处理（当前直接拷贝）
  - `EscapeValue()`: 字符串值转义（调用 EscapeString）
  - `EscapeString()`: 移动原有的转义逻辑，支持单字符和多字符转义

**GenericBuilder 重构**
- 添加第二个模板参数 `configT = BasicConfig<stringT>`
- 更新 `EndArray()` 和 `EndObject()` 方法支持 `kTailComma` 配置
- 修改数字类型的 `AddItem()` 和 `AddMember()` 方法支持 `kQuoteNumber` 配置
- 保持两个重载版本：有 bool 参数的总是加引号，无参数的根据配置决定
- 更新 `AddMemberEscape()` 方法调用 `configT::EscapeString()`
- 移除原有的 `EscapeString()` 静态方法，改用配置类的方法

**新增 AddItemEscape 方法**
为保持与 `AddMemberEscape` 的一致性，添加了完整的 `AddItemEscape` 方法族：
- 支持字符串和 C 字符串的各种重载
- 支持单字符和多字符转义集
- 所有方法都调用 `configT::EscapeString()`

**模板类更新**
- 更新 `GenericObject` 和 `GenericArray` 模板类支持新的 `configT` 参数
- 更新 `ScopeArray()` 和 `ScopeObject()` 方法的返回类型和实现
- 修复类型别名：`RawObject` 和 `RawArray` 使用默认配置

### 技术细节
- 编译期优化：使用 `if constexpr` 确保配置选项在编译期确定
- 向后兼容：保持所有原有 API 和行为，通过默认配置确保兼容性
- 模板参数设计：支持用户继承 BasicConfig 创建自定义配置
- 内存管理：转义方法直接写入目标字符串，避免额外分配

### 修复的问题
- 类型别名错误：修复 `RawObject` 和 `RawArray` 缺少配置参数
- 测试兼容性：更新测试中的 `EscapeString` 调用使用新的 API
- 参数传递：修正 BasicConfig::EscapeString 的参数顺序和类型

### 测试结果
- 编译测试：项目成功编译，重构代码无语法错误
- 功能测试：所有现有测试用例全部通过（8个 PASS，0个 FAIL）
- 兼容性验证：使用默认配置时与原有行为完全一致
- 性能影响：重构后运行时间与原版本基本相同

### 结果
成功完成需求 2025-11-26/2 的所有要求：
✓ 实现了配置化的 BasicConfig 模板类
✓ 成功重构 GenericBuilder 支持配置参数
✓ 保持了完整的向后兼容性
✓ 添加了完整的 AddItemEscape 方法族
✓ 所有测试通过，验证了功能正确性
✓ 为用户提供了灵活的序列化选项配置能力

## TASK:20251127-112133
-----------------------

### 任务概述
完成字符串与转义功能优化，提升 wwjson 库的字符串处理能力和转义灵活性。

### 实现内容

**API 优化**
- 删除了 BasicConfig::EscapeValue 方法，简化配置接口
- 调整 BasicConfig::EscapeString 的参数默认值：
  - char 版本：不提供默认值，需要显式指定转义字符
  - const char* 版本：提供默认值 "\\\n\t\r\"\0"
- 为 EscapeKey 增加了两个重载版本，保持接口一致性
- 定义了 DEFAULT_ESCAPE_CHARS 常量，避免字符串字面量重复

**配置驱动的转义逻辑**
- GenericBuilder::PutKey 方法增加 configT::kAlwaysEscape 判断
- GenericBuilder::PutValue 的字符串重载增加 configT::kAlwaysEscape 判断
- 统一了 AddMemberEscape/AddItemEscape 参数默认值规则

**方法增强**
- 为 GenericArray 增加了 AddItemEscape 转发方法
- 为 GenericObject 增加了 AddMemberEscape 转发方法
- 确保所有字符串方法支持三种重载：const char*、const char,size_t、std::string&

**测试完善**
- 新增 utest/t_escape.cpp 测试文件，包含 5 个测试用例
- 测试覆盖基本转义、配置驱动转义、作用域转义、参数重载等场景
- 更新 CMakeLists.txt 包含新测试文件
- 完善 utest/README.md 文档，添加了当前测试文件结构说明

### 技术细节

**转义机制改进**
- 基于配置类的 kAlwaysEscape 标志决定是否自动转义
- 支持单个字符和多字符转义集合
- 保持了向后兼容性，默认行为与之前一致

**参数一致性**
- 所有 Escape 相关方法的参数默认值规则保持一致
- char 参数版本不提供默认值，const char* 版本提供合理的默认转义字符集

**作用域支持**
- GenericArray/GenericObject 支持转义方法的模板转发
- 保持了 RAII 特性，自动管理 JSON 结构的开启和关闭

### 测试结果
- 编译测试：项目成功编译，所有语法错误已修复
- 功能测试：新增测试用例验证了转义功能的各种场景
- 兼容性验证：现有测试用例全部通过，保证了向后兼容性
- 文档完善：更新了测试文件结构说明，便于维护

### 结果
成功完成需求 2025-11-27/2 的所有要求：
✓ 优化了字符串与转义功能的 API 设计
✓ 实现了配置驱动的自动转义机制
✓ 增强了方法重载支持和参数一致性
✓ 提供了全面的单元测试覆盖
✓ 保持了完整的向后兼容性
✓ 提升了代码的可维护性和易用性

## TASK:20251127-141621
-----------------------

### 任务概述
重构 GenericBuilder 方法封装及顺序调整，增加接口抽象层并按功能分组重新组织代码结构。

### 实现内容

**M1 接口封装方法**
- 增加 PutChar 方法统一调用 stringT::push_back 
- 增加 FixTail 方法处理尾字符替换逻辑，简化 EndArray/EndObject
- 增加 Append 方法重载，统一调用 stringT::append
- 增加 Size/Back/Front/PushBack 方法，提供大写命名的 stringT 接口封装
- 增加 Clear 方法用于清空 JSON 字符串

**方法调用重构**
- 删除 PutComma 方法，替换为 PutNext 方法调用 PutChar(',')
- EndArray 默认分支调用 FixTail(',', ']')
- EndObject 默认分支调用 FixTail(',', '}')
- 所有直接调用 stringT::push_back/append 的地方改为调用 PutChar/Append
- Merge 方法改用 Size/Back/Front 等封装方法

**M0-M8 分组重组**
- M0: 基本方法（构造函数）
- M1: String 接口封装/桥接方法（PutChar/FixTail/Append/Size/Back/Front/PushBack）
- M2: JSON 字符级拼接方法（PutNext/Begin-End Array/Object/EmptyArray/EmptyObject/EndLine）
- M3: JSON 标量值方法（PutNull/PutValue/PutKey）
- M4: JSON 数组元素方法（AddItem）
- M5: JSON 对象元素方法（AddMember）
- M6: 字符串转义方法（*Escape）
- M7: 作用域创建方法（ScopeArray/ScopeObject）
- M8: 高级方法（ReopenObject/Merge）

**分隔符格式化**
- 每个分组添加显著注释和 78 字符分隔线：/* ---------------------------------------------------------------------- */

**单元测试补充**
- t_basic.cpp 中增加 basic_wrapper 测试用例，验证 M1 各方法功能
- t_custom.cpp 中增加 custom_wrapper 测试用例，验证 M1 接口方法可定制

### 技术细节

**依赖隔离**
- M2 组以后的方法不再直接调用 stringT 的方法，全部通过 M1 封装方法
- 确保 M0-M1 组中的方法调用了 stringT 的完整接口，与 StringConcept 一致
- 取消注释，写出完整的方法签名声明

**接口一致性**
- PutNext 替代 PutComma，保持相同的分隔符功能
- FixTail 统一处理 EndArray/EndObject 的尾字符逻辑
- Append 提供三个重载：const char*、const char*+size_t、const std::string&

**向后兼容**
- 保持所有现有公共方法的签名和行为不变
- 确保测试用例全部通过，验证功能不变更

### 测试结果
- 编译测试：修复重复定义错误，成功编译
- 功能测试：所有 15 个测试用例通过，包括新增的 2 个 M1 测试用例
- 兼容性验证：现有测试用例全部通过，保证了向后兼容性

### 结果
成功完成需求 2025-11-27/3 的所有要求：
✓ 增加了 M1 封装方法，隔离了 stringT 接口依赖
✓ 重新组织方法分组，代码结构更清晰
✓ 补充了 M1 方法的单元测试
✓ 保持了完整的功能兼容性
✓ 提升了代码的可维护性和扩展性

## TASK:20251127-171004
-----------------------

### 任务概述
优化 GenericBuilder M8 分组下的 Merge 和 Reopen 功能，扩展支持数组根 JSON，简化方法名并增加类型检查。

### 实现内容

**方法简化和重命名**
- 删除 `ReopenObject()` 方法，替换为 `Reopen()`，支持对象和数组的重开
- 删除 `MergeObject()` 静态方法，替换为 `Merge()` 静态方法重载
- 所有三个方法修改为返回 `bool` 类型，表示操作成功与否

**功能扩展**
- 增加 `Empty()` 方法到 M1 分组，封装 `stringT::empty()` 调用
- 扩展 `Reopen()` 方法支持数组根：检测 `}` 或 `]` 并替换为 `,`
- 扩展 `Merge()` 方法支持数组合并：检查粘连字符对（`}{` 或 `][`）

**类型检查优化**
- 采用简化的粘连逻辑：只检查前一个字符串的末尾字符和后一个字符串的开头字符
- 支持对象与对象合并（`}{` → `,`），数组与数组合并（`][` → `,`）
- 类型不匹配时返回 `false`，保持 wwjson 快速拼接的设计理念

**单元测试补充**
- 创建新测试文件 `utest/t_advance.cpp`
- 测试 `Reopen()` 方法对对象和数组的支持
- 测试 `Merge()` 实例方法和静态方法的合并功能
- 测试类型不匹配的边界情况
- 测试复杂场景的多重合并操作

### 技术细节

**设计原则遵循**
- 保持 wwjson 的快速拼接特性，不进行 JSON 解析验证
- 只检查粘连处的字符对，不验证整个 JSON 字符串的有效性
- 参数有效性由用户保证，库只负责快速拼接

**接口一致性**
- 所有方法使用 M1 分组封装的方法（`Empty()`, `Size()`, `Front()`, `Back()`）
- 保持与现有代码风格和命名约定的一致性

**向后兼容**
- 删除不再需要的 `ReopenObject()` 方法（项目中未使用）
- 新功能不破坏现有接口，测试用例全部通过

**构建系统更新**
- 在 `utest/CMakeLists.txt` 中添加 `t_advance.cpp` 到测试可执行文件

### 测试结果
- 编译测试：成功编译，无错误
- 功能测试：所有 19 个测试用例通过，包括新增的 4 个高级测试用例
- 边界测试：验证了各种类型不匹配和边界情况的正确处理

### 结果
成功完成需求 2025-11-27/4 的所有要求：
✓ 简化了方法名：`ReopenObject` → `Reopen`，`MergeObject` → `Merge`
✓ 扩展支持数组根 JSON 的合并操作
✓ 增加了类型检查，确保只有相同类型才能合并
✓ 所有方法返回 `bool` 表示操作结果
✓ 创建了全面的单元测试覆盖新功能
✓ 保持了 wwjson 快速拼接的设计理念
✓ 所有现有测试继续通过，确保向后兼容

## TASK:20251127-180345
-----------------------

### 任务概述
优化 GenericBuilder 方法间调用，简化代码复用，消除重复逻辑。

### 优化内容

**PutValue 方法优化**
- 让 `PutValue(const char*)` 调用 `PutValue(const char*, size_t)` 核心版本  
- 让 `PutValue(const std::string&)` 调用 `PutValue(const char*, size_t)` 核心版本
- 统一字符串转义逻辑到 `PutValue(const char*, size_t)` 方法
- 优化引号处理：使用 `PutChar('"')` 代替 `Append("\"")` 提升性能

**AddItem/AddMember 方法模板化**
- 使用 `template<typename... Args>` 参数包统一处理所有字符串类型
- 将原来3个字符串重载合并为1个通用模板方法
- 保持数字类型的SFINAE特化，确保 `kQuoteNumber` 配置正常工作
- 向后兼容，API接口保持不变

**Escape 方法简化**  
- 以 `(const char*, size_t)` 版本作为主方法实现
- 其他重载版本调用主方法，减少代码重复
- 使用 `::strlen` 全局命名空间确保函数调用正确

### 技术细节

**模板参数包的应用**
- AddItem: `template<typename... Args> AddItem(Args&&... args)`
- AddMember: `template<typename... Args> AddMember(const char* pszKey, Args&&... args)`
- 完美转发参数到 `PutValue` 方法

**SFINAE 兼容性**
- 数字类型重载的 `std::enable_if_t<std::is_arithmetic_v<numberT>>` 仍然有效
- 模板特化优先级确保数字处理逻辑正确

**性能优化**
- 减少函数调用链长度
- 消除重复的转义和引号处理逻辑
- 使用 `PutChar` 代替字符串拼接提升效率

### 代码统计
- **减少重复代码**: 约35行
- **逻辑集中**: 字符串处理统一到核心方法
- **维护性提升**: 修改逻辑只需调整核心方法

### 测试结果
- 编译测试：成功编译，无错误
- 功能验证：所有现有测试继续通过
- 向后兼容：API接口保持不变
### 结果
成功完成需求 2025-11-27/5 的所有要求：
✓ PutValue方法间调用优化，消除重复逻辑
✓ AddItem/AddMember方法模板化，减少代码重复
✓ Escape方法简化，使用主版本调用模式
✓ 所有优化保持向后兼容和功能正确性
✓ 编译和测试验证通过

## TASK:20251128-100515
-----------------------

### 任务概述
在 utest/t_basic.cpp 测试文件中补充用例，测试构造 JSON 时添加 null、bool、空数组、空对象，并增强 wwjson 库支持 nullptr 参数。

### 实现内容

**库功能增强**
- 在 `include/wwjson.hpp` 中添加 `PutValue(std::nullptr_t)` 重载函数
- 使 `AddMember` 和 `AddItem` 方法可以直接使用 `nullptr` 参数
- 保持向后兼容性，所有现有功能不受影响

**测试用例添加**
- `basic_null_bool_empty`：全面测试 null、bool、空数组和空对象
  - 使用 `AddMember` 和 `nullptr` 添加 null 值
  - 使用 `PutKey` + `PutNull` + `SepItem` 添加 null 值
  - 测试 bool 值（true/false）的添加
  - 测试空数组创建（`EmptyArray()`）
  - 测试空对象创建（`EmptyObject()`）
  - 展示 `SepItem()` 和 `PutNext()` 的使用方法

- `basic_low_level`：使用低层方法重构 basic_builder 功能
  - 使用 `PutKey`/`PutValue`/`PutNext` 替代 `AddMember`
  - 在数组中使用 `PutValue`/`PutNext` 替代 `AddItem`
  - 展示手动添加引号创建字符串形式数字
  - 在末尾添加数组使用低层方法

### 技术细节

**nullptr 支持实现**
- 通过模板参数包完美转发支持 `nullptr` 参数
- `PutValue(std::nullptr_t)` 内部调用 `PutNull()`
- 保持类型安全，避免空指针问题

**测试覆盖全面**
- 覆盖高层便利方法和低层手动控制方法
- 展示多种分隔符使用方法（`SepItem()` vs `PutNext()`）
- 验证数组和对象中混合 null、bool、字符串的处理

### 完成结果
成功完成任务要求：
✓ 增强了 wwjson 库支持 nullptr 参数
✓ 添加了全面的 null、bool、空数组、空对象测试用例
✓ 展示了高层和低层 API 的使用方法
✓ 所有测试通过，保持向后兼容性
✓ 为用户提供了直观的使用示例

## TASK:20251128-113745
-----------------------

### 任务概述
优化 GenericBuilder 类的方法结构，提取 PutNumber 方法统一数字判断逻辑，分析 EscapeString 方法的优化空间。

### 实现内容

**核心优化**
- 在 `include/wwjson.hpp` 中提取 `PutNumber` 方法统一处理数字引号逻辑
- 将 `PutNumber` 方法放置在 `PutKey` 方法之后，保持方法分组逻辑
- 修改 `AddItem` 和 `AddMember` 方法调用 `PutNumber` 替代重复代码
- 分析并保留 `EscapeString` 相关方法的原始设计，避免重载歧义

**代码重构**
- 消除了 `AddItem(numberT)` 和 `AddMember(const char*, numberT)` 中的重复引号判断逻辑
- 保持手动加引号版本（带 `bool` 参数）不变，提供灵活控制
- 恢复原始的 6 个 `AddItemEscape`/`AddMemberEscape` 重载方法

**测试验证**
- 在 `utest/t_custom.cpp` 添加 `custom_putnumber` 测试用例，验证 PutNumber 基本功能
- 添加 `custom_putnumber_quoted` 测试用例，验证自定义配置下的数字引号功能
- 所有 23 个测试用例通过，确保功能正确性和向后兼容性

### 技术细节

**PutNumber 方法实现**
- 使用模板特化和 `std::enable_if_t` 确保仅接受算术类型
- 根据 `configT::kQuoteNumber` 配置决定是否自动添加引号
- 提供统一且可配置的数字处理逻辑

**EscapeString 分析结果**
- 当前设计已优化：允许用户精确控制转义行为
- 避免提取单独的 `EscapeString` 方法，防止重载歧义
- 保持原始 6 个重载方法的完整性和性能优势

### 完成结果
成功完成任务要求：
✓ 提取 PutNumber 方法统一数字处理逻辑
✓ 优化 AddItem/AddMember 方法减少代码重复
✓ 分析 EscapeString 方法设计，确认其合理性
✓ 添加全面测试用例验证 PutNumber 功能
✓ 编译通过，所有 23 个测试用例正常运行
✓ 保持向后兼容性和性能优化特性


## TASK:20251128-163723
-----------------------

### 任务概述
使用转义表重构 BasicConfig 与 GenericBuilder 的 EscapeString 方法，减少参数传递，优化性能和代码维护性。

### 实现内容

**核心重构**
- 在 `include/wwjson.hpp` 的 `BasicConfig` 类中添加了编译期转义表 `kEscapeTable`
- 使用 128 元素的数组仅处理 ASCII 字符 (0-127)，优化内存使用
- 将原来的两个 EscapeString 重载方法替换为单一实现，移除了自定义字符集参数
- 优化转义逻辑：先检查字符范围 (c >= 128)，避免不必要的查表操作

**接口变更**
- 移除单字符转义重载 `EscapeString(dst, src, len, char)`
- 简化多字符转义版本为 `EscapeString(dst, src, len)`，统一使用固定转义表
- 更新 `EscapeKey` 方法，移除不必要的转义参数
- 同步修改 `GenericBuilder` 中所有调用 EscapeString 的方法

**测试重构**
- 从 `utest/t_basic.cpp` 移除 `basic_escape` 测试用例
- 重新设计 `utest/t_escape.cpp`，集中测试转义功能
- 添加覆盖 UTF-8 字符、性能优化和边界情况的测试用例
- 更新自定义配置测试以适配新接口

### 关键技术

**转义表设计**
```cpp
static constexpr auto kEscapeTable = []() constexpr {
    std::array<uint8_t, 128> table{};
    table['\n'] = 'n';
    // 定义常用转义映射 c --> \c
    table['\0'] = '0';
    return table;
}();
```

**性能优化**
- 转义表大小从 256 字节减少到 128 字节
- UTF-8 字符直接跳过，避免查表操作
- 预分配内存策略：`dst.reserve(dst.size() + len + len / 4)`

**兼容性处理**
- 保留 `kAlwaysEscape` 配置功能
- 更新 `AddItemEscape` 和 `AddMemberEscape` 方法使用新的统一接口
- 移除旧的重载版本，保持接口简洁

### 完成结果
成功完成任务要求：
✓ 实现了 128 元素的编译期转义表
✓ 重构 EscapeString 方法使用转义表，减少参数传递
✓ 移除了冗余的单字符转义重载
✓ 更新 GenericBuilder 适配新接口
✓ 重新设计测试用例，集中测试转义功能
✓ 所有测试用例通过（25个PASS，0个FAIL）
✓ 接口更简洁，性能更优，代码更易维护

## TASK:20251128-180546
-----------------------

### 任务概述
扩展 PutKey 方法支持三种字符串重载，将 BasicConfig::kAlwaysEscape 配置拆分为 kEscapeKey 和 kEscapeValue，分别控制键和值的转义。

### 实现内容

**配置重构**
- 在 `include/wwjson.hpp` 的 `BasicConfig` 类中将 `kAlwaysEscape` 拆分为：
  - `kEscapeKey = false` - 控制键的转义
  - `kEscapeValue = false` - 控制值的转义
- 保持向后兼容性，两个配置默认都是 false

**PutKey 方法扩展**
- 添加主方法 `PutKey(const char* pszKey, size_t len)`，支持显式长度
- 保留原有重载 `PutKey(const char* pszKey)`，调用主方法
- 新增重载 `PutKey(const std::string& strKey)`，调用主方法
- 根据 `kEscapeKey` 配置决定是否调用 `EscapeKey`

**PutValue 方法更新**
- 更新 `PutValue(const char* pszVal, size_t len)` 使用 `kEscapeValue` 而非 `kAlwaysEscape`
- 保持其他重载不变

**测试用例增强**
- 在 `utest/t_escape.cpp` 中新增配置类：
  - `EscapeKeyConfig` - 仅转义键，重写 `EscapeKey` 方法调用 `EscapeString`
  - `EscapeValueConfig` - 仅转义值
- 更新 `EscapeConfig` 使用新的分离配置
- 添加测试用例覆盖键值转义分离的各种场景

### 完成结果
成功完成任务要求：
✓ 扩展 PutKey 支持三种字符串重载，与 PutValue 保持一致
✓ 将 kAlwaysEscape 拆分为 kEscapeKey 和 kEscapeValue，独立控制键值转义
✓ 更新测试用例，新增 kEscapeKey 配置测试
✓ 所有测试用例通过（29个PASS，0个FAIL）
✓ PutKey 与 PutValue 的字符串重载支持保持一致
✓ 配置分离使得键值转义控制更加灵活


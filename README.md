# WWJSON - 高性能C++ JSON构建库

[![CI Status](https://github.com/lymslive/wwjson/workflows/CI/badge.svg)](https://github.com/lymslive/wwjson/actions)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Header-only](https://img.shields.io/badge/Header--only-✓-green.svg)](https://en.wikipedia.org/wiki/Header-only)
**语言:** [简体中文](README.md) | [English](README-en.md)

WWJSON是一个**仅头文件**的C++ JSON构建库，专注于**高性能JSON字符串生成**而无需DOM树构造。该库提供了简单、轻量级的JSON序列化方式，通过直接字符串操作实现高性能，特别适合需要快速构建JSON响应的应用场景。

## ✨ 核心特性

- 🚀 **高性能**: 直接字符串操作，无需DOM树构建开销
- 🎯 **轻量级**: 头文件only设计，无运行时依赖
- 🛡️ **类型安全**: 支持 C++17 编译期类型检查
- 🔧 **高度可配置**: 支持自定义字符串类型和序列化配置
- 📦 **易于集成**: 标准 CMake 包，支持 `find_package` 与 `FetchContent` 集成
- 🧪 **完整测试**: 丰富的单元测试 + 性能基准测试
- 🏗️ **模块化设计**: 单头文件提供基本功能，其他头文件拓展高级功能
- 🎨 **易用API**: 提供多种风格与不同抽象层次的 API，可精细控制 JSON 构建过程或快捷转换 JSON.

## 📋 快速开始

### 安装要求

- **C++标准**: C++17 或更高版本
- **编译器**: GCC 7+, Clang 6+, MSVC 2017+
- **构建系统**: CMake 3.15+

### 使用方式

#### 1. 直接包含头文件

拷贝单个头文件 `wwjson.hpp` 即可使用基本功能，例如：

```cpp
#include "wwjson.hpp"

int main() {
    wwjson::RawBuilder builder;
    builder.BeginObject();
    builder.AddMember("name", "wwjson");
    builder.AddMember("version", 1.0);
    builder.AddMember("features", [&]() {
        auto arr = builder.ScopeArray();
        arr.AddItem("fast");
        arr.AddItem("simple");
        arr.AddItem("header-only");
    });
    builder.EndObject();
    
    std::string json = builder.GetResult(); // 可用 MoveResult 更有效率
    // {"name":"wwjson","version":1.0,"features":["fast","simple","header-only"]}
    return 0;
}
```

#### 2. CMake 集成

建议使用 CMake 集成，完整安装，获得更多功能支持。

```cmake
# 查找包
find_package(wwjson 1.1 REQUIRED)

# 链接到你的目标
target_link_libraries(your_target PRIVATE wwjson::wwjson)
```

安装后，所有头文件位于 `include/wwjson/` 目录下，例如 `/usr/local/include/wwjson/`。
使用时，也要求包含相对路径的子目录，例如：

```cpp
#include "wwjson/jbuilder.hpp"

struct User {
    std::string name;
    int age;
    bool active;

    void to_json(wwjson::Builder& builder) const {
        TO_JSON(name);   // wwjson::to_json(builder, "name", name);
        TO_JSON(age);    // wwjson::to_json(builder, "age", age);
        TO_JSON(active); // wwjson::to_json(builder, "active", active);
    }
};

User user{"Alice", 30, true};
std::string json = wwjson::to_json(user);
// {"name":"Alice","age":30,"active":true}
```

### 构建与测试

```bash
# Debug构建 + 单元测试
make build
./build/utest/utwwjson --cout=silent

# Release构建 + 性能测试
make release
./build-release/perf/pfwwjson
```

若仅为使用安装的话，向 `cmake` 传入 `-DWWJSON_LIB_ONLY=ON` 选项，可以跳过编译
测试与示例项目。

详细使用指南请查看 [docs/usage.md](docs/usage.md)。

## 🏗️ 架构设计

### 头文件结构

本项目采用模块化头文件设计，可按需选择包含：

- **wwjson/wwjson.hpp** - 核心构建器与配置系统（必需）
- **wwjson/jstring.hpp** - 高性能字符串缓冲区（可选）
- **wwjson/jbuilder.hpp** - 高级便捷接口（可选）
  - `Builder`, `FastBuilder` - 常用构建器别名
  - `wwjson::to_json` - 统一的序列化 API
  - `TO_JSON` 宏 - 简化字段序列化

所有头文件统一安装到 `wwjson/` 子目录，使用时需包含完整路径。

### 核心组件

- **GenericBuilder**: 主构建器，支持定制字符串类型作为序列化目标
- **GenericObject/GenericArray**: 作用域管理的对象和数组
- **BasicConfig**: 可配置序列化选项（转义、逗号等）
- **StringConcept**: 字符串类型接口规范（std::string 小子集）

### 性能特点

- **小整数优化**: 0-99范围使用查表法，显著提升序列化速度
- **小范围浮点优化**: [0, 9999.9999]范围内快速序列化
- **可控转义**: 仅在必要时进行字符转义，减少开销
- **内存预分配**: 支持预估大小，减少内存重分配
- **额外边界扩容**: 定制的字符串扩容时自带额外安全边界，可减少频繁的格式字符的边界检查

### 推荐使用方式

对于大多数场景，建议使用 `wwjson/jbuilder.hpp` 提供的便捷别名：

- **RawBuilder** - 使用 `std::string`，通用性强
- **Builder** - 使用 `JString`，性能优化（unsafe 操作）
- **FastBuilder** - 使用 `KString`，性能最优（单次分配模式）

常规结构体序列化时，则推荐 `wwjson::to_json` 统一 API，大幅简化代码。

### 配置选项定制

提供编译期定制选项与序列化算法覆盖，以满足特定需求。

```cpp
struct MyConfig : wwjson::BasicConfig<std::string> {
    static constexpr bool kEscapeKey = true;     // 总是转义键名
    static constexpr bool kQuoteNumber = false;  // 数字不加引号
    static constexpr bool kTailComma = false;    // 不允许尾逗号

    // EscapseString: 字符串转义方法定制
    // NumberString: 数字转字符串算法定制
};

wwjson::GenericBuilder<std::string, MyConfig> builder;
```

## 🔬 质量保证

- ✅ **单元测试**: 完备的测试用例，覆盖所有API
- ✅ **性能基准**: 与yyjson等主流库对比测试
- ✅ **CI/CD**: GitHub Actions自动化测试
- ✅ **内存安全**: 无内存泄漏，RAII模式
- ✅ **类型安全**: 编译期类型检查

## 📚 参考文档

- [📖 完整用户指南](docs/usage.md) - 详细的功能介绍和使用示例
- [🔧 API参考文档](https://lymslive.github.io/wwjson/api/) - Doxygen生成的完整API文档
- [📊 性能测试报告](perf/report.md) - 详细的性能分析
- [💡 示例程序](example/README.md) - 实用的代码示例，展示各种高级用法

## 🤝 贡献

欢迎贡献代码、报告问题或提出改进建议！

1. Fork本仓库
2. 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 提交改动 (`git commit -m 'Add amazing feature'`)
4. 推送分支 (`git push origin feature/amazing-feature`)
5. 提交Pull Request

## 📄 许可证

本项目采用 [MIT License](LICENSE) 开源协议。

## 👥 致谢

- [couttast](https://github.com/lymslive/couttast) - 轻量级测试框架
- [yyjson](https://github.com/ibireme/yyjson) - 性能对比参考
- [xyjson](https://github.com/lymslive/xyjson) - 更易用 yyjson 封装，辅助验证工具

---

**注意**: WWJSON专注于JSON构建（序列化），不包含解析功能。如需JSON解析功能，建议结合其他专门的解析库使用。

# WWJSON - 高性能C++ JSON构建库

[![CI Status](https://github.com/lymslive/wwjson/workflows/CI/badge.svg)](https://github.com/lymslive/wwjson/actions)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Header-only](https://img.shields.io/badge/Header--only-✓-green.svg)](https://en.wikipedia.org/wiki/Header-only)

**语言:** [简体中文](README.md) | [English](README-en.md)

WWJSON是一个**仅头文件**的C++ JSON构建库，专注于**高性能JSON字符串生成**而无需DOM树构造。该库提供了简单、轻量级的JSON序列化方式，通过直接字符串操作实现高性能，特别适合需要快速构建JSON响应的应用场景。

## ✨ 核心特性

- 🚀 **高性能**: 直接字符串操作，无需DOM树构建开销
- 🎯 **轻量级**: 头文件only设计，无运行时依赖
- 🛡️ **类型安全**: 支持C++17编译期类型检查
- 🔧 **高度可配置**: 支持自定义字符串类型和序列化配置
- 📦 **易于集成**: 标准CMake包，支持`find_package`
- 🧪 **完整测试**: 丰富的单元测试 + 性能基准测试

## 📋 快速开始

### 安装要求

- **C++标准**: C++17 或更高版本
- **编译器**: GCC 7+, Clang 6+, MSVC 2017+
- **构建系统**: CMake 3.15+

### 使用方式

#### 1. 直接包含头文件

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
    
    std::string json = builder.GetResult();
    // {"name":"wwjson","version":1,"features":["fast","simple","header-only"]}
    return 0;
}
```

#### 2. CMake集成

```cmake
# 查找包
find_package(wwjson 1.0 REQUIRED)

# 链接到你的目标
target_link_libraries(your_target PRIVATE wwjson::wwjson)
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

更多构建选项请查看 [docs/usage.md](docs/usage.md)。

## 🎯 适用场景

- **Web API响应**: 快速生成JSON API响应
- **日志记录**: 高效结构化日志输出
- **配置管理**: 生成配置文件和参数
- **消息队列**: 序列化数据传输格式
- **数据导出**: 将内存数据转换为JSON格式

## 📊 性能特点

- **小整数优化**: 0-99范围使用查表法，显著提升序列化速度
- **小范围浮点优化**: [0, 9999.9999]范围内快速序列化
- **智能转义**: 仅在必要时进行字符转义，减少开销
- **内存预分配**: 支持预估大小，减少内存重分配

## 🏗️ 架构设计

### 核心组件

- **GenericBuilder**: 主构建器，支持定制字符串类型作为序列化目标
- **GenericObject/GenericArray**: 作用域管理的对象和数组
- **BasicConfig**: 可配置序列化选项（转义、逗号等）
- **StringConcept**: 字符串类型接口规范（std::string 小子集）

### 配置选项

```cpp
struct MyConfig : wwjson::BasicConfig<std::string> {
    static constexpr bool kEscapeKey = true;     // 总是转义键名
    static constexpr bool kQuoteNumber = false;  // 数字不加引号
    static constexpr bool kTailComma = false;    // 不允许尾逗号
};

wwjson::GenericBuilder<std::string, MyConfig> builder;
```

## 🔬 质量保证

- ✅ **单元测试**: 完备的测试用例，覆盖所有API
- ✅ **性能基准**: 与yyjson等主流库对比测试
- ✅ **CI/CD**: GitHub Actions自动化测试
- ✅ **内存安全**: 无内存泄漏，RAII模式
- ✅ **类型安全**: 编译期类型检查

## 📚 文档

- [完整用户指南](docs/usage.md) - 详细的功能介绍和使用示例
- [API参考](include/wwjson.hpp) - 头文件中的完整API文档
- [性能测试报告](perf/report.md) - 详细的性能分析

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
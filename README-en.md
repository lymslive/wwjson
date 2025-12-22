# WWJSON - High-Performance C++ JSON Builder

[![CI Status](https://github.com/lymslive/wwjson/workflows/CI/badge.svg)](https://github.com/lymslive/wwjson/actions)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Header-only](https://img.shields.io/badge/Header--only-✓-green.svg)](https://en.wikipedia.org/wiki/Header-only)
**Language:** [简体中文](README.md) | [English](README-en.md)

WWJSON is a **header-only** C++ JSON builder library focused on **high-performance JSON string generation** without DOM tree construction. It provides a simple, lightweight JSON serialization approach through direct string manipulation, ideal for applications requiring fast JSON response generation.

## ✨ Key Features

- 🚀 **High Performance**: Direct string operations without DOM overhead
- 🎯 **Lightweight**: Header-only design, no runtime dependencies
- 🛡️ **Type Safe**: C++17 compile-time type checking
- 🔧 **Highly Configurable**: Custom string types and serialization options
- 📦 **Easy Integration**: Standard CMake package with `find_package` support
- 🧪 **Fully Tested**: rich unit tests + performance benchmarks

## 📋 Quick Start

### Requirements

- **C++ Standard**: C++17 or higher
- **Compiler**: GCC 7+, Clang 6+, MSVC 2017+
- **Build System**: CMake 3.15+

### Usage

#### 1. Direct Header Inclusion

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

#### 2. CMake Integration

```cmake
# Find the package
find_package(wwjson 1.0 REQUIRED)

# Link to your target
target_link_libraries(your_target PRIVATE wwjson::wwjson)
```

### Build & Test

```bash
# Debug build + unit tests
make build
./build/utest/utwwjson --cout=silent

# Release build + performance tests
make release
./build-release/perf/pfwwjson
```

See [docs/usage.md](docs/usage.md) for more build options.

## 🎯 Use Cases

- **Web API Responses**: Fast JSON API response generation
- **Logging**: Efficient structured log output
- **Configuration**: Generate config files and parameters
- **Message Queues**: Serialize data transmission formats
- **Data Export**: Convert in-memory data to JSON format

## 📊 Performance Features

- **Small Integer Optimization**: 0-99 range uses lookup table for significant speedup
- **Small Float Optimization**: [0, 9999.9999] range fast serialization
- **Smart Escaping**: Only escape when necessary to reduce overhead
- **Memory Pre-allocation**: Support estimated size to reduce reallocations

## 🏗️ Architecture

### Core Components

- **GenericBuilder**: Main builder supporting arbitrary string types
- **GenericObject/GenericArray**: Scope-managed objects and arrays
- **BasicConfig**: Configurable serialization options (escaping, comma, etc.)
- **StringConcept**: String type interface specification

### Configuration Options

```cpp
struct MyConfig : wwjson::BasicConfig<std::string> {
    static constexpr bool kEscapeKey = true;     // Always escape keys
    static constexpr bool kQuoteNumber = false;  // Don't quote numbers
    static constexpr bool kTailComma = false;    // No trailing commas
};

wwjson::GenericBuilder<std::string, MyConfig> builder;
```

## 🔬 Quality Assurance

- ✅ **Unit Tests**: rich test cases covering all APIs
- ✅ **Performance Benchmarks**: Comparison with mainstream libraries like yyjson
- ✅ **CI/CD**: GitHub Actions automated testing
- ✅ **Memory Safety**: No memory leaks, RAII patterns
- ✅ **Type Safety**: Compile-time type checking

## 📚 Documentation

- [📖 Complete User Guide](docs/usage.md) - Detailed feature introduction and examples
- [🔧 API Reference](https://lymslive.github.io/wwjson/api/) - Doxygen-generated complete API documentation
- [📊 Performance Report](perf/report.md) - Detailed performance analysis

## 🤝 Contributing

Contributions, issues, and feature requests are welcome!

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the [MIT License](LICENSE).

## 👥 Acknowledgments

- [couttast](https://github.com/lymslive/couttast) - Lightweight testing framework
- [yyjson](https://github.com/ibireme/yyjson) - Performance comparison reference
- [xyjson](https://github.com/lymslive/xyjson) - JSON validation utility

---

**Note**: WWJSON focuses on JSON building (serialization) only, without parsing functionality. For JSON parsing, consider using dedicated parsing libraries alongside WWJSON.

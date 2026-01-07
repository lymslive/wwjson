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
- 📦 **Easy Integration**: Standard CMake package with `find_package` and `FetchContent` support
- 🧪 **Fully Tested**: rich unit tests + performance benchmarks
- 🏗️ **Modular Design**: Single header file provides basic functionality, additional headers extend advanced features
- 🎨 **Easy-to-use API**: Multiple styles and abstraction levels for fine-grained control or quick JSON conversion

## 📋 Quick Start

### Requirements

- **C++ Standard**: C++17 or higher
- **Compiler**: GCC 7+, Clang 6+, MSVC 2017+
- **Build System**: CMake 3.15+

### Usage

#### 1. Direct Header Inclusion

Copy the single header file `wwjson.hpp` to use basic functionality, for example:

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

    std::string json = builder.GetResult(); // Use MoveResult for better efficiency
    // {"name":"wwjson","version":1,"features":["fast","simple","header-only"]}
    return 0;
}
```

#### 2. CMake Integration

Using CMake integration is recommended for complete installation with additional features.

```cmake
# Find package
find_package(wwjson 1.1 REQUIRED)

# Link to your target
target_link_libraries(your_target PRIVATE wwjson::wwjson)
```

After installation, all header files are located in the `include/wwjson/` directory, e.g., `/usr/local/include/wwjson/`.
When using, you need to include with the relative path subdirectory, for example:

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

### Build & Test

```bash
# Debug build + unit tests
make build
./build/utest/utwwjson --cout=silent

# Release build + performance tests
make release
./build-release/perf/pfwwjson
```

If you only need to install, passing `-DWWJSON_LIB_ONLY=ON` to `cmake` can skip compiling tests and example projects.

For detailed usage guide, see [docs/usage.md](docs/usage.md).

## 🏗️ Architecture

### Header File Structure

This project uses a modular header file design, allowing selective inclusion as needed:

- **wwjson/wwjson.hpp** - Core builder and configuration system (required)
- **wwjson/jstring.hpp** - High-performance string buffer (optional)
- **wwjson/jbuilder.hpp** - Advanced convenience interfaces (optional)
  - `Builder`, `FastBuilder` - Common builder aliases
  - `wwjson::to_json` - Unified serialization API
  - `TO_JSON` macro - Simplified field serialization

All header files are uniformly installed to the `wwjson/` subdirectory, and the complete path must be included when used.

### Core Components

- **GenericBuilder**: Main builder supporting custom string types as serialization target
- **GenericObject/GenericArray**: Scope-managed objects and arrays
- **BasicConfig**: Configurable serialization options (escaping, commas, etc.)
- **StringConcept**: String type interface specification (subset of std::string)

### Performance Features

- **Small Integer Optimization**: 0-99 range uses lookup table for significant speedup
- **Small Float Optimization**: [0, 9999.9999] range fast serialization
- **Controlled Escaping**: Only escape when necessary to reduce overhead
- **Memory Pre-allocation**: Support estimated size to reduce reallocations
- **Extra Boundary Expansion**: Custom string expansion with additional safety boundary to reduce frequent boundary checks for format characters

### Recommended Usage

For most scenarios, it is recommended to use the convenience aliases provided by `wwjson/jbuilder.hpp`:

- **RawBuilder** - Uses `std::string`, highly versatile
- **Builder** - Uses `JString`, performance optimized (unsafe operations)
- **FastBuilder** - Uses `KString`, best performance (single-allocation mode)

For regular struct serialization, the `wwjson::to_json` unified API is recommended to significantly simplify code.

### Configuration Customization

Provides compile-time customization options and serialization algorithm overrides to meet specific needs.

```cpp
struct MyConfig : wwjson::BasicConfig<std::string> {
    static constexpr bool kEscapeKey = true;     // Always escape keys
    static constexpr bool kQuoteNumber = false;  // Don't quote numbers
    static constexpr bool kTailComma = false;    // No trailing commas

    // EscapseString: Custom string escaping method
    // NumberString: Custom number-to-string algorithm
};

wwjson::GenericBuilder<std::string, MyConfig> builder;
```

## 🔬 Quality Assurance

- ✅ **Unit Tests**: Complete test cases covering all APIs
- ✅ **Performance Benchmarks**: Comparison with mainstream libraries like yyjson
- ✅ **CI/CD**: GitHub Actions automated testing
- ✅ **Memory Safety**: No memory leaks, RAII patterns
- ✅ **Type Safety**: Compile-time type checking

## 📚 Documentation

- [📖 Complete User Guide](docs/usage.md) - Detailed feature introduction and examples
- [🔧 API Reference](https://lymslive.github.io/wwjson/api/) - Doxygen-generated complete API documentation
- [📊 Performance Report](perf/report.md) - Detailed performance analysis
- [💡 Example Programs](example/README.md) - Practical code examples showcasing various advanced usages

## 🤝 Contributing

Contributions, issues, and feature requests are welcome!

1. Fork this repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the [MIT License](LICENSE).

## 👥 Acknowledgments

- [couttast](https://github.com/lymslive/couttast) - Lightweight testing framework
- [yyjson](https://github.com/ibireme/yyjson) - Performance comparison reference
- [xyjson](https://github.com/lymslive/xyjson) - More convenient yyjson wrapper, auxiliary validation tool

---

**Note**: WWJSON focuses on JSON building (serialization) only, without parsing functionality. For JSON parsing, consider using dedicated parsing libraries alongside WWJSON.

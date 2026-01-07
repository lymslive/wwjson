# WWJSON 示例程序

本目录包含三个示例程序，展示了 WWJSON 库的不同使用场景和高级功能。每个示例都可以独立编译运行，帮助您快速理解库的各种特性。

## 示例列表

### 1. struct_to_json.cpp - 结构体转 JSON

展示如何使用 `wwjson::to_json` API 将多层嵌套的结构体序列化为 JSON。

**特性展示：**
- 使用 `wwjson::to_json(builder, key, value)` 显式 API 调用
- 使用 `TO_JSON(field)` 宏简化字段序列化
- 支持嵌套结构体（Address -> Publisher -> Author -> Book）
- 支持 `std::vector` 等顺序容器
- 顶层结构体的无参数 `to_json()` 方法可直接返回 JSON 字符串

**编译运行：**
```bash
make build
cd build
make example_struct_to_json
./example/example_struct_to_json
```

**代码片段：**
```cpp
struct Book {
    std::string title;
    std::string isbn;
    double price;
    std::vector<Author> authors;
    int publishedYear;

    void to_json(Builder& builder) const {
        TO_JSON(title);
        TO_JSON(isbn);
        TO_JSON(price);
        TO_JSON(authors);  // 自动处理 vector
    }

    std::string to_json() const {
        return wwjson::to_json(*this);
    }
};

Book book;
// ... 填充数据 ...
std::string json = book.to_json();  // 直接得到 JSON 字符串
```

### 2. estimate_size.cpp - JSON 大小估算

演示如何创建自定义 Builder 来估算 JSON 输出的大小，而无需实际构建字符串。这对于预分配缓冲区或确定存储需求非常有用。

**特性展示：**
- 自定义 `EString` 字符串类，只跟踪长度不存储数据
- 自定义 `EConfig` 配置，覆盖字符串转义和数字序列化方法
- 使用 `EstBuilder` 估算大小
- 结合 `FastBuilder`（使用 KString）进行实际构建，通过预分配容量提升性能

**编译运行：**
```bash
make build
cd build
make example_estimate_size
./example/example_estimate_size
```

**代码片段：**
```cpp
// 定义估算用的字符串类
struct EString {
    size_t length;
    void append(const char*, size_t len) { length += len; }
    // ... 其他方法 ...
};

// 定义估算用的配置
struct EConfig : public wwjson::BasicConfig<EString> {
    static void EscapeString(EString& dst, const char* src, size_t len) {
        dst.append(src, len * 2);  // 最坏情况估算
    }

    template <typename numberT>
    static void NumberString(EString& dst, numberT value) {
        if constexpr (sizeof(numberT) <= 4) {
            dst.append("12345678901", 11);  // 32位整数最大长度
        } else {
            dst.append("123456789012345678901", 21);  // 64位整数最大长度
        }
    }
};

using EstBuilder = wwjson::GenericBuilder<EString, EConfig>;

// 估算大小
EstBuilder estBuilder;
// ... 构建 JSON ...
size_t estimatedSize = estBuilder.json.size();

// 使用估算的大小构建 FastBuilder
wwjson::FastBuilder fastBuilder(estimatedSize);
// ... 构建 JSON ...
```

### 3. hex_json.cpp - 十六进制 JSON 表示

演示如何创建自定义配置（HexConfig），将所有字符串和整数转换为十六进制表示。

**特性展示：**
- 自定义 `HexConfig` 配置，继承 `BasicConfig`
- 字符串：每个字节转换为两个十六进制数字
- 整数：转换为十六进制并添加 `0x` 前缀
- 浮点数：使用标准 `%g` 格式
- 支持嵌套结构和数组

**编译运行：**
```bash
make build
cd build
make example_hex_json
./example/example_hex_json
```

**代码片段：**
```cpp
template <typename stringT>
struct HexConfig : public wwjson::BasicConfig<stringT> {
    static constexpr bool kEscapeValue = true;
    static constexpr bool kQuoteNumber = true;

    // 将字符串转换为十六进制
    static void EscapeString(stringT& dst, const char* src, size_t len) {
        static constexpr char hex_table[] = "0123456789abcdef";
        for (size_t i = 0; i < len; ++i) {
            unsigned char c = static_cast<unsigned char>(src[i]);
            dst.push_back(hex_table[c >> 4]);
            dst.push_back(hex_table[c & 0x0F]);
        }
    }

    // 将整数转换为十六进制
    template <typename numberT>
    static std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    NumberString(stringT& dst, numberT value) {
        if constexpr (std::is_integral_v<numberT>) {
            dst.append("0x");
            // ... 转换逻辑 ...
        }
    }
};

using HexBuilder = wwjson::GenericBuilder<std::string, HexConfig<std::string>>;

HexBuilder builder;
// ... 构建 JSON ...
std::string json = builder.MoveResult();
// "Hello" -> "48656c6c6f"
// 255 -> "0xff"
```

## 构建所有示例

```bash
# 构建
make build
cd build
make examples

# 运行所有示例
make run_examples
```

## 使用 WWJSON 作为外部项目

本目录的示例展示了三种不同的使用方式：

### 方式 1：源码树中使用（如本项目的构建）

```cmake
# 在项目根目录
cmake -B build -S .
make build
cd build
make example_struct_to_json  # 构建单个示例
# 或
make examples                # 构建所有示例
```

### 方式 2：先安装 wwjson，再通过 find_package

```bash
# 安装 wwjson
make install

# 在您的项目中
cmake -B build -S .
make example_struct_to_json
```

### 方式 3：不安装 wwjson，使用 FetchContent 集成

```cmake
# 在您的 CMakeLists.txt 中
include(FetchContent)
FetchContent_Declare(
    wwjson
    GIT_REPOSITORY https://github.com/lymslive/wwjson.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(wwjson)

target_link_libraries(your_target PRIVATE wwjson::wwjson)
```

## 技术要点

### Builder 类型选择

- **RawBuilder**: 使用 `std::string` 作为写入目标，通用性强
- **Builder**: 使用 `JString`（`StringBuffer<4>`），支持 unsafe 操作提升性能
- **FastBuilder**: 使用 `KString`（`StringBuffer<255>`），单次分配模式，性能最优

### 配置定制

通过继承 `BasicConfig` 可以自定义序列化行为：
- 覆盖 `EscapeString` 方法自定义字符串转义
- 覆盖 `NumberString` 方法自定义数字序列化
- 调整 `kEscapeKey/kEscapeValue` 控制自动转义
- 调整 `kQuoteNumber` 控制数字是否加引号

### to_json API

提供统一的序列化接口：
- `wwjson::to_json(builder, key, value)`: 带键名
- `wwjson::to_json(builder, value)`: 不带键名
- `TO_JSON(field)`: 宏简化版本，假设 builder 变量名为 `builder`
- 自动支持标量类型、结构体和顺序容器

## 学习建议

1. **入门**: 先阅读 `struct_to_json.cpp`，了解便利的 `to_json` API 用法
2. **进阶**: 阅读 `estimate_size.cpp`，学习如何自定义字符串类和配置
3. **高级**: 阅读 `hex_json.cpp`，深入理解配置系统的强大功能

每个示例都包含详细的注释和输出说明，便于理解和学习。

# WWJSON 用户指南

本文档提供 WWJSON 库的完整使用指南，包括详细的功能介绍、使用方法和实际示例。

- [1 概述简介](#概述简介)
- [2 快速入门](#快速入门)
- [3 基本思路](#基本思路)
- [4 核心用法](#核心用法)
- [5 特殊用法](#特殊用法)
- [6 配置定制](#配置定制)
- [7 快速参考](#快速参考)

## 1 概述简介

WWJSON 是一个专门用于**快速构建 JSON 字符串**的 C++ 库。它采用直接字符串操作的
策略，避免了传统 DOM 树构建的开销，特别适合以下场景：

- 追求序列化 json 速度的场合，wwjson 基础性能优异，还提供编译期定制化的扩展；
- 追求序列化 json 低内存消耗的场合；
- 追求低依赖的场合，如果只有输出 json 而没有解析 json 的需求，wwjson 最适合；

WWJSON 的核心设计理念是读写分离，它只处理写 json 的部分，而不处理读 json 的部
分。而且是“只读可写”，意思是它在构建 json 过程中，对于已经写入的部分，不可再
修改（实际上仅需要修改最后一个尾逗号字符）。

这不同于传统以 DOM 为中心、支持动态修改每个结点的 json 库。wwjson 也相当于伪装
成 json 库的字符串拼接技术，它提供的 api 貌似与其他 json 库一样在添加 json 结
点，实际直接写入目标字符串了。

## 2 快速入门

### 2.1 安装

WWJSON采用头文件only的设计，只需下载头文件即可使用：

```bash
# 下载头文件
wget https://raw.githubusercontent.com/lymslive/wwjson/main/include/wwjson.hpp
```

或者通过CMake集成：

```bash
# 克隆仓库
git clone https://github.com/lymslive/wwjson.git
cd wwjson

# 构建安装
make install
```

### 2.2 第一个示例

<!-- example:usage_2_2_first_example -->
```cpp
#include "wwjson/wwjson.hpp"
#include <iostream>

int main()
{
    // 创建JSON构建器
    wwjson::RawBuilder builder;
    
    // 开始构建对象
    builder.BeginObject();
    builder.AddMember("name", "WWJSON");
    builder.AddMember("version", 1.0);
    builder.AddMember("type", "header-only");
    builder.AddMember("language", "C++");
    builder.AddMember("license", "MIT");
    builder.EndObject();
    
    // 获取结果，更推荐 MoveResult()
    std::string json = builder.GetResult();
    std::cout << json << std::endl;
    
    return 0;
}
```

运行结果：
```json
{"name":"WWJSON","version":1,"type":"header-only","language":"C++","license":"MIT"}
```

更多示例及编译方式请参考源码仓库
[example](https://github.com/lymslive/wwjson/tree/main/example) 子目录。

## 3 基本思路
<!-- WWJSON 构建原理详解 -->

### 3.1 JSON 拼接术的基本原理

假设要写一个很简单的 json ，可以不借用任务三方 json 库，直接用 `snprintf` 或
`sstream` 甚至 `string` 拼接就可以：

<!-- example:usage_3_1_json_concatenation -->
```cpp
int code = 0;
std::string message = "OK";

// 1. 使用 C printf
{
    char buff[64];
    snprintf(buff, sizeof(buff), R"({"code":%d,"message":"%s"})", code, message.c_str());
    std::string json = buff;
}

// 2. 使用 C++ stream
{
    std::ostringstream oss;
    oss << '{'
        << R"("code":)" << code << ','
        << R"("message":)" << "\"" << message << "\""
        << '}';
    std::string json = oss.str();
}

// 3. 直接使用 std::string
{
    std::string json;
    json.reserve(1024);
    json.push_back('{');
    json.push_back('"');
    json.append("code");
    json.push_back('"');
    json.push_back(':');
    json.append(std::to_string(code));
    json.push_back(',');
    json.push_back('"');
    json.append("message");
    json.push_back('"');
    json.push_back(':');
    json.push_back('"');
    json.append(message);
    json.push_back('"');
    json.push_back('}');
}
```

这能产生一个 json 串 `{"code":0,"message":"OK"}`，在一些 api 执行成功时很可能
就返回类似这样简单的 json 。理论上可以这么干，但实践中几乎不可能这么干，代码可
读性与可扩展性都很差，简言之就是没有实用性。

### 3.2 WWJSON 对字符串拼接术的封装

WWJSON 的底层原理，其实就与上例代码的方法 3 一样，直接往目标字符串中添料，但是
提供了高层抽象的 api ：

<!-- example:usage_3_2_wwjson_encapsulation -->
```cpp
wwjson::RawBuilder builder;
builder.BeginObject();
builder.AddMember("code", code);
builder.AddMember("message", message);
builder.EndObject();

std::cout << builder.json << std::endl;
//^ 输出：{"code":0,"message":"OK"},
std::cout << builder.GetResult() << std::endl;
//^ 输出：{"code":0,"message":"OK"}
```

这里的 5 行代码与前例直接使用 `std::string` 类型的 `json` 变量的 17 行代码几乎
做相同的事情。而且 `builder` 对象只有一个唯一的公开成员就叫 `json` ，这是特意
开放暴露底层的，因为它没有什么特别的技术含量。虽然可能不常见，但如有需求，完全
可以在构建 json 之前添加一些前缀字符串，或构建之后附加一些后缀字符串，或者调用
标准库算法将 json 串的中小写字母都转换为大写字母等骚操作。

甚至，如果觉得 `BeginObject` 这方法名太长不好记，也可以直接改用
`.json.push_back('{')` 。当然，我并不鼓励这么做，这不对称，而 `EndObject` 并不
能简单替换回 `push_back('}')` ，它还会处理尾逗号。因为前面的 `AddMember` 会额
外加个逗号 ，所以最后要在 `EnbOBject` 修正该逗号。

WWJOSN 常用方法 与 string 方法简单的对照关系如下：
- RawBuilder 构造函数：默认 resever(1024)，可传预估容量参数
- BeginObject: push back 左大括号 `{`
- AddMember: append `"key":value,`，自动处理各种类型，并加个逗号
- EndObject: 将前面的逗号改为 `}` ，再附加个逗号，因为要适配嵌套结构
- GetResult: pop back 移除末尾的逗号，再返回结果
- MoveResult: 修正尾逗号后 `std::move(json)` ，转移结果

每次 `AddMember` 在末尾自动加逗号，是为了拼接性能更佳，不用每次都判断前面有没
逗号，只用在 `EndOject` 中判断一次逗号。而又由于嵌套对象可能是父对象的成员，所
以 `EndObject` 也像 `AddMember` 一样再附加一个逗号，以便继续拼接。

这就引发另一个小问题，如果用 `EndObject` 结束根对象，它也会多一个尾逗号需要处
理。所以推荐使用 `GetResult` 方法获取最后构建的结果，而不是直接使用 `.json` 成
员。也可以用 `EndRoot` 代替最后的 `EndObject` 明确表示结束根对象，它就不会多加
尾逗号，可以直接用 `.json` ，能节省微不足道的性能开销。

构造另一个 json 容器，数组相关的方法与对象相关方法也类似：

| 操作说明   | 对象方法 API       | 数组方法 API    | 尾逗号   |
|------------|--------------------|-----------------|----------|
| 开启根结构 | BeginRoot()        | BeginRoot('\[') |          |
| 开启子结构 | BeginObject()      | BeginArray()    |          |
| 添加子元素 | AddMember(key,val) | AddItem(val)    | 加逗号   |
| 关闭子结构 | EndObject()        | EndArray()      | 加逗号   |
| 关闭根结构 | EndRoot()          | EndRoot(']')    | 不加逗号 |

由于实践中发现 json 根对象的情况应该比根数组多，所以 `BeginRoot` 与 `EndRoot`
默认处理对象，如果是根数组，需传 `[` 或 `]` 作为可选参数。如果能记住每次取最后
结果时使用 `GetRsult` 或 `MoveResult` 的话，那就没必要记 `BeginRoot` 与
`EndRoot` 的存在。但强调它们的区别也反映了 wwjson 的实现细节。

另外在内部实现中， `AddMember` 在写完 `key` 部分后，其实是直接调用 `AddItem`
写 `val` 部分的。凡是 `AddItem` 支持的参数表，在前面多加一个字符串参数，就能调
用 `AddMember` 。支持以下字符串类型当作 `key` ：

- C Style 字符串 `const char *` ，也包括字面量，与非常量字符数组 buffer ，
- std::string
- std::string_view

字符串当作值参数时，还支持双参数的 `(const char*, size_t)` 形势。对于其他自定
义字符串类，一般实践中会提供 `c_str()` 与 `size()` 方法，也就能转为以上类型了
。

`AddItem(val)` 值参数支持的其他类型包括：
- 整数，从 uint8 与 uint64 的 8 种标准整数
- 浮点数，例如 float, doule
- bool ，写入 json 常量 true/false
- nullptr, 写入 json 特殊值 null
- 生成字符串（json 子串）的 lambda 或函数

### 3.3 与其他 json 库的构建对比

也以上述简单示例 `{"code":0,"message":"OK"}` 为目标，其他主流 C++ 库的写法风格
大约如下（wwjson 也再列于末尾作为对比）：

<!-- example:NO_TEST -->
```cpp
// 1. rapidjson
{
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("code", 0, doc.GetAllocator());
    doc.AddMember("message", "OK", doc.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::string json = buffer.GetString();
}

// 2. yyjson
{
    yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val* root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);
    yyjson_mut_obj_add_int(doc, root, "code", 0);
    yyjson_mut_obj_add_str(doc, root, "message", "OK");

    char* json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
    std::string json = json_str;

    free(json_str);
    yyjson_mut_doc_free(doc);
}

// 3. nlohmann/json
{
    nlohmann::json j;
    j["code"] = 0;
    j["message"] = "OK";

    std::string json = j.dump();
}

// 4. simdjson
{
    simdjson::dom::object obj;
    obj["code"] = 0;
    obj["message"] = "OK";

    std::string json = simdjson::to_string(obj);
}

// 5. wwjson
{
    wwjson::RawBuilder builder;
    builder.BeginObject();
    builder.AddMember("code", 0);
    builder.AddMember("message", "OK");
    builder.EndObject();

    std::string json = builder.MoveResult();
}
```

从代码行数简洁度看，yyjson 因是纯 C 库需手动管理内存就不看了。nlohmann/json 与
simjson 看似最简洁，构造 DOM 只需三行。不过使用操作符的差异不关键，wwjson 也支
持操作符重载的语法糖，用 `builder[key] = val;` 代替 `AddMember(key,val)` 。

WWJSON 选用 `AddMember` 这个方法名，也正源于 rapidjson ，读写分离的思想来源于
yyjson 。所以说 wwjson 是伪装成 json 库的字符串拼接库，它具有与主流 json 库构
造 DOM 类似的 api 。只是多了需要手动配对的 `BeginObject` 与 `EndObject` ，事实
上，wwjson 也支持利用 RAII 特性来避免 `EndObject` 配对的心智开销。当然
`BeginObject` 或 `BeginArray` 还是少不了，其他 json 库也大多需要创建一个对象或
数组的容器变量吧。

从代码形式上看，wwjson 的最后一行 `GetResult` 或 `MoveResult` 对应着其他 json
的 `write/to_string/dump` 等方法。但从本质上讲，那些方法其实是在干类似 wwjson
前面 `BeginObject/AddMember/EndObject` 的事。wwjson 伪装的“构建 DOM”过程直接
写字符串了，最后一行只处理无聊但要紧的一个尾逗号问题。

WWJSON 正是由于跳过了 DOM 的中间层的复杂数据结构，可以大幅降低内存需求，提高运
行效率，使它在序列化输出 json 这个专门的事上有显著优势。

当误用 api 时，wwjson 不保证输出合法的 json 。其他 json 库也无法保证用户不误用
api ，比如往对象中 `AddItem` ，往数组中 `AddMember` ，在数组中用 `[]` 索引字符
串键，在对象中用 `[]` 索引整数键。这会发生什么，在使用其 json 库时是无法预料的
（至少不熟悉官方文档或实现时不能马上回答），有些 json 库可能抛异常（而又没
catch 的话）导致程序崩溃。而在 wwjson 中，这是可预料的行为，不外是拼错 json
串了，json 格式不对的错误是很容易被测试的，日志打印出来也很容易被发现与纠正。

不过 wwjson 不提供 json 解析功能，当需要读取 json 时，应该配合其他高性能 json
解析库使用。当业务需求是解析输入 json ，修改部分数据，再序列化输出回写时，直接
用解析的 json DOM 结构可能更方便。不过，当对 C++ 程序有进一步的性能追求时，重
度依赖三方库的 json DOM 结构也是不合适的，不管这数据结构设计如何精妙，读写 DOM
结点始终有额外开销，不如使用 C++ 原生数据类型。wwjson 适合的场景就是在 C++ 程
序中使用原生结构体或对象管理数据，进行业务计算，当一切数据准备妥当后，可利用
wwjson 进行灵活而高性能的 json 序列化输出。

## 4 核心用法

构建单层对象很简单，乏善可陈，但 json 作为通用数据交换格式，在实践中一般是通过
多层次结构来表达业务意义的。本节通过构建多层 json 结构来介绍 wwjson 的核心功能
与基本用法。

假设要构建如下 json 数据：

```json
{
  "name":"wwjson","version":1.01,"author":"lymslive","url":null,
  "feature":{"standar":"C++17","dom":false,"config":"compile-time"},
  "refer":["rapidjson","nlohmann/json",{"name":"yyjson","lang":"C"}]
}
```

当然，wwjson 的输出都是单行无格式的压缩型 json ，只是文档中为避免单行太长分几
行书写了。

### 4.1 平铺直叙型的 Begin/End 配对法

这是对 wwjson 的基本 API 的简单扩展复用，只要记得 Begin/End 配对即可。手写时一
般建议写完 Begin 语句后，立即写 End 语句，再往中间插语句。

<!-- example:usage_4_1_flat_construction -->
```cpp
wwjson::RawBuilder builder;
builder.BeginRoot();

// 基本字段
builder.AddMember("name", "wwjson");
builder.AddMember("version", 1.01);
builder.AddMember("author", "lymslive");
builder.AddMember("url", nullptr);  // null值

// 嵌套对象 feature
builder.BeginObject("feature");
builder.AddMember("standar", "C++17");
builder.AddMember("dom", false);
builder.AddMember("config", "compile-time");
builder.EndObject();

// 数组 refer
builder.BeginArray("refer");
builder.AddItem("rapidjson");
builder.AddItem("nlohmann/json");

// 数组中的嵌套对象
builder.BeginObject();
builder.AddMember("name", "yyjson");
builder.AddMember("lang", "C");
builder.EndObject();

builder.EndArray();

builder.EndRoot();
```

这里使用 `BeginRoot/EndRoot` 开启与关闭根对象。仅管效果与
`BeginObject/EndObject` 几乎一样，但在有很多子对象时，使用更有区分度的
`EndRoot` 是更佳实践。

然后使用 `BeginObject/Array` 添加子对象或子数组。当上层是对象结构时，需要额外
传入键名参数；当上层是数组结构时，默认空参数即可。其实有以下等效“公式”：

- `BeginObject()` = `{`
- `BeginArray()` = `[`
- `AddItem("val")` = `"val",`
- `AddMember("key","val") = `AddMember("key") + AddItem("val")` = `"key":"val",`
- `BeginObject("key")` = `AddMember("key") + BeginObject()` = `"key":{`
- `BeginArray("key")` = `AddMember("key") + BeginArray()` = `"key":\[`
- `EndObject()` = `},` （近似等效）
- `EndArray()` = `],` （近似等效）

### 4.2 利用 Scope 作用域变量

如果觉得平铺直叙型的构建方式，在源码视觉效果上层次不明显，首先可以尝试对源码格
式作重排，加些裸 `{}` 增加层次缩进，例如：

<!-- example:usage_4_2_brace_indentation -->
```cpp
wwjson::RawBuilder builder;
builder.BeginRoot();
{
    // 基本字段
    builder.AddMember("name", "wwjson");
    builder.AddMember("version", 1.01);
    builder.AddMember("author", "lymslive");
    builder.AddMember("url", nullptr);

    // 嵌套对象 feature
    builder.BeginObject("feature");
    {
        builder.AddMember("standar", "C++17");
        builder.AddMember("dom", false);
        builder.AddMember("config", "compile-time");
    }
    builder.EndObject();

    // 数组 refer
    builder.BeginArray("refer");
    {
        builder.AddItem("rapidjson");
        builder.AddItem("nlohmann/json");

        // 数组中的嵌套对象
        builder.BeginObject();
        {
            builder.AddMember("name", "yyjson");
            builder.AddMember("lang", "C");
        }
        builder.EndObject();
    }
    builder.EndArray();
}
builder.EndRoot();
```

这样，层次感不就上来了。这是把 `BeginObject/EndObject` 放在所关联的 `{}` 的外
面，还有另一种风格选择，把 `BeginObject/EndObject` 分别放在 `{}` 里面的第一条
与最后一条语句。对于这另一种缩进风格，wwjson 还特地设计了方法支持，使之可以省
略 `{}` 内最后一条 `EndObject` 语句。例如：

<!-- example:usage_4_2_scope_variables -->
```cpp
wwjson::RawBuilder builder;
{
    auto _close = builder.ScopeObject();
    // 基本字段
    builder.AddMember("name", "wwjson");
    builder.AddMember("version", 1.01);
    builder.AddMember("author", "lymslive");
    builder.AddMember("url", nullptr);

    // 嵌套对象 feature
    {
        auto _close = builder.ScopeObject("feature");
        builder.AddMember("standar", "C++17");
        builder.AddMember("dom", false);
        builder.AddMember("config", "compile-time");
    }

    // 数组 refer
    {
        auto _close = builder.ScopeArray("refer");
        builder.AddItem("rapidjson");
        builder.AddItem("nlohmann/json");

        // 数组中的嵌套对象
        {
            auto _close = builder.ScopeObject();
            builder.AddMember("name", "yyjson");
            builder.AddMember("lang", "C");
        }
    }
}
std::string json = builder.GetResult();
```

注意到 `ScopeObject` 方法返回一个（C++）对象，其类型也不必记，用 `auto` 接收就
行。返回的这个局部变量可以不显式去用它，实际上用到它的构造函数与析构函数，构造
时自动调用 `BeginOjbec` 方法（也可以传入键名参数），析构时自动调用 `EndObject`
方法。另一个 `ScopeArray` 的行为相似，自动开启关闭数组 `[]` 。相当于以下公式：

- `ScopeObject` = `BeginObject` ... `EndObject` = `{` ... `}`
- `ScopeArray` = `BeginArray` ... `EndArray` = `[` ... `]`

这两个方法返回的局部变量也是可以使用的，能代替当前 `builder` 变量调用
`AddMember` 或 `AddItem` 。取合适的变量名，或能进一步增加代码可读性，例如：

<!-- example:usage_4_2_named_scope_variables -->
```cpp
wwjson::RawBuilder builder;
{
    auto root = builder.ScopeObject();
    // 基本字段
    root.AddMember("name", "wwjson");
    root.AddMember("version", 1.01);
    root.AddMember("author", "lymslive");
    root.AddMember("url", nullptr);

    // 嵌套对象 feature
    {
        auto feature = root.ScopeObject("feature");
        feature.AddMember("standar", "C++17");
        feature.AddMember("dom", false);
        feature.AddMember("config", "compile-time");
    }

    // 数组 refer
    {
        auto refer = root.ScopeArray("refer");
        refer.AddItem("rapidjson");
        refer.AddItem("nlohmann/json");

        // 数组中的嵌套对象
        {
            auto obj = refer.ScopeObject();
            obj.AddMember("name", "yyjson");
            obj.AddMember("lang", "C");
        }
    }
}
std::string json = builder.GetResult();
```

如果觉得裸加 `{}` 很奇怪，有强行划分作用域的生硬感，那么还可以将 `auto` 变量的
创建再提到上面的一条 `if` 语句中，在 `if` 后面接一对大括号就很标准了吧：

<!-- example:usage_4_2_if_statement_scope -->
```cpp
wwjson::RawBuilder builder;
if (auto root = builder.ScopeObject())
{
    // 基本字段
    root.AddMember("name", "wwjson");
    root.AddMember("version", 1.01);
    root.AddMember("author", "lymslive");
    root.AddMember("url", nullptr);

    // 嵌套对象 feature
    if (auto feature = root.ScopeObject("feature"))
    {
        feature.AddMember("standar", "C++17");
        feature.AddMember("dom", false);
        feature.AddMember("config", "compile-time");
    }

    // 数组 refer
    if (auto refer = root.ScopeArray("refer"))
    {
        refer.AddItem("rapidjson");
        refer.AddItem("nlohmann/json");

        // 数组中的嵌套对象
        if (auto obj = refer.ScopeObject())
        {
            obj.AddMember("name", "yyjson");
            obj.AddMember("lang", "C");
        }
    }
}
std::string json = builder.GetResult();
```

在 C++ 中，`if` 语句的小括号内也可以像 `for` 语句一样创建局部变量，然后写第二
个条件判断语句，当然不能再像 `for` 添加第三个分语句表示循环变量更新。当 `if`
语句省略第二分语句时，就将第一个初始化语句的值也当作条件判断。如上例的第一个
`if` 语句，也相当于如下的简写：

<!-- example:NO_TEST -->
```cpp
if (auto root = builder.ScopeObject(); root)
```

而 `wwjson::RawBuilder` 及其用 `ScopeObject` 方法创建的变量类型，重载了
`operator bool` ，所以可以将 `builder` 及 `root` 局部变量放到 `if` 的条件上下
文中。`operator bool` 的实现语义是判断内部所拼接的 json 字符串是否为空，一旦开
始构建，json 串就不会为空，始终返回 `true` ，因此 `if` 语句下面的 `{}` 语句
块始终会被运行。

最后一个小提醒，由于 `Scope` 变量的析构是调用 `EndObject`（或 `EndArray`），会
自动多加一个尾逗号，所以上例 `root` 变量退出作用域后会多一个逗号，必须调用一次
`GetResult` 方法修正。也可以在构建根结点时仍使用 `BeginRoot/EndRoot` 对，对更
深层次的结构才能 `Scope` 变量，能减少一层缩进。

### 4.3 使用 lambda 构建子结构

`AddItem` 与 `AddMember` 的值参数，允许传一个 lambda ，表达构建子结构的意图。

<!-- example:usage_4_3_lambda_substructure -->
```cpp
wwjson::RawBuilder builder;
builder.BeginRoot();

// 基本字段
builder.AddMember("name", "wwjson");
builder.AddMember("version", 1.01);
builder.AddMember("author", "lymslive");
builder.AddMember("url", nullptr);

// 嵌套对象 feature
builder.AddMember("feature", [&builder]() {
    auto feature = builder.ScopeObject();
    feature.AddMember("standar", "C++17");
    feature.AddMember("dom", false);
    feature.AddMember("config", "compile-time");
});

// 数组 refer
builder.AddMember("refer", [&builder]() {
    auto refer = builder.ScopeArray();
    refer.AddItem("rapidjson");
    refer.AddItem("nlohmann/json");

    // 数组中的嵌套对象
    refer.AddItem([&builder]() {
        auto obj = builder.ScopeObject();
        obj.AddMember("name", "yyjson");
        obj.AddMember("lang", "C");
    });
});

builder.EndRoot();
```

支持两种可调用函数签名，lambda 一般使用空参数版本更简洁，直接捕获当前
`builder` 引用。如果是外部函数，则需要传递 `builder` 引用参数。被调函数应该直
接使用 `builder` 原位构建子 json 结构。

在 lambda 函数体中，可以用 `Begin/End` 配对函数，也能用 `Scope` 方法构建子结构，
但显然不宜用 `BeginRoot/EndRoot` 。

### 4.4 操作符重载使用

从以上几种使用风格可知，在 wwjson 中除了表达层次结构的 `Object` 与 `Array` 开
闭方法，使用最多的是 `AddItem` 与 `AddMember` 方法。为此重载了 `[]` 与 `=` 操
作符，可视为这两个方法的语法糖：

- `[int] = val` = `AddItem(val)`
- `[key] = val` = `AddMember(key, val)`

操作符 `[]` 返回 `this` 对象本身，当参数是整数时，参数未使用，可以传任意整数，
但一般建议使用 `[-1]` 更好地表达往数组末尾添加元素的含义。

任意可作为 `AddItem` 参数的类型，都可写在 `=` 左侧，包括 lambda 。

在 `Scope` 局部变量中，还额外支持 `<<` 链式插入符：

- `ScopeArray << v1 << v2 ...` = `AddItem(v1); AddItem(v2); ...`
- `ScopeObject` << k1 << v1 << k2 << v2` = `AddMember(k1,v1); AddMember(k2,v2)`

该操作符不能直接对 `builder` 使用，因为在 json 数组上下文与对象上下文中它有不
同含义，在对象中允许智能交替插入键与值。

下例展示了各种混合用法：

<!-- example:usage_4_4_operator_overloading -->
```cpp
wwjson::RawBuilder builder;
builder.BeginRoot();

// 基本字段
builder["name"] = "wwjson";
builder["version"] = 1.01;
builder["author"] = "lymslive";
builder["url"] = nullptr;

// 嵌套对象 feature
builder.AddMember("feature", [&builder]() {
    auto feature = builder.ScopeObject();
    feature << "standar"<< "C++17";
    feature << "dom"<< false << "config" << "compile-time";
});

// 数组 refer
builder.AddMember("refer");
{
    auto refer = builder.ScopeArray();
    refer[-1] = "rapidjson";
    refer[-1] = "nlohmann/json";

    // 数组中的嵌套对象
    refer << [&builder]() {
        auto obj = builder.ScopeObject();
        obj << "name" << "yyjson" << "lang" << "C";
    };
}

builder.EndRoot();
```

另请注意，`=` 也支持 `builder` 本身的拷贝赋值与移动赋值，在构造 json 过程中应
该与 `[]` 操作符联用。

### 4.5 将 json 构建过程分步拆分方法

在实践中，以上构建 json 的示例代码片断推荐封装在一个单独的函数，不宜与其他业务
代码混在一起。总体模式类似以下函数写法：

<!-- example:usage_4_5_step_entrance -->
```cpp
std::string BuildJson()
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    // ...
    builder.EndRoot();
    return builder.MoveResult();
}
```

即使以 `EndRoot` 方法完成构建过程，也推荐使用 `MoveResult` 方法返回，不要直接
使用 `builder.json` 。

当需要构建很复杂的 json 时，这个函数也许会变得很庞大，那就可以进行合理拆分子函
数，将 `builder` 引用传递给下游函数。然而要注意，在实际项目中，各个字段的值不
可能如上述示例中的字面常量，而是一些变量。假设这些变量被收集为某个类或结构体的
数据成员，那构建 json 的过程就可以是它的方法。

假设上述示例 json 数据，在 C++ 程序中被存于如下结构体，及其构建方法：

<!-- example:usage_4_5_struct_builder -->
```cpp
struct Project
{
    // 为简单起见，各成员直接赋默认值了
    std::string name = "wwjson";
    double version = 1.01;
    std::string author = "lymslive";
    std::string url;

    struct Feature
    {
        std::string standar = "C++17";
        bool dom = false;
        std::string config = "compile-time";
    } feature;

    std::vector<std::string> refer = {"rapidjson", "nlohmann/json"};

    struct Refer
    {
        std::string name = "yyjson";
        std::string lang = "C";
    } special_refer;

    // 其他数据成员或方法 ...

    // 构建 Json 的方法
    std::string BuildJson()
    {
        wwjson::RawBuilder builder;
        builder.BeginRoot();

        // 基本字段
        builder["name"] = name;
        builder["version"] = version;
        builder["author"] = author;
        if (url.empty())
        {
            builder["url"] = nullptr;
        }
        else
        {
            builder["url"] = url;
        }

        builder.AddMember("feature");
        BuildFeature(builder);

        builder.AddMember("refer");
        BuildRefer(builder);

        builder.EndRoot();
        return builder.MoveResult();
    }

    void BuildFeature(wwjson::RawBuilder &builder)
    {
        auto Jfeature = builder.ScopeObject();
        Jfeature["standar"] = feature.standar;
        Jfeature["dom"] = feature.dom;
        Jfeature["config"] = feature.config;
    }

    void BuildRefer(wwjson::RawBuilder &builder)
    {
        auto Jrefer = builder.ScopeArray();
        for (auto &item: refer)
        {
            Jrefer[-1] = item;
            // 或者 Jrefer << item;
        }
        BuildRefer2(builder);
    }

    void BuildRefer2(wwjson::RawBuilder &builder)
    {
        auto Jrefer = builder.ScopeObject();
        Jrefer["name"] = special_refer.name;
        Jrefer["lang"] = special_refer.lang;
    }
};

Project wwProject;
std::string json = wwProject.BuildJson();
std::cout << json << std::endl;
```

进一步地，如果为每个嵌套子结构体也定义一个相同的 `BuildJson` 方法。每个结构体
的该方法负责用 `AddMember` 序列化自己的成员字段，然后调用子成员对象的同名方法
，那就是形成一套通用的结构体转 json 的方案了。其大致结构如下：

<!-- example:NO_TEST -->
```cpp
struct Data
{
    // 省略数据成员定义
    void BuilJson(wwjson::RawBuilder &builder)
    {
        auto json = builder.ScopeObject();
        json["field1"] = field1;
        json["field2"] = field2;
        json.AddMember("sub_data1");
        sub_data1.BuildJson(builder);
        json.AddMember("sub_data2");
        sub_data2.BuildJson(builder);
    }

    std::string BuilJson()
    {
        wwjson::RawBuilder builder;
        BuildJson(builder);
        return builder.MoveResult();
    }
};
```

有些反射库支持一个方法调用将结构体转为 json ，但使用 wwjson 手动拼装能提供更好
的定制化性能与灵活性，比如某些字段需要经过一些较复杂的逻辑判断来决定输出或不输
出。而在没有特殊处理需求时，那些相同模式的重复代码，也是很容易自动生成的。

## 4.6 统一的 to_json 转换函数

由于将结构体的每个字段转为 json 的键值对是个很常见的需求，wwjson 库为此再提供
了一个统一的 `to_json` 函数来进一步简化这项工作。

它要求用户为自己的结构体定义一个 `to_json` 成员方法，接收 builder 引用参数，在
该方法中可用常规的 `AddMember` 将每个字段添加进去，但更推荐也使用统一的
`wwjson::to_json` 函数。因为 `AddMember` 方法不能处理嵌套子结构体成员，而
`to_json` 函数能统一处理简单标量字段或子结构体，当然子结构体也要求有 `to_json`
方法。

用这个思路改写上个示例，大概如下：

<!-- example:usage_4_6_struct_tojson -->
```cpp
// 按更常规的模式将各子结构体先在相同作用域平坦定义
struct Feature
{
    std::string standar = "C++17";
    bool dom = false;
    std::string config = "compile-time";

    void to_json(wwjson::RawBuilder& builder) const
    {
        wwjson::to_json(builder, "standar", standar);
        wwjson::to_json(builder, "dom", dom);
        wwjson::to_json(builder, "config", config);
    }
};

struct Refer
{
    std::string name = "yyjson";
    std::string lang = "C";

    void to_json(wwjson::RawBuilder& builder) const
    {
        // 可用宏进一步简化等效写法
        TO_JSON(name); // wwjson::to_json(builder, "name", name)
        TO_JSON(lang); // wwjson::to_json(builder, "lang", lang)
    }
};

struct Project
{
    std::string name = "wwjson";
    double version = 1.01;
    std::string author = "lymslive";
    std::string url;

    Feature feature;

    std::vector<std::string> refer = {"rapidjson", "nlohmann/json"};

    Refer special_refer;

    void to_json(wwjson::RawBuilder& builder) const
    {
        TO_JSON(name);
        TO_JSON(version);
        TO_JSON(author);
        TO_JSON(url);
        TO_JSON(feature);
        TO_JSON(refer);
        TO_JSON(special_refer);
    }

    // 入口方法
    std::string to_json() const
    {
        wwjson::RawBuilder builder;
        wwjson::to_json(builder, *this);
        return builder.MoveResult();
    }
};

Project prj;
std::cout << prj.to_json() << std::endl;
```

这样看起来就简洁清爽许多。然而结果有个细微的不同，它的输出结果是：

```json
{
  "name":"wwjson","version":1.01,"author":"lymslive","url":"",
  "feature":{"standar":"C++17","dom":false,"config":"compile-time"},
  "refer":["rapidjson","nlohmann/json"],
  "special_refer":{"name":"yyjson","lang":"C"}
}
```

区别在后两个字段，之前的构建结果是
```json
{
  ...
  "refer":["rapidjson","nlohmann/json",{"name":"yyjson","lang":"C"}]
}
```

`wwjson::to_json` 能自动处理 `vector` 数组与嵌套结构体，严格将每个字段转为一个
json 字段。在 C++ 的结构体，没有简单直观的方法将 `Refer` 结构体与另外两个标量
字符串放在一个 `vector` 容器中，所以就序列化成两个字段了。

其实更严谨的数据定义方式是每个 `refer` 都应该是同类类型结构体，转为 json 的对
象数组。如果允许缺失某些字段，`to_json` 也支持 `std::optional` 表示可空，空值
时转为 `null` 。例如，将上例的 `Refer` 结构体重定义一下（并简化其他定义）：

<!-- example:usage_4_6_struct_array -->
```cpp
struct Refer
{
    std::string name = "yyjson";
    std::optional<std::string> lang;

    void to_json(wwjson::RawBuilder& builder) const
    {
        TO_JSON(name);
        TO_JSON(lang);
    }
};

struct Project
{
    std::string name = "wwjson";
    std::string url;
    std::vector<Refer> refer;

    void to_json(wwjson::RawBuilder& builder) const
    {
        TO_JSON(name);
        TO_JSON(url);
        TO_JSON(refer);
    }
};

// 实例化数据结构体
Project prj;
prj.refer = {{"yyjson","C"}, {"rapidjson","C++"}, {"nlohmann/json"}};

wwjson::RawBuilder builder;
wwjson::to_json(builder, prj);
std::string json = builder.MoveResult();

std::cout << json << std::endl;
```

其输出结果是：
```json
{
  "name":"wwjson","url":"","refer":[{"name":"yyjson","lang":"C"},
  {"name":"rapidjson","lang":"C++"},{"name":"nlohmann/json","lang":null}]
}
```

另外要注意的是，只有 `Refer.lang` 字段由于定义成 `std::optional` 才会在空值时
输出 `null`。而 `Prjoect.url` 是字符串类型，空值时输出 `""` 。

所以 `to_json` 适合序列化那种常规的结构体数据表示，当有特殊需求时仍可用基
本的 `AddMember` 方法精细加工。而 `TO_JSON` 只是个简单的宏替换，可减少写两次字
段名的工作，但如果在输出 json 的字段想与结构体字段名不一样时，该用 `to_json`
函数修改第二参数。

最后，顶层结构体（如上例的 `Project` ）的空参数 `to_json` 方法其实是不必要的。
直接调用 `wwjson::to_json(prj)` 也行，但是要将每个结构体的其他 `to_json` 方法
的参数从 `RawBuilder` 改为 `Builder` 。后者是优化版的 json 构建器，详见 6.4 节
。所以默认使用 `Builder` 类。也允许使用任意自定义 builder 类，只要整个结构体层
次中使用相同的 builder 类。

小结一下，`wwjson::to_json` 实际是做了以下事情：
- 统一 `AddMember` 与 `AddItem` 方法，有键名参数的调用前者，少一个键名参数的调
  用后者；
- 通过模板匹配机制自动识别处理嵌套结构体与数组等容器；
- 序列化结构体时在前后自动调用了 `BeginObject` 与 `EndObject` ，中间调用结构体
  自己的 `to_json` 方法，传入当前构建器 `builder`;
- 序列化数组类容器时也会自动调用 `BeginArray` 与 `EndArray` ；
- 处理递归，尽可能为用户处理细节，用户只要为自己的结构体字段调用 `to_json` ；

如果有反射库支持，最后一步也可以替用户做了。但目前，wwjson 是要求用户自定义
`to_json` 方法。这也不复杂，与结构体定义字段一一对应写下每一行即可，在处理含大
量字段的结构体时，`to_json` 的相似代码可以利用其他工具生成。

如果不想侵入式地为自定义结构体增加 `to_json` 方法，也可以在 `wwjson` 命名空间
重载 `to_json` 函数，特化处理自定义结构体。例如：

```cpp
namespace wwjson
{
to_json(RawBuilder& builder, const char* key, const Feature& st)
{
    // 需要自己处理 Begin/EndObject
    builder.BeginObject(key);
    builder.AddMember("standar", st.standar);
    builder.AddMember("dom", st.dom);
    builder.AddMember("config", st.config);
    builder.EndObject();
}
} // wwjson::
```

这样，在父结构体的 `to_json` 方法中，`TO_JSON(feature)` 就会调用这个特化版的
`wwjson::to_json` 函数，而不会调用 wwjson 库提供的通用模板函数（仅就这个示例而
言，它们所做的事情是一样的）。

## 5 特殊用法

WWJSON 的主要功能与应用场景是从基本类型的原子数据开始构建 json ，但如果已经有
一个较小的 json 串，也提供了一些方法将其整合到更大的 json 串中。在另一些场合下，
如果有的 json 片断子串需要反复使用，也可以预构建小 json 串缓存起来复用，避免每
次从头再构建。

### 5.1 添加子串

用 `AddMemberSub` 或 `AddItemSub` 方法可以将一个 json 子串添加到正在构建中的另
一个 json 中，它们也接收字符串参数，与 `AddMember` 或 `AddItem` 的区别在于不会
将字符串值加引号括起来，以及可能需要的转义。例如：

<!-- example:usage_5_1_add_substring -->
```cpp
// 假设先构建子串
std::string feature;
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    builder.AddMember("standar", "C++17");
    builder.AddMember("dom", false);
    builder.AddMember("config", "compile-time");
    builder.EndRoot();
    feature = builder.MoveResult();
}

std::string refer;
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    builder.AddMember("name", "yyjson");
    builder.AddMember("lang", "C");
    builder.EndRoot();
    refer = builder.MoveResult();
}

// 再构建完整 json
std::string project;
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    builder.AddMember("name", "wwjson");
    builder.AddMember("version", 1.01);
    builder.AddMember("author", "lymslive");
    builder.AddMember("url", nullptr);

    builder.AddMemberSub("feature", feature);

    builder.BeginArray("refer");
    builder.AddItem("rapidjson");
    builder.AddItem("nlohmann/json");
    builder.AddItemSub(refer);
    builder.EndArray();

    builder.EndRoot();
    project = builder.MoveResult();
}
```

上例的 `AddMemberSub` 与 `AddItmeSub` 如果换成 `AddMember` 与 `AddItme` 在不转
义引号的情况下，产生的不是合法 json 。而转义后子串就是一个普通的字符串了，不再
保留嵌套结构，且频繁的 `\"` 转义引号导致可读性极差。

但是一般情况下，没必要将复杂 json 先拆成子串来构建，使用多个 `builder` 将导致
性能损耗，用一个 `builder` 从头到尾构建的效率会高一些。只有当每个子串都有独立
使用用途时才值得先构建子串，当然如果该子串不必构建而是可以从其他途径直接获得那
也可以拿来主义。

### 5.2 子串合并

假设已有两个完整 json 子串，当它们是相同类型的容器（对象或数组时），可以用
`Merge` 方法合并；已构建完整 json 的两个 `builder` 也能合并。例如：

<!-- example:usage_5_2_merge_substrings -->
```cpp
// 子串1
wwjson::RawBuilder basic;
basic.BeginRoot();
basic["name"] = "wwjson";
basic["version"] = 1.01;
basic.EndRoot();
std::string strBasic = basic.GetResult();

// 子串2
wwjson::RawBuilder feature;
feature.BeginRoot();
feature["standar"] = "C++17";
feature["dom"] = false;
feature.EndRoot();
std::string strFeature = feature.GetResult();

// 将子串2到子串1
basic.Merge(feature);

// 用静态方法合并子串
wwjson::RawBuilder::Merge(strBasic, strFeature);
```

以上两个 `Merge` 方法合并的结果都是如下 json:
```json
{"name":"wwjson","version":1.01,"standar":"C++17","dom":false}
```

合并成扁平结构，两个对象的字段放在一起了。不同于 `AddMemberSub` 形成嵌套结构。

数组与数组也可以合并，例如 `["rapidjson"]` 与 `["nlohmann/json"]` 合并成为
`[rapidjson","nlohmann/json"]`。但不能再与对象 `{"name":"yyjson","lang":"C"}`
，若想把这个对象放入前面的数组中，那是 `AddItemSub` 方法干的事。

合并的算法非常简单，仍然是算不上算法的字符串拼接，就是把相邻粘连处的前闭后开括
号改成逗号。只接受如下两种模式，将 `right` 合并到 `left` ，不满足合并条件返回
`false`：

- `{...left}{right...}` 合并为 `{...left,right...}`
- `[...left][right...]` 合并为 `[...left,right...]`

调用者需要保证两边的子串是合法 json ，合并后才仍是合法 json 。

## 6 配置定制

WWJSON 支持配置化与定制化，以满足用户对拼装 json 的一些个性化需求。同时为了性
能，这些配置都是编译期生效的，无任何运行时开销。这主要是通过模板类实现的。

### 6.1 GenericBuilder 核心模板类

上文示例使用的 `RawBuilder` 其实只是通用构建器模板类的特化别名：

<!-- example:NO_TEST -->
```cpp
template <typename stringT, typename configT = BasicConfig<stringT>>
struct GenericBuilder;

using RawBuilder = GenericBuilder<std::string>; // 默认构建器
```

`GenericBuilder` 的模板参数：
- `stringT`: 序列化串写入目标，具有 `std::string` 标准接口的字符串类型
- `configT`: 配置类型，有默认值 `BasicConfig<stringT>`

它的 `ScopeObject` 与 `ScopeArray` 方法所创建的局部变量也是模板类，同样有特化
别名：

<!-- example:NO_TEST -->
```cpp
using RawObject = GenericObject<std::string>; // 自动关闭对象类型
using RawArray = GenericArray<std::string>;   // 自动关闭数组类型
```

但如前所述，这两个类型名不用记，推荐使用 `auto` 接收变量就行。

### 6.2 BasicConfig 基础配置类

在 `BasicConfig` 类中，只定义了一些编译期常量与静态方法，用户可以通过继承它覆
盖一些常量或方法，再传给 `GenericBuilder` 的第二模板参数，以达到静态配置 json
序列化行为的目的。

#### 6.2.1 可配置常量

在基类 `BasicConfig` 中，定义了如下常量，默认值都是 `false` :

| 常量         | 默认值| 意义说明                 |
|--------------|-------|--------------------------|
| kEscapeKey   | false | 自动转义键名             |
| kEscapeValue | false | 自动转义字符串值         |
| kQuoteNumber | false | 自动给数字加引号         |
| kTailComma   | false | 保留数组与对象内的尾逗号 |

其中，前三个常量控制的是当使用常规 `AddMember` 与 `AddItem` 方法时是否需要对键
或值作特殊处理。

出于性能与常用场景考虑，默认不对字符串的键与值转义，但又提供两个变体方法可以对
个别字段的键或值作转义：

- `AddItemEsapce(val)` ，对 `val` 转义
- `AddMemberEsapce(key, val)` ，对 `val` 转义
- `AddMemberEsapce(key)` ，对 `key` 转义

再次提醒，`AddMember` 允许传一个参数，表示只写入键部分，要求立即调用 `AddItem`
写入值部分，或用 Begin 方法开启一个子结构。所以 `AddMemberEscape` 传一个参数时
只参键转义，传两个（以上）参数时只对值转义。

在实践项目中，一般应规范键名使用，最好只用合乎大部分编程语句的标记符规则，所以
转义键的需求较少。但是对于来自外部输入的值，无法预测是否有特殊字符，如果安全要
求高的项目，可以覆盖 `kEscapeValue` 的值。例如：

<!-- example:usage_6_2_1_safe_config -->
```cpp
struct SafeConfig : wwjson::BasicConfig<std::string>
{
    static constexpr bool kEscapeValue = true;
};

using SafeBuilder = wwjson::GenericBuilder<std::string, SafeConfig>;
SafeBuilder builder;
builder.BeginRoot();
builder["greet"] = "Hello\t!\nWorld\t!!";
builder.EndRoot();

// 结果：{"greet":"Hello\t!\nWorld\t!!"}

// 默认 builder 需要调用显式转义方法
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    builder.AddMemberEscape("greet", "Hello\t!\nWorld\t!!");
    builder.EndRoot();
}
```

常量 `kQuoteNumber` 的作用是将数字类型序列化为字符串类型，加上引号。这一般是不
推荐的，尤其是在 C++ 等强类型语言中，保持类型信息很重要。但实际项目中可能会遭
遇一些奇葩需求要将数字加引号，例如：

- 数字来源于前端，文本框的原始输入都是字符串，即使输入数字，然后传染到后端；
- 有些系统只支持字符串，以便统一接口；
- 浮点数存为字符串可保证在传输中或不同 json 库解析时不丢失精度。

与转义控制常量一样，数字引号也有手动挡的调用方法，只是通过在 `AddItem` 或
`AddMember` 参数表中额外传个 `true` 来表示。例如：

<!-- example:usage_6_2_1_quote_numbers -->
```cpp
wwjson::RawBuilder builder;
builder.BeginRoot();
builder.AddMember("name", "wwjson");
builder.AddMember("version", 1.01, true);
builder.EndRoot();

// 结果：{"name":"wwjson","version":"1.01"}
```

为什么手动转义的方法不统一加个额外 `true` 参数表示？它们的前置参数分别是数字与
字符串类型，本是可以区分的。但是字符串值类型还支持传 `(const char \*, size_t)`
的形式，已经是有额外可选参数了，所以手动转义改为用方法名区分。事实上，转义的需
求应该比数字加引号更常见些，方法改名的区分度更高。

最后一个常量 `kTailComma` 是控制 `EndObject` 与 `EndArray` 方法要不要删除前面
的尾逗号。如果该值为 `true` ，就保留了尾逗号，允许生成形如 `[1, 2,]` 或
`{"i":1,"j":2,}` 非标准格式的 json ，但是有些 json 库解析时可兼容这种格式。

就 wwjson 的实现而言，保留尾逗号已无任何优势。将尾逗号修改为闭括号，比多加一个
闭括号更简单高效，也节省最终结果的数据长度。但是如果合作的解析端要求统一有尾逗
号会更简单高效的话，可以考虑覆盖该配置项。

#### 6.2.2 字符串转义方法

常量配置 `kEscapeKey` 与 `kEscapeValue` 或手动的 `AddMemberEscape` 方法只控制
是否要转义，而具体如何转义是由另外两个静态方法 `EscapeKey` 与 `EscapeString`
实现的。在 `BasicConfig` 中，`EscapeKey` 简单转发 `EscapeString` ，所以一般情
况下，在派生类中只需要覆盖后者即可。定义两个方法是允许用户在需要时分别为键与值
定制不同的转义方法。

基类 `BasicConfig` 对字符串转义的处理逻辑如下：
- 转义 json 必须转义的两个字符，引号与反斜杠；
- 其他 C 语言的标准转义字符，包括 `\n\r\t\v\a\b\f\0` 等；
- 其他小于 128 的不可打印字符串转义为 `\.` （斜杠加点号）；
- 大于 128 的非 ASCII 字符串不转义，这包括 utf-8 字符不转义。

在实现中缓存了一个长度为 128 的转义表，其中许多不可打印字符存的是 `.` ，这使得
它可以与其他标准转义符用统一的逻辑处理。如果使用标准 Unicode 转义 `\uXXXX` 就
会复杂得多。不建议往 json 中写入二进制数据，转义 `.` 只是个保底退化策略，只保
证 json 仍然可读可打印。将二进制安全写入 json 的正确做法是用 base64 编码或转为
十六进制串。

如果用户不满意如上转义逻辑，或者能用更高效的转义算法，可以覆盖 `EscapeString`
方法（也可以对个别值自行先转义再按当成普通字符串添加）。另一方面，也可以将方法
覆盖为空操作，完全禁止转义，即使调用 `AddMemberEscape` 也实际上不转义，例如：

<!-- example:usage_6_2_2_unsafe_config -->
```cpp
struct UnsafeConfig : wwjson::BasicConfig<std::string>
{
    static void EscapeString(std::string &dst, const char *src, size_t len)
    {
        dst.append(src, len);
    }
};

using UnsafeBuilder = wwjson::GenericBuilder<std::string, UnsafeConfig>;
UnsafeBuilder builder;
builder.BeginRoot();
builder.AddMemberEscape("greet", "Hello\t!\nWorld\t!!");
builder.EndRoot();
// 结果中将会有制表符与换行符
```

注意上例仅为说明用法，实际中请不要这么用。

#### 6.2.3 数字转字符串方法

与 `EscapeString` 方法类似，`BasicConfig` 提供了一个叫 `NumberString` 的方法用
于将各种数字类型转为字符串表示（不加引号）。这在基类中是个模板方法，能匹配各种
整数与浮点数，用户可以选择性地重载特定数字类型转字符串的方法，或者也提供个通用
的模板方法完全替换基类的实现。

其实数字转字符串是另一个独立且复杂的问题，尤其是高精度的浮点数转十进制数字串，
那可能就不仅是编程技术问题，更是数学或学术问题。所以 wwjson 不打算深入这个问题
，只提供了一个基础实现版本，主要基于 C++17 标准库的 `std::to_chars` 方法，做了
小整数优化，与小范围的定点数（4位小数）优化。

从一项性能测试的结果看，当全是数字的 json 数组序列化时，wwjson 的性能不如
yyjson 。但一般项目实用的 json ，至少有字符串的键名，当字符串占比更大时，
wwjson 的性能超过 yyjson 。所以在数字占比大的情况下，如果有进一步的性能追求，
可以采用更先进的数字转字符串算法去覆盖 `NumberString` 配置方法。

例如，大量用到浮点数时，可以如下派生定制：

<!-- example:NO_TEST -->
```cpp
struct MyConfig : wwjson::BasicConfig<std::string>
{
    static void NumberString(std::string &dst, double value)
    {
        // 某种复杂的算法将浮点数写入目标字符串
    }
};
```

### 6.3 自定义序列化目标

`GenericBuilder` 的第一个模板参数允许自定义字符串类型，只要它实现了
`std::string` 的（部分）常用接口，基本要求如下：

- 有默认构造函数，复制构造函数与移动构造函数；
- `reserve` 可以预留容量；
- `push_back` 可以追加一个字符；
- `append` 可以追加一个字符串；
- `back` 可以读取与修改最后一个字符；
- 如果用到 `Merge` 方法，还要支持 `front`/`size`/`c_str` 等方法，`Scope` 对象
  想直接放在 `if()` 括号中，要求支持 `empty` 方法判断非空；

不过，但凡看不上标准库的 `std::string` 要自己重造个字符串轮子的话，应该都会很
自然地支持以上方法。其实更灵活的是，不一定要当作字符串类使用，凡是提供了以上接
口的类，都可以用于 wwjson 的序列化目标。

当然这种灵活性也是有一定代价的，因为 `GenericBuilder` 不自己管理内存，它只能调
用序列化目标对象的 `push_back` 与 `append` 方法，所以在拼装 json 时，涉及频繁
地写入一个个的引号、冒号与逗号单字符，这种碎片化的写入会涉及反复的边界安全判断。
这是不可避免的取舍。

#### 6.3.1 定制实例：日志行目标

WWJSON 不一定用于只构建完整的 json 字符串，它也允许在已有的任意字符串基础上继
续组装 json 。`GenericBuilder` 的构造函数可接收一个有内容的字符串，通过复制或
移动它到 `builder` 内部。

一个可能的应用场景是在日志中以 json 格式打印一些关键数据，但是日志系统本身已有
其他的格式要求，比如分几部分，仅内容部分打印 json 。如此就可以先填充日志行前缀
部分，再移动给 `builder` 拼装 json ，也可以再移动回来继续加后缀。

这样的日志行也许是个封装有特定功能的对象，只要它有字符串的接口，就能用 wwjson
组装 json 。在下面示例中，为简便起见，直接让它空继承 `std::string` 了，但实践
中可以为它增加更复杂的功能：

<!-- example:usage_6_3_1_log_line_target -->
```cpp
struct LogLine : public std::string {};
using LogLineBuilder = wwjson::GenericBuilder<LogLine>;

LogLine fullLine;
fullLine.append("[Log] "); // 前缀部分
LogLineBuilder builder(std::move(fullLine)); // 建议用移动的性能更高
builder.BeginRoot();
builder.AddMember("name", "wwjson");
builder.AddMember("version", 1.01);
builder.EndRoot();

fullLine = builder.MoveResult();
fullLine.append(" [End]"); // 可能的后缀部分
std::cout << fullLine.c_str() << std::endl;
// 结果：[Log] {"name":"wwjson","version":1.01} [End]
```

#### 6.3.2 定制实例：消息流目标

另一个场景，不只构建一个 json ，而是想连续构建多个 json 。假设每个 json 是个消
息，那多个 json 就构成消息队列，或消息流。当然每个 json 建议用一个换行符分隔，
因为正常作为数据交换的 json 是单行的，每行表示一个消息，即使保存在文件中也易读
易解析。

对此可以封装一个 json 消息流缓冲管理对象，内部实现可能很复杂，但只要表现出类似
字符串写入的接口，就能作为 wwjson 的序列化目标。在下面的示例中，仍然以空继承
`std::string` 省略实际的复杂性：

<!-- example:usage_6_3_2_message_stream_target -->
```cpp
struct MessageBuffer : public std::string {};
using MessageBuilder = wwjson::GenericBuilder<MessageBuffer>;

MessageBuilder builder;
builder.BeginRoot();
builder.AddMember("name", "wwjson");
builder.AddMember("version", 1.01);
builder.EndRoot();

builder.EndLine();

builder.BeginRoot();
builder.AddMember("name", "yyjson");
builder.AddMember("version", 1.02);
builder.EndRoot();

builder.EndLine();
```

其中，`EndLine` 方法顾名思义也很简单，就是写入一个换行符。内部对象可以监听换行
符作特殊处理，比如刷入硬盘文件，或通过 tcp socket 发送出去。

### 6.4 Json 字符串优化

WWJSON 自 v1.1 版本始，也专门针对 json 序列化定制了一种字符串，严格来说是一种
字符串缓冲类。它定义为带一个 `uint8_t` 整数模板参数的类，但有两个常用别名：

```cpp
using JString = StringBuffer<4>;
using KString = StringBuffer<255>;
```

以及利用它们实例化的 `GenericBuilder` 构建器类：

```cpp
using Builder = GenericBuilder<JString>;
using FastBuilder = GenericBuilder<KString>;
```

它们在用法上与基于 `std::string` 的 `RawBuilder` 几乎一样。

### 6.4.1 不安全等级的概念

这个整数模板参数叫做“不安全等级”，也是静态常量 `StringBuffer::kUnsafeLevel` 。
它的意思是每当使用安全的写入方法后，还能保证安全地使用不安全方法写入多少字节。

安全的写入方法，就如常见的标准 `push_back` 与 `append` 方法，它会像
`std::string` 一样检查字符串容量是否足够当前写入，不够的话扩容。

`wwjson::StringBuffer` 类定义了对应的不安全方法如 `unsafe_push_back` 与
`unsafe_append` ，它假定容量足够，不会检查边界，也自然不会触发扩容。那如何预知
不会写越界呢，那就是不安全等级 `kUnsafeLevel` 的意义了。

例如，当调用 `append(str, n)` 准备写入 `n` 个字节时，常规的字符串类只会检查剩
余容量是否满足 `n` ，如果恰好只剩下 `n` 容量，它也允许写入。但 `StringBuffer`
会检查是否满足 `n + kUnsafeLevel` ，如果不满足，就会扩容。也就是说，当写完这
`n` 个字节后，还能保证至少剩余 `kUnsafeLevel` 个字节的边界余量，允许用户不加检
查地写入这么多的额外字节。

这个小小的改动，为什么在 json 序列化中能提升效率呢？观察如下 json 数据:

```json
{"name":"WWJSON","language":"C++"}
```

它有信息含量的东西就是两个键与两个值，当然一对大括号也提供了结构化信息。除此之
外的引号、冒号与逗号就纯粹是无信息含量的格式字符了，在那些解析为 DOM 的 json
库处理后也不会存这些格式字符（但大括号与中括号会保存为容器结点）。

所以简单统计一下，有意义的 token 共有 `6` 个，无意义但必需的格式字符 token 有
`2 * 4 + 3 = 11` 个。

于是，wwjson 在利用 `JString` 序列化时，对有意义的 token 也就是主要内容使用安
全的写入方法，对次要的格式字符使用不安全的写入方法。一个合法的 json ，最多会有
`3` 个连续的格式字符，这就是为什么 `JString` 的不安全等级设定为 `4` 。

从这个角度也能反观为什么括号在 json 是有信息意义的，因为我们没法界定一个合法的
json ，它最多允许几个连续的括号。例如：

```json
[[[[[[[[[[]]]]]]]]]]
```

这串中括号虽然似乎没有实质内容，但它确实是合法有效的 json 。在实践中，虽然连续
开大括号不存在，但在复杂多层的 json 末尾，一串闭大括号却是司空见惯的。所以，尽
管括号与引号看似一样，都只一个字符，但在 `JString` 中，它们必须以安全的
`push_back` 方法写入。

至于 `KString` ，它的不安全等级达到 `uint8_t` 的最大值。除了满足正常定义会附带
`255` 字节的额外边界外，它还表达一种“极不安全”的意义。也就是说即使使用常规的
安全写入方法如 `push_back` 与 `append` ，它也不会检查边界，相信可写就直接写了
。因此它能比 `JString` 节省更多的边界检查开销，进一步提升性能。代价是必须预估
容量，在构造函数时传入足够大的初始容量，或者中途某个时刻显式地再次调用
`reserve` 预留容量。

### 6.4.2 边界检查节约度

先看一个 `wwjson::KString` 的应用于 json 构建的简单示例：

<!-- example:usage_6_4_2_check_ratio -->
```cpp
wwjson::FastBuilder builder(8*1024); // 初始容量 8K + 255

builder.BeginArray();
for (int i = 0; i < 10; ++i)
{
    for (int j = 0; j < 1024; ++j)
    {
        builder.AddItem("abcde"); // 每次写入 8 字节，包括引号与逗号
    }
    builder.json.reserve_ex(8*1024); // 每千次再扩容 8K + 255
}
builder.EndArray();

if (builder.json.overflow()) return;
std::string result = builder.GetResult().str();
```

其中，`reserve_ex` 是相对于当前 `size()` 预留额外容量，与标准的 `reserve` 方法
预留绝对容量略有不同含义。如果使用 `std::string` 或 `wwjson::JString` 的话，构
建过程中是没必要手动调用 `reserve` 的，它们会在每次写入前检查容量自动扩容。

上例的每次 `builder.AddItem` 调用，会写入 4 个 token ，在使用不同底层字符串类
型时所需做边界检查（类似 `if (size() < capacity())` 的操作）的对照表如下：

| Token | std::string | JString | KString |
|-------|-------------|---------|---------|
| "     | YES         | NO      | NO      |
| abcde | YES         | YES     | NO      |
| "     | YES         | NO      | NO      |
| ,     | YES         | NO      | NO      |

可见 `KString` 完全不做边界的安全检查，只是为了可用性，约每千次显式调用
`reserve_ex` 会做一次检查。所以相对于 `JString` 的边界检查频率大约是千分之一，
相对于 `std::string` 是四千分之一。

当然这个数值仅针对这个简单示例的参考。`KString` 一般适用的场景是能预估容量，在
构造函数申请一次内存之后就不再扩容了。除非是要构建很大的对象数组，仅能估算每个
对象的大小，而不好估计数组长度时，则可参考这个示例中途显式扩容。

构建完毕后，也提供了个判断是否写溢界的方法 `overflow` ，聊胜于无，或可在调试阶
段有用，若在生产环境中若真写溢界了，可能尽早崩溃才是上计了。

故在一般情况下，使用基于 `JString` 的 `Builder` 类，能在安全与性能之间取得较好
的平衡。没有需要特别注意的地方，只是毕竟与 `std::string` 不是同一个类，它们之
间不能隐式转换（因为会发生拷贝），所以需要显式转换。不介意 `JString` 类型扩散
到其他地方的话，也可以直接使用它避免一次转换拷贝，或者转换为更低成本的
`std::string_view` ，但要保证 `JString` 本身的生命周期。

### 6.4.3 安全边距

效率与安全是一对矛盾，不安全等级从另一个角度看也是安全边距。`std::string` 的安
全边距相当于 0，故而不能提供不安全写入方法。它与 `JString` 关于安全边距的最显
著特征，可以用如下一个简单示例呈现：

<!-- example:usage_6_4_3_safe_margin -->
```cpp
std::string str;
str.reserve(256);
while(str.size() < str.capacity())
{
    str.push_back('x');
    if (str.size() > 1024*1024)
        break; // 防死循环
}
std::cout << "size=" << str.size() << std::endl;
std::cout << "capacity=" << str.capacity() << std::endl;

wwjson::JString jstr;
jstr.reserve(256);
while(jstr.size() < jstr.capacity())
{
    jstr.push_back('x');
    if (jstr.size() > 1024*1024)
        break; // 防死循环
}
std::cout << "size=" << jstr.size() << std::endl;
std::cout << "capacity=" << jstr.capacity() << std::endl;
```

循环向 `std::string` 写入一个字符，能恰好写满，它应该能正常退出循环。但如果
换成 `wwjson::JString` ，由于存在 4 个安全边距，理论上它永远写不满（假设内存无
限的话），每当快写满时，它就会扩容。而 `std::string` 允许恰好写满的情况，所以
每次写入都要做安全检查。

这是与它们的设计场景相适应的。标准 `std::string` 适用于基本不变的字符串，而
`JString` 本质是 `StringBuffer` ，适用于需要频繁写入增长的情况。wwjson 库一开
始选择 `std::string` 作为基底，只为了方便与通用性。当真正追求性能时，通用的
`std::string` 显然就不合适了。

### 6.4.4 外借协同写入

标准 `std::string` 不适合作为 string buffer 的另一个特性是它的封闭性，对所持内
存的管控更为严格，不能（安全地）用外部函数写入它的内存区，只能调用它的公开接口
写入内容。这对于面向对象封装来说似乎是理所应当的事，但用于字符串拼接场景它就有
失灵活与效率了。

比如想向 `std::string` 末尾追加一个整数的序列化格式，一般应用可这么写：

<!-- example:usage_6_4_4_std_tostr -->
```cpp
std::string str{"prefix:"};
str += std::to_string(314);

// 或者用 snprintf
char buffer[16];
snprintf(buffer, sizeof(buffer), "%d", 159);
str += buffer;
```

但不管用 `std::to_string` 还是 `snprintf` ，它都需要先写入一个临时缓冲区，再追
加到 `std::string` 末尾。这就多了一次字符串拷贝，对于高性能场景是会介意的。一
个常见错误是试图先预留空间，然后用 `snprintf` 直接打印到 `std::string` 末尾，
例如：

<!-- example:usage_6_4_4_err_tostr -->
```cpp
std::string str{"prefix:"};
str.reserve(str.size() + 16);
int nWritten = snprintf(str.data() + str.size(), 16, "%d", 314);

std::cout << str.c_str() << std::endl; // 可能正确 prefix:314
std::cout << str << std::endl;         // 不正确 prefix:
std::cout << str.size() << std::endl;  // 实际长度仍为 7

int oldSize = str.size();
str.resize(oldSize + nWritten);
std::cout << str.size() << str.c_str() << std::endl; // 10prefix:
std::cout << (str[7] != '3') << std::endl;
std::cout << (str[7] == '\0') << std::endl;
```

仅管我们可以偷出 `std::string` 的内存指针，并往里面写东西。但它无法感知外界的
动作，它仍只认它自己管理过的那部分内容。也没好办法去同步外界的操作结果，如果调
用 `resize` 去增加长度，对增加的部分会调用 `char` 的默认构造，也就是写入 `\0`
字符，覆盖了原来 `snprintf` 写入的内容。`resize` 的这种行为是与其他标准容器如
`std::vector<char>` 保持一致的，它要保证容器内的每个元素是有效对象，所以
`resize` 超出原 `size` 部分只好默认构造一个元素在那里了。

因此，对于这种场景需要 `wwjson::JString` 这种设计为 string buffer 的类型来应对
。简单来说，它相当于重新定义了 `resize` 的意义，它相信用户扩展字符串时已经写入
了有效的内容。实际上，这里的 `JString` 使用三指针的方案管理内存，允许直接调用
`set_end` 方法调整尾指针（下次应写入的指针位置）。

<!-- example:usage_6_4_4_jstr_tostr -->
```cpp
wwjson::JString jstr;
jstr.append("prefix:");

jstr.reserve_ex(16);
int nWritten = snprintf(jstr.end(), 16, "%d", 314);
jstr.set_end(jstr.end() + nWritten);
// 或 jstr.resize(jstr.size() + nWritten);

std::cout << jstr.c_str() << std::endl; // 正确 prefix:314
std::cout << jstr.str() << std::endl;   // 正确 prefix:314
std::cout << jstr.size() << std::endl;  // 正确 10
```

注意，`JString` 实质是 `StringBuffer` ，它不是要平替 `std::string` ，所以没有
提供后者的所有接口，比如构造函数就不能接受 `prefix:` 初始内容。它的默认构造就
会预留 1024 字节，所以上例的 `reserve_ex` 是不必要，但实践中每次借出内存时安全
预留是必要的。

在 json 序列化，另一个关键性能瓶颈就是数字尤其是浮点数的序列化以及字符串转义。
一些高性能算法经常接受一组指针与长度参数，假定可在用户提供的空间安全写入。因此
`JString` 的这个特性就能与之协同合作，将 json 格式化工程与数字格式化算法解耦。
而 `GenericBuilder` 的可配置化也允许用户尝试更高效或更有针对性的算法。

## 7 快速参考

最后再总结列出一些快速参考条目，其中大多已在前面正文有所讲叙，故只作简单汇总。

### 7.1 常用 API 方法

- 构建根结构：BeginRoot/EndRoot, BeginRoot('[')/EndRoot(']')
- 构建子结构：BeginObject/EndObject, BeginArray/EndArray
- 添加子元素：AddMember, AddItem
- 添加子串：AddMemberSub, AddIteSubm
- 添加转义字符串：AddMemberEscape, AddItemEscape
- 获取最终结果：GetResult/MoveResult

### 7.2 常见错误用法

- Begin 与 End 的容器类型不匹配，导致结构混乱；
- Add 方法在不匹配的容器内使用，如在数组中使用 `AddMember` ，在对象中使用
`AddItem`；
- 在容器关闭后仍然使用 Add 方法；
- 根结点用 `Scope` 方法创建时没有套 `{}` 导致没有触发析构关闭；
- 不是以 `EndRoot` 结束时没有调用 `GetResult` 或 `MoveResult`；
- 含有特殊字符的字符串值没有转义；
- 使用单参数的 `AddMember` 没有马上添加值；
- 在对象结构内增加子结构忘记添加键名；

如果担心误用 `AddItem` 与 `AddMember` 的，可以自立规范，不在 `builder` 上直接
调用这两个方法，而是先用 `ScopeArray` 与 `ScopeObject` 再分别调用这俩方法，那
就能在编译期检出错误。

### 7.3 常见优化手段

- 给 builder 构建函数传入合适的预估容量；
- 用 MoveResult 获取最后构建结果；
- 根据业务需求只对必要字段作转义；
- 如有可能，避免使用浮点数，转为扩大的整数；
- 如果不是历史兼容原因，不要给数字加引号变成字符串；
- 如果考虑数据大小，不要写 `null` 字段与各种空值（空字符串、空数组与空对象），
还可用 `1/0` 代替 `true/false`；如果考虑尽可能保留结构信息，则反之。

### 7.4 构建器选用

- 基于 std::string 的 RawBuilder: 通用性强
- 基于 wwjson::JString 的 Builder: 性能与安全的平衡
- 基于 wwjson::KString 的 FastBuilder: 追求性能，手动预估容量

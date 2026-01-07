#include "couttast/tinytast.hpp"
#include "test_util.h"
#include "jbuilder.hpp"
#include <string>

using namespace wwjson;

/// @brief Test for unsafe_level compile-time trait
/// @{

DEF_TAST(jbuilder_unsafe_level, "unsafe_level 编译期特征萃取测试")
{
    DESC("标准库类型返回 0");
    {
        COUT((int)detail::unsafe_level<std::string>::value, 0);
        COUT((int)detail::unsafe_level_v<std::string>, 0);
        COUT((int)detail::unsafe_level<const std::string>::value, 0);
    }

    DESC("BufferView 和相关类型");
    {
        COUT((int)detail::unsafe_level<BufferView>::value, 0);
        COUT((int)detail::unsafe_level<UnsafeBuffer>::value, 255);
    }

    DESC("StringBuffer 模板类型");
    {
        COUT((int)detail::unsafe_level<StringBuffer<0>>::value, 0);
        COUT((int)detail::unsafe_level<StringBuffer<4>>::value, 4);
        COUT((int)detail::unsafe_level<StringBuffer<255>>::value, 255);
    }

    DESC("类型别名");
    {
        COUT((int)detail::unsafe_level<JString>::value, 4);
        COUT((int)detail::unsafe_level<KString>::value, 255);
        COUT(detail::unsafe_level<JString>::value, detail::unsafe_level_v<JString>);
    }

    DESC("Builder 内部的 unsafe_level");
    {
        COUT((int)detail::unsafe_level<Builder::string_type>::value, 4);
        COUT((int)detail::unsafe_level<FastBuilder::string_type>::value, 255);
    }
}

/// @}

/// @brief Test for Builder type alias with JString
/// @{

DEF_TAST(jbuilder_basic, "Builder 基本功能测试")
{
    Builder builder;
    builder.BeginObject();

    builder.AddMember("int", 123);
    builder.AddMember("float", 3.14);
    builder.AddMember("string", "hello");
    builder.AddMemberEscape("escape", "hello\nworld");

    builder.EndObject();
    std::string result = builder.MoveResult().str();

    COUT(result);
    std::string expect = R"({"int":123,"float":3.14,"string":"hello","escape":"hello\nworld"})";
    COUT(result, expect);
    COUT(test::IsJsonValid(result), true);
}

DEF_TAST(jbuilder_nested, "Builder 嵌套结构测试")
{
    Builder builder;
    builder.BeginObject();

    builder.AddMember("name", "test");
    {
        auto arr = builder.ScopeArray("items");
        arr.AddItem(1);
        arr.AddItem(2);
        arr.AddItem(3);
    }

    {
        auto obj = builder.ScopeObject("config");
        obj.AddMember("enabled", true);
        obj.AddMember("timeout", 30);
    }

    builder.EndObject();
    std::string result = builder.MoveResult().str();

    COUT(result);
    // Expected: {"name":"test","items":[1,2,3],"config":{"enabled":true,"timeout":30}}
    std::string expect = R"({"name":"test","items":[1,2,3],"config":{"enabled":true,"timeout":30}})";
    COUT(result, expect);
    COUT(test::IsJsonValid(result), true);
}

DEF_TAST(jbuilder_raii, "Builder RAII 包装器测试")
{
    Builder builder;
    builder.BeginObject();

    {
        JObject obj(builder, "user");
        obj.AddMember("name", "Alice");
        obj.AddMember("age", 25);
    }

    {
        JArray arr(builder, "items");
        arr.AddItem("apple");
        arr.AddItem("banana");
        arr.AddItem(42);
    }

    builder.EndObject();

    std::string result = builder.MoveResult().str();
    COUT(result);
    // Expected: {"user":{"name":"Alice","age":25},"items":["apple","banana",42]}
    std::string expect = R"({"user":{"name":"Alice","age":25},"items":["apple","banana",42]})";
    COUT(result, expect);
    COUT(test::IsJsonValid(result), true);
}

/// @}

/// @brief Test for FastBuilder type alias with KString
/// @{

DEF_TAST(jbuilder_fast_basic, "FastBuilder 基本功能测试")
{
    // 推荐用法：直接传容量参数
    DESC("直接传容量参数");
    {
        FastBuilder builder(2048);
        builder.BeginObject();
        builder.AddMember("name", "fast");
        builder.AddMember("count", 1000);
        builder.EndObject();
        std::string result = builder.MoveResult().str();
        COUT(result);
        COUT(result, R"({"name":"fast","count":1000})");
        COUT(test::IsJsonValid(result), true);
    }

    // 拷贝构造：测试 StringBuffer<255> reserve 修复
    DESC("拷贝构造");
    {
        KString buffer(2048);
        FastBuilder builder(buffer);
        builder.BeginObject();
        builder.AddMember("name", "copied");
        builder.EndObject();
        std::string result = builder.MoveResult().str();
        COUT(result);
        COUT(result, R"({"name":"copied"})");
        COUT(test::IsJsonValid(result), true);
    }

    // 移动构造
    DESC("移动构造");
    {
        KString buffer(2048);
        FastBuilder builder(std::move(buffer));
        builder.BeginObject();
        builder.AddMember("moved", true);
        builder.EndObject();
        std::string result = builder.MoveResult().str();
        COUT(result);
        COUT(result, R"({"moved":true})");
        COUT(test::IsJsonValid(result), true);
    }

    // 带前缀构造
    DESC("带前缀构造");
    {
        KString prefix(2048);
        prefix.append("{\"init\":123,");
        FastBuilder builder(std::move(prefix), 1024);
        builder.AddMember("added", "value");
        builder.EndObject();
        std::string result = builder.MoveResult().str();
        COUT(result);
        COUT(result, R"({"init":123,"added":"value"})");
        COUT(test::IsJsonValid(result), true);
    }

    // 作用域测试
    DESC("作用域测试");
    {
        FastBuilder builder(2048);
        builder.BeginObject();
        builder.AddMember("active", true);
        {
            auto arr = builder.ScopeArray("data");
            arr << 1 << 2 << 3;
        }
        builder.EndObject();
        std::string result = builder.MoveResult().str();
        COUT(result);
        COUT(result, R"({"active":true,"data":[1,2,3]})");
        COUT(test::IsJsonValid(result), true);
    }
}

/// @}

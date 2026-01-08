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

// ============================================================================
// to_json Helper Functions Tests
// ============================================================================

#include <vector>
#include <array>

DEF_TAST(to_json_scalars, "to_json scalar types and array elements")
{
    // Scalar types in object
    DESC("scalar types with key");
    {
        Builder builder;
        builder.BeginObject();
        wwjson::to_json(builder, "int_val", 42);
        wwjson::to_json(builder, "double_val", 3.14159);
        wwjson::to_json(builder, "bool_true", true);
        wwjson::to_json(builder, "bool_false", false);
        wwjson::to_json(builder, "str_val", std::string("hello"));
        wwjson::to_json(builder, "cstr_val", "world");
        builder.EndObject();
        std::string result = builder.MoveResult().str();
        std::string expect = R"({"int_val":42,"double_val":3.14159,"bool_true":true,"bool_false":false,"str_val":"hello","cstr_val":"world"})";
        COUT(result, expect);
        COUT(test::IsJsonValid(result), true);
    }

    // Array elements without key
    DESC("array elements without key");
    {
        Builder builder;
        builder.BeginArray();
        wwjson::to_json(builder, 1);
        wwjson::to_json(builder, 2.5);
        wwjson::to_json(builder, std::string("three"));
        wwjson::to_json(builder, true);
        builder.EndArray();
        std::string result = builder.MoveResult().str();
        std::string expect = R"([1,2.5,"three",true])";
        COUT(result, expect);
        COUT(test::IsJsonValid(result), true);
    }
}

DEF_TAST(to_json_containers, "to_json containers and nested structs")
{
    // Container types
    DESC("vector and array");
    {
        Builder builder;
        builder.BeginObject();
        std::vector<int> numbers = {1, 2, 3, 4, 5};
        std::vector<std::string> names = {"alice", "bob"};
        std::array<int, 3> arr = {10, 20, 30};
        wwjson::to_json(builder, "numbers", numbers);
        wwjson::to_json(builder, "names", names);
        wwjson::to_json(builder, "fixed_arr", arr);
        builder.EndObject();
        std::string result = builder.MoveResult().str();
        std::string expect = R"({"numbers":[1,2,3,4,5],"names":["alice","bob"],"fixed_arr":[10,20,30]})";
        COUT(result, expect);
        COUT(test::IsJsonValid(result), true);
    }

    // Nested struct with to_json method
    DESC("nested struct");
    {
        struct Address {
            std::string street;
            std::string city;
            void to_json(Builder& builder) const {
                wwjson::to_json(builder, "street", street);
                wwjson::to_json(builder, "city", city);
            }
        };

        struct Person {
            std::string name;
            int age;
            Address address;
            void to_json(Builder& builder) const {
                wwjson::to_json(builder, "name", name);
                wwjson::to_json(builder, "age", age);
                wwjson::to_json(builder, "address", address);
            }
        };

        Person person{"John", 30, {"123 Main St", "Boston"}};
        Builder builder;
        builder.BeginObject();
        wwjson::to_json(builder, "person", person);
        builder.EndObject();
        std::string result = builder.MoveResult().str();
        std::string expect = R"({"person":{"name":"John","age":30,"address":{"street":"123 Main St","city":"Boston"}}})";
        COUT(result, expect);
        COUT(test::IsJsonValid(result), true);
    }

    // Complex nested structure
    DESC("complex nested struct");
    {
        struct Book {
            std::string title;
            std::vector<std::string> authors;
            double price;
            void to_json(Builder& builder) const {
                wwjson::to_json(builder, "title", title);
                wwjson::to_json(builder, "authors", authors);
                wwjson::to_json(builder, "price", price);
            }
        };

        struct Library {
            std::string name;
            std::vector<Book> books;
            void to_json(Builder& builder) const {
                wwjson::to_json(builder, "name", name);
                wwjson::to_json(builder, "books", books);
            }
        };

        Library lib{"City Library"};
        lib.books.push_back({"C++ Primer", {"Lippman"}, 49.99});
        lib.books.push_back({"Effective C++", {"Meyers"}, 44.99});

        std::string result = wwjson::to_json(lib);
        COUT(result);
        COUT(test::IsJsonValid(result), true);
    }
}

DEF_TAST(to_json_macro, "TO_JSON macro usage")
{
    // Simple struct with TO_JSON
    DESC("simple struct with TO_JSON");
    {
        struct Person {
            std::string name;
            int age;
            bool active;
            void to_json(Builder& builder) const {
                TO_JSON(name);
                TO_JSON(age);
                TO_JSON(active);
            }
        };

        Person p{"Alice", 30, true};
        Builder builder;
        builder.BeginObject();
        wwjson::to_json(builder, "person", p);
        builder.EndObject();
        std::string result = builder.MoveResult().str();
        std::string expect = R"({"person":{"name":"Alice","age":30,"active":true}})";
        COUT(result, expect);
        COUT(test::IsJsonValid(result), true);
    }

    // Nested structs with TO_JSON
    DESC("nested structs with TO_JSON");
    {
        struct Address {
            std::string street;
            std::string city;
            void to_json(Builder& builder) const {
                TO_JSON(street);
                TO_JSON(city);
            }
        };

        struct User {
            std::string username;
            Address address;
            void to_json(Builder& builder) const {
                TO_JSON(username);
                TO_JSON(address);
            }
        };

        User u{"bob", {"456 Oak Ave", "Seattle"}};
        Builder builder;
        builder.BeginObject();
        wwjson::to_json(builder, "user", u);
        builder.EndObject();
        std::string result = builder.MoveResult().str();
        std::string expect = R"({"user":{"username":"bob","address":{"street":"456 Oak Ave","city":"Seattle"}}})";
        COUT(result, expect);
        COUT(test::IsJsonValid(result), true);
    }
}

// 自定义重载 wwjson::to_json 函数测试
namespace test
{
// 先声明 Address 结构体（含 to_json 方法声明）
struct Address {
    std::string street;
    std::string city;
    void to_json(Builder& builder) const;
};
} // test::

namespace wwjson
{
// test::Address 结构体可不提供 to_json 方法，
// 但这个重载必须在调用者 test::Person::to_json 定义之前声明
void to_json(wwjson::Builder& dst, const char* key, const test::Address& self)
{
    DESC("[DEBUG] Non-template to_json called, key=%s", key);
    auto addr = dst.ScopeObject(key);
    addr.AddMember("City", self.city);
    wwjson::to_json(dst, "Street", "Unkown");
}

} // wwjson::

namespace test
{
// 现在可以定义 Person，其 to_json 方法能看到 wwjson::to_json 的重载
struct Person {
    std::string name;
    int age;
    Address addr;
    void to_json(Builder& builder) const {
        wwjson::to_json(builder, "Name", name);
        wwjson::to_json(builder, "Age", age);
        wwjson::to_json(builder, "Addr", addr);  // 此时能看到 wwjson::to_json(Builder&, const char*, const Address&)
    }
};

// Address 的 to_json 方法定义
// 如果没有重载，会被通用方法调用
inline void Address::to_json(Builder& builder) const {
    DESC("[DEBUG] template to_json called, key=%s");
    wwjson::to_json(builder, "street", street);
    wwjson::to_json(builder, "city", city);
}

} // test::

DEF_TAST(to_json_standalone, "standalone wwjson::to_json(struct)")
{
    struct Address {
        std::string street;
        std::string city;
        void to_json(Builder& builder) const {
            wwjson::to_json(builder, "street", street);
            wwjson::to_json(builder, "city", city);
        }
    };

    struct Person {
        std::string name;
        int age;
        Address addr;
        void to_json(Builder& builder) const {
            wwjson::to_json(builder, "name", name);
            wwjson::to_json(builder, "age", age);
            wwjson::to_json(builder, "addr", addr);
        }
    };

    Person p{"Alice", 35, {"789 Pine Rd", "Denver"}};
    std::string result = wwjson::to_json(p);
    std::string expect = R"({"name":"Alice","age":35,"addr":{"street":"789 Pine Rd","city":"Denver"}})";
    COUT(result, expect);
    COUT(test::IsJsonValid(result), true);

    DESC("custom wwjson::to_json for some struct");
    {
        test::Person p{"Alice", 35, {"789 Pine Rd", "Denver"}};
        std::string result = wwjson::to_json(p);
        std::string expect = R"({"Name":"Alice","Age":35,"Addr":{"City":"Denver","Street":"Unkown"}})";
        COUT(result, expect);
        COUT(test::IsJsonValid(result), true);
    }
}

/// @}

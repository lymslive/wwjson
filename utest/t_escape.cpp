#include "couttast/tinytast.hpp"
#include "wwjson.hpp"

struct EscapeConfig : wwjson::BasicConfig<std::string> {
    static constexpr bool kAlwaysEscape = true;
};

struct NoEscapeConfig : wwjson::BasicConfig<std::string> {
    static void EscapeKey(std::string& dst, const char* key, size_t len, char ec) {
        dst.append(key, len);
    }
    
    static void EscapeKey(std::string& dst, const char* key, size_t len, const char* ecs = wwjson::DEFAULT_ESCAPE_CHARS) {
        dst.append(key, len);
    }
    
    static void EscapeString(std::string& dst, const char* src, size_t len, char ec) {
        dst.append(src, len);
    }
    
    static void EscapeString(std::string& dst, const char* src, size_t len, const char* ecs = wwjson::DEFAULT_ESCAPE_CHARS) {
        dst.append(src, len);
    }
};

DEF_TAST(escape_basic, "test basic escape functionality")
{
    wwjson::RawBuilder json;
    json.AddItemEscape("hello\"world");
    std::string expect = R"("hello\"world",)";
    COUT(json.json, expect);
    
    wwjson::RawBuilder json2;
    json2.AddMemberEscape("key", "value\nwith\tabs");
    std::string expect2 = R"("key":"value\nwith\tabs",)";
    COUT(json2.json, expect2);
    
    wwjson::RawBuilder json3;
    json3.AddItemEscape("path\\to\\file", '\\');
    std::string expect3 = R"("path\\to\\file",)";
    COUT(json3.json, expect3);
    
    wwjson::RawBuilder json4;
    json4.AddMemberEscape("key", "multi\r\nchars\r\ntest", "\r\n");
    std::string expect4 = R"("key":"multi\r\nchars\r\ntest",)";
    COUT(json4.json, expect4);
}

DEF_TAST(escape_always_config, "test custom config with always escape")
{
    using EscapeBuilder = wwjson::GenericBuilder<std::string, EscapeConfig>;
    
    EscapeBuilder json;
    json.AddItem("hello\"world");
    std::string expect = R"("hello\"world",)";
    COUT(json.json, expect);
    
    EscapeBuilder json2;
    json2.AddMember("key", "value\nwith\tabs");
    std::string expect2 = R"("key":"value\nwith\tabs",)";
    COUT(json2.json, expect2);
    
    EscapeBuilder json3;
    json3.BeginArray();
    json3.AddItem("test");
    json3.AddItem("another\"test");
    json3.EndArray();
    std::string expect3 = R"(["test","another\"test"])";
    COUT(json3.json, expect3);
}

DEF_TAST(escape_scope, "test escape functionality with scope objects")
{
    wwjson::RawBuilder json;
    {
        auto arr = json.ScopeArray("items");
        arr.AddItemEscape("first\"item");
        arr.AddItemEscape("second\nitem");
    }
    std::string expect = R"("items":["first\"item","second\nitem"])";
    COUT(json.json, expect);
    
    wwjson::RawBuilder json2;
    {
        auto obj = json2.ScopeObject("data");
        obj.AddMemberEscape("text", "quote\"here", '\"');
        obj.AddMemberEscape("path", "C:\\path", '\\');
    }
    std::string expect2 = R"("data":{"text":"quote\"here","path":"C:\\path"})";
    COUT(json2.json, expect2);
}

DEF_TAST(escape_overloads, "test various string parameter overloads")
{
    wwjson::RawBuilder json;
    // Test const char*
    json.AddItem("test");
    // Test const char*, size_t
    json.AddItem("partial", 4);
    // Test std::string
    std::string str = "string";
    json.AddItem(str);
    
    std::string expect = R"("test","part","string",)";
    COUT(json.json, expect);
    
    wwjson::RawBuilder json2;
    // Test const char*
    json2.AddMember("key1", "value1");
    // Test std::string
    std::string key = "key2";
    std::string value = "value2";
    json2.AddMember(key.c_str(), value);
    
    std::string expect2 = R"("key1":"value1","key2":"value2",)";
    COUT(json2.json, expect2);
    
    wwjson::RawBuilder json3;
    // Test escape overloads
    json3.AddItemEscape("char\"", '\"');
    json3.AddItemEscape("chars\n\t", "\n\t");
    json3.AddItemEscape("cstr", 't');
    
    std::string expect3 = R"("char\"","chars\n\t","cs\tr",)";
    COUT(json3.json, expect3);
    
    wwjson::RawBuilder json4;
    // Test member escape overloads
    json4.AddMemberEscape("k1", "v1", '1');
    json4.AddMemberEscape("k2", "v2", "2");
    json4.AddMemberEscape("k3", "v3", 2, '3');
    
    std::string expect4 = R"("k1":"v\1","k2":"v\2","k3":"v\3",)";
    COUT(json4.json, expect4);
}

DEF_TAST(escape_no_config, "test custom config with no escaping")
{
    using NoEscapeBuilder = wwjson::GenericBuilder<std::string, NoEscapeConfig>;
    
    NoEscapeBuilder json;
    json.AddItemEscape("hello\"world");
    std::string expect = R"("hello"world",)";
    COUT(json.json, expect);
    
    NoEscapeBuilder json2;
    json2.AddMemberEscape("key", "value\nwith\ttabs");
    std::string expect2 = R"("key":"value
with	tabs",)";
    COUT(json2.json, expect2);
}

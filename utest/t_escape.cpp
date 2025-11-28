#include "couttast/tinytast.hpp"
#include "wwjson.hpp"

struct EscapeConfig : wwjson::BasicConfig<std::string> {
    static constexpr bool kEscapeKey = true;
    static constexpr bool kEscapeValue = true;
};

struct NoEscapeConfig : wwjson::BasicConfig<std::string> {
    static void EscapeString(std::string& dst, const char* src, size_t len) {
        dst.append(src, len);
    }
};

struct EscapeKeyConfig : wwjson::BasicConfig<std::string> {
    static constexpr bool kEscapeKey = true;
    static constexpr bool kEscapeValue = false;
    static void EscapeKey(std::string& dst, const char* src, size_t len) {
        EscapeString(dst, src, len);
    }
};

struct EscapeValueConfig : wwjson::BasicConfig<std::string> {
    static constexpr bool kEscapeKey = false;
    static constexpr bool kEscapeValue = true;
};

DEF_TAST(escape_table_basic, "test escape table functionality")
{
    // Test basic escape characters
    std::string dst;
    wwjson::BasicConfig<std::string>::EscapeString(dst, "Hello\nWorld\tTest", ::strlen("Hello\nWorld\tTest"));
    std::string expect = "Hello\\nWorld\\tTest";
    COUT(dst, expect);
    
    // Test quotes and backslash
    dst.clear();
    wwjson::BasicConfig<std::string>::EscapeString(dst, "Quote\"Here\\Back", ::strlen("Quote\"Here\\Back"));
    expect = "Quote\\\"Here\\\\Back";
    COUT(dst, expect);
    
    // Test null character
    dst.clear();
    const char* src_with_null = "Test\0Null";
    wwjson::BasicConfig<std::string>::EscapeString(dst, src_with_null, 9);
    expect = "Test\\0Null";
    COUT(dst, expect);
    
    // Test carriage return
    dst.clear();
    wwjson::BasicConfig<std::string>::EscapeString(dst, "Line1\r\nLine2", 12);
    expect = "Line1\\r\\nLine2";
    COUT(dst, expect);
}

DEF_TAST(escape_table_utf8, "test UTF-8 characters are not escaped")
{
    // Test UTF-8 characters should pass through unchanged
    std::string dst;
    const char* utf8_str = "你好世界";  // Chinese characters in UTF-8
    wwjson::BasicConfig<std::string>::EscapeString(dst, utf8_str, ::strlen(utf8_str));
    std::string expect = "你好世界";
    COUT(dst, expect);
    
    // Test mixed ASCII and UTF-8
    dst.clear();
    const char* mixed = "Hello 世界\nTest";
    wwjson::BasicConfig<std::string>::EscapeString(dst, mixed, ::strlen(mixed));
    expect = "Hello 世界\\nTest";
    COUT(dst, expect);
}

DEF_TAST(escape_table_mapping, "test escape table map optimization")
{
    // Verify the escape table size is 128 bytes
    constexpr size_t table_size = wwjson::BasicConfig<std::string>::kEscapeTable.size();
    COUT(table_size, size_t{128});
    
    // Verify some escape mappings
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\n'], uint8_t{'n'});
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\t'], uint8_t{'t'});
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\r'], uint8_t{'r'});
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['"'], uint8_t{'"'});
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\\'], uint8_t{'\\'});
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\0'], uint8_t{'0'});
    
    // Verify a non-escaped character returns 0
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['A'], uint8_t{0});
}

DEF_TAST(escape_builder_api, "test escape methods in builder")
{
    wwjson::RawBuilder json;
    
    // Test AddItemEscape (now uses default escape table)
    json.AddItemEscape("hello\"world");
    std::string expect = R"("hello\"world",)";
    COUT(json.json, expect);
    
    // Test AddMemberEscape
    json.Clear();
    json.AddMemberEscape("key", "value\nwith\ttabs");
    expect = R"("key":"value\nwith\ttabs",)";
    COUT(json.json, expect);

    // Test std::string as key
    std::string key = "key";
    json.Clear();
    json.AddMemberEscape(key, "value\nwith\ttabs");
    expect = R"("key":"value\nwith\ttabs",)";
    COUT(json.json, expect);
    
    // Test with std::string
    json.Clear();
    std::string str = "test\\path";
    json.AddItemEscape(str);
    expect = R"("test\\path",)";
    COUT(json.json, expect);
}

DEF_TAST(escape_always_config, "test custom config with always escape")
{
    using EscapeBuilder = wwjson::GenericBuilder<std::string, EscapeConfig>;
    
    EscapeBuilder json;
    json.AddItem("hello\"world");
    std::string expect = R"("hello\"world",)";
    COUT(json.json, expect);
    
    json.Clear();
    json.AddMember("key", "value\nwith\ttabs");
    expect = R"("key":"value\nwith\ttabs",)";
    COUT(json.json, expect);
    
    json.Clear();
    json.BeginArray();
    json.AddItem("test");
    json.AddItem("another\"test");
    json.EndArray();
    expect = R"(["test","another\"test"])";
    COUT(json.json, expect);
    
    // Test that even regular PutValue uses escaping when kAlwaysEscape=true
    json.Clear();
    json.PutValue("quote\"here");
    expect = R"("quote\"here")";
    COUT(json.json, expect);
}

DEF_TAST(escape_key_config, "test custom config with key escaping only")
{
    using EscapeKeyBuilder = wwjson::GenericBuilder<std::string, EscapeKeyConfig>;
    
    EscapeKeyBuilder json;
    json.AddMember("key\"with\"quotes", "value\nwith\ttabs");
    std::string expect = R"("key\"with\"quotes":"value
with	tabs",)";
    COUT(json.json, expect);
    
    // Test PutKey overloads
    json.Clear();
    std::string key = "test\"key";
    json.PutKey(key);
    json.PutValue("value");
    expect = R"("test\"key":"value")";
    COUT(json.json, expect);
    
    json.Clear();
    json.PutKey("key", ::strlen("key"));
    json.PutValue("value");
    expect = R"("key":"value")";
    COUT(json.json, expect);
}

DEF_TAST(escape_value_config, "test custom config with value escaping only")
{
    using EscapeValueBuilder = wwjson::GenericBuilder<std::string, EscapeValueConfig>;
    
    EscapeValueBuilder json;
    json.AddMember("key\"with\"quotes", "value\nwith\ttabs");
    std::string expect = R"("key"with"quotes":"value\nwith\ttabs",)";
    COUT(json.json, expect);
    
    // Test PutKey doesn't escape keys
    json.Clear();
    json.PutKey("key\"with\"quotes");
    json.PutValue("value\nwith\ttabs");
    expect = R"("key"with"quotes":"value\nwith\ttabs")";
    COUT(json.json, expect);
}

DEF_TAST(escape_no_config, "test custom config with no escaping")
{
    using NoEscapeBuilder = wwjson::GenericBuilder<std::string, NoEscapeConfig>;
    
    NoEscapeBuilder json;
    json.AddItemEscape("hello\"world");
    std::string expect = R"("hello"world",)";
    COUT(json.json, expect);
    
    json.Clear();
    json.AddMemberEscape("key", "value\nwith\ttabs");
    expect = R"("key":"value
with	tabs",)";
    COUT(json.json, expect);
}

DEF_TAST(escape_scope_objects, "test escape functionality with scope objects")
{
    wwjson::RawBuilder json;
    {
        auto arr = json.ScopeArray("items");
        arr.AddItemEscape("first\"item");
        arr.AddItemEscape("second\nitem");
    }
    std::string expect = R"("items":["first\"item","second\nitem"])";
    COUT(json.json, expect);
    
    json.Clear();
    {
        auto obj = json.ScopeObject("data");
        obj.AddMemberEscape("text", "quote\"here");
        obj.AddMemberEscape("path", "C:\\path");
    }
    expect = R"("data":{"text":"quote\"here","path":"C:\\path"})";
    COUT(json.json, expect);
}

DEF_TAST(escape_edge_cases, "test edge cases for escape functionality")
{
    // Empty string
    std::string dst;
    wwjson::BasicConfig<std::string>::EscapeString(dst, "", 0);
    std::string expect = "";
    COUT(dst, expect);
    
    // String with only non-escaped characters
    dst.clear();
    wwjson::BasicConfig<std::string>::EscapeString(dst, "ABC123", 6);
    expect = "ABC123";
    COUT(dst, expect);
    
    // String with only escaped characters
    dst.clear();
    wwjson::BasicConfig<std::string>::EscapeString(dst, "\n\t\r\"\0\\", 6);
    expect = "\\n\\t\\r\\\"\\0\\\\";
    COUT(dst, expect);
    
    // Builder with empty escape
    wwjson::RawBuilder json;
    json.AddItemEscape("");
    expect = R"("",)";
    COUT(json.json, expect);
}

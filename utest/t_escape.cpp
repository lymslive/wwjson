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
    std::string dst;
    wwjson::BasicConfig<std::string>::EscapeString(dst, "Hello\nWorld\tTest", ::strlen("Hello\nWorld\tTest"));
    std::string expect = "Hello\\nWorld\\tTest";
    COUT(dst, expect);
    
    dst.clear();
    wwjson::BasicConfig<std::string>::EscapeString(dst, "Quote\"Here\\Back", ::strlen("Quote\"Here\\Back"));
    expect = "Quote\\\"Here\\\\Back";
    COUT(dst, expect);
    
    dst.clear();
    const char* src_with_null = "Test\0Null";
    wwjson::BasicConfig<std::string>::EscapeString(dst, src_with_null, 9);
    expect = "Test\\0Null";
    COUT(dst, expect);
    
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
    
    // Test all standard C/C++ escape sequences
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\a'], uint8_t{'a'});  // bell
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\b'], uint8_t{'b'});  // backspace
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\t'], uint8_t{'t'});  // tab
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\n'], uint8_t{'n'});  // newline
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\v'], uint8_t{'v'});  // vertical tab
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\f'], uint8_t{'f'});  // form feed
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\r'], uint8_t{'r'});  // carriage return
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['"'], uint8_t{'"'});  // double quote
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\\'], uint8_t{'\\'}); // backslash
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['\0'], uint8_t{'0'});  // null
    
    // Test non-printable characters map to '.'
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable[0x01], uint8_t{'.'});  // SOH
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable[0x02], uint8_t{'.'});  // STX
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable[0x03], uint8_t{'.'});  // ETX
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable[0x7F], uint8_t{'.'});  // DEL
    
    // Test some printable characters return 0 (no escape)
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['A'], uint8_t{0});
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable[' '], uint8_t{0});
    COUT(wwjson::BasicConfig<std::string>::kEscapeTable['9'], uint8_t{0});
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
    COUT(json.GetResult(), expect);
    
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
    COUT(json.GetResult(), expect);
    
    json.Clear();
    {
        auto obj = json.ScopeObject("data");
        obj.AddMemberEscape("text", "quote\"here");
        obj.AddMemberEscape("path", "C:\\path");
    }
    expect = R"("data":{"text":"quote\"here","path":"C:\\path"})";
    COUT(json.GetResult(), expect);
}

DEF_TAST(escape_std_ascii, "test standard ASCII escape characters")
{
    // Test C/C++ standard escape sequences
    std::string dst;
    
    // Test bell character (\a)
    dst.clear();
    wwjson::BasicConfig<std::string>::EscapeString(dst, "Bell\aRing", 9);
    std::string expect = "Bell\\aRing";
    COUT(dst, expect);
    
    // Test backspace (\b)
    dst.clear();
    wwjson::BasicConfig<std::string>::EscapeString(dst, "Back\bSpace", 10);
    expect = "Back\\bSpace";
    COUT(dst, expect);
    
    // Test form feed (\f)
    dst.clear();
    wwjson::BasicConfig<std::string>::EscapeString(dst, "Form\fFeed", 9);
    expect = "Form\\fFeed";
    COUT(dst, expect);
    
    // Test vertical tab (\v)
    dst.clear();
    wwjson::BasicConfig<std::string>::EscapeString(dst, "Vertical\vTab", 12);
    expect = "Vertical\\vTab";
    COUT(dst, expect);
    
    // Test non-printable characters replaced with '\.'
    dst.clear();
    const char non_printable[] = {0x01, 0x02, 0x03, 0x04, 0}; // SOH, STX, ETX, EOT
    wwjson::BasicConfig<std::string>::EscapeString(dst, non_printable, 4);
    expect = "\\.\\.\\.\\.";
    COUT(dst, expect);
    
    // Test DEL character (0x7F)
    dst.clear();
    char with_del[] = "Test  "; // Space for DEL and null terminator
    with_del[4] = 0x7F; with_del[5] = 0;
    wwjson::BasicConfig<std::string>::EscapeString(dst, with_del, 5);
    expect = "Test\\.";
    COUT(dst, expect);
    
    // Test mixed printable and non-printable
    dst.clear();
    const char mixed[] = {0x01, 'A', 0x02, 'B', 0x03, 0};
    wwjson::BasicConfig<std::string>::EscapeString(dst, mixed, 5);
    expect = "\\.A\\.B\\.";
    COUT(dst, expect);
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

#include "couttast/tinytast.hpp"
#include "wwjson.hpp"
#include "test_util.h"
#include <string>
#include <limits>

DEF_TAST(basic_builder, "test json builder with raw string")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    builder.AddMember("int", 123);
    builder.AddMember("string", "123");
    builder.AddMember("char", '1');

    unsigned char c = '2';
    builder.AddMember("uchar", c);

    short sh = 280;
    builder.AddMember("short", sh);

    double half = 0.5;
    builder.AddMember("double", half);

    double third = 1.0/3.0;
    builder.AddMember("double", third);

    // add number as string with extra argument, no matter true/false
    builder.AddMember("ints", 124, true);
    builder.AddMember("intf", 125, false);

    builder.EndObject();
    builder.GetResult();

    std::string expect = R"({"int":123,"string":"123","char":49,"uchar":50,"short":280,"double":0.5,"double":0.333333,"ints":"124","intf":"125"})";
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(basic_builder_root, "test json builder using BeginRoot")
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    builder.AddMember("int", 123);
    builder.AddMember("string", "123");
    builder.AddMember("char", '1');
    builder.EndRoot();

    std::string expect = R"({"int":123,"string":"123","char":49})";
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);

    builder.Clear();
    builder.BeginRoot('[');
    builder.AddItem(123);
    builder.AddItem("123");
    builder.AddItem('1');
    builder.EndRoot(']');

    expect = R"([123,"123",49])";
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(basic_builder_nest, "test build nest json with array of object")
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();

    std::string title = "Title";
    builder.AddMember("title", title);

    builder.PutKey("head");
    builder.BeginObject();
    builder.AddMember("int", 123);
    builder.AddMember("string", "123");
    builder.EndObject();

    builder.PutKey("bodys");
    builder.BeginArray();

    builder.BeginObject();
    builder.AddMember("char", '1');
    unsigned char c = '2';
    builder.AddMember("uchar", c);
    builder.EndObject();

    builder.BeginObject();
    short sh = 280;
    builder.AddMember("short", sh);
    double half = 0.5;
    builder.AddMember("double", half);
    double third = 1.0/3.0;
    builder.AddMember("double", third);
    builder.EndObject();

    builder.EndArray();

    builder.EndRoot();
    // Not neccessay after EndRoot(), but needed after EndObject
//  builder.GetResult();

    std::string expect = R"({"title":"Title","head":{"int":123,"string":"123"},"bodys":[{"char":49,"uchar":50},{"short":280,"double":0.5,"double":0.333333}]})";
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(basic_wrapper, "test M1 string interface wrapper methods")
{
    wwjson::RawBuilder builder(64);
    
    // Test PutChar
    builder.PutChar('{');
    builder.PutChar('}');
    std::string expect1 = "{}";
    COUT(builder.json, expect1);
    COUT(test::IsJsonValid(builder.json), true);
    
    // Test FixTail - replace trailing '}' with ','
    builder.FixTail('}', ',');
    std::string expect2 = "{,";
    COUT(builder.json, expect2);
    COUT(test::IsJsonValid(builder.json), false); // Invalid JSON
    
    // Test FixTail - add ']' when tail doesn't match
    builder.FixTail('{', ']');
    std::string expect3 = "{,]";
    COUT(builder.json, expect3);
    COUT(test::IsJsonValid(builder.json), false); // Invalid JSON
    
    // Test Append methods
    builder.Clear();
    builder.Append("test");
    builder.Append(" string", 7);
    std::string expect4 = "test string";
    COUT(builder.json, expect4);
    
    // Test Size
    size_t size = builder.Size();
    size_t expectSize = 11;
    COUT(size, expectSize);
    
    // Test Back and Front
    char front = builder.Front();
    char back = builder.Back();
    COUT(front, 't');
    COUT(back, 'g');
    
    // Test PushBack
    builder.PushBack('!');
    std::string expect5 = "test string!";
    COUT(builder.json, expect5);
}

DEF_TAST(basic_null_bool_empty, "test null, bool, empty array and empty object")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Test null value using AddMember with nullptr
    builder.AddMember("null_value", nullptr);
    
    // Test null value using PutKey + PutNull + SepItem
    builder.PutKey("null_direct");
    builder.PutNull();
    builder.SepItem();

    // Test bool values using AddMember
    builder.AddMember("bool_true", true);
    builder.AddMember("bool_false", false);
    
    // Test bool values using PutKey + PutValue + SepItem
    builder.PutKey("bool_direct_true");
    builder.PutValue(true);
    builder.SepItem();
    builder.PutKey("bool_direct_false");
    builder.PutValue(false);
    builder.SepItem();

    // Test empty array using EmptyArray
    builder.PutKey("empty_array_direct");
    builder.EmptyArray();
    builder.SepItem();

    // Test empty object using EmptyObject
    builder.PutKey("empty_object_direct");
    builder.EmptyObject();
    builder.PutNext();  // Using PutNext instead of SepItem

    // Test array with null and bool values using AddItem
    builder.PutKey("array_with_null_bool");
    builder.BeginArray();
    builder.AddItem(nullptr);
    builder.AddItem(true);
    builder.AddItem(false);
    builder.AddItem("string");  // For comparison
    builder.EndArray();

    // Test object with null and bool values
    builder.PutKey("object_with_null_bool");
    builder.BeginObject();
    builder.AddMember("null_field", nullptr);
    builder.AddMember("true_field", true);
    builder.AddMember("false_field", false);
    builder.EndObject();

    builder.EndObject();
    builder.GetResult();

    std::string expect = R"({"null_value":null,"null_direct":null,"bool_true":true,"bool_false":false,"bool_direct_true":true,"bool_direct_false":false,"empty_array_direct":[],"empty_object_direct":{},"array_with_null_bool":[null,true,false,"string"],"object_with_null_bool":{"null_field":null,"true_field":true,"false_field":false}})";
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(basic_low_level, "test using low-level methods PutKey/PutValue/PutNext")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Using low-level methods instead of AddMember
    builder.PutKey("int");
    builder.PutValue(123);
    builder.PutNext();

    builder.PutKey("string");
    builder.PutValue("123");
    builder.PutNext();

    builder.PutKey("char");
    builder.PutValue('1');
    builder.PutNext();

    unsigned char c = '2';
    builder.PutKey("uchar");
    builder.PutValue(c);
    builder.PutNext();

    short sh = 280;
    builder.PutKey("short");
    builder.PutValue(sh);
    builder.PutNext();

    double half = 0.5;
    builder.PutKey("double");
    builder.PutValue(half);
    builder.PutNext();

    double third = 1.0/3.0;
    builder.PutKey("double");
    builder.PutValue(third);
    builder.PutNext();

    // Add numbers as strings using low-level methods
    builder.PutKey("ints");
    builder.PutChar('"');
    builder.PutValue(124);
    builder.PutChar('"');
    builder.PutNext();

    builder.PutKey("intf");
    builder.PutChar('"');
    builder.PutValue(125);
    builder.PutChar('"');

    // Add an array at the end using low-level methods
    builder.PutNext();
    builder.PutKey("numbers");
    builder.BeginArray();
    
    // Add array elements using PutValue/PutNext instead of AddItem
    builder.PutValue(1);
    builder.PutNext();
    builder.PutValue(2);
    builder.PutNext();
    builder.PutValue(3);
    
    builder.EndArray();

    builder.EndObject();

    std::string expect = R"({"int":123,"string":"123","char":49,"uchar":50,"short":280,"double":0.5,"double":0.333333,"ints":"124","intf":"125","numbers":[1,2,3]})";
    COUT(builder.GetResult(), expect);
}

DEF_TAST(basic_addmember_overloads, "test new AddMember overloads with different key parameter types")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Test AddMember with const char* key (original)
    builder.AddMember("str_key", "string_value");
    builder.AddMember("int_key", 42);

    // Test AddMember with const char* key + size_t length
    //! not support
//  builder.AddMember("len_key", 5, "hello");
//  builder.AddMember("len_key2", 4, 3.14);

    // Test AddMember with std::string key
    std::string string_key = "std_key";
    builder.AddMember(string_key, "std_value");
    builder.AddMember(string_key, 123);

    builder.EndObject();

    std::string expect = R"({"str_key":"string_value","int_key":42,"std_key":"std_value","std_key":123})";
    COUT(builder.GetResult(), expect);
}

DEF_TAST(basic_string_view_support, "test std::string_view support for keys and values")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Test AddMember with std::string_view key
    std::string_view sv_key1 = "sv_key1";
    builder.AddMember(sv_key1, "string_value");
    builder.AddMember(sv_key1, 42);

    // Test AddMember with std::string_view value
    std::string_view sv_value = "sv_value";
    builder.AddMember("sv_value_key", sv_value);

    // Test operator[] with std::string_view key
    std::string_view sv_key2 = "sv_key2";
    builder[sv_key2] = "assigned_value";

    // Test operator[] with std::string_view key and number assignment
    std::string_view sv_key3 = "sv_key3";
    builder[sv_key3] = 3.14;

    // Test AddMemberEscape with std::string_view key
    std::string_view sv_key4 = "sv_key4";
    builder.AddMemberEscape(sv_key4, "escaped\nvalue");

    // Test with std::string_view value that needs escaping
    std::string_view sv_escape_value = "value\twith\ttabs";
    builder.AddMemberEscape("escape_sv_value", sv_escape_value);

    builder.EndObject();
    builder.GetResult();

    std::string expect = R"({"sv_key1":"string_value","sv_key1":42,"sv_value_key":"sv_value","sv_key2":"assigned_value","sv_key3":3.14,"sv_key4":"escaped\nvalue","escape_sv_value":"value\twith\ttabs"})";
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(basic_getresult, "test GetResult removes trailing comma")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();
    builder.AddMember("key1", "value1");
    builder.AddMember("key2", "value2");
    builder.EndObject();

    // const GetResult keep trailing comma
    {
        const wwjson::RawBuilder& cb = const_cast<const wwjson::RawBuilder&>(builder);
        auto& result = cb.GetResult();
        std::string expect = R"({"key1":"value1","key2":"value2"},)";
        COUT(result, expect);
    }

    // Non-const GetResult should remove the trailing comma
    auto& result = builder.GetResult();
    std::string expect = R"({"key1":"value1","key2":"value2"})";
    COUT(result, expect);
    COUT(builder.json, expect);
}

// Best practice example: standalone function returning JSON string
// NOTE: We use MoveResult() instead of direct return for optimal performance
// because NRVO (Named Return Value Optimization) only applies to local variables,
// not to member variables of objects. The builder.json is a member variable,
// so returning it directly would trigger a copy. MoveResult() uses move semantics
// to transfer ownership without copying the string content.
std::string buildJsonString() {
    wwjson::RawBuilder builder;
    builder.BeginObject();
    builder.AddMember("function", "buildJsonString");
    builder.AddMember("optimized", true);
    builder.AddMember("method", "MoveResult");
    builder.EndObject();
    
    // Best practice: use MoveResult() for zero-copy performance
    // This moves the string content instead of copying it
    return builder.MoveResult();
}

// Overload version: build JSON string into provided reference parameter
// This approach can be useful when you want to avoid any potential moves
// or when you need to build multiple JSON strings in sequence
void buildJsonString(std::string& output) {
    wwjson::RawBuilder builder;
    builder.BeginObject();
    builder.AddMember("function", "buildJsonString");
    builder.AddMember("output_param", true);
    builder.AddMember("method", "MoveResult");
    builder.EndObject();
    
    // Assign the moved result to the output parameter
    // This is still zero-copy due to move assignment
    output = builder.MoveResult();
}

DEF_TAST(basic_moveresult, "test MoveResult method")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();
    builder.AddMember("moved", true);
    builder.EndObject();

    // Test MoveResult - transfer ownership
    std::string moved_result = builder.MoveResult();
    std::string expect = R"({"moved":true})";
    COUT(moved_result, expect);

    // After MoveResult, the builder should be empty
    COUT(builder.json, R"()");
    
    // Test best practice: standalone function returning JSON string
    std::string function_result = buildJsonString();
    std::string expect_function = R"({"function":"buildJsonString","optimized":true,"method":"MoveResult"})";
    COUT(function_result, expect_function);
    
    // Test overload version: build into reference parameter
    std::string ref_result;
    buildJsonString(ref_result);
    std::string expect_ref = R"({"function":"buildJsonString","output_param":true,"method":"MoveResult"})";
    COUT(ref_result, expect_ref);
    
    // Performance recommendation summary:
    // 1. For most cases: use return builder.MoveResult() - zero-copy move semantics
    // 2. For reference parameter: use output = builder.MoveResult() - still zero-copy
    // 3. Avoid: return builder.json - triggers copy, NRVO doesn't apply to members
    // 4. Avoid: return std::move(builder.json) - same as MoveResult() but less clear
}

DEF_TAST(basic_null_string, "test null pointer safety in string edge cases")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();
    
    // Test null pointer safety - should not crash and return early
    builder.PutKey((const char*)nullptr);
    builder.PutKey((const char*)nullptr, 10);
    builder.PutValue((const char*)nullptr);
    builder.PutValue((const char*)nullptr, 10);
    builder.AddItemEscape((const char*)nullptr);
    builder.AddItemEscape((const char*)nullptr, 10);
    builder.Append((const char*)nullptr);
    builder.Append((const char*)nullptr, 10);
    
    // Test null pointer in static escape methods
    std::string str;
    wwjson::BasicConfig<std::string>::EscapeKey(str, (const char*)nullptr, 10);
    wwjson::BasicConfig<std::string>::EscapeString(str, (const char*)nullptr, 10);
    
    // Add some valid content to verify builder still works
    builder.AddMember("valid_key", "valid_value");
    
    builder.EndObject();
    
    // Result should contain only the valid member, null calls should be ignored
    std::string expect = R"({"valid_key":"valid_value"})";
    COUT(builder.GetResult(), expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(basic_empty_string, "test not null but empty string edge cases")
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    
    // Test zero-length strings - should work properly
    builder.PutKey("", 0);
    builder.PutValue("", 0);
    builder.AddItemEscape("", 0);
    builder.Append("", 0);
    
    // Test empty C-string - should work properly
    builder.PutKey("");
    builder.PutValue("");
    builder.AddItemEscape("");
    builder.Append("");
    
    // Add separator after empty strings that add keys
    builder.SepItem();
    
    // Add a valid member to separate from empty key-value pairs
    builder.AddMember("valid_key", "valid_value");
    
    builder.EndRoot();
    
    // Empty strings should be processed and added as empty JSON strings with quotes
    std::string expect = R"({"":"""","":"""",,"valid_key":"valid_value"})";
    COUT(builder.json, expect);
    
    // Check if it's valid JSON (might be invalid due to empty keys)
    COUT(test::IsJsonValid(builder.json), false);  // Likely false due to empty keys or malformed structure
    
    // Test a cleaner version with just one empty string
    wwjson::RawBuilder builder2;
    builder2.BeginRoot();
    builder2.PutKey("empty_key");
    builder2.PutValue("");
    builder2.EndRoot();
    
    std::string expect2 = R"({"empty_key":""})";
    COUT(builder2.json, expect2);
    COUT(test::IsJsonValid(builder2.json), true);
    
    // Test empty key with empty value
    wwjson::RawBuilder builder3;
    builder3.BeginRoot();
    builder3.PutKey("");
    builder3.PutValue("empty_val");
    builder3.EndRoot();
    
    std::string expect3 = R"({"":"empty_val"})";
    COUT(builder3.json, expect3);
    COUT(test::IsJsonValid(builder3.json), true);  // JSON is actually valid even with empty key
}

DEF_TAST(basic_integer_member, "test serialization of 8 standard integer types")
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();

    // Test all signed integer types
    int8_t i8 = -128;
    int16_t i16 = -32768;
    int32_t i32 = -2147483648;
    int64_t i64 = -9223372036854775807LL;

    builder.AddMember("int8_t", i8);
    builder.AddMember("int16_t", i16);
    builder.AddMember("int32_t", i32);
    builder.AddMember("int64_t", i64);

    // Test all unsigned integer types
    uint8_t u8 = 255;
    uint16_t u16 = 65535;
    uint32_t u32 = 4294967295U;
    uint64_t u64 = 18446744073709551615ULL;

    builder.AddMember("uint8_t", u8);
    builder.AddMember("uint16_t", u16);
    builder.AddMember("uint32_t", u32);
    builder.AddMember("uint64_t", u64);

    // Test special values: zero
    builder.AddMember("zero", 0);

    // Test small integers (0-99) which should use digit pair optimization
    builder.AddMember("small_pos", 42);
    builder.AddMember("small_neg", -42);

    // Test boundary values around 100
    builder.AddMember("border_99", 99);
    builder.AddMember("border_100", 100);
    builder.AddMember("border_101", 101);

    // Test boundary values around 1000
    builder.AddMember("border_999", 999);
    builder.AddMember("border_1000", 1000);
    builder.AddMember("border_1001", 1001);

    // Test boundary values around 10000
    builder.AddMember("border_9999", 9999);
    builder.AddMember("border_10000", 10000);
    builder.AddMember("border_10001", 10001);

    builder.EndRoot();

    // Expected JSON string
    std::string expect = R"({"int8_t":-128,"int16_t":-32768,"int32_t":-2147483648,"int64_t":-9223372036854775807,"uint8_t":255,"uint16_t":65535,"uint32_t":4294967295,"uint64_t":18446744073709551615,"zero":0,"small_pos":42,"small_neg":-42,"border_99":99,"border_100":100,"border_101":101,"border_999":999,"border_1000":1000,"border_1001":1001,"border_9999":9999,"border_10000":10000,"border_10001":10001})";
    
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(basic_integer_item, "test serialization of integers in arrays")
{
    wwjson::RawBuilder builder;
    builder.BeginRoot('[');

    // Add various integer values to array
    builder.AddItem(static_cast<int8_t>(-128));
    builder.AddItem(static_cast<uint8_t>(255));
    builder.AddItem(static_cast<int16_t>(-32768));
    builder.AddItem(static_cast<uint16_t>(65535));
    builder.AddItem(static_cast<int32_t>(-2147483648));
    builder.AddItem(static_cast<uint32_t>(4294967295U));
    builder.AddItem(static_cast<int64_t>(-9223372036854775807LL));
    builder.AddItem(static_cast<uint64_t>(18446744073709551615ULL));

    // Add small integers that should use optimization
    builder.AddItem(0);
    builder.AddItem(1);
    builder.AddItem(42);
    builder.AddItem(99);
    builder.AddItem(-1);
    builder.AddItem(-42);

    // Add boundary values
    builder.AddItem(100);
    builder.AddItem(101);
    builder.AddItem(999);
    builder.AddItem(1000);
    builder.AddItem(1001);
    builder.AddItem(9999);
    builder.AddItem(10000);
    builder.AddItem(10001);

    builder.EndRoot(']');

    std::string expect = R"([-128,255,-32768,65535,-2147483648,4294967295,-9223372036854775807,18446744073709551615,0,1,42,99,-1,-42,100,101,999,1000,1001,9999,10000,10001])";
    
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(basic_float_serialization, "test various floating-point serialization scenarios")
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    
    // Test basic floating-point values
    builder.AddMember("zero", 0.0);
    builder.AddMember("positive", 3.14159);
    builder.AddMember("negative", -2.71828);
    builder.AddMember("small", 0.00123);
    builder.AddMember("large", 1234567.89);
    
    // Test special values - NaN and Inf should become "null" for valid JSON
    float positive_inf = std::numeric_limits<float>::infinity();
    float negative_inf = -std::numeric_limits<float>::infinity();
    float nan_val = std::numeric_limits<float>::quiet_NaN();
    
    builder.AddMember("pos_inf", positive_inf);
    builder.AddMember("neg_inf", negative_inf);
    builder.AddMember("nan_val", nan_val);
    
    // Test precision for different floating-point types
    builder.AddMember("float_val", 3.14159f);
    builder.AddMember("double_val", 2.718281828459045);
    
    // Test very precise values
    builder.AddMember("precise_float", 1.23456789f);
    builder.AddMember("precise_double", 1.23456789012345);
    
    builder.EndRoot();
    
    // Expected output based on to_chars support and format choice
    std::string expect;
    
    // Default to high precision expectations
    expect = R"({"zero":0,"positive":3.14159,"negative":-2.71828,"small":0.00123,"large":1234567.89,"pos_inf":null,"neg_inf":null,"nan_val":null,"float_val":3.14159,"double_val":2.718281828459045,"precise_float":1.2345679,"precise_double":1.23456789012345})";
    
    // If no to_chars support, adjust expectations based on format choice
    if (!::wwjson::has_float_to_chars_v<double>) {
#if WWJSON_USE_SIMPLE_FLOAT_FORMAT
        // Simple format expectations
        expect = R"({"zero":0,"positive":3.14159,"negative":-2.71828,"small":0.00123,"large":1.23457e+06,"pos_inf":null,"neg_inf":null,"nan_val":null,"float_val":3.14159,"double_val":2.71828,"precise_float":1.23457,"precise_double":1.23457})";
#else
        // High precision fallback expectations (same as default)
        expect = R"({"zero":0,"positive":3.14159,"negative":-2.71828,"small":0.00123,"large":1234567.89,"pos_inf":null,"neg_inf":null,"nan_val":null,"float_val":3.14159,"double_val":2.718281828459045,"precise_float":1.2345679,"precise_double":1.23456789012345})";
#endif
    }
    
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}


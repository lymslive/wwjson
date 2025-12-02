#include "couttast/tinytast.hpp"
#include "wwjson.hpp"
#include <string>

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

    std::string expect = R"({"int":123,"string":"123","char":49,"uchar":50,"short":280,"double":0.500000,"double":0.333333,"ints":"124","intf":"125"})";
    COUT(builder.json, expect);
}

DEF_TAST(basic_builder_nest, "test build nest json with array of object")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    std::string title = "Title";
    builder.AddMember("title", title);

    builder.PutKey("head");
    builder.BeginObject();
    builder.AddMember("int", 123);
    builder.AddMember("string", "123");
    builder.EndObject();

    builder.SepItem();

    builder.PutKey("bodys");
    builder.BeginArray();

    builder.BeginObject();
    builder.AddMember("char", '1');
    unsigned char c = '2';
    builder.AddMember("uchar", c);
    builder.EndObject();

    builder.SepItem();

    builder.BeginObject();
    short sh = 280;
    builder.AddMember("short", sh);
    double half = 0.5;
    builder.AddMember("double", half);
    double third = 1.0/3.0;
    builder.AddMember("double", third);
    builder.EndObject();

    builder.EndArray();

    builder.EndObject();

    std::string expect = R"({"title":"Title","head":{"int":123,"string":"123"},"bodys":[{"char":49,"uchar":50},{"short":280,"double":0.500000,"double":0.333333}]})";
    COUT(builder.json, expect);
}


DEF_TAST(basic_wrapper, "test M1 string interface wrapper methods")
{
    wwjson::RawBuilder builder(64);
    
    // Test PutChar
    builder.PutChar('{');
    builder.PutChar('}');
    std::string expect1 = "{}";
    COUT(builder.json, expect1);
    
    // Test FixTail - replace trailing '}' with ','
    builder.FixTail('}', ',');
    std::string expect2 = "{,";
    COUT(builder.json, expect2);
    
    // Test FixTail - add ']' when tail doesn't match
    builder.FixTail('{', ']');
    std::string expect3 = "{,]";
    COUT(builder.json, expect3);
    
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
    builder.SepItem();

    // Test object with null and bool values
    builder.PutKey("object_with_null_bool");
    builder.BeginObject();
    builder.AddMember("null_field", nullptr);
    builder.AddMember("true_field", true);
    builder.AddMember("false_field", false);
    builder.EndObject();

    builder.EndObject();

    std::string expect = R"({"null_value":null,"null_direct":null,"bool_true":true,"bool_false":false,"bool_direct_true":true,"bool_direct_false":false,"empty_array_direct":[],"empty_object_direct":{},"array_with_null_bool":[null,true,false,"string"],"object_with_null_bool":{"null_field":null,"true_field":true,"false_field":false}})";
    COUT(builder.json, expect);
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

    std::string expect = R"({"int":123,"string":"123","char":49,"uchar":50,"short":280,"double":0.500000,"double":0.333333,"ints":"124","intf":"125","numbers":[1,2,3]})";
    COUT(builder.json, expect);
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
    COUT(builder.json, expect);
}

DEF_TAST(basic_getresult, "test GetResult removes trailing comma")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();
    builder.AddMember("key1", "value1");
    builder.AddMember("key2", "value2");
    // Simulate case where trailing comma might exist
    builder.EndObject(true);

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

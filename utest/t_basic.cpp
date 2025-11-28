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

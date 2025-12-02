#include "couttast/tinytast.hpp"
#include "wwjson.hpp"
#include <string>

DEF_TAST(operator_string_key, "test operator[] with string keys and assignment")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Test C-string key with string value
    builder["name"] = "John";
    builder["city"] = "New York";

    // Test std::string key with string value  
    std::string key1 = "age";
    std::string key2 = "salary";
    builder[key1] = 25;
    builder[key2] = 50000.5;

    // Test boolean and null values
    builder["active"] = true;
    builder["middle_name"] = nullptr;

    builder.EndObject();

    std::string expect = R"({"name":"John","city":"New York","age":25,"salary":50000.500000,"active":true,"middle_name":null})";
    COUT(builder.json, expect);
}

DEF_TAST(operator_array_index, "test operator[] with array indices and assignment")
{
    wwjson::RawBuilder builder;
    builder.BeginArray();

    // Test array indices with string values
    builder[0] = "first";
    builder[1] = "second";
    builder[2] = "third";

    // Test negative and large indices
    builder[-1] = "negative_test";
    builder[100] = "large_index";

    // Test with numeric values
    builder[3] = 42;
    builder[4] = 3.14;
    builder[5] = false;

    builder.EndArray();

    std::string result = builder.GetResult();
    // Verify array format
    COUT(result.front(), '[');
    COUT(result.back(), ']');
    
    // Verify content presence
    COUT(result.find("\"first\"") != std::string::npos, true);
    COUT(result.find("\"second\"") != std::string::npos, true);
    COUT(result.find("\"negative_test\"") != std::string::npos, true);
    COUT(result.find("\"large_index\"") != std::string::npos, true);
    COUT(result.find("42") != std::string::npos, true);
    COUT(result.find("3.14") != std::string::npos, true);
    COUT(result.find("false") != std::string::npos, true);
}

DEF_TAST(operator_mixed_usage, "test operator[] mixed with traditional methods")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Traditional method
    builder.AddMember("traditional_key", "traditional_value");

    // Operator method
    builder["operator_key"] = "operator_value";

    // Mix them in nested structure
    builder.BeginArray("items");
    builder[0] = "item1";
    builder.AddItem("item2");  // Traditional array method
    builder[2] = "item3";
    builder.EndArray();

    // Mix in object
    builder.BeginObject("nested");
    builder["nested_operator"] = "nested_value";
    builder.AddMember("nested_traditional", "traditional_value");
    builder.EndObject();

    builder.EndObject();

    std::string result = builder.GetResult();
    // Verify all methods work together
    COUT(result.find("\"traditional_key\":\"traditional_value\"") != std::string::npos, true);
    COUT(result.find("\"operator_key\":\"operator_value\"") != std::string::npos, true);
    COUT(result.find("\"items\":[") != std::string::npos, true);
    COUT(result.find("\"item1\"") != std::string::npos, true);
    COUT(result.find("\"item2\"") != std::string::npos, true);
    COUT(result.find("\"item3\"") != std::string::npos, true);
    COUT(result.find("\"nested\":{") != std::string::npos, true);
    COUT(result.find("\"nested_operator\":\"nested_value\"") != std::string::npos, true);
    COUT(result.find("\"nested_traditional\":\"traditional_value\"") != std::string::npos, true);
}

DEF_TAST(operator_edge_cases, "test operator[] edge cases and special characters")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Empty string key
    builder[""] = "empty_key_value";
    builder[""] = 42;

    // Special characters in keys
    builder["key with spaces"] = "space value";
    builder["key\"with\"quotes"] = "quote value";
    builder["key\\with\\backslash"] = "backslash value";

    // Unicode content
    builder["emoji"] = "😀🎉";
    builder["chinese"] = "中文测试";
    builder["russian"] = "привет";

    builder.EndObject();

    std::string result = builder.GetResult();
    // Verify special characters are properly handled
    COUT(result.find("\"\":\"empty_key_value\"") != std::string::npos, true);
    COUT(result.find("\":42") != std::string::npos, true);
    COUT(result.find("\"key with spaces\":\"space value\"") != std::string::npos, true);
    COUT(result.find("\"key\"with\"quotes\":\"quote value\"") != std::string::npos, true);
    COUT(result.find("\"key\\with\\backslash\":\"backslash value\"") != std::string::npos, true);
    COUT(result.find("\"emoji\":\"😀🎉\"") != std::string::npos, true);
    COUT(result.find("\"chinese\":\"中文测试\"") != std::string::npos, true);
    COUT(result.find("\"russian\":\"привет\"") != std::string::npos, true);
}

DEF_TAST(operator_type_safety, "test operator[] with different data types")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Integer promotion
    int int_val = 42;
    short short_val = 10;
    long long_val = 1000;
    builder["int"] = int_val;
    builder["short"] = short_val;
    builder["long"] = long_val;

    // Floating point types
    float f_val = 3.14f;
    double d_val = 2.71828;
    builder["float"] = f_val;
    builder["double"] = d_val;

    // Boolean types
    bool true_val = true;
    bool false_val = false;
    builder["true_val"] = true_val;
    builder["false_val"] = false_val;

    // Character types
    char c_val = 'A';
    unsigned char uc_val = 'B';
    builder["char"] = c_val;
    builder["uchar"] = uc_val;

    builder.EndObject();

    std::string result = builder.GetResult();
    // Verify type handling
    COUT(result.find("\"int\":42") != std::string::npos, true);
    COUT(result.find("\"short\":10") != std::string::npos, true);
    COUT(result.find("\"long\":1000") != std::string::npos, true);
    COUT(result.find("\"float\":3.14") != std::string::npos, true);
    COUT(result.find("\"double\":2.71828") != std::string::npos, true);
    COUT(result.find("\"true_val\":true") != std::string::npos, true);
    COUT(result.find("\"false_val\":false") != std::string::npos, true);
    COUT(result.find("\"char\":65") != std::string::npos, true);
    COUT(result.find("\"uchar\":66") != std::string::npos, true);
}

DEF_TAST(operator_nested_structures, "test operator[] with nested objects and arrays")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Nested object using operator[]
    builder.BeginObject("person");
    builder["name"] = "John Doe";
    builder["age"] = 30;

    // Further nesting
    builder.BeginObject("address");
    builder["street"] = "123 Main St";
    builder["city"] = "Anytown";
    builder["zipcode"] = "12345";
    builder.EndObject();

    builder.EndObject();

    // Nested array using operator[]
    builder.BeginArray("matrix");
    builder.BeginArray();
    builder[0] = 1;
    builder[1] = 2;
    builder[2] = 3;
    builder.EndArray();

    builder.BeginArray();
    builder[0] = 4;
    builder[1] = 5;
    builder[2] = 6;
    builder.EndArray();
    builder.EndArray();

    builder.EndObject();

    std::string result = builder.GetResult();
    // Verify nested structure
    COUT(result.find("\"person\":{") != std::string::npos, true);
    COUT(result.find("\"name\":\"John Doe\"") != std::string::npos, true);
    COUT(result.find("\"age\":30") != std::string::npos, true);
    COUT(result.find("\"address\":{") != std::string::npos, true);
    COUT(result.find("\"street\":\"123 Main St\"") != std::string::npos, true);
    COUT(result.find("\"city\":\"Anytown\"") != std::string::npos, true);
    COUT(result.find("\"zipcode\":\"12345\"") != std::string::npos, true);
    COUT(result.find("\"matrix\":[") != std::string::npos, true);
    COUT(result.find("[1,2,3]") != std::string::npos, true);
    COUT(result.find("[4,5,6]") != std::string::npos, true);
}

DEF_TAST(operator_raii_compatibility, "test operator[] compatibility with RAII scope classes")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    {
        // Test with RawObject scope
        wwjson::RawObject obj = builder.ScopeObject("scoped");
        obj["inner_key"] = "inner_value";
        obj["inner_number"] = 42;

        // Nested array in scoped object
        wwjson::RawArray arr = obj.ScopeArray("inner_array");
        arr[0] = "array_item_1";
        arr[1] = "array_item_2";
        arr[-1] = "array_item_3";
    }

    // Test with RawArray scope
    {
        wwjson::RawArray arr = builder.ScopeArray("scoped_array");
        arr[0] = "arr_value_1";
        arr[1] = 123;
        arr[2] = true;

        // Nested object in scoped array
        wwjson::RawObject nested_obj = arr.ScopeObject();
        nested_obj["nested_key"] = "nested_value";
    }

    builder.EndObject();

    std::string result = builder.GetResult();
    // Verify RAII compatibility
    COUT(result.find("\"scoped\":{") != std::string::npos, true);
    COUT(result.find("\"inner_key\":\"inner_value\"") != std::string::npos, true);
    COUT(result.find("\"inner_number\":42") != std::string::npos, true);
    COUT(result.find("\"inner_array\":[") != std::string::npos, true);
    COUT(result.find("\"array_item_1\"") != std::string::npos, true);
    COUT(result.find("\"array_item_2\"") != std::string::npos, true);
    COUT(result.find("\"array_item_3\"") != std::string::npos, true);
    COUT(result.find("\"scoped_array\":[") != std::string::npos, true);
    COUT(result.find("\"arr_value_1\"") != std::string::npos, true);
    COUT(result.find("123") != std::string::npos, true);
    COUT(result.find("true") != std::string::npos, true);
    // Note: nested object in array should also be present
}

DEF_TAST(operator_performance, "test operator[] performance with many operations")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Test with many operations
    for (int i = 0; i < 100; ++i) {
        builder["key_" + std::to_string(i)] = "value_" + std::to_string(i);
    }

    builder.EndObject();

    std::string result = builder.GetResult();
    // Verify multiple operations
    COUT(result.find("\"key_0\":\"value_0\"") != std::string::npos, true);
    COUT(result.find("\"key_99\":\"value_99\"") != std::string::npos, true);
    
    // Should be valid JSON (no trailing comma)
    COUT(result.back(), '}');
    COUT(result.front(), '{');
}

DEF_TAST(operator_chaining, "test operator[] chaining behavior")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Simple chaining
    builder["key1"] = "value1";
    builder["key2"] = "value2";
    builder["key3"] = 42;

    // Test with different types
    builder["string_key"] = "string_value";
    builder["int_key"] = 123;
    builder["double_key"] = 3.14159;
    builder["bool_key"] = true;
    builder["null_key"] = nullptr;

    builder.EndObject();

    std::string result = builder.GetResult();
    // Verify chaining results
    COUT(result.find("\"key1\":\"value1\"") != std::string::npos, true);
    COUT(result.find("\"key2\":\"value2\"") != std::string::npos, true);
    COUT(result.find("\"key3\":42") != std::string::npos, true);
    COUT(result.find("\"string_key\":\"string_value\"") != std::string::npos, true);
    COUT(result.find("\"int_key\":123") != std::string::npos, true);
    COUT(result.find("\"double_key\":3.14159") != std::string::npos, true);
    COUT(result.find("\"bool_key\":true") != std::string::npos, true);
    COUT(result.find("\"null_key\":null") != std::string::npos, true);
}

DEF_TAST(operator_copy_move, "test operator[] with copy and move operations")
{
    // Test copy constructor
    {
        wwjson::RawBuilder builder1;
        builder1.BeginObject();
        builder1["copy_test"] = "copy_value";
        builder1.EndObject();

        wwjson::RawBuilder builder2 = builder1;  // Copy constructor
        std::string result1 = builder1.GetResult();
        std::string result2 = builder2.GetResult();
        COUT(result1, result2);
    }

    // Test move constructor
    {
        wwjson::RawBuilder builder1;
        builder1.BeginObject();
        builder1["move_test"] = "move_value";
        builder1.EndObject();

        wwjson::RawBuilder builder2 = std::move(builder1);  // Move constructor
        std::string result2 = builder2.GetResult();
        std::string expect = R"({"move_test":"move_value"})";
        COUT(result2, expect);
    }

    // Test copy assignment
    {
        wwjson::RawBuilder builder1, builder2;
        builder1.BeginObject();
        builder1["copy_assign"] = "copy_assign_value";
        builder1.EndObject();

        builder2 = builder1;  // Copy assignment
        std::string result1 = builder1.GetResult();
        std::string result2 = builder2.GetResult();
        COUT(result1, result2);
    }

    // Test move assignment
    {
        wwjson::RawBuilder builder1, builder2;
        builder1.BeginObject();
        builder1["move_assign"] = "move_assign_value";
        builder1.EndObject();

        builder2 = std::move(builder1);  // Move assignment
        std::string result2 = builder2.GetResult();
        std::string expect = R"({"move_assign":"move_assign_value"})";
        COUT(result2, expect);
    }
}

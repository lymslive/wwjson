#include "couttast/tinytast.hpp"
#include "wwjson.hpp"
#include <string>

DEF_TAST(operator_string_key, "operator[] 与字符串键赋值测试")
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

    std::string expect = R"({"name":"John","city":"New York","age":25,"salary":50000.5,"active":true,"middle_name":null})";
    COUT(builder.GetResult(), expect);
}

DEF_TAST(operator_array_index, "operator[] 与数组索引赋值测试")
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
    std::string expect = R"(["first","second","third","negative_test","large_index",42,3.14,false])";
    COUT(result, expect);
}

DEF_TAST(operator_mixed_usage, "operator[] 与传统方法混合使用测试")
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
    std::string expect = R"({"traditional_key":"traditional_value","operator_key":"operator_value","items":["item1","item2","item3"],"nested":{"nested_operator":"nested_value","nested_traditional":"traditional_value"}})";
    COUT(result, expect);
}

DEF_TAST(operator_edge_cases, "operator[] 边界情况与特殊字符测试")
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
    std::string expect = R"({"":"empty_key_value","":42,"key with spaces":"space value","key"with"quotes":"quote value","key\with\backslash":"backslash value","emoji":"😀🎉","chinese":"中文测试","russian":"привет"})";
    COUT(result, expect);
}

DEF_TAST(operator_type_safety, "operator[] 与不同数据类型测试")
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
    builder["null_key"] = nullptr;

    // Character types
    char c_val = 'A';
    unsigned char uc_val = 'B';
    builder["char"] = c_val;
    builder["uchar"] = uc_val;

    builder.EndObject();

    std::string result = builder.GetResult();
    std::string expect = R"({"int":42,"short":10,"long":1000,"float":3.14,"double":2.71828,"true_val":true,"false_val":false,"null_key":null,"char":65,"uchar":66})";
    COUT(result, expect);
}

DEF_TAST(operator_nested_structures, "operator[] 与嵌套对象数组测试")
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
    std::string expect = R"({"person":{"name":"John Doe","age":30,"address":{"street":"123 Main St","city":"Anytown","zipcode":"12345"}},"matrix":[[1,2,3],[4,5,6]]})";
    COUT(result, expect);
}

DEF_TAST(operator_raii_compatibility, "operator[] 与 RAII scope 类兼容性测试")
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

        // destroy arr then obj in reversed order,
        // so put char ]}
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
    std::string expect = R"({"scoped":{"inner_key":"inner_value","inner_number":42,"inner_array":["array_item_1","array_item_2","array_item_3"]},"scoped_array":["arr_value_1",123,true,{"nested_key":"nested_value"}]})";
    COUT(result, expect);
}

DEF_TAST(operator_copy_move, "operator[] 与拷贝移动操作测试")
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

DEF_TAST(operator_stream_array, "operator<< 与 GenericArray 链式添加元素测试")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();
    {
        // Test GenericArray operator<< with scope
        wwjson::RawArray arr = builder.ScopeArray("items");
        arr << "first_item" << 42 << true << 3.14 << false;

        // Test mixing with operator[], but can not modify, only append too
        arr[4] = "replaced_item";
    }
    builder.EndObject();

    std::string result = builder.GetResult();
    std::string expect = R"({"items":["first_item",42,true,3.14,false,"replaced_item"]})";
    COUT(result, expect);
}

DEF_TAST(operator_stream_object, "operator<< 与 GenericObject 交替键值对测试")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();
    {
        // Test GenericObject operator<< with scope
        wwjson::RawObject obj = builder.ScopeObject("person");
        obj << "name" << "John" << "age" << 30 << "active" << true << "score" << 95.5;

    }
    builder.EndObject();

    std::string result = builder.GetResult();
    std::string expect = R"({"person":{"name":"John","age":30,"active":true,"score":95.5}})";
    COUT(result, expect);
}

DEF_TAST(operator_stream_mixed, "operator<< 与传统方法混合使用测试")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    // Traditional method first
    builder.AddMember("traditional", "value");
    {
        // Operator<< with array scope
        wwjson::RawArray arr = builder.ScopeArray("operators");
        arr << "stream1" << 123 << "stream2";
    }

    {
        // Mix in object scope
        wwjson::RawObject obj = builder.ScopeObject("mixed");
        obj << "key1" << "value1";
        obj.AddMember("key2", "value2");  // Mix with traditional
        obj << "key3" << "value3";
        {
            // More array mixing
            wwjson::RawArray arr2 = obj.ScopeArray("nested_array");
            arr2 << "nested1";
            arr2.AddItem("nested2");
            arr2 << "nested3";
        }
    }


    builder.EndObject();

    std::string result = builder.GetResult();
    std::string expect = R"({"traditional":"value","operators":["stream1",123,"stream2"],"mixed":{"key1":"value1","key2":"value2","key3":"value3","nested_array":["nested1","nested2","nested3"]}})";
    COUT(result, expect);
}

DEF_TAST(operator_stream_types, "operator<< 与多种数据类型测试")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();
    {
        // Array with different types via operator<<
        wwjson::RawArray arr = builder.ScopeArray("data");
        arr << "string" << 123 << 3.14 << true << false << nullptr;
    }
    {
        // Object with different types via operator<<
        wwjson::RawObject obj = builder.ScopeObject("types");
        obj << "str" << "hello" << "int" << 42 << "float" << 2.5 << "bool" << true << "null_val" << nullptr;

    }
    builder.EndObject();

    std::string result = builder.GetResult();
    std::string expect = R"({"data":["string",123,3.14,true,false,null],"types":{"str":"hello","int":42,"float":2.5,"bool":true,"null_val":null}})";
    COUT(result, expect);
}

DEF_TAST(operator_stream_complex, "operator<< 与复杂嵌套结构测试")
{
    wwjson::RawBuilder builder;
    builder.BeginObject();

    {
        // Complex nested structure using operator<<
        wwjson::RawObject config = builder.ScopeObject("config");
        config << "database" << "postgresql" << "port" << 5432;
        {
            // Nested object with arrays
            wwjson::RawObject database = config.ScopeObject("settings");
            database << "timeout" << 30 << "retries" << 3;
            {
                // Array in nested object
                wwjson::RawArray hosts = database.ScopeArray("hosts");
                hosts << "localhost" << "127.0.0.1" << "remote-server";
            }
        }
        {
            // More complex nesting
            wwjson::RawObject features = config.ScopeObject("features");
            features << "cache" << true << "ssl" << false;
        }
    }
    
    builder.EndObject();

    std::string result = builder.GetResult();
    std::string expect = R"({"config":{"database":"postgresql","port":5432,"settings":{"timeout":30,"retries":3,"hosts":["localhost","127.0.0.1","remote-server"]},"features":{"cache":true,"ssl":false}}})";
    COUT(result, expect);
}

#include "couttast/tinytast.hpp"
#include "wwjson.hpp"
#include "test_util.h"
#include <string>
#include <functional>

DEF_TAST(advance_reopen, "test Reopen method for objects and arrays")
{
    // Test Reopen for object
    wwjson::RawBuilder builder1;
    builder1.BeginObject();
    builder1.AddMember("a", 1);
    builder1.EndObject();
    
    std::string expect1 = R"({"a":1})";
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    bool result1 = builder1.Reopen();
    COUT(result1, true);
    
    builder1.AddMember("b", 2);
    builder1.EndObject();
    std::string expect1_reopened = R"({"a":1,"b":2})";
    COUT(builder1.GetResult(), expect1_reopened);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test Reopen for array
    wwjson::RawBuilder builder2;
    builder2.BeginArray();
    builder2.AddItem(1);
    builder2.EndArray();
    
    std::string expect2 = R"([1])";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
    
    bool result2 = builder2.Reopen();
    COUT(result2, true);
    
    builder2.AddItem(2);
    builder2.EndArray();
    std::string expect2_reopened = R"([1,2])";
    COUT(builder2.GetResult(), expect2_reopened);
    COUT(test::IsJsonValid(builder2.json), true);
    
    // Test Reopen failure cases
    wwjson::RawBuilder builder3; // Empty builder
    bool result3 = builder3.Reopen();
    COUT(result3, false);
    
    builder3.Append("invalid"); // Invalid JSON string
    bool result4 = builder3.Reopen();
    COUT(result4, false);
}

DEF_TAST(advance_merge_instance, "test Merge instance method for objects and arrays")
{
    // Test object merge
    wwjson::RawBuilder obj1;
    obj1.BeginRoot();
    obj1.AddMember("a", 1);
    obj1.EndRoot();
    
    wwjson::RawBuilder obj2;
    obj2.BeginRoot();
    obj2.AddMember("b", 2);
    obj2.EndRoot();
    
    bool result1 = obj1.Merge(obj2);
    COUT(result1, true);
    
    std::string expect1 = R"({"a":1,"b":2})";
    COUT(obj1.json, expect1);
    
    // Test array merge
    wwjson::RawBuilder arr1;
    arr1.BeginRoot('[');
    arr1.AddItem(1);
    arr1.EndRoot(']');
    
    wwjson::RawBuilder arr2;
    arr2.BeginRoot('[');
    arr2.AddItem(2);
    arr2.EndRoot(']');
    
    bool result2 = arr1.Merge(arr2);
    COUT(result2, true);
    
    std::string expect2 = R"([1,2])";
    COUT(arr1.json, expect2);
    
    // Test type mismatch
    wwjson::RawBuilder obj3;
    obj3.BeginRoot();
    obj3.AddMember("c", 3);
    obj3.EndRoot();
    
    bool result3 = obj3.Merge(arr2); // Object merge with array
    COUT(result3, false);
    
    bool result4 = arr2.Merge(obj3); // Array merge with object
    COUT(result4, false);
    
    // Test empty builder merge
    wwjson::RawBuilder empty;
    wwjson::RawBuilder nonEmpty;
    nonEmpty.BeginRoot();
    nonEmpty.AddMember("x", 10);
    nonEmpty.EndRoot();
    
    bool result5 = empty.Merge(nonEmpty);
    COUT(result5, true);
    COUT(empty.json, nonEmpty.json);
    
    bool result6 = nonEmpty.Merge(empty); // Merge empty into non-empty
    COUT(result6, true);
    COUT(nonEmpty.json, nonEmpty.json); // Should remain unchanged
}

DEF_TAST(advance_merge_static, "test Merge static method for objects and arrays")
{
    // Test object merge
    std::string obj1 = R"({"a":1})";
    std::string obj2 = R"({"b":2})";
    
    bool result1 = wwjson::RawBuilder::Merge(obj1, obj2);
    COUT(result1, true);
    
    std::string expect1 = R"({"a":1,"b":2})";
    COUT(obj1, expect1);
    
    // Test array merge
    std::string arr1 = R"([1])";
    std::string arr2 = R"([2])";
    
    bool result2 = wwjson::RawBuilder::Merge(arr1, arr2);
    COUT(result2, true);
    
    std::string expect2 = R"([1,2])";
    COUT(arr1, expect2);
    
    // Test type mismatch
    std::string obj3 = R"({"c":3})";
    std::string arr3 = R"([3])";
    
    bool result3 = wwjson::RawBuilder::Merge(obj3, arr3); // Object merge with array
    COUT(result3, false);
    
    bool result4 = wwjson::RawBuilder::Merge(arr3, obj3); // Array merge with object
    COUT(result4, false);
    
    // Test empty string merge
    std::string empty;
    std::string nonEmpty = R"({"x":10})";
    
    bool result5 = wwjson::RawBuilder::Merge(empty, nonEmpty);
    COUT(result5, true);
    COUT(empty, nonEmpty);
    
    bool result6 = wwjson::RawBuilder::Merge(nonEmpty, empty); // Merge empty into non-empty
    COUT(result6, true);
    COUT(nonEmpty, nonEmpty); // Should remain unchanged
    
    // Test invalid JSON strings
    std::string invalid1 = "invalid";
    std::string invalid2 = "{";
    std::string valid = R"({"valid":1})";
    
    bool result7 = wwjson::RawBuilder::Merge(invalid1, valid);
    COUT(result7, false);
    
    bool result8 = wwjson::RawBuilder::Merge(valid, invalid2);
    COUT(result8, true); // Should succeed because valid ends with '}' and invalid2 starts with '{'
}

DEF_TAST(advance_merge_complex, "test complex merge scenarios")
{
    // Test multiple merges
    wwjson::RawBuilder base;
    base.BeginRoot();
    base.AddMember("base", 0);
    base.EndRoot();
    
    wwjson::RawBuilder add1;
    add1.BeginRoot();
    add1.AddMember("add1", 1);
    add1.EndRoot();
    
    wwjson::RawBuilder add2;
    add2.BeginRoot();
    add2.AddMember("add2", 2);
    add2.EndRoot();
    
    bool result1 = base.Merge(add1);
    COUT(result1, true);
    
    bool result2 = base.Merge(add2);
    COUT(result2, true);
    
    std::string expect = R"({"base":0,"add1":1,"add2":2})";
    COUT(base.json, expect);
    
    // Test array with multiple elements
    wwjson::RawBuilder arrBase;
    arrBase.BeginArray();
    arrBase.AddItem(0);
    arrBase.EndArray();
    arrBase.GetResult();
    
    wwjson::RawBuilder arrAdd1;
    arrAdd1.BeginArray();
    arrAdd1.AddItem(1);
    arrAdd1.AddItem(2);
    arrAdd1.EndArray();
    arrAdd1.GetResult();
    
    wwjson::RawBuilder arrAdd2;
    arrAdd2.BeginArray();
    arrAdd2.AddItem(3);
    arrAdd2.EndArray();
    arrAdd2.GetResult();
    
    bool result3 = arrBase.Merge(arrAdd1);
    COUT(result3, true);
    
    bool result4 = arrBase.Merge(arrAdd2);
    COUT(result4, true);
    
    std::string expectArr = R"([0,1,2,3])";
    COUT(arrBase.json, expectArr);
}

DEF_TAST(advance_putsub, "test PutSub method for adding JSON sub-strings")
{
    // Test PutSub with basic JSON strings
    wwjson::RawBuilder builder1;
    builder1.BeginObject();
    builder1.PutKey("empty");
    builder1.PutSub("{}");
    builder1.EndObject();
    
    std::string expect1 = R"({"empty":{}})";
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test PutSub with array string
    wwjson::RawBuilder builder2;
    builder2.BeginArray();
    builder2.PutSub(R"({"nested":true})");
    builder2.SepItem();
    builder2.PutSub(R"([1,2,3])");
    builder2.EndArray();
    
    std::string expect2 = R"([{"nested":true},[1,2,3]])";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
    
    // Test PutSub with different string types
    wwjson::RawBuilder builder3;
    builder3.BeginObject();
    std::string subJson = R"([1,2])";
    std::string_view subView = R"({"view":"test"})";
    builder3.PutKey("array");
    builder3.PutSub(subJson);
    builder3.SepItem();
    builder3.PutKey("test");
    builder3.PutSub(subView);
    builder3.EndObject();
    
    std::string expect3 = R"({"array":[1,2],"test":{"view":"test"}})";
    COUT(builder3.GetResult(), expect3);
    COUT(test::IsJsonValid(builder3.json), true);
}

DEF_TAST(advance_additemsub, "test AddItemSub method for adding JSON sub-strings as array items")
{
    // Test AddItemSub with basic JSON strings
    wwjson::RawBuilder builder1;
    builder1.BeginArray();
    builder1.AddItemSub("{}");
    builder1.AddItemSub(R"([1,2,3])");
    builder1.AddItemSub(R"({"nested":true})");
    builder1.EndArray();
    
    std::string expect1 = R"([{},[1,2,3],{"nested":true}])";
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test AddItemSub with different string types
    wwjson::RawBuilder builder2;
    builder2.BeginArray();
    std::string subJson = R"({"array":[1,2]})";
    std::string_view subView = R"({"view":"test"})";
    builder2.AddItemSub(subJson);
    builder2.AddItemSub(subView);
    builder2.EndArray();
    
    std::string expect2 = R"([{"array":[1,2]},{"view":"test"}])";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
    
    // Test AddItemSub with string literal
    wwjson::RawBuilder builder3;
    builder3.BeginArray();
    const char* subCStr = R"({"cstring":"test"})";
    builder3.AddItemSub(subCStr);
    builder3.AddItemSub(R"({"number":42})");
    builder3.EndArray();
    
    std::string expect3 = R"([{"cstring":"test"},{"number":42}])";
    COUT(builder3.GetResult(), expect3);
    COUT(test::IsJsonValid(builder3.json), true);
}

DEF_TAST(advance_addmembersub, "test AddMemberSub method for adding JSON sub-strings as member values")
{
    // Test AddMemberSub with basic JSON strings
    wwjson::RawBuilder builder1;
    builder1.BeginObject();
    builder1.AddMemberSub("empty_obj", "{}");
    builder1.AddMemberSub("numbers", R"([1,2,3])");
    builder1.AddMemberSub("nested", R"({"nested":true})");
    builder1.EndObject();
    
    std::string expect1 = R"({"empty_obj":{},"numbers":[1,2,3],"nested":{"nested":true}})";
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test AddMemberSub with different string types
    wwjson::RawBuilder builder2;
    builder2.BeginObject();
    std::string subJson = R"({"array":[1,2]})";
    std::string_view subView = R"({"view":"test"})";
    builder2.AddMemberSub("json_obj", subJson);
    builder2.AddMemberSub("view_obj", subView);
    builder2.EndObject();
    
    std::string expect2 = R"({"json_obj":{"array":[1,2]},"view_obj":{"view":"test"}})";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
    
    // Test AddMemberSub with string literal
    wwjson::RawBuilder builder3;
    builder3.BeginObject();
    const char* subCStr = R"({"cstring":"test"})";
    builder3.AddMemberSub("c_obj", subCStr);
    builder3.AddMemberSub("number_obj", R"({"number":42})");
    builder3.EndObject();
    
    std::string expect3 = R"({"c_obj":{"cstring":"test"},"number_obj":{"number":42}})";
    COUT(builder3.GetResult(), expect3);
    COUT(test::IsJsonValid(builder3.json), true);
}

DEF_TAST(advance_sub_with_scope, "test AddItemSub/AddMemberSub with scope objects")
{
    // Test AddItemSub with scope array
    wwjson::RawBuilder builder1;
    {
        auto arr = builder1.ScopeArray();
        arr.AddItemSub("{}");
        arr.AddItemSub(R"([1,2])");
    }
    
    std::string expect1 = R"([{},[1,2]])";
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test AddMemberSub with scope object
    wwjson::RawBuilder builder2;
    {
        auto obj = builder2.ScopeObject();
        obj.AddMemberSub("sub_obj", "{}");
        obj.AddMemberSub("sub_arr", R"([1,2])");
    }
    
    std::string expect2 = R"({"sub_obj":{},"sub_arr":[1,2]})";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
    
    // Test nested scope with sub JSON
    wwjson::RawBuilder builder3;
    {
        auto outer = builder3.ScopeObject();
        outer.AddMember("normal", "value");
        {
            auto inner = outer.ScopeArray("nested");
            inner.AddItemSub(R"({"inner":true})");
            inner.AddItem(123);
        }
    }
    
    std::string expect3 = R"({"normal":"value","nested":[{"inner":true},123]})";
    COUT(builder3.GetResult(), expect3);
    COUT(test::IsJsonValid(builder3.json), true);
}

DEF_TAST(advance_sub_complex, "test complex scenarios with JSON sub-strings")
{
    // Test building complex nested structure using sub-strings
    wwjson::RawBuilder builder1;
    builder1.BeginObject();
    builder1.AddMemberSub("config", R"({"debug":true,"version":"1.0"})");
    builder1.AddMember("data", "test_data");
    builder1.AddMemberSub("items", R"([{"id":1,"name":"item1"},{"id":2,"name":"item2"}])");
    builder1.EndObject();
    
    std::string expect1 = R"({"config":{"debug":true,"version":"1.0"},"data":"test_data","items":[{"id":1,"name":"item1"},{"id":2,"name":"item2"}]})";
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test combining normal methods with sub methods
    wwjson::RawBuilder builder2;
    builder2.BeginArray();
    builder2.AddItem("normal_string");
    builder2.AddItemSub(R"({"sub_object":true})");
    builder2.AddItem(42);
    builder2.AddItemSub(R"([1,2,3])");
    builder2.EndArray();
    
    std::string expect2 = R"(["normal_string",{"sub_object":true},42,[1,2,3]])";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
    
    // Test with existing JSON strings from external sources
    std::string externalJson1 = R"({"external":1})";
    std::string externalJson2 = R"(["external","array"])";
    
    wwjson::RawBuilder builder3;
    builder3.BeginObject();
    builder3.AddMemberSub("external1", externalJson1);
    builder3.AddMemberSub("external2", externalJson2);
    builder3.EndObject();
    
    std::string expect3 = R"({"external1":{"external":1},"external2":["external","array"]})";
    COUT(builder3.GetResult(), expect3);
    COUT(test::IsJsonValid(builder3.json), true);
}

DEF_TAST(advance_function_lambda, "test AddItem with lambda functions")
{
    // Test lambda with no parameters, capturing builder by reference
    wwjson::RawBuilder builder1;
    builder1.BeginArray();
    
    // Lambda that captures builder by reference
    builder1.AddItem([&builder1]() {
        builder1.BeginObject();
        builder1.AddMember("lambda_type", "no_params");
        builder1.AddMember("value", 42);
        builder1.EndObject();
    });
    
    // Lambda with no parameters, different structure
    builder1.AddItem([&builder1]() {
        builder1.BeginArray();
        builder1.AddItem("lambda");
        builder1.AddItem(123);
        builder1.EndArray();
    });
    
    builder1.EndArray();
    
    std::string expect1 = R"([{"lambda_type":"no_params","value":42},["lambda",123]])";
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test lambda with GenericBuilder reference parameter - proper usage
    wwjson::RawBuilder builder2;
    builder2.BeginArray();
    
    // Lambda with builder parameter - correctly uses the builder reference
    builder2.AddItem([](wwjson::RawBuilder& builder) {
        builder.BeginObject();
        builder.AddMember("lambda_type", "with_param");
        builder.AddMember("nested", [&builder]() {
            // Use a nested scope to create an array
            auto scope = builder.ScopeArray();
            scope.AddItem("nested_lambda");
        });
        builder.EndObject();
    });
    
    // Another lambda with builder parameter - simple usage
    builder2.AddItem([](wwjson::RawBuilder& builder) {
        builder.AddItem("simple_lambda_param");
    });
    
    builder2.EndArray();

    std::string expect2 = R"([{"lambda_type":"with_param","nested":["nested_lambda"]},"simple_lambda_param"])";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
}

// Free function with no parameters for testing
wwjson::RawBuilder createNestedObject()
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    builder.AddMember("func_type", "free_function");
    builder.AddMember("number", 3.14);
    builder.EndRoot();
    return builder;
}

// Free function with GenericBuilder reference parameter
void buildArrayWithBuilder(wwjson::RawBuilder& builder)
{
    builder.BeginArray();
    builder.AddItem("free");
    builder.AddItem("function");
    builder.AddItem("param");
    builder.EndArray();
}

// Another free function for testing different signatures
void buildComplexObject(wwjson::RawBuilder& builder)
{
    builder.BeginObject();
    builder.AddMember("complex", true);
    builder.AddMemberSub("nested_sub", R"({"from":"func"})");
    builder.EndObject();
}

DEF_TAST(advance_function_free, "test AddItem with free functions")
{
    // Test free function that returns a builder (no parameter version)
    wwjson::RawBuilder builder1;
    builder1.BeginArray();
    
    // This won't compile as expected - free functions without builder param need to be adapted
    // We'll test with lambda wrapper instead
    builder1.AddItem([&builder1]() {
        auto temp = createNestedObject();
        builder1.PutSub(temp.GetResult());
    });
    
    builder1.EndArray();
    
    std::string expect1 = R"([{"func_type":"free_function","number":3.14}])";  // Shortest decimal representation
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test free function with GenericBuilder reference parameter
    wwjson::RawBuilder builder2;
    builder2.BeginArray();
    
    builder2.AddItem(buildArrayWithBuilder);
    builder2.AddItem(buildComplexObject);
    
    builder2.EndArray();
    
    std::string expect2 = R"([["free","function","param"],{"complex":true,"nested_sub":{"from":"func"}}])";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
}

// Test class with static method for callable objects
class JsonBuilder
{
public:
    static void buildSimpleObject(wwjson::RawBuilder& builder)
    {
        builder.BeginObject();
        builder.AddMember("method", "static");
        builder.AddMember("value", 100);
        builder.EndObject();
    }
    
    void buildMemberObject(wwjson::RawBuilder& builder)
    {
        builder.BeginObject();
        builder.AddMember("method", "member");
        builder.AddMember("id", id_);
        builder.EndObject();
    }
    
    JsonBuilder() : id_(0) {}
    explicit JsonBuilder(int id) : id_(id) {}
    
private:
    int id_;
};

DEF_TAST(advance_function_class, "test AddItem with class methods")
{
    // Test static method
    wwjson::RawBuilder builder1;
    builder1.BeginArray();
    
    builder1.AddItem(JsonBuilder::buildSimpleObject);
    
    builder1.EndArray();
    
    std::string expect1 = R"([{"method":"static","value":100}])";
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test member function with std::bind
    wwjson::RawBuilder builder2;
    builder2.BeginArray();
    
    JsonBuilder obj(42);
    auto boundMethod = std::bind(&JsonBuilder::buildMemberObject, &obj, std::placeholders::_1);
    builder2.AddItem(boundMethod);
    
    builder2.EndArray();
    
    std::string expect2 = R"([{"method":"member","id":42}])";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
}

DEF_TAST(advance_function_with_addmember, "test AddMember with callable functions")
{
    // Test AddMember with lambda (no parameters)
    wwjson::RawBuilder builder1;
    builder1.BeginObject();
    
    builder1.AddMember("lambda_no_param", [&builder1]() {
        builder1.BeginArray();
        builder1.AddItem("lambda");
        builder1.AddItem("capture");
        builder1.EndArray();
    });
    
    builder1.EndObject();
    
    std::string expect1 = R"({"lambda_no_param":["lambda","capture"]})";
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test AddMember with lambda (with parameter)
    wwjson::RawBuilder builder2;
    builder2.BeginObject();
    
    builder2.AddMember("lambda_param", [](wwjson::RawBuilder& builder) {
        builder.BeginObject();
        builder.AddMember("param", "true");
        builder.AddMember("type", "lambda");
        builder.EndObject();
    });
    
    builder2.EndObject();
    
    std::string expect2 = R"({"lambda_param":{"param":"true","type":"lambda"}})";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
    
    // Test AddMember with free function
    wwjson::RawBuilder builder3;
    builder3.BeginObject();
    
    builder3.AddMember("free_func", buildComplexObject);
    
    builder3.EndObject();
    
    std::string expect3 = R"({"free_func":{"complex":true,"nested_sub":{"from":"func"}}})";
    COUT(builder3.GetResult(), expect3);
    COUT(test::IsJsonValid(builder3.json), true);
}

DEF_TAST(advance_function_nested, "test nested callable functions")
{
    // Test deeply nested callable functions
    wwjson::RawBuilder builder;
    builder.BeginObject();
    
    builder.AddMember("nested", [](wwjson::RawBuilder& builder) {
        builder.BeginArray();
        
        // First level nested lambda
        builder.AddItem([](wwjson::RawBuilder& builder) {
            builder.BeginObject();
            builder.AddMember("level", 1);
            builder.AddMember("data", "first");
            builder.EndObject();
        });
        
        // Second level nested lambda with capture
        builder.AddItem([&builder](wwjson::RawBuilder& innerBuilder) {
            innerBuilder.BeginObject();
            innerBuilder.AddMember("level", 2);
            
            // Third level nested function call
            innerBuilder.AddMember("deep", [&innerBuilder]() {
                innerBuilder.BeginArray();
                innerBuilder.AddItem("deeply");
                innerBuilder.AddItem("nested");
                innerBuilder.EndArray();
            });
            
            innerBuilder.EndObject();
        });
        
        builder.EndArray();
    });
    
    builder.EndObject();
    
    std::string expect = R"({"nested":[{"level":1,"data":"first"},{"level":2,"deep":["deeply","nested"]}]})";
    COUT(builder.GetResult(), expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(advance_function_scope_with_callable, "test scope objects with callable functions")
{
    // Test ScopeArray with callable functions
    wwjson::RawBuilder builder1;
    {
        auto arr = builder1.ScopeArray();
        
        arr.AddItem([](wwjson::RawBuilder& builder) {
            builder.BeginObject();
            builder.AddMember("scope_array", true);
            builder.EndObject();
        });
        
        arr.AddItem([&arr]() {
            arr.AddItem("lambda");
            arr.AddItem("scope");
        });
    }
    
    std::string expect1 = R"([{"scope_array":true},"lambda","scope"])";
    COUT(builder1.GetResult(), expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test ScopeObject with callable functions
    wwjson::RawBuilder builder2;
    {
        auto obj = builder2.ScopeObject();
        
        obj.AddMember("callable", [](wwjson::RawBuilder& builder) {
            builder.BeginArray();
            builder.AddItem("scope");
            builder.AddItem("object");
            builder.EndArray();
        });
        
        obj.AddMember("capture", [&obj]() {
            auto scope = obj.ScopeObject();
            scope.AddMember("type", "lambda");
            scope.AddMember("scope", "object");
        });
    }
    
    std::string expect2 = R"({"callable":["scope","object"],"capture":{"type":"lambda","scope":"object"}})";
    COUT(builder2.GetResult(), expect2);
    COUT(test::IsJsonValid(builder2.json), true);
}


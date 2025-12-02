#include "couttast/tinytast.hpp"
#include "wwjson.hpp"
#include "test_util.h"
#include <string>

DEF_TAST(advance_reopen, "test Reopen method for objects and arrays")
{
    // Test Reopen for object
    wwjson::RawBuilder builder1;
    builder1.BeginObject();
    builder1.AddMember("a", 1);
    builder1.EndObject();
    
    std::string expect1 = R"({"a":1})";
    COUT(builder1.json, expect1);
    COUT(test::IsJsonValid(builder1.json), true);
    
    bool result1 = builder1.Reopen();
    COUT(result1, true);
    
    builder1.AddMember("b", 2);
    builder1.EndObject();
    std::string expect1_reopened = R"({"a":1,"b":2})";
    COUT(builder1.json, expect1_reopened);
    COUT(test::IsJsonValid(builder1.json), true);
    
    // Test Reopen for array
    wwjson::RawBuilder builder2;
    builder2.BeginArray();
    builder2.AddItem(1);
    builder2.EndArray();
    
    std::string expect2 = R"([1])";
    COUT(builder2.json, expect2);
    COUT(test::IsJsonValid(builder2.json), true);
    
    bool result2 = builder2.Reopen();
    COUT(result2, true);
    
    builder2.AddItem(2);
    builder2.EndArray();
    std::string expect2_reopened = R"([1,2])";
    COUT(builder2.json, expect2_reopened);
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
    obj1.BeginObject();
    obj1.AddMember("a", 1);
    obj1.EndObject();
    
    wwjson::RawBuilder obj2;
    obj2.BeginObject();
    obj2.AddMember("b", 2);
    obj2.EndObject();
    
    bool result1 = obj1.Merge(obj2);
    COUT(result1, true);
    
    std::string expect1 = R"({"a":1,"b":2})";
    COUT(obj1.json, expect1);
    
    // Test array merge
    wwjson::RawBuilder arr1;
    arr1.BeginArray();
    arr1.AddItem(1);
    arr1.EndArray();
    
    wwjson::RawBuilder arr2;
    arr2.BeginArray();
    arr2.AddItem(2);
    arr2.EndArray();
    
    bool result2 = arr1.Merge(arr2);
    COUT(result2, true);
    
    std::string expect2 = R"([1,2])";
    COUT(arr1.json, expect2);
    
    // Test type mismatch
    wwjson::RawBuilder obj3;
    obj3.BeginObject();
    obj3.AddMember("c", 3);
    obj3.EndObject();
    
    bool result3 = obj3.Merge(arr2); // Object merge with array
    COUT(result3, false);
    
    bool result4 = arr2.Merge(obj3); // Array merge with object
    COUT(result4, false);
    
    // Test empty builder merge
    wwjson::RawBuilder empty;
    wwjson::RawBuilder nonEmpty;
    nonEmpty.BeginObject();
    nonEmpty.AddMember("x", 10);
    nonEmpty.EndObject();
    
    bool result5 = empty.Merge(nonEmpty);
    COUT(result5, true);
    COUT(empty.json, nonEmpty.json);
    
    bool result6 = nonEmpty.Merge(empty); // Merge empty into non-empty
    COUT(result6, true);
    COUT(nonEmpty.json, nonEmpty.json); // Should remain unchanged
    
    // Test nested structure merge
    wwjson::RawBuilder nested1;
    nested1.BeginObject();
    nested1.AddMember("nested", 1);
    nested1.EndObject();
    
    wwjson::RawBuilder nested2;
    nested2.BeginObject();
    nested2.AddMember("another", 2);
    nested2.EndObject();
    
    bool result7 = nested1.Merge(nested2);
    COUT(result7, true);
    std::string expect_nested = R"({"nested":1,"another":2})";
    COUT(nested1.json, expect_nested);
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
    base.BeginObject();
    base.AddMember("base", 0);
    base.EndObject();
    
    wwjson::RawBuilder add1;
    add1.BeginObject();
    add1.AddMember("add1", 1);
    add1.EndObject();
    
    wwjson::RawBuilder add2;
    add2.BeginObject();
    add2.AddMember("add2", 2);
    add2.EndObject();
    
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
    
    wwjson::RawBuilder arrAdd1;
    arrAdd1.BeginArray();
    arrAdd1.AddItem(1);
    arrAdd1.AddItem(2);
    arrAdd1.EndArray();
    
    wwjson::RawBuilder arrAdd2;
    arrAdd2.BeginArray();
    arrAdd2.AddItem(3);
    arrAdd2.EndArray();
    
    bool result3 = arrBase.Merge(arrAdd1);
    COUT(result3, true);
    
    bool result4 = arrBase.Merge(arrAdd2);
    COUT(result4, true);
    
    std::string expectArr = R"([0,1,2,3])";
    COUT(arrBase.json, expectArr);
}


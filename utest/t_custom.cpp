/**
 * @file t_custom.cpp
 * @brief Test custom string compatibility with wwjson library
 */

#include "custom_string.h"
#include "wwjson.hpp"
#include "couttast/tinytast.hpp"
#include "test_util.h"

DEF_TAST(custom_builder, "test json builder with custom string")
{
    wwjson::GenericBuilder<test::string> builder;
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
    COUT(builder.json.c_str(), expect);
    COUT(test::IsJsonValid(builder.json.c_str()), true);
}

DEF_TAST(custom_wrapper, "test M1 string interface wrapper methods with custom string")
{
    wwjson::GenericBuilder<test::string> builder(64);
    
    // Test PutChar
    builder.PutChar('{');
    builder.PutChar('}');
    std::string expect1 = "{}";
    COUT(builder.json.c_str(), expect1);
    COUT(test::IsJsonValid(builder.json.c_str()), true);
    
    // Test FixTail - replace trailing '}' with ','
    builder.FixTail('}', ',');
    std::string expect2 = "{,";
    COUT(builder.json.c_str(), expect2);
    
    // Test FixTail - add ']' when tail doesn't match
    builder.FixTail('{', ']');
    std::string expect3 = "{,]";
    COUT(builder.json.c_str(), expect3);
    
    // Test Append methods
    builder.Clear();
    builder.Append("test");
    builder.Append(" string", 7);
    std::string expect4 = "test string";
    COUT(builder.json.c_str(), expect4);
    
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
    COUT(builder.json.c_str(), expect5);
}

DEF_TAST(custom_scope, "test build nest json with custom string using auto close scope methods")
{
    wwjson::GenericBuilder<test::string> builder;
    {
        auto root = builder.ScopeObject();

        std::string title = "Title";
        root.AddMember("title", title);

        // need scope for head, to auto close {}
        {
            auto head = builder.ScopeObject("head", true);
            head.AddMember("int", 123);
            head.AddMember("string", "123");
        }

        {
            auto bodys = builder.ScopeArray("bodys");
            {
                auto body = builder.ScopeObject(true);
                body.AddMember("char", '1');
                unsigned char c = '2';
                body.AddMember("uchar", c);
            }
            bodys.AddItem("simple");
            {
                auto body = builder.ScopeObject(true);
                short sh = 280;
                body.AddMember("short", sh);
                double half = 0.5;
                body.AddMember("double", half);
                double third = 1.0/3.0;
                body.AddMember("double", third);
            }
        }
    } // auto add right brace when destruct root beyond scope

    std::string expect = R"({"title":"Title","head":{"int":123,"string":"123"},"bodys":[{"char":49,"uchar":50},"simple",{"short":280,"double":0.500000,"double":0.333333}]})";
    COUT(builder.json.c_str(), expect);
}

// Custom config that quotes numbers
struct QuoteNumberConfig : wwjson::BasicConfig<test::string> {
    static constexpr bool kQuoteNumber = true;
};

DEF_TAST(custom_number_quoted, "test number quoting behavior with AddItem/AddMember methods")
{
    // Test default config (no quoting)
    wwjson::GenericBuilder<test::string> builder1;
    builder1.BeginArray();
    builder1.AddItem(123);
    builder1.AddItem(45.67);
    builder1.EndArray();
    std::string expect1 = "[123,45.670000]";
    COUT(builder1.json.c_str(), expect1);
    
    builder1.Clear();
    builder1.BeginObject();
    builder1.AddMember("int", 456);
    builder1.AddMember("float", 7.89);
    builder1.EndObject();
    std::string expect2 = R"({"int":456,"float":7.890000})";
    COUT(builder1.json.c_str(), expect2);
    
    // Test manual quoting with second parameter true
    wwjson::GenericBuilder<test::string> builder2;
    builder2.BeginArray();
    builder2.AddItem(123, true);
    builder2.AddItem(45.67, true);
    builder2.EndArray();
    std::string expect3 = "[\"123\",\"45.670000\"]";
    COUT(builder2.json.c_str(), expect3);
    
    builder2.Clear();
    builder2.BeginObject();
    builder2.AddMember("int", 456, true);
    builder2.AddMember("float", 7.89, true);
    builder2.EndObject();
    std::string expect4 = R"({"int":"456","float":"7.890000"})";
    COUT(builder2.json.c_str(), expect4);
    
    // Test custom config with kQuoteNumber = true
    wwjson::GenericBuilder<test::string, QuoteNumberConfig> builder3;
    builder3.BeginArray();
    builder3.AddItem(999);
    builder3.AddItem(3.14);
    builder3.EndArray();
    std::string expect5 = "[\"999\",\"3.140000\"]";
    COUT(builder3.json.c_str(), expect5);
    
    builder3.Clear();
    builder3.BeginObject();
    builder3.AddMember("int", 456);
    builder3.AddMember("float", 7.89);
    builder3.EndObject();
    std::string expect6 = R"({"int":"456","float":"7.890000"})";
    COUT(builder3.json.c_str(), expect6);
}
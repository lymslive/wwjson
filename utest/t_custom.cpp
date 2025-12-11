/**
 * @file t_custom.cpp
 * @brief Test custom string compatibility with wwjson library
 */

#include "custom_string.h"
#include "wwjson.hpp"
#include "couttast/tinytast.hpp"
#include "test_util.h"

DEF_TAST(custom_builder, "自定义字符串的 JSON 构建器测试")
{
    wwjson::GenericBuilder<test::string> builder;
    builder.BeginRoot();

    builder.AddMember("int", 123);
    builder.AddMember("string", "123");
    builder.AddMember("char", '1');

    unsigned char c = '2';
    builder.AddMember("uchar", c);

    short sh = 280;
    builder.AddMember("short", sh);

    double half = 0.5;
    builder.AddMember("double", half);

    double quarter = 1.0/4.0;
    builder.AddMember("double", quarter);

    // add number as string with extra argument, no matter true/false
    builder.AddMember("ints", 124, true);
    builder.AddMember("intf", 125, false);

    builder.EndRoot();

    std::string expect = R"({"int":123,"string":"123","char":49,"uchar":50,"short":280,"double":0.5,"double":0.25,"ints":"124","intf":"125"})";
    COUT(builder.json.c_str(), expect);
    COUT(test::IsJsonValid(builder.json.c_str()), true);
}

DEF_TAST(custom_wrapper, "自定义字符串的 M1 接口封装方法测试")
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

DEF_TAST(custom_scope, "使用自定义字符串和自动关闭 scope 方法构建嵌套 JSON")
{
    wwjson::GenericBuilder<test::string> builder;
    {
        auto root = builder.ScopeObject();

        std::string title = "Title";
        root.AddMember("title", title);

        // need scope for head, to auto close {}
        {
            auto head = builder.ScopeObject("head");
            head.AddMember("int", 123);
            head.AddMember("string", "123");
        }

        {
            auto bodys = builder.ScopeArray("bodys");
            {
                auto body = builder.ScopeObject();
                body.AddMember("char", '1');
                unsigned char c = '2';
                body.AddMember("uchar", c);
            }
            bodys.AddItem("simple");
            {
                auto body = builder.ScopeObject();
                short sh = 280;
                body.AddMember("short", sh);
                double half = 0.5;
                body.AddMember("double", half);
                double quarter = 1.0/4.0;
                body.AddMember("double", quarter);
            }
        }
    } // auto add right brace when destruct root beyond scope

    builder.GetResult();
    std::string expect = R"({"title":"Title","head":{"int":123,"string":"123"},"bodys":[{"char":49,"uchar":50},"simple",{"short":280,"double":0.5,"double":0.25}]})";
    COUT(builder.json.c_str(), expect);
}

// Custom config that quotes numbers
struct QuoteNumberConfig : wwjson::BasicConfig<test::string> {
    static constexpr bool kQuoteNumber = true;
};

DEF_TAST(custom_number_quoted, "AddItem/AddMember 方法的数字引号行为测试")
{
    // Test default config (no quoting)
    wwjson::GenericBuilder<test::string> builder1;
    builder1.BeginArray();
    builder1.AddItem(123);
    builder1.AddItem(45.67);
    builder1.EndArray();
    std::string expect1 = "[123,45.67]";
    COUT(builder1.GetResult().c_str(), expect1);
    
    builder1.Clear();
    builder1.BeginObject();
    builder1.AddMember("int", 456);
    builder1.AddMember("float", 7.89);
    builder1.EndObject();
    std::string expect2 = R"({"int":456,"float":7.89})";
    COUT(builder1.GetResult().c_str(), expect2);
    
    // Test manual quoting with second parameter true
    wwjson::GenericBuilder<test::string> builder2;
    builder2.BeginArray();
    builder2.AddItem(123, true);
    builder2.AddItem(45.67, true);
    builder2.EndArray();
    std::string expect3 = "[\"123\",\"45.67\"]";
    COUT(builder2.GetResult().c_str(), expect3);
    
    builder2.Clear();
    builder2.BeginObject();
    builder2.AddMember("int", 456, true);
    builder2.AddMember("float", 7.89, true);
    builder2.EndObject();
    std::string expect4 = R"({"int":"456","float":"7.89"})";
    COUT(builder2.GetResult().c_str(), expect4);
    
    // Test custom config with kQuoteNumber = true
    wwjson::GenericBuilder<test::string, QuoteNumberConfig> builder3;
    builder3.BeginArray();
    builder3.AddItem(999);
    builder3.AddItem(3.14);
    builder3.EndArray();
    std::string expect5 = "[\"999\",\"3.14\"]";
    COUT(builder3.GetResult().c_str(), expect5);
    
    builder3.Clear();
    builder3.BeginObject();
    builder3.AddMember("int", 456);
    builder3.AddMember("float", 7.89);
    builder3.EndObject();
    std::string expect6 = R"({"int":"456","float":"7.89"})";
    COUT(builder3.GetResult().c_str(), expect6);
}

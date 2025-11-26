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

DEF_TAST(basic_builder_array, "test build json with array of object")
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

DEF_TAST(basic_escape, "test add string with escape")
{
    wwjson::RawBuilder child;
    child.BeginObject();
    child.AddMember("int", 123);
    child.AddMember("string", "123");
    child.AddMember("char", '1');
    child.EndObject();

    std::string childExpect = R"({"int":123,"string":"123","char":49})";
    COUT(child.json, childExpect);

    wwjson::RawBuilder root;
    root.BeginObject();
    root.AddMember("version", 1);
    root.AddMemberEscape("child", child.json);
    root.EndObject();

    std::string rootExpect = R"({"version":1,"child":"{\"int\":123,\"string\":\"123\",\"char\":49}"})";
    COUT(root.json, rootExpect);

    {
        wwjson::RawBuilder child;
        child.BeginObject();
        child.AddMember("int", 123);
        child.AddMember("string", "\\1\t2\t3\\");
        child.AddMember("char", '1');
        child.EndObject();

        std::string childExpect = R"({"int":123,"string":"\1	2	3\","char":49})";
        COUT(child.json, childExpect);

        wwjson::RawBuilder root;
        root.BeginObject();
        root.AddMember("version", 1);
        root.AddMemberEscape("child", child.json, "\"\\\t");
        root.EndObject();

        std::string rootExpect = R"({"version":1,"child":"{\"int\":123,\"string\":\"\\1\t2\t3\\\",\"char\":49}"})";
        COUT(root.json, rootExpect);
    }

    {
        std::string src = "\\1\t2\t3\\";
        std::string dst;
        wwjson::RawBuilder::EscapeString(src, dst, "\"\\\t");
        COUT(src);
        COUT(dst);
    }
}

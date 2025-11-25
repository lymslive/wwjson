#include "couttast/tinytast.hpp"
#include "wwjson.hpp"
#include <string>
// #include <charconv>

DEF_TAST(basic_builder, "test json builder with raw string")
{
    wwjson::RawJsonBuilder builder;
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

    // 数字当作字符串类型添加，额外参数区分重载，不管 true/false
    builder.AddMember("ints", 124, true);
    builder.AddMember("intf", 125, false);

    builder.EndObject();

    std::string expect = R"({"int":123,"string":"123","char":49,"uchar":50,"short":280,"double":0.500000,"double":0.333333,"ints":"124","intf":"125"})";
    COUT(builder.json, expect);
}

DEF_TAST(basic_builder_array, "test build json with array of object")
{
    wwjson::RawJsonBuilder builder;
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

DEF_TAST(basic_builder_nest, "test build nest json with auto close")
{
    wwjson::RawJsonBuilder builder;
    {
        wwjson::RawJsonObject root(builder);

        std::string title = "Title";
        root.AddMember("title", title);

        // need scope for head, to auto close {}
        {
            wwjson::RawJsonObject head(builder, "head", true);
            head.AddMember("int", 123);
            head.AddMember("string", "123");
        }

        {
            wwjson::RawJsonArray bodys(builder, "bodys");
            {
                wwjson::RawJsonObject body(builder, true);
                body.AddMember("char", '1');
                unsigned char c = '2';
                body.AddMember("uchar", c);
            }
            bodys.AddItem("simple");
            {
                wwjson::RawJsonObject body(builder, true);
                short sh = 280;
                body.AddMember("short", sh);
                double half = 0.5;
                body.AddMember("double", half);
                double third = 1.0/3.0;
                body.AddMember("double", third);
            }
        }
    } // root 析构添加右大括号

    std::string expect = R"({"title":"Title","head":{"int":123,"string":"123"},"bodys":[{"char":49,"uchar":50},"simple",{"short":280,"double":0.500000,"double":0.333333}]})";
    COUT(builder.json, expect);
}

#if 0
DEF_TOOL(to_chars, "test std::to_chars")
{
    std::string json;
    std::array<char, 32> buffer;
    auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), 100);
    json.append(buffer.data(), ptr);
    COUT(json);

    // 传参 double 编不过
    {
        std::string json;
        std::array<char, 32> buffer;
        auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(),500);
        json.append(buffer.data(), ptr);
        COUT(json);
    }

    {
        uint64_t maxInt = -1;
        std::string strMaxInt = std::to_string(maxInt);
        COUT(strMaxInt);
        COUT(strMaxInt.size());
    }
}
#endif

DEF_TAST(basic_escape, "test add string with escape")
{
    wwjson::RawJsonBuilder child;
    child.BeginObject();
    child.AddMember("int", 123);
    child.AddMember("string", "123");
    child.AddMember("char", '1');
    child.EndObject();

    std::string childExpect = R"({"int":123,"string":"123","char":49})";
    COUT(child.json, childExpect);

    wwjson::RawJsonBuilder root;
    root.BeginObject();
    root.AddMember("version", 1);
    root.AddMemberEscape("child", child.json);
    root.EndObject();

    std::string rootExpect = R"({"version":1,"child":"{\"int\":123,\"string\":\"123\",\"char\":49}"})";
    COUT(root.json, rootExpect);

    {
        wwjson::RawJsonBuilder child;
        child.BeginObject();
        child.AddMember("int", 123);
        child.AddMember("string", "\\1\t2\t3\\");
        child.AddMember("char", '1');
        child.EndObject();

        std::string childExpect = R"({"int":123,"string":"\1	2	3\","char":49})";
        COUT(child.json, childExpect);

        wwjson::RawJsonBuilder root;
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
        wwjson::RawJsonBuilder::EscapeString(src, dst, "\"\\\t");
        COUT(src);
        COUT(dst);
    }
}

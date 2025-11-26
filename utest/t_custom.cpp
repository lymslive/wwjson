/**
 * @file t_custom.cpp
 * @brief Test custom string compatibility with wwjson library
 */

#include "custom_string.h"
#include "wwjson.hpp"
#include "couttast/tinytast.hpp"

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
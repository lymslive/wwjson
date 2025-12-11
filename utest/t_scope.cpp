#include "couttast/tinytast.hpp"
#include "test_util.h"
#include "wwjson.hpp"
#include <string>

DEF_TAST(scope_ctor_nest, "RAII 自动关闭的嵌套 JSON 构建")
{
    wwjson::RawBuilder builder;
    {
        wwjson::RawObject root(builder);

        std::string title = "Title";
        root.AddMember("title", title);

        // need scope for head, to auto close {}
        {
            wwjson::RawObject head(builder, "head");
            head.AddMember("int", 123);
            head.AddMember("string", "123");
        }

        {
            wwjson::RawArray bodys(builder, "bodys");
            {
                wwjson::RawObject body(builder);
                body.AddMember("char", '1');
                unsigned char c = '2';
                body.AddMember("uchar", c);
            }
            bodys.AddItem("simple");
            {
                wwjson::RawObject body(builder);
                short sh = 280;
                body.AddMember("short", sh);
                double half = 0.5;
                body.AddMember("double", half);
                double quarter = 1.0 / 4.0;
                body.AddMember("double", quarter);
            }
        }
    } // auto add right brace when destruct root beyond scope

    std::string expect =
        R"({"title":"Title","head":{"int":123,"string":"123"},"bodys":[{"char":49,"uchar":50},"simple",{"short":280,"double":0.5,"double":0.25}]})";
    COUT(builder.GetResult(), expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(scope_auto_nest, "使用 scope 方法自动关闭的嵌套 JSON 构建")
{
    wwjson::RawBuilder builder;
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
                double quarter = 1.0 / 4.0;
                body.AddMember("double", quarter);
            }
        }
    } // auto add right brace when destruct root beyond scope

    std::string expect =
        R"({"title":"Title","head":{"int":123,"string":"123"},"bodys":[{"char":49,"uchar":50},"simple",{"short":280,"double":0.5,"double":0.25}]})";
    COUT(builder.GetResult(), expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(scope_vs_constructor, "scope 方法与构造方法对比")
{
    // Test using scope methods (new approach)
    wwjson::RawBuilder scopeBuilder;
    {
        auto root = scopeBuilder.ScopeObject();
        root.AddMember("name", "test");
        {
            auto items = scopeBuilder.ScopeArray("items");
            items.AddItem(1);
            items.AddItem(2);
            {
                auto nested = scopeBuilder.ScopeObject();
                nested.AddMember("key", "value");
            }
        }
    }
    scopeBuilder.GetResult();

    // Test using constructors (old approach)
    wwjson::RawBuilder ctorBuilder;
    {
        wwjson::RawObject root(ctorBuilder);
        root.AddMember("name", "test");
        {
            wwjson::RawArray items(ctorBuilder, "items");
            items.AddItem(1);
            items.AddItem(2);
            {
                wwjson::RawObject nested(ctorBuilder);
                nested.AddMember("key", "value");
            }
        }
    }
    ctorBuilder.GetResult();

    std::string expect = R"({"name":"test","items":[1,2,{"key":"value"}]})";
    COUT(scopeBuilder.json, expect);
    COUT(ctorBuilder.json, expect);
    COUT(scopeBuilder.json == ctorBuilder.json, true);
}

DEF_TAST(scope_if_bool_operator, "scope 变量的 if 语句中 operator bool 测试")
{
    wwjson::RawBuilder builder;

    // Test basic if statement with GenericBuilder
    if (auto root = builder.ScopeObject())
    {
        root.AddMember("name", "test_if");

        // Test nested if with GenericArray
        if (auto items = builder.ScopeArray("items"))
        {
            items.AddItem(1);
            items.AddItem(2);

            // Test deeply nested if with GenericObject
            if (auto nested = builder.ScopeObject())
            {
                nested.AddMember("nested_key", "nested_value");
            }
        }

        // Test if statement with key in scope
        if (auto config = builder.ScopeObject("config"))
        {
            config.AddMember("debug", true);
            config.AddMember("version", 1.0);
        }
    }

    std::string expect =
        R"({"name":"test_if","items":[1,2,{"nested_key":"nested_value"}],"config":{"debug":true,"version":1}})";
    COUT(builder.GetResult(), expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(scope_if_bool_vs_constructor, "构造方法中的 if bool 语法测试")
{
    // Test if syntax with scope methods
    wwjson::RawBuilder ifBuilder;
    if (auto root = ifBuilder.ScopeObject())
    {
        root.AddMember("method", "if_syntax");
        if (auto arr = ifBuilder.ScopeArray("data"))
        {
            arr.AddItem("first");
            arr.AddItem("second");
        }
    }
    ifBuilder.GetResult();

    // Test traditional constructor approach
    wwjson::RawBuilder ctorBuilder;
    {
        wwjson::RawObject root(ctorBuilder);
        root.AddMember("method", "if_syntax");
        {
            wwjson::RawArray arr(ctorBuilder, "data");
            arr.AddItem("first");
            arr.AddItem("second");
        }
    }
    ctorBuilder.GetResult();

    std::string expect = R"({"method":"if_syntax","data":["first","second"]})";
    COUT(ifBuilder.json, expect);
    COUT(ctorBuilder.json, expect);
    COUT(ifBuilder.json == ctorBuilder.json, true);
}

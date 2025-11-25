#include "couttast/tinytast.hpp"
#include "wwjson.hpp"
#include <string>

DEF_TAST(scope_builder_nest, "test build nest json with auto close using scope methods")
{
    wwjson::RawBuilder builder;
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
    COUT(builder.json, expect);
}

DEF_TAST(scope_vs_constructor, "test scope methods vs constructor approach")
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
                auto nested = scopeBuilder.ScopeObject(true);
                nested.AddMember("key", "value");
            }
        }
    }
    
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
                wwjson::RawObject nested(ctorBuilder, true);
                nested.AddMember("key", "value");
            }
        }
    }
    
    std::string expect = R"({"name":"test","items":[1,2,{"key":"value"}]})";
    COUT(scopeBuilder.json, expect);
    COUT(ctorBuilder.json, expect);
    COUT(scopeBuilder.json == ctorBuilder.json, true);
}

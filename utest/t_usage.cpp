/**
 * @file t_usage.cpp
 * @author lymslive
 * @date 2025-12-20
 * @brief docs/usage.md 文档示例的单元测试
 */
#include "couttast/couttast.h"
#include "wwjson.hpp"
#include "jbuilder.hpp"

#include <iostream>

// use to mark code snippet from document
#define MARKDOWN_CODE_SNIPPET

DEF_TAST(readme_1, "example from readme")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::RawBuilder builder;
    builder.BeginObject();
    builder.AddMember("name", "wwjson");
    builder.AddMember("version", 1.0);
    builder.AddMember("features", [&]() {
            auto arr = builder.ScopeArray();
            arr.AddItem("fast");
            arr.AddItem("simple");
            arr.AddItem("header-only");
            });
    builder.EndObject();

    std::string json = builder.GetResult();
    COUT(json, R"({"name":"wwjson","version":1.0,"features":["fast","simple","header-only"]})");
#endif
}

DEF_TAST(readme_2, "example from readme")
{
    struct User
    {
        std::string name;
        int age;
        bool active;

        void to_json(wwjson::Builder& builder) const
        {
            TO_JSON(name);   // wwjson::to_json(builder, "name", name);
            TO_JSON(age);    // wwjson::to_json(builder, "age", age);
            TO_JSON(active); // wwjson::to_json(builder, "active", active);
        }
    };

#ifdef MARKDOWN_CODE_SNIPPET
    User user{"Alice", 30, true};
    std::string json = wwjson::to_json(user);
#endif
    COUT(json, R"({"name":"Alice","age":30,"active":true})");
}

// 期望的JSON结果，用于第4节示例测试
static const std::string EXPECTED_SECTION_4_JSON = 
    R"({"name":"wwjson","version":1.01,"author":"lymslive","url":null,"feature":{"standar":"C++17","dom":false,"config":"compile-time"},"refer":["rapidjson","nlohmann/json",{"name":"yyjson","lang":"C"}]})";

DEF_TAST(usage_2_2_first_example, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
//+ #include "wwjson/wwjson.hpp"
//+ #include <iostream>
    
//+ int main()
    {
        // 创建JSON构建器
        wwjson::RawBuilder builder;
        
        // 开始构建对象
        builder.BeginObject();
        builder.AddMember("name", "WWJSON");
        builder.AddMember("version", 1.0);
        builder.AddMember("type", "header-only");
        builder.AddMember("language", "C++");
        builder.AddMember("license", "MIT");
        builder.EndObject();
        
        // 获取结果，更推荐 MoveResult()
        std::string json = builder.GetResult();
        COUT(json, R"({"name":"WWJSON","version":1.0,"type":"header-only","language":"C++","license":"MIT"})");
//+     std::cout << json << std::endl;
        
//+     return 0;
    }
#endif
}

DEF_TAST(usage_3_1_json_concatenation, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    int code = 0;
    std::string message = "OK";
    
    // 1. 使用 C printf
    {
        char buff[64];
        snprintf(buff, sizeof(buff), R"({"code":%d,"message":"%s"})", code, message.c_str());
        std::string json = buff;
        COUT(json, R"({"code":0,"message":"OK"})");
    }
    
    // 2. 使用 C++ stream
    {
        std::ostringstream oss;
        oss << '{'
            << R"("code":)" << code << ','
            << R"("message":)" << "\"" << message << "\""
            << '}';
        std::string json = oss.str();
        COUT(json, R"({"code":0,"message":"OK"})");
    }
    
    // 3. 直接使用 std::string
    {
        std::string json;
        json.reserve(1024);
        json.push_back('{');
        json.push_back('"');
        json.append("code");
        json.push_back('"');
        json.push_back(':');
        json.append(std::to_string(code));
        json.push_back(',');
        json.push_back('"');
        json.append("message");
        json.push_back('"');
        json.push_back(':');
        json.push_back('"');
        json.append(message);
        json.push_back('"');
        json.push_back('}');
        COUT(json, R"({"code":0,"message":"OK"})");
    }
#endif
}

DEF_TAST(usage_3_2_wwjson_encapsulation, "example from docs/usage.md")
{
    int code = 0;
    std::string message = "OK";
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::RawBuilder builder;
    builder.BeginObject();
    builder.AddMember("code", code);
    builder.AddMember("message", message);
    builder.EndObject();
    
//+ std::cout << builder.json << std::endl;
    //^ 输出：{"code":0,"message":"OK"},
    COUT(builder.json, R"({"code":0,"message":"OK"},)");
//+ std::cout << builder.GetResult() << std::endl;
    //^ 输出：{"code":0,"message":"OK"}
#endif
    std::string json = builder.GetResult();
    COUT(json, R"({"code":0,"message":"OK"})");
}

DEF_TAST(usage_4_1_flat_construction, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    
    // 基本字段
    builder.AddMember("name", "wwjson");
    builder.AddMember("version", 1.01);
    builder.AddMember("author", "lymslive");
    builder.AddMember("url", nullptr);  // null值
    
    // 嵌套对象 feature
    builder.BeginObject("feature");
    builder.AddMember("standar", "C++17");
    builder.AddMember("dom", false);
    builder.AddMember("config", "compile-time");
    builder.EndObject();
    
    // 数组 refer
    builder.BeginArray("refer");
    builder.AddItem("rapidjson");
    builder.AddItem("nlohmann/json");
    
    // 数组中的嵌套对象
    builder.BeginObject();
    builder.AddMember("name", "yyjson");
    builder.AddMember("lang", "C");
    builder.EndObject();
    
    builder.EndArray();
    
    builder.EndRoot();
#endif

    std::string json = builder.GetResult();
    COUT(json, EXPECTED_SECTION_4_JSON);
}

DEF_TAST(usage_4_2_brace_indentation, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    {
        // 基本字段
        builder.AddMember("name", "wwjson");
        builder.AddMember("version", 1.01);
        builder.AddMember("author", "lymslive");
        builder.AddMember("url", nullptr);
    
        // 嵌套对象 feature
        builder.BeginObject("feature");
        {
            builder.AddMember("standar", "C++17");
            builder.AddMember("dom", false);
            builder.AddMember("config", "compile-time");
        }
        builder.EndObject();
    
        // 数组 refer
        builder.BeginArray("refer");
        {
            builder.AddItem("rapidjson");
            builder.AddItem("nlohmann/json");
    
            // 数组中的嵌套对象
            builder.BeginObject();
            {
                builder.AddMember("name", "yyjson");
                builder.AddMember("lang", "C");
            }
            builder.EndObject();
        }
        builder.EndArray();
    }
    builder.EndRoot();
#endif

    std::string json = builder.GetResult();
    COUT(json, EXPECTED_SECTION_4_JSON);
}

DEF_TAST(usage_4_2_scope_variables, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::RawBuilder builder;
    {
        auto _close = builder.ScopeObject();
        // 基本字段
        builder.AddMember("name", "wwjson");
        builder.AddMember("version", 1.01);
        builder.AddMember("author", "lymslive");
        builder.AddMember("url", nullptr);
    
        // 嵌套对象 feature
        {
            auto _close = builder.ScopeObject("feature");
            builder.AddMember("standar", "C++17");
            builder.AddMember("dom", false);
            builder.AddMember("config", "compile-time");
        }
    
        // 数组 refer
        {
            auto _close = builder.ScopeArray("refer");
            builder.AddItem("rapidjson");
            builder.AddItem("nlohmann/json");
    
            // 数组中的嵌套对象
            {
                auto _close = builder.ScopeObject();
                builder.AddMember("name", "yyjson");
                builder.AddMember("lang", "C");
            }
        }
    }
    std::string json = builder.GetResult();
#endif
    COUT(json, EXPECTED_SECTION_4_JSON);
}

DEF_TAST(usage_4_2_named_scope_variables, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::RawBuilder builder;
    {
        auto root = builder.ScopeObject();
        // 基本字段
        root.AddMember("name", "wwjson");
        root.AddMember("version", 1.01);
        root.AddMember("author", "lymslive");
        root.AddMember("url", nullptr);
    
        // 嵌套对象 feature
        {
            auto feature = root.ScopeObject("feature");
            feature.AddMember("standar", "C++17");
            feature.AddMember("dom", false);
            feature.AddMember("config", "compile-time");
        }
    
        // 数组 refer
        {
            auto refer = root.ScopeArray("refer");
            refer.AddItem("rapidjson");
            refer.AddItem("nlohmann/json");
    
            // 数组中的嵌套对象
            {
                auto obj = refer.ScopeObject();
                obj.AddMember("name", "yyjson");
                obj.AddMember("lang", "C");
            }
        }
    }
    std::string json = builder.GetResult();
#endif
    COUT(json, EXPECTED_SECTION_4_JSON);
}

DEF_TAST(usage_4_2_if_statement_scope, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::RawBuilder builder;
    if (auto root = builder.ScopeObject())
    {
        // 基本字段
        root.AddMember("name", "wwjson");
        root.AddMember("version", 1.01);
        root.AddMember("author", "lymslive");
        root.AddMember("url", nullptr);
    
        // 嵌套对象 feature
        if (auto feature = root.ScopeObject("feature"))
        {
            feature.AddMember("standar", "C++17");
            feature.AddMember("dom", false);
            feature.AddMember("config", "compile-time");
        }
    
        // 数组 refer
        if (auto refer = root.ScopeArray("refer"))
        {
            refer.AddItem("rapidjson");
            refer.AddItem("nlohmann/json");
    
            // 数组中的嵌套对象
            if (auto obj = refer.ScopeObject())
            {
                obj.AddMember("name", "yyjson");
                obj.AddMember("lang", "C");
            }
        }
    }
    std::string json = builder.GetResult();
#endif
    COUT(json, EXPECTED_SECTION_4_JSON);
}

DEF_TAST(usage_4_3_lambda_substructure, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    
    // 基本字段
    builder.AddMember("name", "wwjson");
    builder.AddMember("version", 1.01);
    builder.AddMember("author", "lymslive");
    builder.AddMember("url", nullptr);
    
    // 嵌套对象 feature
    builder.AddMember("feature", [&builder]() {
        auto feature = builder.ScopeObject();
        feature.AddMember("standar", "C++17");
        feature.AddMember("dom", false);
        feature.AddMember("config", "compile-time");
    });
    
    // 数组 refer
    builder.AddMember("refer", [&builder]() {
        auto refer = builder.ScopeArray();
        refer.AddItem("rapidjson");
        refer.AddItem("nlohmann/json");
    
        // 数组中的嵌套对象
        refer.AddItem([&builder]() {
            auto obj = builder.ScopeObject();
            obj.AddMember("name", "yyjson");
            obj.AddMember("lang", "C");
        });
    });
    
    builder.EndRoot();
#endif

    std::string json = builder.GetResult();
    COUT(json, EXPECTED_SECTION_4_JSON);
}

DEF_TAST(usage_4_4_operator_overloading, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    
    // 基本字段
    builder["name"] = "wwjson";
    builder["version"] = 1.01;
    builder["author"] = "lymslive";
    builder["url"] = nullptr;
    
    // 嵌套对象 feature
    builder.AddMember("feature", [&builder]() {
        auto feature = builder.ScopeObject();
        feature << "standar"<< "C++17";
        feature << "dom"<< false << "config" << "compile-time";
    });
    
    // 数组 refer
    builder.AddMember("refer");
    {
        auto refer = builder.ScopeArray();
        refer[-1] = "rapidjson";
        refer[-1] = "nlohmann/json";
    
        // 数组中的嵌套对象
        refer << [&builder]() {
            auto obj = builder.ScopeObject();
            obj << "name" << "yyjson" << "lang" << "C";
        };
    }
    
    builder.EndRoot();
#endif

    std::string json = builder.GetResult();
    COUT(json, EXPECTED_SECTION_4_JSON);
}

DEF_TAST(usage_4_5_step_entrance, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
//+ std::string BuildJson()
    {
        wwjson::RawBuilder builder;
        builder.BeginRoot();
        // ...
        builder.EndRoot();
//+     return builder.MoveResult();
        COUT(builder.GetResult(), "{}");
    }
#endif
}

DEF_TAST(usage_4_5_struct_builder, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    struct Project
    {
        // 为简单起见，各成员直接赋默认值了
        std::string name = "wwjson";
        double version = 1.01;
        std::string author = "lymslive";
        std::string url;

        struct Feature
        {
            std::string standar = "C++17";
            bool dom = false;
            std::string config = "compile-time";
        } feature;

        std::vector<std::string> refer = {"rapidjson", "nlohmann/json"};

        struct Refer
        {
            std::string name = "yyjson";
            std::string lang = "C";
        } special_refer;

        // 其他数据成员或方法 ...

        // 构建 Json 的方法
        std::string BuildJson()
        {
            wwjson::RawBuilder builder;
            builder.BeginRoot();

            // 基本字段
            builder["name"] = name;
            builder["version"] = version;
            builder["author"] = author;
            if (url.empty())
            {
                builder["url"] = nullptr;
            }
            else
            {
                builder["url"] = url;
            }

            builder.AddMember("feature");
            BuildFeature(builder);

            builder.AddMember("refer");
            BuildRefer(builder);

            builder.EndRoot();
            return builder.MoveResult();
        }

        void BuildFeature(wwjson::RawBuilder &builder)
        {
            auto Jfeature = builder.ScopeObject();
            Jfeature["standar"] = feature.standar;
            Jfeature["dom"] = feature.dom;
            Jfeature["config"] = feature.config;
        }

        void BuildRefer(wwjson::RawBuilder &builder)
        {
            auto Jrefer = builder.ScopeArray();
            for (auto &item: refer)
            {
                Jrefer[-1] = item;
                // 或者 Jrefer << item;
            }
            BuildRefer2(builder);
        }

        void BuildRefer2(wwjson::RawBuilder &builder)
        {
            auto Jrefer = builder.ScopeObject();
            Jrefer["name"] = special_refer.name;
            Jrefer["lang"] = special_refer.lang;
        }
    };

    Project wwProject;
    std::string json = wwProject.BuildJson();
//+ std::cout << json << std::endl;
    COUT(json, EXPECTED_SECTION_4_JSON);
#endif
}

DEF_TAST(usage_4_6_struct_tojson, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    // 按更常规的模式将各子结构体先在相同作用域平坦定义
    struct Feature
    {
        std::string standar = "C++17";
        bool dom = false;
        std::string config = "compile-time";

        void to_json(wwjson::RawBuilder& builder) const
        {
            wwjson::to_json(builder, "standar", standar);
            wwjson::to_json(builder, "dom", dom);
            wwjson::to_json(builder, "config", config);
        }
    };

    struct Refer
    {
        std::string name = "yyjson";
        std::string lang = "C";

        void to_json(wwjson::RawBuilder& builder) const
        {
            // 可用宏进一步简化等效写法
            TO_JSON(name); // wwjson::to_json(builder, "name", name)
            TO_JSON(lang); // wwjson::to_json(builder, "lang", lang)
        }
    };

    struct Project
    {
        std::string name = "wwjson";
        double version = 1.01;
        std::string author = "lymslive";
        std::string url;

        Feature feature;

        std::vector<std::string> refer = {"rapidjson", "nlohmann/json"};

        Refer special_refer;

        void to_json(wwjson::RawBuilder& builder) const
        {
            TO_JSON(name);
            TO_JSON(version);
            TO_JSON(author);
            TO_JSON(url);
            TO_JSON(feature);
            TO_JSON(refer);
            TO_JSON(special_refer);
        }

        // 入口方法
        std::string to_json() const
        {
            wwjson::RawBuilder builder;
            wwjson::to_json(builder, *this);
            return builder.MoveResult();
        }
    };

    Project prj;
//+ std::cout << prj.to_json() << std::endl;
#endif
    std::string expect = R"({"name":"wwjson","version":1.01,"author":"lymslive","url":"","feature":{"standar":"C++17","dom":false,"config":"compile-time"},"refer":["rapidjson","nlohmann/json"],"special_refer":{"name":"yyjson","lang":"C"}})";
    COUT(prj.to_json());
}

DEF_TAST(usage_4_6_struct_array, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    struct Refer
    {
        std::string name = "yyjson";
        std::optional<std::string> lang;

        void to_json(wwjson::RawBuilder& builder) const
        {
            TO_JSON(name);
            TO_JSON(lang);
        }
    };

    struct Project
    {
        std::string name = "wwjson";
        std::string url;
        std::vector<Refer> refer;

        void to_json(wwjson::RawBuilder& builder) const
        {
            TO_JSON(name);
            TO_JSON(url);
            TO_JSON(refer);
        }
    };

    // 实例化数据结构体
    Project prj;
    prj.refer = {{"yyjson","C"}, {"rapidjson","C++"}, {"nlohmann/json"}};

    wwjson::RawBuilder builder;
    wwjson::to_json(builder, prj);
    std::string json = builder.MoveResult();

//+ std::cout << json << std::endl;
#endif
    std::string expect = R"({"name":"wwjson","url":"","refer":[{"name":"yyjson","lang":"C"},{"name":"rapidjson","lang":"C++"},{"name":"nlohmann/json","lang":null}]})";
    COUT(json);
}

DEF_TAST(usage_5_1_add_substring, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    // 假设先构建子串
    std::string feature;
    {
        wwjson::RawBuilder builder;
        builder.BeginRoot();
        builder.AddMember("standar", "C++17");
        builder.AddMember("dom", false);
        builder.AddMember("config", "compile-time");
        builder.EndRoot();
        feature = builder.MoveResult();
    }
    COUT(feature, R"({"standar":"C++17","dom":false,"config":"compile-time"})");
    
    std::string refer;
    {
        wwjson::RawBuilder builder;
        builder.BeginRoot();
        builder.AddMember("name", "yyjson");
        builder.AddMember("lang", "C");
        builder.EndRoot();
        refer = builder.MoveResult();
    }
    COUT(refer, R"({"name":"yyjson","lang":"C"})");
    
    // 再构建完整 json
    std::string project;
    {
        wwjson::RawBuilder builder;
        builder.BeginRoot();
        builder.AddMember("name", "wwjson");
        builder.AddMember("version", 1.01);
        builder.AddMember("author", "lymslive");
        builder.AddMember("url", nullptr);
    
        builder.AddMemberSub("feature", feature);
    
        builder.BeginArray("refer");
        builder.AddItem("rapidjson");
        builder.AddItem("nlohmann/json");
        builder.AddItemSub(refer);
        builder.EndArray();
    
        builder.EndRoot();
        project = builder.MoveResult();
    }
    COUT(project, EXPECTED_SECTION_4_JSON);
#endif
}

DEF_TAST(usage_5_2_merge_substrings, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    // 子串1
    wwjson::RawBuilder basic;
    basic.BeginRoot();
    basic["name"] = "wwjson";
    basic["version"] = 1.01;
    basic.EndRoot();
    std::string strBasic = basic.GetResult();
    COUT(strBasic, R"({"name":"wwjson","version":1.01})");
    
    // 子串2
    wwjson::RawBuilder feature;
    feature.BeginRoot();
    feature["standar"] = "C++17";
    feature["dom"] = false;
    feature.EndRoot();
    std::string strFeature = feature.GetResult();
    COUT(strFeature, R"({"standar":"C++17","dom":false})");
    
    // 将子串2到子串1
    basic.Merge(feature);
    
    // 用静态方法合并子串
    wwjson::RawBuilder::Merge(strBasic, strFeature);
#endif
    std::string merged = basic.GetResult();
    COUT(merged, R"({"name":"wwjson","version":1.01,"standar":"C++17","dom":false})");
    COUT(strBasic, R"({"name":"wwjson","version":1.01,"standar":"C++17","dom":false})");
}

struct SafeConfig : wwjson::BasicConfig<std::string>
{
    static constexpr bool kEscapeValue = true;
};
DEF_TAST(usage_6_2_1_safe_config, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
//+ struct SafeConfig : wwjson::BasicConfig<std::string>
//+ {
//+     static constexpr bool kEscapeValue = true;
//+ };

    using SafeBuilder = wwjson::GenericBuilder<std::string, SafeConfig>;
    SafeBuilder builder;
    builder.BeginRoot();
    builder["greet"] = "Hello\t!\nWorld\t!!";
    builder.EndRoot();
    
    // 结果：{"greet":"Hello\t!\nWorld\t!!"}
    COUT(builder.GetResult(), R"({"greet":"Hello\t!\nWorld\t!!"})");
    
    // 默认 builder 需要调用显式转义方法
    {
        wwjson::RawBuilder builder;
        builder.BeginRoot();
        builder.AddMemberEscape("greet", "Hello\t!\nWorld\t!!");
        builder.EndRoot();
        COUT(builder.GetResult(), R"({"greet":"Hello\t!\nWorld\t!!"})");
    }
#endif
}

DEF_TAST(usage_6_2_1_quote_numbers, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    builder.AddMember("name", "wwjson");
    builder.AddMember("version", 1.01, true);
    builder.EndRoot();
    
    // 结果：{"name":"wwjson","version":"1.01"}
#endif
    std::string json = builder.GetResult();
    COUT(json, R"({"name":"wwjson","version":"1.01"})");
}

struct UnsafeConfig : wwjson::BasicConfig<std::string>
{
    static void EscapeString(std::string &dst, const char *src, size_t len)
    {
        dst.append(src, len);
    }
};
DEF_TAST(usage_6_2_2_unsafe_config, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
//+ struct UnsafeConfig : wwjson::BasicConfig<std::string>
//+ {
//+     static void EscapeString(std::string &dst, const char *src, size_t len)
//+     {
//+         dst.append(src, len);
//+     }
//+ };
    
    using UnsafeBuilder = wwjson::GenericBuilder<std::string, UnsafeConfig>;
    UnsafeBuilder builder;
    builder.BeginRoot();
    builder.AddMemberEscape("greet", "Hello\t!\nWorld\t!!");
    builder.EndRoot();
    // 结果中将会有制表符与换行符
#endif
    std::string json = builder.GetResult();
    COUT(json, "{\"greet\":\"Hello\t!\nWorld\t!!\"}");
}

struct LogLine : public std::string {};
DEF_TAST(usage_6_3_1_log_line_target, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
//+ struct LogLine : public std::string {};
    using LogLineBuilder = wwjson::GenericBuilder<LogLine>;
    
    LogLine fullLine;
    fullLine.append("[Log] "); // 前缀部分
    LogLineBuilder builder(std::move(fullLine)); // 建议用移动的性能更高
    builder.BeginRoot();
    builder.AddMember("name", "wwjson");
    builder.AddMember("version", 1.01);
    builder.EndRoot();
    
    fullLine = builder.MoveResult();
    fullLine.append(" [End]"); // 可能的后缀部分
//+ std::cout << fullLine.c_str() << std::endl;
    // 结果：[Log] {"name":"wwjson","version":1.01} [End]
    COUT(fullLine, "[Log] {\"name\":\"wwjson\",\"version\":1.01} [End]");
#endif
}

struct MessageBuffer : public std::string {};
DEF_TAST(usage_6_3_2_message_stream_target, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
//+ struct MessageBuffer : public std::string {};
    using MessageBuilder = wwjson::GenericBuilder<MessageBuffer>;
    
    MessageBuilder builder;
    builder.BeginRoot();
    builder.AddMember("name", "wwjson");
    builder.AddMember("version", 1.01);
    builder.EndRoot();
    
    builder.EndLine();
    
    builder.BeginRoot();
    builder.AddMember("name", "yyjson");
    builder.AddMember("version", 1.02);
    builder.EndRoot();
    
    builder.EndLine();
#endif

    std::string messages = builder.GetResult();
    COUT(messages, R"({"name":"wwjson","version":1.01}
{"name":"yyjson","version":1.02}
)");
}

DEF_TAST(usage_6_4_2_check_ratio, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::FastBuilder builder(8*1024); // 初始容量 8K + 255

    builder.BeginArray();
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 1024; ++j)
        {
            builder.AddItem("abcde"); // 每次写入 8 字节，包括引号与逗号
        }
        builder.json.reserve_ex(8*1024); // 每千次再扩容 8K + 255
    }
    builder.EndArray();

    if (builder.json.overflow()) return;
    std::string result = builder.GetResult().str();
#endif
    COUT(builder.json.overflow(), false);
    COUT(result.size(), 8 * 10 * 1024 + 1);
    COUT(builder.json.capacity());
    COUT(builder.json.capacity() > result.size(), true);
}

DEF_TAST(usage_6_4_3_safe_margin, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    std::string str;
    str.reserve(256);
    while(str.size() < str.capacity())
    {
        str.push_back('x');
        if (str.size() > 1024*1024)
            break; // 防死循环
    }
//+ std::cout << "size=" << str.size() << std::endl;
//+ std::cout << "capacity=" << str.capacity() << std::endl;
    COUT(str.size(), 256);
    COUT(str.capacity(), 256);

    wwjson::JString jstr;
    jstr.reserve(256);
    while(jstr.size() < jstr.capacity())
    {
        jstr.push_back('x');
        if (jstr.size() > 1024*1024)
            break; // 防死循环
    }
//+ std::cout << "size=" << jstr.size() << std::endl;
//+ std::cout << "capacity=" << jstr.capacity() << std::endl;
    COUT(jstr.capacity() > jstr.size(), true);
    COUT(jstr.size() > 1024*1024, true);
#endif
}

DEF_TAST(usage_6_4_4_std_tostr, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    std::string str{"prefix:"};
    str += std::to_string(314);
    COUT(str, "prefix:314");

    // 或者用 snprintf
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", 159);
    str += buffer;
    COUT(str, "prefix:314159");
#endif
}

DEF_TAST(usage_6_4_4_err_tostr, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    std::string str{"prefix:"};
    str.reserve(str.size() + 16);
    int nWritten = snprintf(str.data() + str.size(), 16, "%d", 314);

//+ std::cout << str.c_str() << std::endl; // 可能正确 prefix:314
//+ std::cout << str << std::endl;         // 不正确 prefix:
//+ std::cout << str.size() << std::endl;  // 实际长度仍为 7
    COUT(str.size(), 7);
    COUT(str, "prefix:");

    int oldSize = str.size();
    str.resize(oldSize + nWritten);
//+ std::cout << str.size() << str.c_str() << std::endl; // 10prefix:
//+ std::cout << (str[7] != '3') << std::endl;
//+ std::cout << (str[7] == '\0') << std::endl;
    COUT(str.size(), 10);
    COUT(str.c_str(), "prefix:");
    COUT(str[7] != '3', true);
    COUT(str[7] == '\0', true);
#endif
}

DEF_TAST(usage_6_4_4_jstr_tostr, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
    wwjson::JString jstr;
    jstr.append("prefix:");

    jstr.reserve_ex(16);
    int nWritten = snprintf(jstr.end(), 16, "%d", 314);
    jstr.set_end(jstr.end() + nWritten);
    // 或 jstr.resize(jstr.size() + nWritten);

//+ std::cout << jstr.c_str() << std::endl; // 正确 prefix:314
//+ std::cout << jstr.str() << std::endl;   // 正确 prefix:314
//+ std::cout << jstr.size() << std::endl;  // 正确 10
    COUT(jstr.c_str(), "prefix:314");
    COUT(jstr.str(), "prefix:314");
    COUT(jstr.size(), 10);
#endif
}

// template
DEF_TAST(usage_last_empty, "example from docs/usage.md")
{
#ifdef MARKDOWN_CODE_SNIPPET
#endif
}


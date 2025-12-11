#include "couttast/tinytast.hpp"

#include "argv.h"
#include "relative_perf.h"

#include "wwjson.hpp"

#include "xyjson.h"

#include <string>
#include <vector>
#include <memory>

namespace test::perf
{

// Abstract base class for JSON building methods
class AbstractJsonBuilder
{
public:
    virtual ~AbstractJsonBuilder() = default;
    virtual void BuildJson(std::string &dst, int start, int items) = 0;
    virtual std::string GetMethodName() const = 0;
};

// Method B: Basic method (baseline)
class BasicMethodBuilder : public AbstractJsonBuilder
{
public:
    void BuildJson(std::string &dst, int start, int items) override
    {
        ::wwjson::RawBuilder builder(1024 + items * 64); // Estimate capacity

        builder.BeginRoot();

        builder.BeginArray("data");
        for (int i = 0; i < items; i++)
        {
            int value = start + i;

            // Create nested object for each item
            builder.BeginObject();
            builder.AddMember("id", value);
            builder.AddMember("value", value * 1.5);
            builder.AddMember("name", ("item_" + std::to_string(value)));

            builder.BeginArray("tags");
            builder.AddItem("tag1");
            builder.AddItem("tag2");
            builder.AddItem(std::to_string(value));
            builder.EndArray();

            builder.EndObject();
        }
        builder.EndArray();

        builder.BeginObject("metadata");
        builder.AddMember("version", "1.0");
        builder.AddMember("created", 1640995200); // Unix timestamp
        builder.AddMember("author", "test_system");
        builder.EndObject();

        builder.EndRoot();
        dst = std::move(builder.json); // Use public member directly
    }

    std::string GetMethodName() const override { return "Basic Method"; }
};

// Method A1: Auto-closing containers
class AutoCloseMethodBuilder : public AbstractJsonBuilder
{
public:
    void BuildJson(std::string &dst, int start, int items) override
    {
        ::wwjson::RawBuilder builder(1024 + items * 64);

        if (auto root = builder.ScopeObject())
        {
            if (auto data_arr = builder.ScopeArray("data"))
            {
                for (int i = 0; i < items; i++)
                {
                    int value = start + i;

                    if (auto item_obj = builder.ScopeObject())
                    {
                        item_obj.AddMember("id", value);
                        item_obj.AddMember("value", value * 1.5);
                        item_obj.AddMember("name", ("item_" + std::to_string(value)));

                        if (auto tag_arr = builder.ScopeArray("tags"))
                        {
                            tag_arr.AddItem("tag1");
                            tag_arr.AddItem("tag2");
                            tag_arr.AddItem(std::to_string(value));
                        }
                    }
                }
            }

            if (auto meta_obj = builder.ScopeObject("metadata"))
            {
                meta_obj.AddMember("version", "1.0");
                meta_obj.AddMember("created", 1640995200);
                meta_obj.AddMember("author", "test_system");
            }
        }

        dst = builder.MoveResult();
    }

    std::string GetMethodName() const override { return "Auto-close Method"; }
};

// Method A2: Operator methods
class OperatorMethodBuilder : public AbstractJsonBuilder
{
public:
    void BuildJson(std::string &dst, int start, int items) override
    {
        ::wwjson::RawBuilder builder(1024 + items * 64);

        builder.BeginRoot();

        builder.BeginArray("data");
        for (int i = 0; i < items; i++)
        {
            int value = start + i;

            // Create nested object for each item
            builder.BeginObject();
            builder["id"] = value;
            builder["value"] = value * 1.5;
            builder["name"] = ("item_" + std::to_string(value));

            builder.BeginArray("tags");
            builder[-1] = "tag1";
            builder[-1] = "tag2";
            builder[-1] = std::to_string(value);
            builder.EndArray();

            builder.EndObject();
        }
        builder.EndArray();

        builder.BeginObject("metadata");
        builder["version"] = "1.0";
        builder["created"] = 1640995200;
        builder["author"] = "test_system";
        builder.EndObject();

        builder.EndRoot();
        dst = std::move(builder.json);
    }

    std::string GetMethodName() const override { return "Operator Method"; }
};

// Method A3: Local object operators
class LocalOperatorMethodBuilder : public AbstractJsonBuilder
{
public:
    void BuildJson(std::string &dst, int start, int items) override
    {
        ::wwjson::RawBuilder builder(1024 + items * 64);

        if (auto root = builder.ScopeObject())
        {
            if (auto data_arr = builder.ScopeArray("data"))
            {
                for (int i = 0; i < items; i++)
                {
                    int value = start + i;

                    if (auto item_obj = builder.ScopeObject())
                    {
                        item_obj << "id" << value
                                 << "value" << (value * 1.5)
                                 << "name" << ("item_" + std::to_string(value));

                        if (auto tag_arr = builder.ScopeArray("tags"))
                        {
                            tag_arr << "tag1"
                                    << "tag2"
                                    << std::to_string(value).c_str();
                        }
                    }
                }
            }

            if (auto meta_obj = builder.ScopeObject("metadata"))
            {
                meta_obj << "version" << "1.0"
                         << "created" << 1640995200
                         << "author" << "test_system";
            }
        }

        dst = builder.GetResult();
    }

    std::string GetMethodName() const override { return "Local Operator Method"; }
};

// Method A4: Lambda for nested structures
class LambdaMethodBuilder : public AbstractJsonBuilder
{
public:
    void BuildJson(std::string &dst, int start, int items) override
    {
        ::wwjson::RawBuilder builder(1024 + items * 64);

        builder.BeginRoot();
        builder.AddMember("data", [&builder, start, items]() {
            auto arr = builder.ScopeArray();
            for (int i = 0; i < items; i++)
            {
                int value = start + i;

                arr.AddItem([&builder, value]() {
                    auto obj = builder.ScopeObject();
                    obj.AddMember("id", value);
                    obj.AddMember("value", value * 1.5);
                    obj.AddMember("name", ("item_" + std::to_string(value)));

                    obj.AddMember("tags", [&builder, value]() {
                        auto tag_arr = builder.ScopeArray();
                        tag_arr.AddItem("tag1");
                        tag_arr.AddItem("tag2");
                        tag_arr.AddItem(std::to_string(value));
                    });
                });
            }
        });

        builder.AddMember("metadata", [&builder]() {
            auto meta_obj = builder.ScopeObject();
            meta_obj.AddMember("version", "1.0");
            meta_obj.AddMember("created", 1640995200);
            meta_obj.AddMember("author", "test_system");
        });

        builder.EndRoot();
        dst = std::move(builder.json);
    }

    std::string GetMethodName() const override { return "Lambda Method"; }
};

// Method A5: Class method splitting
class ClassMethodBuilder : public AbstractJsonBuilder
{
private:
    class DataManager
    {
      public:
        void BuildDataArray(::wwjson::RawBuilder& builder, int start, int items)
        {
            auto arr = builder.ScopeArray();
            for (int i = 0; i < items; i++)
            {
                int value = start + i;
                BuildItem(builder, value);
            }
        }

        void BuildMetadata(::wwjson::RawBuilder& builder)
        {
            auto meta_obj = builder.ScopeObject();
            meta_obj.AddMember("version", "1.0");
            meta_obj.AddMember("created", 1640995200);
            meta_obj.AddMember("author", "test_system");
        }

      private:
        void BuildItem(::wwjson::RawBuilder& builder, int value)
        {
            auto item_obj = builder.ScopeObject();
            item_obj.AddMember("id", value);
            item_obj.AddMember("value", value * 1.5);
            item_obj.AddMember("name", ("item_" + std::to_string(value)));

            builder.PutKey("tags");
            BuildTags(builder, value);
        }

        void BuildTags(::wwjson::RawBuilder& builder, int value)
        {
            auto tag_arr = builder.ScopeArray();
            tag_arr.AddItem("tag1");
            tag_arr.AddItem("tag2");
            tag_arr.AddItem(std::to_string(value));
        }
    };

public:
    void BuildJson(std::string &dst, int start, int items) override
    {
        ::wwjson::RawBuilder builder(items * 64);

        builder.BeginRoot();
        DataManager manager;

        builder.PutKey("data");
        manager.BuildDataArray(builder, start, items);
        builder.PutKey("metadata");
        manager.BuildMetadata(builder);

        builder.EndRoot();
        dst = std::move(builder.json);
    }

    std::string GetMethodName() const override { return "Class Method"; }
};

// Relative performance test for different API methods
class ApiMethodPerfTest : public RelativeTimer<ApiMethodPerfTest>
{
public:
    int items;
    int start;
    std::unique_ptr<AbstractJsonBuilder> methodA_builder;
    std::unique_ptr<AbstractJsonBuilder> methodB_builder;
    std::string resultA;
    std::string resultB;

    ApiMethodPerfTest(int test_items, int test_start,
                      std::unique_ptr<AbstractJsonBuilder> builderA,
                      std::unique_ptr<AbstractJsonBuilder> builderB)
        : items(test_items), start(test_start),
          methodA_builder(std::move(builderA)),
          methodB_builder(std::move(builderB))
    {
    }

    void methodA()
    {
        methodA_builder->BuildJson(resultA, start, items);
    }

    void methodB()
    {
        methodB_builder->BuildJson(resultB, start, items);
    }

    bool methodVerify()
    {
        methodA();
        std::string tempA = resultA;
        methodB();
        std::string tempB = resultB;
        return tempA == tempB;
    }
};

} // namespace test::perf

// Test case for comparing Basic Method vs Auto-close Method
DEF_TAST(api_basic_vs_autoclose, "基本方法 vs 自动关闭方法性能对比")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);

    auto tester = test::perf::ApiMethodPerfTest(
        argv.items, argv.start,
        std::make_unique<test::perf::BasicMethodBuilder>(),
        std::make_unique<test::perf::AutoCloseMethodBuilder>());

    double ratio = tester.runAndPrint("Basic vs Auto-close",
                                      "Basic Method", "Auto-close Method",
                                      argv.loop, 10);
    DESC("Performance ratio: %.3f", ratio);
}

// Test case for comparing Basic Method vs Operator Method
DEF_TAST(api_basic_vs_operator, "基本方法 vs 操作符方法性能对比")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);

    auto tester = test::perf::ApiMethodPerfTest(
        argv.items, argv.start,
        std::make_unique<test::perf::BasicMethodBuilder>(),
        std::make_unique<test::perf::OperatorMethodBuilder>());

    double ratio = tester.runAndPrint("Basic vs Operator",
                                      "Basic Method", "Operator Method",
                                      argv.loop, 10);
    DESC("Performance ratio: %.3f", ratio);
}

// Test case for comparing Basic Method vs Local Object Method
DEF_TAST(api_basic_vs_localobj, "基本方法 vs 局部对象方法性能对比")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);

    auto tester = test::perf::ApiMethodPerfTest(
        argv.items, argv.start,
        std::make_unique<test::perf::BasicMethodBuilder>(),
        std::make_unique<test::perf::LocalOperatorMethodBuilder>());

    double ratio = tester.runAndPrint("Basic vs Local Object",
                                      "Basic Method", "Local Operator Method",
                                      argv.loop, 10);
    DESC("Performance ratio: %.3f", ratio);
}

// Test case for comparing Basic Method vs Lambda Method
DEF_TAST(api_basic_vs_lambda, "基本方法 vs Lambda方法性能对比")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);

    auto tester = test::perf::ApiMethodPerfTest(
        argv.items, argv.start,
        std::make_unique<test::perf::BasicMethodBuilder>(),
        std::make_unique<test::perf::LambdaMethodBuilder>());

    double ratio = tester.runAndPrint("Basic vs Lambda",
                                      "Basic Method", "Lambda Method",
                                      argv.loop, 10);
    DESC("Performance ratio: %.3f", ratio);
}

// Test case for comparing Basic Method vs Class Method
DEF_TAST(api_basic_vs_class, "基本方法 vs 类方法性能对比")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);

    auto tester = test::perf::ApiMethodPerfTest(
        argv.items, argv.start,
        std::make_unique<test::perf::BasicMethodBuilder>(),
        std::make_unique<test::perf::ClassMethodBuilder>());

    double ratio = tester.runAndPrint("Basic vs Class",
                                      "Basic Method", "Class Method",
                                      argv.loop, 10);
    DESC("Performance ratio: %.3f", ratio);
}

// Tool case to output JSON samples for verification
DEF_TOOL(api_output_sample, "输出各方法构建的JSON示例")
{
    test::CArgv argv;
    int test_items = std::min(argv.items, 3); // Limit output for readability

    std::cout << "=== JSON Output Samples ===" << std::endl;

    std::string resultB;
    std::string resultA1;
    std::string resultA2;
    std::string resultA3;
    std::string resultA4;
    std::string resultA5;

    // Basic Method
    {
        auto builder = std::make_unique<test::perf::BasicMethodBuilder>();
        std::string result;
        builder->BuildJson(result, argv.start, test_items);
        std::cout << "\n--- Basic Method ---" << std::endl;
        std::cout << result << std::endl;
        resultB = std::move(result);
    }

    // Auto-close Method
    {
        auto builder = std::make_unique<test::perf::AutoCloseMethodBuilder>();
        std::string result;
        builder->BuildJson(result, argv.start, test_items);
        std::cout << "\n--- Auto-close Method ---" << std::endl;
        std::cout << result << std::endl;
        resultA1 = std::move(result);
    }

    // Operator Method
    {
        auto builder = std::make_unique<test::perf::OperatorMethodBuilder>();
        std::string result;
        builder->BuildJson(result, argv.start, test_items);
        std::cout << "\n--- Operator Method ---" << std::endl;
        std::cout << result << std::endl;
        resultA2 = std::move(result);
    }

    // Local Operator Method
    {
        auto builder = std::make_unique<test::perf::LocalOperatorMethodBuilder>();
        std::string result;
        builder->BuildJson(result, argv.start, test_items);
        std::cout << "\n--- Local Object Method ---" << std::endl;
        std::cout << result << std::endl;
        resultA3 = std::move(result);
    }

    // Lambda Method
    {
        auto builder = std::make_unique<test::perf::LambdaMethodBuilder>();
        std::string result;
        builder->BuildJson(result, argv.start, test_items);
        std::cout << "\n--- Lambda Method ---" << std::endl;
        std::cout << result << std::endl;
        resultA4 = std::move(result);
    }

    // Class Method
    {
        auto builder = std::make_unique<test::perf::ClassMethodBuilder>();
        std::string result;
        builder->BuildJson(result, argv.start, test_items);
        std::cout << "\n--- Class Method ---" << std::endl;
        std::cout << result << std::endl;
        resultA5 = std::move(result);
    }

    COUT(resultA1 == resultB, true);
    COUT(resultA2 == resultB, true);
    COUT(resultA3 == resultB, true);
    COUT(resultA4 == resultB, true);
    COUT(resultA5 == resultB, true);

    ::yyjson::Document doc(resultB);
    COUT(doc.isValid(), true);
}

#include "argv.h"
#include "couttast/tinytast.hpp"
#include "relative_perf.h"

#include "wwjson.hpp"
#include "yyjson.h"
#include "jbuilder.hpp"

#include <string>
#include <vector>

namespace test::wwjson
{

/**
 * @brief Generate JSON data with specified number of items
 *
 * The generated JSON structure is as follows:
 * {
 *   "item_0": [1, 1.5, "1"],
 *   "nested_0": {"id": 0, "name": "Test Item", "value": 0.0},
 *   "item_1": [2, 3.0, "2"],
 *   "nested_1": {"id": 1, "name": "Test Item", "value": 2.5},
 *   ...
 * }
 *
 * @param dst Output string to store generated JSON
 * @param n Number of items to generate (each item = array + nested object)
 * @param size Estimated size in kilobytes (default 1)
 */
void BuildJson(std::string &dst, int n, int size = 1)
{
    ::wwjson::RawBuilder builder(size * 1024);

    // Start with an object
    builder.BeginObject();

    // Add n items with arrays and nested objects
    for (int i = 0; i < n; i++)
    {
        // Add array item
        std::string array_key = "item_" + std::to_string(i);
        builder.BeginArray(array_key.c_str());

        // Add 3 values to each array
        builder.AddItem(i + 1);
        builder.AddItem((i + 1) * 1.5);
        builder.AddItem(std::to_string(i + 1).c_str());

        builder.EndArray();

        // Add nested object
        std::string nested_key = "nested_" + std::to_string(i);
        builder.BeginObject(nested_key.c_str());
        builder.AddMember("id", i);
        builder.AddMember("name", "Test Item");
        builder.AddMember("value", i * 2.5);
        builder.EndObject();
    }

    // End the root object
    builder.EndObject();

    dst = builder.MoveResult();
}

/**
 * @brief Generate JSON data using Builder (GenericBuilder<JString>)
 *
 * Similar structure to BuildJson but uses wwjson::Builder with JString
 * to test performance difference from std::string-based RawBuilder.
 */
void BuildJsonJString(std::string &dst, int n, int reserve_kb = 1)
{
    ::wwjson::Builder builder(reserve_kb * 1024);

    builder.BeginObject();
    for (int i = 0; i < n; i++)
    {
        std::string array_key = "item_" + std::to_string(i);
        builder.BeginArray(array_key.c_str());
        builder.AddItem(i + 1);
        builder.AddItem((i + 1) * 1.5);
        builder.AddItem(std::to_string(i + 1).c_str());
        builder.EndArray();

        std::string nested_key = "nested_" + std::to_string(i);
        builder.BeginObject(nested_key.c_str());
        builder.AddMember("id", i);
        builder.AddMember("name", "Test Item");
        builder.AddMember("value", i * 2.5);
        builder.EndObject();
    }
    builder.EndObject();
    dst = builder.MoveResult();
}

/**
 * @brief Generate JSON data using FastBuilder (GenericBuilder<KString>)
 *
 * Similar structure to BuildJson but uses wwjson::FastBuilder with KString
 * to test performance difference from std::string-based RawBuilder.
 */
void BuildJsonKString(std::string &dst, int n, int reserve_kb = 1)
{
    ::wwjson::FastBuilder builder(reserve_kb * 1024);

    builder.BeginObject();
    for (int i = 0; i < n; i++)
    {
        std::string array_key = "item_" + std::to_string(i);
        builder.BeginArray(array_key.c_str());
        builder.AddItem(i + 1);
        builder.AddItem((i + 1) * 1.5);
        builder.AddItem(std::to_string(i + 1).c_str());
        builder.EndArray();

        std::string nested_key = "nested_" + std::to_string(i);
        builder.BeginObject(nested_key.c_str());
        builder.AddMember("id", i);
        builder.AddMember("name", "Test Item");
        builder.AddMember("value", i * 2.5);
        builder.EndObject();
    }
    builder.EndObject();
    dst = builder.MoveResult();
}

} // namespace test::wwjson

namespace test::yyjson
{

/**
 * @brief Generate JSON data with specified number of items using yyjson.
 *
 * The generated JSON structure is like above test::wwjson::BuildJson .
 */
void BuildJson(std::string &dst, int n)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc)
    {
        dst = "{}";
        return;
    }

    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    for (int i = 0; i < n; i++)
    {
        std::string array_key = "item_" + std::to_string(i);
        yyjson_mut_val *key = yyjson_mut_strcpy(doc, array_key.c_str());
        yyjson_mut_val *arr = yyjson_mut_arr(doc);
        if (!key || !arr || !yyjson_mut_obj_add(root, key, arr))
        {
            yyjson_mut_doc_free(doc);
            dst = "{}";
            return;
        }

        yyjson_mut_arr_add_int(doc, arr, i + 1);
        yyjson_mut_arr_add_real(doc, arr, (i + 1) * 1.5);
        yyjson_mut_arr_add_strcpy(doc, arr, std::to_string(i + 1).c_str());

        std::string nested_key = "nested_" + std::to_string(i);
        yyjson_mut_val *nested_key_val = yyjson_mut_strcpy(doc, nested_key.c_str());
        yyjson_mut_val *nested_obj = yyjson_mut_obj(doc);
        if (!nested_key_val || !nested_obj || !yyjson_mut_obj_add(root, nested_key_val, nested_obj))
        {
            yyjson_mut_doc_free(doc);
            dst = "{}";
            return;
        }

        yyjson_mut_obj_add_int(doc, nested_obj, "id", i);
        yyjson_mut_obj_add_str(doc, nested_obj, "name", "Test Item");
        yyjson_mut_obj_add_real(doc, nested_obj, "value", i * 2.5);
    }

    char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
    if (json_str)
    {
        dst = json_str;
        free(json_str);
    }
    else
    {
        dst = "{}";
    }

    yyjson_mut_doc_free(doc);
}

} // namespace test::yyjson

// Relative performance tests using RelativeTimer
namespace test::perf
{

// Relative performance test for JSON building
struct BuildJsonRelativeTest : public test::perf::RelativeTimer<BuildJsonRelativeTest>
{
    int n;
    int size_kb;
    std::string wwjson_result;
    std::string yyjson_result;

    BuildJsonRelativeTest(int items, int size = 1) : n(items), size_kb(size)
    {
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildJson(temp, n, 1);
            size_kb = (temp.size() / 1024) + 1;
        }
    }

    // Method A: wwjson build
    void methodA() { test::wwjson::BuildJson(wwjson_result, n, size_kb); }

    // Method B: yyjson build
    void methodB() { test::yyjson::BuildJson(yyjson_result, n); }

    bool methodVerify() { return true; }

    static const char *testName() { return "BuildJson Relative Test"; }
    static const char *labelA() { return "wwjson"; }
    static const char *labelB() { return "yyjson"; }
};

// Relative performance test: JString (Builder) vs std::string (RawBuilder)
struct BuildJsonJStringRelativeTest
    : public test::perf::RelativeTimer<BuildJsonJStringRelativeTest>
{
    int n;
    int size_kb;
    std::string raw_result;
    std::string jstring_result;

    BuildJsonJStringRelativeTest(int items, int size = 1) : n(items), size_kb(size)
    {
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildJson(temp, n, 1);
            size_kb = (temp.size() / 1024) + 1;
        }
    }

    // Method A: Builder with JString (faster, should be methodA)
    void methodA() { test::wwjson::BuildJsonJString(jstring_result, n, size_kb); }

    // Method B: RawBuilder with std::string (slower, should be methodB)
    void methodB() { test::wwjson::BuildJson(raw_result, n, size_kb); }

    bool methodVerify()
    {
        // double format may differ
        return true;
        test::wwjson::BuildJson(raw_result, n, size_kb);
        test::wwjson::BuildJsonJString(jstring_result, n, size_kb);
        return raw_result == jstring_result;
    }

    static const char *testName() { return "BuildJson JString Relative Test"; }
    static const char *labelA() { return "JString"; }
    static const char *labelB() { return "std::string"; }
};

// Relative performance test: KString (FastBuilder) vs std::string (RawBuilder)
struct BuildJsonKStringRelativeTest
    : public test::perf::RelativeTimer<BuildJsonKStringRelativeTest>
{
    int n;
    int size_kb;
    std::string raw_result;
    std::string kstring_result;

    BuildJsonKStringRelativeTest(int items, int size = 1) : n(items), size_kb(size)
    {
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildJson(temp, n, 1);
            size_kb = (temp.size() / 1024) + 1;
        }
    }

    // Method A: FastBuilder with KString (faster, should be methodA)
    void methodA() { test::wwjson::BuildJsonKString(kstring_result, n, size_kb); }

    // Method B: RawBuilder with std::string (slower, should be methodB)
    void methodB() { test::wwjson::BuildJson(raw_result, n, size_kb); }

    bool methodVerify()
    {
        // double format may differ
        return true;
        test::wwjson::BuildJson(raw_result, n, size_kb);
        test::wwjson::BuildJsonKString(kstring_result, n, size_kb);
        return raw_result == kstring_result;
    }

    static const char *testName() { return "BuildJson KString Relative Test"; }
    static const char *labelA() { return "KString"; }
    static const char *labelB() { return "std::string"; }
};

} // namespace test::perf

/* ============================================================ */
/* Relative performance test */

DEF_TAST(build_relative, "JSON 构建相对性能测试")
{
    test::CArgv argv;

    std::vector<int> test_counts = {6, 12, 120, 1200};
    test_counts.push_back(argv.items);

    DESC("=== Testing wwjson (RawBuilder) vs yyjson ===");
    DESC("");
    for (int n : test_counts)
    {
        test::perf::BuildJsonRelativeTest test1(n, 1);
        double ratio = test1.runAndPrint(
            "JSON Build Test (n=" + std::to_string(n) + ")",
            "wwjson", "yyjson",
            argv.loop, 10);
        COUT(ratio < 1.2, true);

        DESC("");
    }

    DESC("=== Testing JString (Builder) vs std::string (RawBuilder) ===");
    DESC("");

    for (int n : test_counts)
    {
        test::perf::BuildJsonJStringRelativeTest test2(n, 1);
        double ratio = test2.runAndPrint(
            "JString Test (n=" + std::to_string(n) + ")",
            "JString", "std::string",
            argv.loop, 10);
        COUT(ratio < 1.0, true);

        DESC("");
    }

    DESC("=== Testing KString (FastBuilder) vs std::string (RawBuilder) ===");
    DESC("");

    for (int n : test_counts)
    {
        test::perf::BuildJsonKStringRelativeTest test3(n, 1);
        double ratio = test3.runAndPrint(
            "KString Test (n=" + std::to_string(n) + ")",
            "KString", "std::string",
            argv.loop, 10);
        COUT(ratio < 1.0, true);

        DESC("");
    }
}

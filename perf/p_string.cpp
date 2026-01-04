#include "couttast/tinytast.hpp"
#include "relative_perf.h"

#include "argv.h"

#include "wwjson.hpp"
#include "yyjson.h"
#include "jbuilder.hpp" // For Builder and FastBuilder

#include <string>
#include <type_traits>
#include <vector>

// String serialization functions for performance testing
namespace test
{
namespace wwjson
{

/**
 * @brief Build JSON array of stringified integers
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integers to convert to strings
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildStringArray(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::RawBuilder builder(size_k << 10); // * 1024
    builder.BeginArray();

    for (int i = 0; i < count; i++)
    {
        builder.AddItem(std::to_string(start + i));
    }

    builder.EndArray();
    dst = builder.MoveResult();
}

/**
 * @brief Build JSON object with string keys and values
 *
 * Keys will be "k" + integer, values will be stringified integers
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of key-value pairs to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildStringObject(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::RawBuilder builder(size_k << 10); // * 1024
    builder.BeginObject();

    for (int i = 0; i < count; i++)
    {
        std::string value = std::to_string(start + i);
        std::string key = "k" + value;
        builder.AddMember(key, value);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

/**
 * @brief Build JSON object with escaped string values
 *
 * Keys will be "k" + integer, values will be the sub-json string:
 * {"key":"value"} Uses AddMemberEscape for explicit escaping
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of key-value pairs to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildEscapeObject(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::RawBuilder builder(size_k << 10); // * 1024
    builder.BeginObject();

    const std::string sub_json = "{\"key\":\"value\"}";

    for (int i = 0; i < count; i++)
    {
        std::string key = "k" + std::to_string(start + i);
        builder.AddMemberEscape(key, sub_json);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

/**
 * @brief Build JSON object with string keys and values using Builder (JString)
 *
 * Keys will be "k" + integer, values will be stringified integers
 * Uses wwjson::Builder with JString and UnsafeConfig for optimized escaping
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of key-value pairs to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildStringObjectJString(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::Builder builder(size_k << 10); // * 1024
    builder.BeginObject();

    for (int i = 0; i < count; i++)
    {
        std::string value = std::to_string(start + i);
        std::string key = "k" + value;
        builder.AddMember(key, value);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

/**
 * @brief Build JSON object with string keys and values using FastBuilder (KString)
 *
 * Keys will be "k" + integer, values will be stringified integers
 * Uses wwjson::FastBuilder with KString and UnsafeConfig for maximum performance
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of key-value pairs to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildStringObjectKString(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::FastBuilder builder(size_k << 10); // * 1024
    builder.BeginObject();

    for (int i = 0; i < count; i++)
    {
        std::string value = std::to_string(start + i);
        std::string key = "k" + value;
        builder.AddMember(key, value);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

/**
 * @brief Build JSON object with escaped string values using Builder (JString)
 *
 * Keys will be "k" + integer, values will be the sub-json string:
 * {"key":"value"} Uses AddMemberEscape for explicit escaping
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of key-value pairs to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildEscapeObjectJString(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::Builder builder(size_k << 10); // * 1024
    builder.BeginObject();

    const std::string sub_json = "{\"key\":\"value\"}";

    for (int i = 0; i < count; i++)
    {
        std::string key = "k" + std::to_string(start + i);
        builder.AddMemberEscape(key, sub_json);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

/**
 * @brief Build JSON object with escaped string values using FastBuilder (KString)
 *
 * Keys will be "k" + integer, values will be the sub-json string:
 * {"key":"value"} Uses AddMemberEscape for explicit escaping
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of key-value pairs to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildEscapeObjectKString(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::FastBuilder builder(size_k << 10); // * 1024
    builder.BeginObject();

    const std::string sub_json = "{\"key\":\"value\"}";

    for (int i = 0; i < count; i++)
    {
        std::string key = "k" + std::to_string(start + i);
        builder.AddMemberEscape(key, sub_json);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

} // namespace wwjson

namespace yyjson
{

/**
 * @brief Build JSON array of stringified integers using yyjson
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integers to convert to strings
 */
void BuildStringArray(std::string &dst, int start, int count)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc)
    {
        dst = "[]";
        return;
    }

    yyjson_mut_val *root = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, root);

    for (int i = 0; i < count; i++)
    {
        std::string str_val = std::to_string(start + i);
        yyjson_mut_val *str =
            yyjson_mut_strncpy(doc, str_val.c_str(), str_val.length());
        yyjson_mut_arr_add_val(root, str);
    }

    char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
    if (json_str)
    {
        dst = json_str;
        free(json_str);
    }
    else
    {
        dst = "[]";
    }

    yyjson_mut_doc_free(doc);
}

/**
 * @brief Build JSON object with string keys and values using yyjson
 *
 * Keys will be "k" + integer, values will be stringified integers
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of key-value pairs to generate
 */
void BuildStringObject(std::string &dst, int start, int count)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc)
    {
        dst = "{}";
        return;
    }

    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    for (int i = 0; i < count; i++)
    {
        std::string value = std::to_string(start + i);
        std::string key = "k" + value;
        yyjson_mut_val *key_str =
            yyjson_mut_strncpy(doc, key.c_str(), key.length());
        yyjson_mut_val *val_str =
            yyjson_mut_strncpy(doc, value.c_str(), value.length());
        yyjson_mut_obj_add(root, key_str, val_str);
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

/**
 * @brief Build JSON object with escaped string values using yyjson
 *
 * Keys will be "k" + integer, values will be the sub-json string:
 * {"key":"value"} yyjson automatically handles escaping
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of key-value pairs to generate
 */
void BuildEscapeObject(std::string &dst, int start, int count)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc)
    {
        dst = "{}";
        return;
    }

    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    // Use string value with automatic escaping
    const std::string sub_json = "{\"key\":\"value\"}";

    for (int i = 0; i < count; i++)
    {
        std::string key = "k" + std::to_string(start + i);
        yyjson_mut_val *key_str =
            yyjson_mut_strncpy(doc, key.c_str(), key.length());
        yyjson_mut_val *val_str =
            yyjson_mut_strncpy(doc, sub_json.c_str(), sub_json.length());
        yyjson_mut_obj_add(root, key_str, val_str);
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

} // namespace yyjson
} // namespace test

/**
 * @brief Performance test suite for comparing string serialization
 *
 * This test suite creates multiple test cases that measure the performance
 * of wwjson::RawBuilder versus yyjson's mutable document API for generating
 * JSON string arrays, objects, and escaped content.
 *
 * The test cases use --start= parameter for the starting value and
 * --items= parameter for the count of items to generate.
 */

// Performance test for wwjson string array building
DEF_TAST(string_array_wwjson, "wwjson 字符串数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    // Auto-estimate capacity only when argv.size is default (1)
    if (argv.size == 1)
    {
        // Run once to estimate capacity
        test::wwjson::BuildStringArray(json_data, argv.start, argv.items, 1);
        int estimated_size =
            (json_data.size() / 1024) + 1; // Convert to KB, round up
        argv.size = estimated_size;
    }

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::wwjson::BuildStringArray(json_data, argv.start, argv.items,
                                       argv.size);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --size=%d", argv.start, argv.items,
         argv.size);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d", argv.items);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for yyjson string array building
DEF_TAST(string_array_yyjson, "yyjson 字符串数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::yyjson::BuildStringArray(json_data, argv.start, argv.items);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d", argv.items);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for wwjson string object building
DEF_TAST(string_object_wwjson, "wwjson 字符串对象构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    // Auto-estimate capacity only when argv.size is default (1)
    if (argv.size == 1)
    {
        // Run once to estimate capacity
        test::wwjson::BuildStringObject(json_data, argv.start, argv.items, 1);
        int estimated_size =
            (json_data.size() / 1024) + 1; // Convert to KB, round up
        argv.size = estimated_size;
    }

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::wwjson::BuildStringObject(json_data, argv.start, argv.items,
                                        argv.size);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --size=%d", argv.start, argv.items,
         argv.size);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Object members: %d", argv.items);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for yyjson string object building
DEF_TAST(string_object_yyjson, "yyjson 字符串对象构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::yyjson::BuildStringObject(json_data, argv.start, argv.items);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Object members: %d", argv.items);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for wwjson escaped string object building
DEF_TAST(string_escape_wwjson, "wwjson 转义字符串对象构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    // Auto-estimate capacity only when argv.size is default (1)
    if (argv.size == 1)
    {
        // Run once to estimate capacity
        test::wwjson::BuildEscapeObject(json_data, argv.start, argv.items, 1);
        int estimated_size =
            (json_data.size() / 1024) + 1; // Convert to KB, round up
        argv.size = estimated_size;
    }

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::wwjson::BuildEscapeObject(json_data, argv.start, argv.items,
                                        argv.size);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --size=%d", argv.start, argv.items,
         argv.size);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Object members: %d", argv.items);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for yyjson escaped string object building
DEF_TAST(string_escape_yyjson, "yyjson 转义字符串对象构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::yyjson::BuildEscapeObject(json_data, argv.start, argv.items);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Object members: %d", argv.items);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Comparison test for wwjson vs yyjson output
DEF_TOOL(string_compare, "比较 wwjson 和 yyjson 字符串函数输出")
{
    test::CArgv argv;
    int test_count = 5;

    // Test string arrays
    COUT("=== String Array Comparison ===");
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildStringArray(wwjson_output, argv.start, test_count);
        test::yyjson::BuildStringArray(yyjson_output, argv.start, test_count);
        COUT(wwjson_output);
        COUT(yyjson_output);
        COUT(wwjson_output == yyjson_output, true);
    }

    // Test string objects
    COUT("=== String Object Comparison ===");
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildStringObject(wwjson_output, argv.start, test_count);
        test::yyjson::BuildStringObject(yyjson_output, argv.start, test_count);
        COUT(wwjson_output);
        COUT(yyjson_output);
        COUT(wwjson_output == yyjson_output, true);
    }

    // Test escaped objects
    COUT("=== Escaped Object Comparison ===");
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildEscapeObject(wwjson_output, argv.start, test_count);
        test::yyjson::BuildEscapeObject(yyjson_output, argv.start, test_count);
        COUT(wwjson_output);
        COUT(yyjson_output);
        COUT(wwjson_output == yyjson_output, true);
    }
}

// Relative performance tests using RelativeTimer
namespace test::perf
{

// Relative performance test for string object building
struct StringObjectRelativeTest : public test::perf::RelativeTimer<StringObjectRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string wwjson_result;
    std::string yyjson_result;

    StringObjectRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size) 
    {
        // Auto-estimate size if default (1)
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildStringObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1; // Convert to KB, round up
        }
    }

    // Method A: wwjson build
    void methodA()
    {
        test::wwjson::BuildStringObject(wwjson_result, start, count, size_kb);
    }

    // Method B: yyjson build
    void methodB()
    {
        test::yyjson::BuildStringObject(yyjson_result, start, count);
    }

    // Verify the outputs are functionally equivalent
    bool methodVerify()
    {
        // Generate outputs first
        methodA();
        methodB();

        // For string objects without floating-point values, we can directly compare the strings
        // This is stricter and simpler than JSON document comparison
        return wwjson_result == yyjson_result;
    }

    static const char* testName() { return "StringObject Relative Test"; }
    static const char* labelA() { return "wwjson"; }
    static const char* labelB() { return "yyjson"; }
};

// Relative performance test for escaped string object building
struct EscapeObjectRelativeTest : public test::perf::RelativeTimer<EscapeObjectRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string wwjson_result;
    std::string yyjson_result;

    EscapeObjectRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size) 
    {
        // Auto-estimate size if default (1)
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildEscapeObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1; // Convert to KB, round up
        }
    }

    // Method A: wwjson build
    void methodA()
    {
        test::wwjson::BuildEscapeObject(wwjson_result, start, count, size_kb);
    }

    // Method B: yyjson build
    void methodB()
    {
        test::yyjson::BuildEscapeObject(yyjson_result, start, count);
    }

    // Verify the outputs are functionally equivalent
    bool methodVerify()
    {
        // Generate outputs first
        methodA();
        methodB();

        // For escaped string objects, we can directly compare the strings
        // This is stricter and simpler than JSON document comparison
        return wwjson_result == yyjson_result;
    }

    static const char* testName() { return "EscapeObject Relative Test"; }
    static const char* labelA() { return "wwjson"; }
    static const char* labelB() { return "yyjson"; }
};

// Relative performance test: std::string (RawBuilder) vs JString (Builder)
struct StringObjectJStringRelativeTest
    : public test::perf::RelativeTimer<StringObjectJStringRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string raw_result;
    std::string jstring_result;

    StringObjectJStringRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size)
    {
        // Auto-estimate size if default (1)
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildStringObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1; // Convert to KB, round up
        }
    }

    // Method A: RawBuilder with std::string
    void methodA()
    {
        test::wwjson::BuildStringObject(raw_result, start, count, size_kb);
    }

    // Method B: Builder with JString
    void methodB()
    {
        test::wwjson::BuildStringObjectJString(jstring_result, start, count, size_kb);
    }

    // Verify the outputs are functionally equivalent
    bool methodVerify()
    {
        // Generate outputs first
        methodA();
        methodB();

        // For string objects without floating-point values, we can directly compare the strings
        return raw_result == jstring_result;
    }

    static const char* testName() { return "StringObject JString Relative Test"; }
    static const char* labelA() { return "std::string"; }
    static const char* labelB() { return "JString"; }
};

// Relative performance test: std::string (RawBuilder) vs KString (FastBuilder)
struct StringObjectKStringRelativeTest
    : public test::perf::RelativeTimer<StringObjectKStringRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string raw_result;
    std::string kstring_result;

    StringObjectKStringRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size)
    {
        // Auto-estimate size if default (1)
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildStringObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1; // Convert to KB, round up
        }
    }

    // Method A: RawBuilder with std::string
    void methodA()
    {
        test::wwjson::BuildStringObject(raw_result, start, count, size_kb);
    }

    // Method B: FastBuilder with KString
    void methodB()
    {
        test::wwjson::BuildStringObjectKString(kstring_result, start, count, size_kb);
    }

    // Verify the outputs are functionally equivalent
    bool methodVerify()
    {
        // Generate outputs first
        methodA();
        methodB();

        // For string objects without floating-point values, we can directly compare the strings
        return raw_result == kstring_result;
    }

    static const char* testName() { return "StringObject KString Relative Test"; }
    static const char* labelA() { return "std::string"; }
    static const char* labelB() { return "KString"; }
};

// Relative performance test: std::string (RawBuilder) vs JString (Builder) for escape
struct EscapeObjectJStringRelativeTest
    : public test::perf::RelativeTimer<EscapeObjectJStringRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string raw_result;
    std::string jstring_result;

    EscapeObjectJStringRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size)
    {
        // Auto-estimate size if default (1)
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildEscapeObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1; // Convert to KB, round up
        }
    }

    // Method A: RawBuilder with std::string
    void methodA()
    {
        test::wwjson::BuildEscapeObject(raw_result, start, count, size_kb);
    }

    // Method B: Builder with JString
    void methodB()
    {
        test::wwjson::BuildEscapeObjectJString(jstring_result, start, count, size_kb);
    }

    // Verify the outputs are functionally equivalent
    bool methodVerify()
    {
        // Generate outputs first
        methodA();
        methodB();

        // For escaped string objects, we can directly compare the strings
        return raw_result == jstring_result;
    }

    static const char* testName() { return "EscapeObject JString Relative Test"; }
    static const char* labelA() { return "std::string"; }
    static const char* labelB() { return "JString"; }
};

// Relative performance test: std::string (RawBuilder) vs KString (FastBuilder) for escape
struct EscapeObjectKStringRelativeTest
    : public test::perf::RelativeTimer<EscapeObjectKStringRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string raw_result;
    std::string kstring_result;

    EscapeObjectKStringRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size)
    {
        // Auto-estimate size if default (1)
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildEscapeObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1; // Convert to KB, round up
        }
    }

    // Method A: RawBuilder with std::string
    void methodA()
    {
        test::wwjson::BuildEscapeObject(raw_result, start, count, size_kb);
    }

    // Method B: FastBuilder with KString
    void methodB()
    {
        test::wwjson::BuildEscapeObjectKString(kstring_result, start, count, size_kb);
    }

    // Verify the outputs are functionally equivalent
    bool methodVerify()
    {
        // Generate outputs first
        methodA();
        methodB();

        // For escaped string objects, we can directly compare the strings
        return raw_result == kstring_result;
    }

    static const char* testName() { return "EscapeObject KString Relative Test"; }
    static const char* labelA() { return "std::string"; }
    static const char* labelB() { return "KString"; }
};

} // namespace test::perf

// Relative performance test for string object building
DEF_TAST(string_object_relative, "字符串对象构建相对性能测试（wwjson vs yyjson）")
{
    test::CArgv argv;

    // Test with predefined item counts and also include argv.items
    std::vector<int> test_counts = {10, 50, 100, 500}; // Different object sizes
    test_counts.push_back(argv.items); // Add user-specified items

    DESC("=== Testing wwjson (RawBuilder) vs yyjson ===");
    DESC("");
    for (int n : test_counts)
    {
        test::perf::StringObjectRelativeTest test(argv.start, n, 1); // size will be auto-estimated
        double ratio = test.runAndPrint(
            "String Object Test (items=" + std::to_string(n) + ")",
            "wwjson", "yyjson",
            argv.loop, 10
        );

        DESC("");
    }

    DESC("=== Testing std::string(RawBuilder) vs JString (Builder) ===");
    DESC("");
    for (int n : test_counts)
    {
        test::perf::StringObjectJStringRelativeTest test(argv.start, n, 1);
        double ratio = test.runAndPrint(
            "String Object JString Test (items=" + std::to_string(n) + ")",
            "std::string", "JString",
            argv.loop, 10
        );

        DESC("");
    }

    DESC("=== Testing std::string(RawBuilder) vs KString (FastBuilder) ===");
    DESC("");
    for (int n : test_counts)
    {
        test::perf::StringObjectKStringRelativeTest test(argv.start, n, 1);
        double ratio = test.runAndPrint(
            "String Object KString Test (items=" + std::to_string(n) + ")",
            "std::string", "KString",
            argv.loop, 10
        );

        DESC("");
    }
}

// Relative performance test for escaped string object building
DEF_TAST(string_escape_relative, "转义字符串对象构建相对性能测试（wwjson vs yyjson）")
{
    test::CArgv argv;

    // Test with predefined item counts and also include argv.items
    std::vector<int> test_counts = {10, 50, 100, 500}; // Different object sizes
    test_counts.push_back(argv.items); // Add user-specified items

    DESC("=== Testing wwjson (RawBuilder) vs yyjson ===");
    DESC("");
    for (int n : test_counts)
    {
        test::perf::EscapeObjectRelativeTest test(argv.start, n, 1); // size will be auto-estimated
        double ratio = test.runAndPrint(
            "Escape Object Test (items=" + std::to_string(n) + ")",
            "wwjson", "yyjson",
            argv.loop, 10
        );

        DESC("");
    }

    DESC("=== Testing std::string(RawBuilder) vs JString (Builder) ===");
    DESC("");
    for (int n : test_counts)
    {
        test::perf::EscapeObjectJStringRelativeTest test(argv.start, n, 1);
        double ratio = test.runAndPrint(
            "Escape Object JString Test (items=" + std::to_string(n) + ")",
            "std::string", "JString",
            argv.loop, 10
        );

        DESC("");
    }

    DESC("=== Testing std::string(RawBuilder) vs KString (FastBuilder) ===");
    DESC("");
    for (int n : test_counts)
    {
        test::perf::EscapeObjectKStringRelativeTest test(argv.start, n, 1);
        double ratio = test.runAndPrint(
            "Escape Object KString Test (items=" + std::to_string(n) + ")",
            "std::string", "KString",
            argv.loop, 10
        );

        DESC("");
    }
}

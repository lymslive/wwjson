#include "couttast/tinytast.hpp"

#include "argv.h"

#include "wwjson.hpp"
#include "yyjson.h"

#include <string>
#include <type_traits>

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

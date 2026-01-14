/**
 * @file tic_number.cpp
 * @brief Absolute time performance tests for number serialization
 *
 * @details
 * This file contains DEF_TAST test cases that use absolute timing (TIME_TIC/TIME_TOC)
 * for measuring number serialization performance. These tests were separated from p_number.cpp
 * to keep relative performance tests (using RelativeTimer) separate from absolute timing tests.
 *
 * These tests are compiled into the ticwwjson target and compare wwjson number
 * serialization performance against yyjson for various numeric types.
 */

#include "couttast/tinytast.hpp"

#include "argv.h"
#include "relative_perf.h"

#include "wwjson.hpp"

#include "yyjson.h"

#include <string>
#include <type_traits>
#include <vector>
#include <memory>

namespace test
{
namespace wwjson
{

/**
 * @brief Template function to build JSON arrays of integer pairs
 *
 * @tparam uintT Unsigned integer type (uint8_t, uint16_t, uint32_t, uint64_t)
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integer pairs to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
template <typename uintT>
void BuildIntArray(std::string &dst, uintT start, int count, int size_k = 1)
{
    using sintT = std::make_signed_t<uintT>;

    ::wwjson::RawBuilder builder(size_k << 10); //  * 1024
    builder.BeginArray();

    uintT current = start;

    for (int i = 0; i < count; i++)
    {
        uintT positive = current;
        sintT negative = -static_cast<sintT>(positive);

        builder.AddItem(positive);
        builder.AddItem(negative);

        current++;
    }

    builder.EndArray();
    dst = builder.MoveResult();
}

/**
 * @brief Function to build JSON arrays of float values
 *
 * For each integer, generates 4 floating point values: +0.0, +1/5, +1/3, +1/2
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integers to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildFloatArray(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::RawBuilder builder(size_k << 10); //  * 1024
    builder.BeginArray();

    for (int i = 0; i < count; i++)
    {
        int value = start + i;

        // Generate 4 floating point values for each integer
        builder.AddItem(static_cast<float>(value + 0.0f));
        builder.AddItem(static_cast<float>(value + 1.0f / 5.0f));
        builder.AddItem(static_cast<float>(value + 1.0f / 3.0f));
        builder.AddItem(static_cast<float>(value + 1.0f / 2.0f));
    }

    builder.EndArray();
    dst = builder.MoveResult();
}

/**
 * @brief Function to build JSON arrays of double values
 *
 * For each integer, generates 4 floating point values: +0.0, +1/5, +1/3, +1/2
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integers to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildDoubleArray(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::RawBuilder builder(size_k << 10); //  * 1024
    builder.BeginArray();

    for (int i = 0; i < count; i++)
    {
        int value = start + i;

        // Generate 4 floating point values for each integer
        builder.AddItem(static_cast<double>(value + 0.0));
        builder.AddItem(static_cast<double>(value + 1.0 / 5.0));
        builder.AddItem(static_cast<double>(value + 1.0 / 3.0));
        builder.AddItem(static_cast<double>(value + 1.0 / 2.0));
    }

    builder.EndArray();
    dst = builder.MoveResult();
}

} // namespace wwjson

namespace yyjson
{

/**
 * @brief Template function to build JSON arrays of integer pairs using yyjson
 *
 * @tparam uintT Unsigned integer type (uint8_t, uint16_t, uint32_t, uint64_t)
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integer pairs to generate
 */
template <typename uintT>
void BuildIntArray(std::string &dst, uintT start, int count)
{
    using sintT = std::make_signed_t<uintT>;

    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc)
    {
        dst = "[]";
        return;
    }

    yyjson_mut_val *root = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, root);

    uintT current = start;

    for (int i = 0; i < count; i++)
    {
        uintT positive = current;
        sintT negative = -static_cast<sintT>(positive);

        yyjson_mut_arr_add_uint(doc, root, positive);
        yyjson_mut_arr_add_sint(doc, root, negative);

        current++;
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
 * @brief Function to build JSON arrays of float values using yyjson
 *
 * For each integer, generates 4 floating point values: +0.0, +1/5, +1/3, +1/2
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integers to generate
 */
void BuildFloatArray(std::string &dst, int start, int count)
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
        int value = start + i;

        // Generate 4 floating point values for each integer using
        // yyjson_mut_arr_add_float
        yyjson_mut_arr_add_real(doc, root, static_cast<float>(value + 0.0f));
        yyjson_mut_arr_add_real(doc, root,
                                static_cast<float>(value + 1.0f / 5.0f));
        yyjson_mut_arr_add_real(doc, root,
                                static_cast<float>(value + 1.0f / 3.0f));
        yyjson_mut_arr_add_real(doc, root,
                                static_cast<float>(value + 1.0f / 2.0f));
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
 * @brief Function to build JSON arrays of double values using yyjson
 *
 * For each integer, generates 4 floating point values: +0.0, +1/5, +1/3, +1/2
 *
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integers to generate
 */
void BuildDoubleArray(std::string &dst, int start, int count)
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
        int value = start + i;

        // Generate 4 floating point values for each integer using
        // yyjson_mut_arr_add_double
        yyjson_mut_arr_add_real(doc, root, static_cast<double>(value + 0.0));
        yyjson_mut_arr_add_real(doc, root,
                                static_cast<double>(value + 1.0 / 5.0));
        yyjson_mut_arr_add_real(doc, root,
                                static_cast<double>(value + 1.0 / 3.0));
        yyjson_mut_arr_add_real(doc, root,
                                static_cast<double>(value + 1.0 / 2.0));
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

} // namespace yyjson
} // namespace test

/* ============================================================ */
/* Absolute time performance tests (DEF_TAST with TIME_TIC/TIME_TOC) */
/* ============================================================ */

// Performance test for wwjson int8 array building
DEF_TAST(tic_number_int8_wwjson, "wwjson int8 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    // Auto-estimate capacity only when argv.size is default (1)
    if (argv.size == 1)
    {
        // Run once to estimate capacity
        test::wwjson::BuildIntArray(json_data, static_cast<uint8_t>(argv.start),
                                    argv.items, 1);
        int estimated_size =
            (json_data.size() / 1024) + 1; // Convert to KB, round up
        argv.size = estimated_size;
    }

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::wwjson::BuildIntArray(json_data, static_cast<uint8_t>(argv.start),
                                    argv.items, argv.size);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --size=%d", argv.start, argv.items,
         argv.size);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for yyjson int8 array building
DEF_TAST(tic_number_int8_yyjson, "yyjson int8 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::yyjson::BuildIntArray(json_data, static_cast<uint8_t>(argv.start),
                                    argv.items);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for wwjson int16 array building
DEF_TAST(tic_number_int16_wwjson, "wwjson int16 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    // Auto-estimate capacity only when argv.size is default (1)
    if (argv.size == 1)
    {
        // Run once to estimate capacity
        test::wwjson::BuildIntArray(
            json_data, static_cast<uint16_t>(argv.start), argv.items, 1);
        int estimated_size =
            (json_data.size() / 1024) + 1; // Convert to KB, round up
        argv.size = estimated_size;
    }

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::wwjson::BuildIntArray(json_data,
                                    static_cast<uint16_t>(argv.start),
                                    argv.items, argv.size);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --size=%d", argv.start, argv.items,
         argv.size);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for yyjson int16 array building
DEF_TAST(tic_number_int16_yyjson, "yyjson int16 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::yyjson::BuildIntArray(
            json_data, static_cast<uint16_t>(argv.start), argv.items);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for wwjson int32 array building
DEF_TAST(tic_number_int32_wwjson, "wwjson int32 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    // Auto-estimate capacity only when argv.size is default (1)
    if (argv.size == 1)
    {
        // Run once to estimate capacity
        test::wwjson::BuildIntArray(
            json_data, static_cast<uint32_t>(argv.start), argv.items, 1);
        int estimated_size =
            (json_data.size() / 1024) + 1; // Convert to KB, round up
        argv.size = estimated_size;
    }

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::wwjson::BuildIntArray(json_data,
                                    static_cast<uint32_t>(argv.start),
                                    argv.items, argv.size);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --size=%d", argv.start, argv.items,
         argv.size);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for yyjson int32 array building
DEF_TAST(tic_number_int32_yyjson, "yyjson int32 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::yyjson::BuildIntArray(
            json_data, static_cast<uint32_t>(argv.start), argv.items);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for wwjson int64 array building
DEF_TAST(tic_number_int64_wwjson, "wwjson int64 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    // Auto-estimate capacity only when argv.size is default (1)
    if (argv.size == 1)
    {
        // Run once to estimate capacity
        test::wwjson::BuildIntArray(
            json_data, static_cast<uint64_t>(argv.start), argv.items, 1);
        int estimated_size =
            (json_data.size() / 1024) + 1; // Convert to KB, round up
        argv.size = estimated_size;
    }

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::wwjson::BuildIntArray(json_data,
                                    static_cast<uint64_t>(argv.start),
                                    argv.items, argv.size);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --size=%d", argv.start, argv.items,
         argv.size);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for yyjson int64 array building
DEF_TAST(tic_number_int64_yyjson, "yyjson int64 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::yyjson::BuildIntArray(
            json_data, static_cast<uint64_t>(argv.start), argv.items);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for wwjson float array building
DEF_TAST(tic_number_float_wwjson, "wwjson float 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    // Auto-estimate capacity only when argv.size is default (1)
    if (argv.size == 1)
    {
        // Run once to estimate capacity
        test::wwjson::BuildFloatArray(json_data, argv.start, argv.items, 1);
        int estimated_size =
            (json_data.size() / 1024) + 1; // Convert to KB, round up
        argv.size = estimated_size;
    }

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::wwjson::BuildFloatArray(json_data, argv.start, argv.items,
                                      argv.size);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --size=%d", argv.start, argv.items,
         argv.size);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (4*items)", argv.items * 4);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for yyjson float array building
DEF_TAST(tic_number_float_yyjson, "yyjson float 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::yyjson::BuildFloatArray(json_data, argv.start, argv.items);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (4*items)", argv.items * 4);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for wwjson double array building
DEF_TAST(tic_number_double_wwjson, "wwjson double 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    // Auto-estimate capacity only when argv.size is default (1)
    if (argv.size == 1)
    {
        // Run once to estimate capacity
        test::wwjson::BuildDoubleArray(json_data, argv.start, argv.items, 1);
        int estimated_size =
            (json_data.size() / 1024) + 1; // Convert to KB, round up
        argv.size = estimated_size;
    }

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::wwjson::BuildDoubleArray(json_data, argv.start, argv.items,
                                       argv.size);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --size=%d", argv.start, argv.items,
         argv.size);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (4*items)", argv.items * 4);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// Performance test for yyjson double array building
DEF_TAST(tic_number_double_yyjson, "yyjson double 数组构建性能测试")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++)
    {
        test::yyjson::BuildDoubleArray(json_data, argv.start, argv.items);
    }
    TIME_TOC;

    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (4*items)", argv.items * 4);

    // Print JSON content for single iteration
    if (argv.loop == 1)
    {
        COUT(json_data);
    }
}

// ============================================================================
// Tool case for output comparison
// ============================================================================

DEF_TOOL(tic_number_array_compare, "比较 wwjson 和 yyjson BuildIntArray 输出")
{
    test::CArgv argv;
    int test_count = 10;

    // Test uint8_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(
            wwjson_output, static_cast<uint8_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(
            yyjson_output, static_cast<uint8_t>(argv.start), test_count);
        COUT(wwjson_output);
        COUT(yyjson_output);
    }

    // Test uint16_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(
            wwjson_output, static_cast<uint16_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(
            yyjson_output, static_cast<uint16_t>(argv.start), test_count);
        COUT(wwjson_output);
        COUT(yyjson_output);
    }

    // Test uint32_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(
            wwjson_output, static_cast<uint32_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(
            yyjson_output, static_cast<uint32_t>(argv.start), test_count);
        COUT(wwjson_output);
        COUT(yyjson_output);
    }

    // Test uint64_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(
            wwjson_output, static_cast<uint64_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(
            yyjson_output, static_cast<uint64_t>(argv.start), test_count);
        COUT(wwjson_output);
        COUT(yyjson_output);
    }
}

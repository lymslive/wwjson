#include "couttast/tinytast.hpp"

#include "argv.h"
#include "relative_perf.h"

#include "wwjson.hpp"
#include "xyjson.h"
#include "yyjson.h"

#include <string>
#include <type_traits>

// Template-based BuildIntArray functions for performance testing
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

// Relative performance test classes
namespace test
{
namespace perf
{

/**
 * @brief Relative performance test for integer array building with random
 * values
 *
 * This class compares the performance between wwjson builder and yyjson API
 * when building JSON arrays of randomly generated integers.
 */
class RandomIntArray : public RelativeTimer<RandomIntArray>
{
  public:
    // Configuration
    int items;
    int seed;

    // Test data
    std::string result;
    std::vector<int> random_numbers;

    RandomIntArray(int items_count, int random_seed)
        : items(items_count), seed(random_seed)
    {
        generateRandomNumbers();
    }

    void generateRandomNumbers()
    {
        std::srand(seed);
        random_numbers.clear();
        random_numbers.reserve(items);

        for (int i = 0; i < items; i++)
        {
            // Generate random integer in int32 range
            int random_val =
                std::rand() % 2000001 - 1000000; // Range: -1000000 to 1000000
            random_numbers.push_back(random_val);
        }
    }

    void methodA()
    {
        // Using wwjson builder with random numbers
        ::wwjson::RawBuilder builder(items * 10); // Estimate capacity
        builder.BeginArray();

        for (int num : random_numbers)
        {
            builder.AddItem(num);
            builder.AddItem(-num); // Add negative counterpart
        }

        builder.EndArray();
        result = builder.MoveResult();
    }

    void methodB()
    {
        // Using yyjson API with random numbers
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_arr(doc);
        yyjson_mut_doc_set_root(doc, root);

        for (int num : random_numbers)
        {
            yyjson_mut_arr_add_int(doc, root, num);
            yyjson_mut_arr_add_int(doc, root, -num); // Add negative counterpart
        }

        char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
        if (json_str)
        {
            result = json_str;
            free(json_str);
        }
        else
        {
            result = "[]";
        }

        yyjson_mut_doc_free(doc);
    }

    bool methodVerify()
    {
        // Verify that both methods produce equivalent JSON output
        methodA();
        std::string resultA = result;
        methodB();
        std::string resultB = result;

        // Compare the JSON outputs
        return resultA == resultB;
    }
};

/**
 * @brief Relative performance test for double array building with random values
 *
 * This class compares the performance between wwjson builder and yyjson API
 * when building JSON arrays of randomly generated double precision numbers.
 * Each double is generated as f = m + 1/n where m and n are random integers,
 * and both +f and -f are added to the array.
 */
class RandomDoubleArray : public RelativeTimer<RandomDoubleArray>
{
  public:
    // Configuration
    int items;
    int seed;

    // Test data
    std::string result;
    std::vector<double> random_doubles;

    RandomDoubleArray(int items_count, int random_seed)
        : items(items_count), seed(random_seed)
    {
        generateRandomNumbers();
    }

    void generateRandomNumbers()
    {
        std::srand(seed);
        random_doubles.clear();
        random_doubles.reserve(items);

        for (int i = 0; i < items; i++)
        {
            // Generate random integers m and n in int32 range
            int m = std::rand() % 2000001 - 1000000; // Range: -1000000 to 1000000
            int n = std::rand() % 10000;

            // Generate double f = m + n/10000;
            double f = static_cast<double>(m) + n / 10000.0;
            random_doubles.push_back(f);
        }
    }

    void methodA()
    {
        // Using wwjson builder with random doubles
        ::wwjson::RawBuilder builder(items * 20); // Estimate capacity
        builder.BeginArray();

        for (double f : random_doubles)
        {
            builder.AddItem(f);
            builder.AddItem(-f); // Add negative counterpart
        }

        builder.EndArray();
        result = builder.MoveResult();
    }

    void methodB()
    {
        // Using yyjson API with random doubles
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_arr(doc);
        yyjson_mut_doc_set_root(doc, root);

        for (double f : random_doubles)
        {
            yyjson_mut_arr_add_real(doc, root, f);
            yyjson_mut_arr_add_real(doc, root, -f); // Add negative counterpart
        }

        char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
        if (json_str)
        {
            result = json_str;
            free(json_str);
        }
        else
        {
            result = "[]";
        }

        yyjson_mut_doc_free(doc);
    }

    bool methodVerify()
    {
        // Verify that both methods produce equivalent JSON output
        // For floating-point numbers, we need to handle precision differences
        methodA();
        std::string resultA = result;
        methodB();
        std::string resultB = result;

        if (resultA == resultB)
        {
            return true;
        }

        // If strings don't match, parse and compare individual values using
        // xyjson
        ::yyjson::Document docA(resultA);
        ::yyjson::Document docB(resultB);
        if (!docA.isValid() || !docB.isValid())
        {
            return false;
        }

        ::yyjson::Value rootA = docA.root();
        ::yyjson::Value rootB = docB.root();
        if (!rootA.isArray() || !rootB.isArray())
        {
            return false;
        }

        size_t lenA = rootA.size();
        size_t lenB = rootB.size();
        if (lenA != lenB)
        {
            return false;
        }

        // Compare each array element with relative tolerance for floating-point
        // precision
        const double tolerance = 1e-12;

        for (size_t i = 0; i < lenA; ++i)
        {
            double numA = rootA / i | 0.0;
            double numB = rootB / i | 0.0;
            double max_val = std::max(std::abs(numA), std::abs(numB));
            double rel_tolerance = tolerance * std::max(1.0, max_val);
            if (std::abs(numA - numB) > rel_tolerance)
            {
                return false;
            }
        }

        return true;
    }
};

} // namespace perf
} // namespace test

/**
 * @brief Performance test suite for comparing integer array serialization
 *
 * This test suite creates multiple test cases that measure the performance
 * of wwjson::RawBuilder versus yyjson's mutable document API for generating
 * JSON integer arrays of different sizes and types.
 *
 * The test cases use --start= parameter for the starting value and
 * --items= parameter for the count of integer pairs to generate.
 */

// Performance test for wwjson int8 array building
DEF_TAST(number_int8_wwjson, "wwjson int8 数组构建性能测试")
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
DEF_TAST(number_int8_yyjson, "yyjson int8 数组构建性能测试")
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
DEF_TAST(number_int16_wwjson, "wwjson int16 数组构建性能测试")
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
DEF_TAST(number_int16_yyjson, "yyjson int16 数组构建性能测试")
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
DEF_TAST(number_int32_wwjson, "wwjson int32 数组构建性能测试")
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
DEF_TAST(number_int32_yyjson, "yyjson int32 数组构建性能测试")
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
DEF_TAST(number_int64_wwjson, "wwjson int64 数组构建性能测试")
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
DEF_TAST(number_int64_yyjson, "yyjson int64 数组构建性能测试")
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

// Comparison test for wwjson vs yyjson output
DEF_TOOL(number_array_compare, "比较 wwjson 和 yyjson BuildIntArray 输出")
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
        COUT(wwjson_output, yyjson_output);
    }

    // Test uint16_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(
            wwjson_output, static_cast<uint16_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(
            yyjson_output, static_cast<uint16_t>(argv.start), test_count);
        COUT(wwjson_output, yyjson_output);
    }

    // Test uint32_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(
            wwjson_output, static_cast<uint32_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(
            yyjson_output, static_cast<uint32_t>(argv.start), test_count);
        COUT(wwjson_output, yyjson_output);
    }

    // Test uint64_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(
            wwjson_output, static_cast<uint64_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(
            yyjson_output, static_cast<uint64_t>(argv.start), test_count);
        COUT(wwjson_output, yyjson_output);
    }
}

// Performance test for wwjson float array building
DEF_TAST(number_float_wwjson, "wwjson float 数组构建性能测试")
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
DEF_TAST(number_float_yyjson, "yyjson float 数组构建性能测试")
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
DEF_TAST(number_double_wwjson, "wwjson double 数组构建性能测试")
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
DEF_TAST(number_double_yyjson, "yyjson double 数组构建性能测试")
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

// Relative performance test for random integer arrays
DEF_TAST(number_int_rel, "随机整数数组相对性能测试")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);

    test::perf::RandomIntArray tester(argv.items, argv.start);
    double ratio = tester.runAndPrint("Random Integer Array", "wwjson builder",
                                      "yyjson API", argv.loop, 10);
    DESC("Performance ratio: %.3f", ratio);
}

// Relative performance test for random double arrays
DEF_TAST(number_double_rel, "随机 double 数组相对性能测试")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);

    test::perf::RandomDoubleArray tester(argv.items, argv.start);
    double ratio = tester.runAndPrint("Random Double Array", "wwjson builder",
                                      "yyjson API", argv.loop, 10);
    DESC("Performance ratio: %.3f", ratio);
}

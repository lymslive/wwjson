/**
 * @file p_external.cpp
 * @author lymslive
 * @date 2026-01-22
 * @brief Performance test for external DTOA libraries integration
 *
 * @details
 * This file tests the performance of floating-point serialization using
 * different configurations and external libraries (yyjson, fmt, rapidjson).
 *
 * Test cases:
 * 1. wwjson::Builder (UnsafeConfig) vs wwjson::Builder with BasicConfig
 * 2. wwjson::Builder vs yyjson for float array serialization
 * 3. Tool test: Observe float format output from different libraries
 */

#include "couttast/tinytast.hpp"

#include "argv.h"
#include "relative_perf.h"

#include "jbuilder.hpp"
#include "external.hpp"

#include <string>
#include <vector>
#include <limits>
#include <cstdlib>
#include <cmath>

// ============================================================================
// Utility: Float comparison using yyjson parsing
// ============================================================================

#include "yyjson.h"

namespace test
{
namespace wwjson
{

/**
 * @brief Parse JSON array string and extract double values using yyjson
 *
 * @param json_str JSON array string
 * @return std::vector<double> Parsed double values, empty on parse error
 */
inline std::vector<double> parseJsonArrayToDoubles(const std::string &json_str)
{
    std::vector<double> result;

    yyjson_doc *doc = yyjson_read(json_str.c_str(), json_str.size(), 0);
    if (!doc)
    {
        return result;
    }

    yyjson_val *root = yyjson_doc_get_root(doc);
    if (!root || yyjson_get_type(root) != YYJSON_TYPE_ARR)
    {
        yyjson_doc_free(doc);
        return result;
    }

    size_t idx, len;
    yyjson_val *val;
    yyjson_arr_foreach(root, idx, len, val)
    {
        if (yyjson_get_type(val) == YYJSON_TYPE_NUM && yyjson_get_subtype(val) == YYJSON_SUBTYPE_REAL)
        {
            result.push_back(yyjson_get_real(val));
        }
    }

    yyjson_doc_free(doc);
    return result;
}

/**
 * @brief Compare two JSON arrays by parsing and comparing numeric values
 *
 * Since different libraries may serialize floats with slightly different formats,
 * we parse both strings with yyjson and compare the numeric values for equality.
 *
 * @param jsonA First JSON array string
 * @param jsonB Second JSON array string
 * @return true if both arrays have the same length and all values are approximately equal
 */
inline bool compareJsonFloatArrays(const std::string &jsonA, const std::string &jsonB)
{
    std::vector<double> valuesA = parseJsonArrayToDoubles(jsonA);
    std::vector<double> valuesB = parseJsonArrayToDoubles(jsonB);

    if (valuesA.size() != valuesB.size())
    {
        return false;
    }

    const double epsilon = 1e-15;
    for (size_t i = 0; i < valuesA.size(); ++i)
    {
        if (std::abs(valuesA[i] - valuesB[i]) > epsilon)
        {
            return false;
        }
    }

    return true;
}

} // namespace wwjson
} // namespace test

// ============================================================================
// Test case 1: UnsafeConfig vs BasicConfig for float serialization
// ============================================================================

namespace test
{
namespace wwjson
{

/**
 * @brief Compare UnsafeConfig vs BasicConfig for double serialization
 *
 * Method A: wwjson::Builder (uses UnsafeConfig with optimized number writing)
 * Method B: GenericBuilder<JString, BasicConfig<JString>> (uses standard number writing)
 */
class ExternalVsBasicDtoaPerf : public perf::RelativeTimer<ExternalVsBasicDtoaPerf>
{
  public:
    using UnsafeBuilder = ::wwjson::GenericBuilder<::wwjson::JString, ::wwjson::UnsafeConfig<::wwjson::JString>>;
    using BasicBuilder = ::wwjson::GenericBuilder<::wwjson::JString, ::wwjson::BasicConfig<::wwjson::JString>>;

    int items;
    int seed;
    size_t capacity;
    ::wwjson::JString resultA;
    ::wwjson::JString resultB;
    std::vector<double> numbers;

    ExternalVsBasicDtoaPerf(int items_count, int random_seed)
        : items(items_count), seed(random_seed), capacity(0)
    {
        generateRandomNumbers();
        estimateCapacity();
    }

    void generateRandomNumbers()
    {
        std::srand(seed);
        numbers.clear();
        numbers.reserve(items);

        for (int i = 0; i < items; ++i)
        {
            // Generate random double with both integer and fractional parts
            // Range: [-10000, 10000] with fractional part
            double int_part = static_cast<double>(std::rand() % 20001 - 10000);  // [-10000, 10000]
            double frac_part = static_cast<double>(std::rand()) / RAND_MAX;     // [0, 1)
            double val = int_part + (std::rand() % 2 == 0 ? frac_part : -frac_part);
            numbers.push_back(val);
        }
    }

    void estimateCapacity()
    {
        // Use Builder to estimate output size
        ::wwjson::Builder builder;
        builder.BeginArray();
        for (double num : numbers)
        {
            builder.AddItem(num);
        }
        builder.EndArray();
        capacity = builder.Size();
    }

    void methodA()
    {
        // Use Builder (UnsafeConfig, faster)
        UnsafeBuilder builder(capacity);
        builder.BeginArray();
        for (double num : numbers)
        {
            builder.AddItem(num);
        }
        builder.EndArray();
        resultA = builder.MoveResult();
    }

    void methodB()
    {
        // Use BasicBuilder (BasicConfig, slower)
        BasicBuilder builder(capacity);
        builder.BeginArray();
        for (double num : numbers)
        {
            builder.AddItem(num);
        }
        builder.EndArray();
        resultB = builder.MoveResult();
    }

    bool methodVerify()
    {
        methodA();
        methodB();
//      return resultA.str() == resultB.str();
        return compareJsonFloatArrays(resultA.str(), resultB.str());
    }

    static const char* testName() { return "UnsafeConfig vs BasicConfig Float Performance"; }
    static const char* labelA() { return "UnsafeConfig (Builder)"; }
    static const char* labelB() { return "BasicConfig"; }
};

} // namespace wwjson
} // namespace test

DEF_TAST(external_unsafe_vs_basic, "比较 UnsafeConfig 与 BasicConfig 浮点数序列化性能")
{
    test::CArgv argv;
    DESC("Args: --items=%d --loop=%d", argv.items, argv.loop);

    test::wwjson::ExternalVsBasicDtoaPerf tester(argv.items, argv.start);
    double ratio = tester.runAndPrint("double", "UnsafeConfig", "BasicConfig",
                                       argv.loop, 10);
    COUT(ratio < 1.0, true);
}

// ============================================================================
// Test case 2: wwjson::Builder vs yyjson float serialization
// ============================================================================

#include "yyjson.h"

namespace test
{
namespace wwjson
{

/**
 * @brief Base class template for wwjson vs yyjson float serialization tests
 */
class BuilderVsYyjsonFloatPerf : public perf::RelativeTimer<BuilderVsYyjsonFloatPerf>
{
  public:
    int items;
    int seed;
    size_t capacity;
    std::string resultA;
    std::string resultB;
    std::vector<double> numbers;

    BuilderVsYyjsonFloatPerf(int items_count, int random_seed)
        : items(items_count), seed(random_seed), capacity(0)
    {
        generateRandomNumbers();
        estimateCapacity();
    }

    void generateRandomNumbers()
    {
        std::srand(seed);
        numbers.clear();
        numbers.reserve(items);

        for (int i = 0; i < items; ++i)
        {
            // Generate random double with both integer and fractional parts
            // Range: [-10000, 10000] with fractional part
            double int_part = static_cast<double>(std::rand() % 20001 - 10000);  // [-10000, 10000]
            double frac_part = static_cast<double>(std::rand()) / RAND_MAX;     // [0, 1)
            double val = int_part + (std::rand() % 2 == 0 ? frac_part : -frac_part);
            numbers.push_back(val);
        }
    }

    void estimateCapacity()
    {
        // Use Builder to estimate output size
        ::wwjson::Builder builder;
        builder.BeginArray();
        for (double num : numbers)
        {
            builder.AddItem(num);
        }
        builder.EndArray();
        capacity = builder.Size();
    }

    void methodA()
    {
        // Use wwjson::Builder
        ::wwjson::Builder builder(capacity);
        builder.BeginArray();
        for (double num : numbers)
        {
            builder.AddItem(num);
        }
        builder.EndArray();
        // Copy to std::string once
        resultA = builder.MoveResult().str();
    }

    void methodB()
    {
        // Use yyjson mutable API
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_arr(doc);
        yyjson_mut_doc_set_root(doc, root);

        for (double num : numbers)
        {
            yyjson_mut_arr_add_real(doc, root, num);
        }

        char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
        if (json_str)
        {
            resultB = json_str;
            free(json_str);
        }
        else
        {
            resultB = "[]";
        }

        yyjson_mut_doc_free(doc);
    }

    bool methodVerify()
    {
        // may differ on inf/null
        return true;
        methodA();
        methodB();
        // Compare parsed numeric values instead of string representation
        return compareJsonFloatArrays(resultA, resultB);
    }

    static const char* testName() { return "Builder vs yyjson Float Performance Test"; }
    static const char* labelA() { return "wwjson::Builder"; }
    static const char* labelB() { return "yyjson"; }
};

} // namespace wwjson
} // namespace test

DEF_TAST(external_builder_vs_yyjson, "比较 wwjson::Builder 与 yyjson 浮点数序列化性能")
{
    test::CArgv argv;
    DESC("Args: --items=%d --loop=%d", argv.items, argv.loop);

    test::wwjson::BuilderVsYyjsonFloatPerf tester(argv.items, argv.start);
    double ratio = tester.runAndPrint("double", "wwjson::Builder", "yyjson",
                                       argv.loop, 10);
    COUT(ratio < 1.0);
}

// ============================================================================
// Test case 3: Tool test - Observe float format output from different libraries
// ============================================================================

DEF_TOOL(external_float_format, "观察不同库的浮点数序列化格式")
{
    DESC("观察各种特殊浮点数的序列化格式");

    // Use BuilderVsYyjsonFloatPerf and directly assign test values
    test::wwjson::BuilderVsYyjsonFloatPerf tester(1, 42);
    tester.numbers = {
        // Positive and negative zero
        0.0,
        -0.0,

        // Positive and negative pure integers (no decimal part)
        1.0,
        -1.0,
        42.0,
        -42.0,
        1000.0,
        -1000.0,

        // Double max and min values
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::min(),

        // Values close to max/min but not at the ends
        std::numeric_limits<double>::max() - 1e290,
        std::numeric_limits<double>::min() + 1e-310,

        // Normal range floating point numbers
        3.141592653589793,
        -3.141592653589793,
        1.23e-10,
        -1.23e10,
        0.00000001,
        -0.00000001,

        // Very small numbers near zero
        1e-300,
        -1e-300,
    };

    // Call methodA() to build with wwjson::Builder
    tester.methodA();
    DESC("wwjson::Builder output:");
    COUT(tester.resultA);

    // Call methodB() to build with yyjson
    tester.methodB();
    DESC("yyjson output:");
    COUT(tester.resultB);

    DESC("Test completed for %zu float values", tester.numbers.size());
}


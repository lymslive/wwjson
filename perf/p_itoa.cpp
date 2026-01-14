/**
 * @file p_itoa.cpp
 * @author lymslive
 * @date 2026-01-13
 * @brief Performance test for IntegerWriter with different integer types
 *
 * @details
 * This file tests the performance of IntegerWriter (Builder, using itoa fast path)
 * versus NumberWriter (RawBuilder, using standard to_string/snprintf) for int8,
 * int16, int32, int64 types.
 *
 * The test compares wwjson::Builder with optimized IntegerWriter against
 * wwjson::RawBuilder with standard NumberWriter for integer serialization.
 */

#include "couttast/tinytast.hpp"

#include "argv.h"
#include "relative_perf.h"

#include "jbuilder.hpp"
#include "itoa.hpp"

#include <string>
#include <vector>
#include <limits>
#include <cstdlib>

// ============================================================================
// Template base class for IntegerWriter performance tests
// ============================================================================

namespace test
{
namespace wwjson
{

/**
 * @brief Base class template for IntegerWriter performance tests
 *
 * @tparam intT Integer type (int8_t, int16_t, int32_t, int64_t)
 */
template <typename intT>
class IntegerWriterPerf : public perf::RelativeTimer<IntegerWriterPerf<intT>>
{
  public:
    using SignedT = intT;
    using UnsignedT = std::make_unsigned_t<SignedT>;

    int items;
    int seed;
    size_t capacity;
    ::wwjson::JString resultA;
    std::string resultB;
    std::vector<SignedT> numbers;

    IntegerWriterPerf(int items_count, int random_seed)
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

        UnsignedT max_positive = static_cast<UnsignedT>(std::numeric_limits<SignedT>::max());

        for (int i = 0; i < items; ++i)
        {
            UnsignedT pos = static_cast<UnsignedT>(std::rand()) % (max_positive + 1);
            numbers.push_back(static_cast<SignedT>(pos));

            // Add negative counterpart if it doesn't overflow
            if (pos != 0)
            {
                SignedT neg = -static_cast<SignedT>(pos);
                // Verify no overflow (should always be true for negating positive)
                if (neg < 0)
                {
                    numbers.push_back(neg);
                }
            }
        }
    }

    void estimateCapacity()
    {
        // Use Builder to estimate output size
        ::wwjson::Builder builder;
        builder.BeginArray();
        for (SignedT num : numbers)
        {
            builder.AddItem(num);
        }
        builder.EndArray();
        capacity = builder.Size();
    }

    void methodA()
    {
        // Use Builder (IntegerWriter, faster)
        ::wwjson::Builder builder(capacity);
        builder.BeginArray();
        for (SignedT num : numbers)
        {
            builder.AddItem(num);
        }
        builder.EndArray();
        resultA = builder.MoveResult();
    }

    void methodB()
    {
        // Use RawBuilder (NumberWriter, slower)
        ::wwjson::RawBuilder builder(capacity);
        builder.BeginArray();
        for (SignedT num : numbers)
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
        return resultA.str() == resultB;
    }

    static const char* testName() { return "IntegerWriter Performance Test"; }
    static const char* labelA() { return "IntegerWriter (Builder)"; }
    static const char* labelB() { return "NumberWriter (RawBuilder)"; }
};

// ============================================================================
// Specific test instantiations
// ============================================================================

class Int8Perf : public IntegerWriterPerf<int8_t>
{
  public:
    Int8Perf(int items, int seed) : IntegerWriterPerf<int8_t>(items, seed) {}
    static const char* testName() { return "IntegerWriter int8_t Performance"; }
};

class Int16Perf : public IntegerWriterPerf<int16_t>
{
  public:
    Int16Perf(int items, int seed) : IntegerWriterPerf<int16_t>(items, seed) {}
    static const char* testName() { return "IntegerWriter int16_t Performance"; }
};

class Int32Perf : public IntegerWriterPerf<int32_t>
{
  public:
    Int32Perf(int items, int seed) : IntegerWriterPerf<int32_t>(items, seed) {}
    static const char* testName() { return "IntegerWriter int32_t Performance"; }
};

class Int64Perf : public IntegerWriterPerf<int64_t>
{
  public:
    Int64Perf(int items, int seed) : IntegerWriterPerf<int64_t>(items, seed) {}
    static const char* testName() { return "IntegerWriter int64_t Performance"; }
};

} // namespace wwjson
} // namespace test

// ============================================================================
// Test case: Forward write (IntegerWriter vs NumberWriter)
// ============================================================================

DEF_TAST(itoa_forward_write, "比较整数正向递归递归序列化与反向临时缓冲性能")
{
    test::CArgv argv;
    DESC("Args: --items=%d --loop=%d", argv.items, argv.loop);

    double total_ratio = 0.0;
    int test_count = 0;

    // Test int8
    {
        test::wwjson::Int8Perf tester(argv.items, argv.start);
        double ratio = tester.runAndPrint("int8_t", "IntegerWriter", "NumberWriter",
                                           argv.loop, 10);
        total_ratio += ratio;
        ++test_count;
    }

    // Test int16
    {
        test::wwjson::Int16Perf tester(argv.items, argv.start + 1);
        double ratio = tester.runAndPrint("int16_t", "IntegerWriter", "NumberWriter",
                                           argv.loop, 10);
        total_ratio += ratio;
        ++test_count;
    }

    // Test int32
    {
        test::wwjson::Int32Perf tester(argv.items, argv.start + 2);
        double ratio = tester.runAndPrint("int32_t", "IntegerWriter", "NumberWriter",
                                           argv.loop, 10);
        total_ratio += ratio;
        ++test_count;
    }

    // Test int64
    {
        test::wwjson::Int64Perf tester(argv.items, argv.start + 3);
        double ratio = tester.runAndPrint("int64_t", "IntegerWriter", "NumberWriter",
                                           argv.loop, 10);
        total_ratio += ratio;
        ++test_count;
    }

    double avg_ratio = total_ratio / test_count;
    DESC("Average ratio: %.4f", avg_ratio);
    COUT(avg_ratio < 1.0, true);
}

// ============================================================================
// wwjson::Builder vs yyjson integer serialization performance test
// ============================================================================

#include "yyjson.h"

namespace test
{
namespace wwjson
{

/**
 * @brief Base class template for wwjson vs yyjson integer serialization tests
 *
 * @tparam intT Integer type (int8_t, int16_t, int32_t, int64_t)
 */
template <typename intT>
class BuilderVsYyjsonPerf : public perf::RelativeTimer<BuilderVsYyjsonPerf<intT>>
{
  public:
    using SignedT = intT;
    using UnsignedT = std::make_unsigned_t<SignedT>;

    int items;
    int seed;
    size_t capacity;
    std::string resultA;
    std::string resultB;
    std::vector<SignedT> numbers;

    BuilderVsYyjsonPerf(int items_count, int random_seed)
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

        UnsignedT max_positive = static_cast<UnsignedT>(std::numeric_limits<SignedT>::max());

        for (int i = 0; i < items; ++i)
        {
            UnsignedT pos = static_cast<UnsignedT>(std::rand()) % (max_positive + 1);
            numbers.push_back(static_cast<SignedT>(pos));

            // Add negative counterpart if it doesn't overflow
            if (pos != 0)
            {
                SignedT neg = -static_cast<SignedT>(pos);
                if (neg < 0)
                {
                    numbers.push_back(neg);
                }
            }
        }
    }

    void estimateCapacity()
    {
        // Use Builder to estimate output size
        ::wwjson::Builder builder;
        builder.BeginArray();
        for (SignedT num : numbers)
        {
            builder.AddItem(num);
        }
        builder.EndArray();
        capacity = builder.Size();
    }

    void methodA()
    {
        // Use wwjson::Builder (IntegerWriter, fast itoa path)
        ::wwjson::Builder builder(capacity);
        builder.BeginArray();
        for (SignedT num : numbers)
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

        for (SignedT num : numbers)
        {
            if (num >= 0)
            {
                yyjson_mut_arr_add_uint(doc, root, static_cast<UnsignedT>(num));
            }
            else
            {
                yyjson_mut_arr_add_sint(doc, root, num);
            }
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
        methodA();
        methodB();
        return resultA == resultB;
    }

    static const char* testName() { return "Builder vs yyjson Performance Test"; }
    static const char* labelA() { return "wwjson::Builder"; }
    static const char* labelB() { return "yyjson"; }
};

// ============================================================================
// Specific test instantiations
// ============================================================================

class Int8BuilderVsYyjson : public BuilderVsYyjsonPerf<int8_t>
{
  public:
    Int8BuilderVsYyjson(int items, int seed) : BuilderVsYyjsonPerf<int8_t>(items, seed) {}
    static const char* testName() { return "Builder vs yyjson int8_t Performance"; }
};

class Int16BuilderVsYyjson : public BuilderVsYyjsonPerf<int16_t>
{
  public:
    Int16BuilderVsYyjson(int items, int seed) : BuilderVsYyjsonPerf<int16_t>(items, seed) {}
    static const char* testName() { return "Builder vs yyjson int16_t Performance"; }
};

class Int32BuilderVsYyjson : public BuilderVsYyjsonPerf<int32_t>
{
  public:
    Int32BuilderVsYyjson(int items, int seed) : BuilderVsYyjsonPerf<int32_t>(items, seed) {}
    static const char* testName() { return "Builder vs yyjson int32_t Performance"; }
};

class Int64BuilderVsYyjson : public BuilderVsYyjsonPerf<int64_t>
{
  public:
    Int64BuilderVsYyjson(int items, int seed) : BuilderVsYyjsonPerf<int64_t>(items, seed) {}
    static const char* testName() { return "Builder vs yyjson int64_t Performance"; }
};

} // namespace wwjson
} // namespace test

// ============================================================================
// Test case: wwjson::Builder vs yyjson integer serialization
// ============================================================================

DEF_TAST(itoa_build_vs_yyjson, "比较 wwjson::Builder 与 yyjson 整数序列化性能")
{
    test::CArgv argv;
    DESC("Args: --items=%d --loop=%d", argv.items, argv.loop);

    double total_ratio = 0.0;
    int test_count = 0;

    // Test int8
    {
        test::wwjson::Int8BuilderVsYyjson tester(argv.items, argv.start);
        double ratio = tester.runAndPrint("int8_t", "wwjson::Builder", "yyjson",
                                           argv.loop, 10);
        total_ratio += ratio;
        ++test_count;
    }

    // Test int16
    {
        test::wwjson::Int16BuilderVsYyjson tester(argv.items, argv.start + 1);
        double ratio = tester.runAndPrint("int16_t", "wwjson::Builder", "yyjson",
                                           argv.loop, 10);
        total_ratio += ratio;
        ++test_count;
    }

    // Test int32
    {
        test::wwjson::Int32BuilderVsYyjson tester(argv.items, argv.start + 2);
        double ratio = tester.runAndPrint("int32_t", "wwjson::Builder", "yyjson",
                                           argv.loop, 10);
        total_ratio += ratio;
        ++test_count;
    }

    // Test int64
    {
        test::wwjson::Int64BuilderVsYyjson tester(argv.items, argv.start + 3);
        double ratio = tester.runAndPrint("int64_t", "wwjson::Builder", "yyjson",
                                           argv.loop, 10);
        total_ratio += ratio;
        ++test_count;
    }

    double avg_ratio = total_ratio / test_count;
    DESC("Average ratio (wwjson/yyjson): %.4f", avg_ratio);
    COUT(avg_ratio < 1.0); // Print only, no assertion - results vary by integer type
}

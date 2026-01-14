#include "couttast/tinytast.hpp"

#include "argv.h"
#include "relative_perf.h"

#include "wwjson.hpp"
#include "jbuilder.hpp"

#include "xyjson.h"

#include <string>
#include <vector>
#include <memory>

namespace test::perf
{

// Relative performance test for integer array building with random values
class RandomIntArray : public RelativeTimer<RandomIntArray>
{
  public:
    int items;
    int seed;
    size_t capacity;

    std::string result;
    std::vector<int> random_numbers;

    RandomIntArray(int items_count, int random_seed)
        : items(items_count), seed(random_seed), capacity(0)
    {
        generateRandomNumbers();
        estimateCapacity();
    }

    void generateRandomNumbers()
    {
        std::srand(seed);
        random_numbers.clear();
        random_numbers.reserve(items);

        for (int i = 0; i < items; i++)
        {
            int random_val =
                std::rand() % 2000001 - 1000000;
            random_numbers.push_back(random_val);
        }
    }

    void estimateCapacity()
    {
        methodA();
        capacity = result.size();
    }

    void methodA()
    {
        ::wwjson::RawBuilder builder(capacity);
        builder.BeginArray();

        for (int num : random_numbers)
        {
            builder.AddItem(num);
            builder.AddItem(-num);
        }

        builder.EndArray();
        result = builder.MoveResult();
    }

    void methodB()
    {
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_arr(doc);
        yyjson_mut_doc_set_root(doc, root);

        for (int num : random_numbers)
        {
            yyjson_mut_arr_add_int(doc, root, num);
            yyjson_mut_arr_add_int(doc, root, -num);
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
        methodA();
        std::string resultA = result;
        methodB();
        std::string resultB = result;

        return resultA == resultB;
    }
};

// Relative performance test for double array building with random values
class RandomDoubleArray : public RelativeTimer<RandomDoubleArray>
{
  public:
    int items;
    int seed;
    size_t capacity;

    std::string result;
    std::vector<double> random_doubles;

    RandomDoubleArray(int items_count, int random_seed)
        : items(items_count), seed(random_seed), capacity(0)
    {
        generateRandomNumbers();
        estimateCapacity();
    }

    void generateRandomNumbers()
    {
        std::srand(seed);
        random_doubles.clear();
        random_doubles.reserve(items);

        for (int i = 0; i < items; i++)
        {
            int m = std::rand() % 2000001 - 1000000;
            int n = std::rand() % 10000;
            double f = static_cast<double>(m) + n / 10000.0;
            random_doubles.push_back(f);
        }
    }

    void estimateCapacity()
    {
        methodA();
        capacity = result.size();
    }

    void methodA()
    {
        ::wwjson::RawBuilder builder(capacity);
        builder.BeginArray();

        for (double f : random_doubles)
        {
            builder.AddItem(f);
            builder.AddItem(-f);
        }

        builder.EndArray();
        result = builder.MoveResult();
    }

    void methodB()
    {
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_arr(doc);
        yyjson_mut_doc_set_root(doc, root);

        for (double f : random_doubles)
        {
            yyjson_mut_arr_add_real(doc, root, f);
            yyjson_mut_arr_add_real(doc, root, -f);
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
        methodA();
        std::string resultA = result;
        methodB();
        std::string resultB = result;

        if (resultA == resultB)
        {
            return true;
        }

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

// Relative performance test: JString (Builder) vs std::string (RawBuilder) for integers
// FIX: Swapped methodA/methodB to fix the inverted ratio issue
class RandomIntJStringRel : public RelativeTimer<RandomIntJStringRel>
{
  public:
    int items;
    int seed;
    size_t capacity;
    std::string raw_result;
    std::string jstring_result;
    std::vector<int> random_numbers;

    RandomIntJStringRel(int items_count, int random_seed)
        : items(items_count), seed(random_seed), capacity(0)
    {
        generateRandomNumbers();
        estimateCapacity();
    }

    void generateRandomNumbers()
    {
        std::srand(seed);
        random_numbers.clear();
        random_numbers.reserve(items);

        for (int i = 0; i < items; i++)
        {
            int random_val =
                std::rand() % 2000001 - 1000000;
            random_numbers.push_back(random_val);
        }
    }

    void estimateCapacity()
    {
        ::wwjson::RawBuilder builder(items * 10);
        builder.BeginArray();
        for (int num : random_numbers)
        {
            builder.AddItem(num);
            builder.AddItem(-num);
        }
        builder.EndArray();
        capacity = builder.MoveResult().size();
    }

    // Method A: Builder with JString (faster, should be methodA)
    void methodA()
    {
        ::wwjson::Builder builder(capacity);
        builder.BeginArray();

        for (int num : random_numbers)
        {
            builder.AddItem(num);
            builder.AddItem(-num);
        }

        builder.EndArray();
        jstring_result = builder.MoveResult();
    }

    // Method B: RawBuilder with std::string (slower, should be methodB)
    void methodB()
    {
        ::wwjson::RawBuilder builder(capacity);
        builder.BeginArray();

        for (int num : random_numbers)
        {
            builder.AddItem(num);
            builder.AddItem(-num);
        }

        builder.EndArray();
        raw_result = builder.MoveResult();
    }

    bool methodVerify()
    {
        methodA();
        methodB();
        return raw_result == jstring_result;
    }

    static const char* testName() { return "Random Int JString Relative Test"; }
    static const char* labelA() { return "JString"; }
    static const char* labelB() { return "std::string"; }
};

// Relative performance test: KString (FastBuilder) vs std::string (RawBuilder) for integers
// FIX: Swapped methodA/methodB to fix the inverted ratio issue
class RandomIntKStringRel : public RelativeTimer<RandomIntKStringRel>
{
  public:
    int items;
    int seed;
    size_t capacity;
    std::string raw_result;
    std::string kstring_result;
    std::vector<int> random_numbers;

    RandomIntKStringRel(int items_count, int random_seed)
        : items(items_count), seed(random_seed), capacity(0)
    {
        generateRandomNumbers();
        estimateCapacity();
    }

    void generateRandomNumbers()
    {
        std::srand(seed);
        random_numbers.clear();
        random_numbers.reserve(items);

        for (int i = 0; i < items; i++)
        {
            int random_val =
                std::rand() % 2000001 - 1000000;
            random_numbers.push_back(random_val);
        }
    }

    void estimateCapacity()
    {
        methodB();
        capacity = raw_result.size();
    }

    // Method A: FastBuilder with KString (faster, should be methodA)
    void methodA()
    {
        ::wwjson::FastBuilder builder(capacity);
        builder.BeginArray();

        for (int num : random_numbers)
        {
            builder.AddItem(num);
            builder.AddItem(-num);
        }

        builder.EndArray();
        kstring_result = builder.MoveResult();
    }

    // Method B: RawBuilder with std::string (slower, should be methodB)
    void methodB()
    {
        ::wwjson::RawBuilder builder(capacity);
        builder.BeginArray();

        for (int num : random_numbers)
        {
            builder.AddItem(num);
            builder.AddItem(-num);
        }

        builder.EndArray();
        raw_result = builder.MoveResult();
    }

    bool methodVerify()
    {
        methodA();
        methodB();
        return raw_result == kstring_result;
    }

    static const char* testName() { return "Random Int KString Relative Test"; }
    static const char* labelA() { return "KString"; }
    static const char* labelB() { return "std::string"; }
};

// Relative performance test: JString (Builder) vs std::string (RawBuilder) for doubles
// FIX: Swapped methodA/methodB to fix the inverted ratio issue
class RandomDoubleJStringRel : public RelativeTimer<RandomDoubleJStringRel>
{
  public:
    int items;
    int seed;
    size_t capacity;
    std::string raw_result;
    std::string jstring_result;
    std::vector<double> random_doubles;

    RandomDoubleJStringRel(int items_count, int random_seed)
        : items(items_count), seed(random_seed), capacity(0)
    {
        generateRandomNumbers();
        estimateCapacity();
    }

    void generateRandomNumbers()
    {
        std::srand(seed);
        random_doubles.clear();
        random_doubles.reserve(items);

        for (int i = 0; i < items; i++)
        {
            int m = std::rand() % 2000001 - 1000000;
            int n = std::rand() % 10000;
            double f = static_cast<double>(m) + n / 10000.0;
            random_doubles.push_back(f);
        }
    }

    void estimateCapacity()
    {
        methodB();
        capacity = raw_result.size();
    }

    // Method A: Builder with JString (faster, should be methodA)
    void methodA()
    {
        ::wwjson::Builder builder(capacity);
        builder.BeginArray();

        for (double f : random_doubles)
        {
            builder.AddItem(f);
            builder.AddItem(-f);
        }

        builder.EndArray();
        jstring_result = builder.MoveResult();
    }

    // Method B: RawBuilder with std::string (slower, should be methodB)
    void methodB()
    {
        ::wwjson::RawBuilder builder(capacity);
        builder.BeginArray();

        for (double f : random_doubles)
        {
            builder.AddItem(f);
            builder.AddItem(-f);
        }

        builder.EndArray();
        raw_result = builder.MoveResult();
    }

    bool methodVerify()
    {
        methodA();
        methodB();
        return raw_result == jstring_result;
    }

    static const char* testName() { return "Random Double JString Relative Test"; }
    static const char* labelA() { return "JString"; }
    static const char* labelB() { return "std::string"; }
};

// Relative performance test: KString (FastBuilder) vs std::string (RawBuilder) for doubles
// FIX: Swapped methodA/methodB to fix the inverted ratio issue
class RandomDoubleKStringRel : public RelativeTimer<RandomDoubleKStringRel>
{
  public:
    int items;
    int seed;
    size_t capacity;
    std::string raw_result;
    std::string kstring_result;
    std::vector<double> random_doubles;

    RandomDoubleKStringRel(int items_count, int random_seed)
        : items(items_count), seed(random_seed), capacity(0)
    {
        generateRandomNumbers();
        estimateCapacity();
    }

    void generateRandomNumbers()
    {
        std::srand(seed);
        random_doubles.clear();
        random_doubles.reserve(items);

        for (int i = 0; i < items; i++)
        {
            int m = std::rand() % 2000001 - 1000000;
            int n = std::rand() % 10000;
            double f = static_cast<double>(m) + n / 10000.0;
            random_doubles.push_back(f);
        }
    }

    void estimateCapacity()
    {
        methodB();
        capacity = raw_result.size();
    }

    // Method A: FastBuilder with KString (faster, should be methodA)
    void methodA()
    {
        ::wwjson::FastBuilder builder(capacity);
        builder.BeginArray();

        for (double f : random_doubles)
        {
            builder.AddItem(f);
            builder.AddItem(-f);
        }

        builder.EndArray();
        kstring_result = builder.MoveResult();
    }

    // Method B: RawBuilder with std::string (slower, should be methodB)
    void methodB()
    {
        ::wwjson::RawBuilder builder(capacity);
        builder.BeginArray();

        for (double f : random_doubles)
        {
            builder.AddItem(f);
            builder.AddItem(-f);
        }

        builder.EndArray();
        raw_result = builder.MoveResult();
    }

    bool methodVerify()
    {
        methodA();
        methodB();
        return raw_result == kstring_result;
    }

    static const char* testName() { return "Random Double KString Relative Test"; }
    static const char* labelA() { return "KString"; }
    static const char* labelB() { return "std::string"; }
};

} // namespace test

/* ============================================================ */
/* Relative performance tests and utility tools */

DEF_TAST(number_int_rel, "随机整数数组相对性能测试")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);

    // Test 1: wwjson vs yyjson
    test::perf::RandomIntArray tester1(argv.items, argv.start);
    double ratio1 = tester1.runAndPrint("wwjson vs yyjson", "wwjson builder",
                                        "yyjson API", argv.loop, 10);
    COUT(ratio1 < 1.0);

    // Test 2: JString vs std::string
    // FIX: Now methodA=JString (faster), methodB=std::string (slower)
    // So ratio = timeA/timeB < 1.0 when JString is faster
    test::perf::RandomIntJStringRel tester2(argv.items, argv.start);
    double ratio2 = tester2.runAndPrint("JString vs std::string", "JString",
                                        "std::string", argv.loop, 10);
    COUT(ratio2 < 1.0, true);

    // Test 3: KString vs std::string
    // FIX: Now methodA=KString (faster), methodB=std::string (slower)
    // So ratio = timeA/timeB < 1.0 when KString is faster
    test::perf::RandomIntKStringRel tester3(argv.items, argv.start);
    double ratio3 = tester3.runAndPrint("KString vs std::string", "KString",
                                        "std::string", argv.loop, 10);
    COUT(ratio3 < 1.0, true);
}

DEF_TAST(number_double_rel, "随机 double 数组相对性能测试")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);

    // Test 1: wwjson vs yyjson
    test::perf::RandomDoubleArray tester1(argv.items, argv.start);
    double ratio1 = tester1.runAndPrint("wwjson vs yyjson", "wwjson builder",
                                        "yyjson API", argv.loop, 10);
    COUT(ratio1 < 1.05);

    // Test 2: JString vs std::string
    // FIX: Now methodA=JString (faster), methodB=std::string (slower)
    // So ratio = timeA/timeB < 1.0 when JString is faster
    test::perf::RandomDoubleJStringRel tester2(argv.items, argv.start);
    double ratio2 = tester2.runAndPrint("JString vs std::string", "JString",
                                        "std::string", argv.loop, 10);
    COUT(ratio2 < 1.05, true);

    // Test 3: KString vs std::string
    // FIX: Now methodA=KString (faster), methodB=std::string (slower)
    // So ratio = timeA/timeB < 1.0 when KString is faster
    test::perf::RandomDoubleKStringRel tester3(argv.items, argv.start);
    double ratio3 = tester3.runAndPrint("KString vs std::string", "KString",
                                        "std::string", argv.loop, 10);
    COUT(ratio3 < 1.05, true);
}

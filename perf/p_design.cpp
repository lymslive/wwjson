#include "couttast/tinytast.hpp"

#include "argv.h"
#include "relative_perf.h"

#include "wwjson.hpp"

#include <charconv>
#include <cmath>
#include <cstdio>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace test
{
namespace perf
{

// ========== 测试1: 小整数优化验证 ==========

/**
 * @brief 小整数优化验证测试
 * 对比 NumberWriter::WriteSmall vs std::to_chars
 */
class SmallIntOptimizationTest : public RelativeTimer<SmallIntOptimizationTest>
{
  public:
    int items;
    uint32_t seed;
    std::string result;
    std::vector<uint32_t> test_numbers;

    SmallIntOptimizationTest(int count, uint32_t s) : items(count), seed(s)
    {
        generateTestNumbers();
    }

    void generateTestNumbers()
    {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<uint32_t> dis(0, 9999);

        test_numbers.clear();
        test_numbers.reserve(items);
        for (int i = 0; i < items; ++i)
        {
            test_numbers.push_back(dis(gen));
        }
    }

    void methodA()
    {
        // 方法A: 调用 WriteUnsigned（内部使用 WriteSmall 优化）
        result.clear();
        for (uint32_t num : test_numbers)
        {
            wwjson::NumberWriter<std::string>::WriteUnsigned(result, num);
        }
    }

    void methodB()
    {
        // 方法B: 直接调用 std::to_chars，封装为相同接口
        result.clear();
        char buffer[12];
        for (uint32_t num : test_numbers)
        {
            auto [ptr, ec] =
                std::to_chars(buffer, buffer + sizeof(buffer), num);
            result.append(buffer, ptr - buffer);
        }
    }

    bool methodVerify()
    {
        // 验证两个方法产生相同的输出
        methodA();
        std::string resultA = result;
        methodB();
        std::string resultB = result;

        // 对于整数，输出应该完全一致
        return resultA == resultB;
    }
};

// ========== 测试2: 大整数优化验证 ==========

/**
 * @brief 大整数优化验证测试
 * 对比 NumberWriter::WriteUnsigned vs std::to_chars
 * 测试大整数（>9999）的序列化性能
 */
class LargeIntOptimizationTest : public RelativeTimer<LargeIntOptimizationTest>
{
  public:
    int items;
    uint32_t seed;
    std::string result;
    std::vector<uint32_t> test_numbers;

    LargeIntOptimizationTest(int count, uint32_t s) : items(count), seed(s)
    {
        generateTestNumbers();
    }

    void generateTestNumbers()
    {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<uint32_t> dis(10000, UINT32_MAX);

        test_numbers.clear();
        test_numbers.reserve(items);
        for (int i = 0; i < items; ++i)
        {
            test_numbers.push_back(dis(gen));
        }
    }

    void methodA()
    {
        // 方法A: 调用当前实现 WriteUnsigned（包含大整数路径）
        result.clear();
        for (uint32_t num : test_numbers)
        {
            wwjson::NumberWriter<std::string>::WriteUnsigned(result, num);
        }
    }

    void methodB()
    {
        // 方法B: 直接调用 std::to_chars，封装为相同接口
        result.clear();
        char buffer[12];
        for (uint32_t num : test_numbers)
        {
            auto [ptr, ec] =
                std::to_chars(buffer, buffer + sizeof(buffer), num);
            result.append(buffer, ptr - buffer);
        }
    }

    bool methodVerify()
    {
        // 验证两个方法产生相同的输出
        methodA();
        std::string resultA = result;
        methodB();
        std::string resultB = result;

        // 对于大整数，输出应该完全一致
        return resultA == resultB;
    }
};

// ========== 测试3: 小范围浮点数优化验证 ==========

/**
 * @brief 小范围浮点数优化验证测试
 * 对比 NumberWriter::WriteSmall(double) vs std::to_chars/snprintf
 */
class SmallFloatOptimizationTest
    : public RelativeTimer<SmallFloatOptimizationTest>
{
  public:
    int items;
    uint32_t seed;
    std::string result;
    std::vector<double> test_numbers;

    SmallFloatOptimizationTest(int count, uint32_t s) : items(count), seed(s)
    {
        generateTestNumbers();
    }

    void generateTestNumbers()
    {
        std::mt19937 gen(seed);
        // std::uniform_real_distribution<double> dis(0.0, 9999.9999);
        std::uniform_int_distribution<uint32_t> dis(0, 9999);

        test_numbers.clear();
        test_numbers.reserve(items);
        for (int i = 0; i < items; ++i)
        {
            // Generate double f = m + n/10000
            int m = dis(gen);
            int n = dis(gen);
            double f = static_cast<double>(m) + n / 10000.0;
            test_numbers.push_back(f);
            // test_numbers.push_back(dis(gen));
        }
    }

    void methodA()
    {
        // 方法A: 调用 double 参数的 NumberWriter::WriteSmall
        result.clear();
        for (double num : test_numbers)
        {
            wwjson::NumberWriter<std::string>::WriteSmall(result, num);
        }
    }

    template <typename floatT>
    void to_chars(floatT num, char *buffer, size_t bufsz)
    {
        if constexpr (wwjson::has_float_to_chars_v<double>)
        {
            auto [ptr, ec] = std::to_chars(buffer, buffer + bufsz, num);
            result.append(buffer, ptr - buffer);
        }
        else
        {
            int len = std::snprintf(buffer, bufsz, "%.17g", num);
            result.append(buffer, len);
        }
    }
    void methodB()
    {
        // 方法B: 根据环境支持情况选择 std::to_chars 或 snprintf %17.g
        result.clear();
        char buffer[64] = {0};

        // 尝试使用 std::to_chars，如果编译失败则回退到 snprintf
        for (double num : test_numbers)
        {
            to_chars(num, buffer, sizeof(buffer));
        }
    }

    bool methodVerify()
    {
        // 验证浮点数输出，考虑精度差异

        for (double num : test_numbers)
        {
            // 分别使用两个方法序列化同一个数字
            std::string strA;
            std::string strB;

            wwjson::NumberWriter<std::string>::WriteSmall(strA, num);

            char buffer[64] = {0};
            to_chars(num, buffer, sizeof(buffer));
            strB = buffer;

            // 如果字符串完全一致，继续下一个数字
            if (strA == strB)
            {
                continue;
            }

            // 否则，将字符串转回double进行比较
            double numA;
            double numB;
            try
            {
                numA = std::stod(strA);
                numB = std::stod(strB);
            }
            catch (const std::exception &)
            {
                // 转换失败，输出调试信息并返回false
                COUTF(strA, strB);
                return false;
            }

            // 允许相对误差 1e-10
            if (std::abs(numA - numB) > 1e-10)
            {
                COUTF(strA, strB);
                return false;
            }
        }

        return true;
    }
};

} // namespace perf
} // namespace test

// ========== 性能测试用例定义 ==========

// 测试1: 小整数优化验证
DEF_TAST(design_small_int, "NumberWriter 小整数优化验证")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    test::perf::SmallIntOptimizationTest tester(
        argv.items, static_cast<uint32_t>(argv.start));
    tester.runAndPrint("Small Integer Optimization", "NumberWriter::WriteSmall",
                       "std::to_chars", argv.loop, 10);
}

// 测试2: 大整数优化验证
DEF_TAST(design_large_int, "大整数 WriteUnsigned 与 std::to_chars 性能对比")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    test::perf::LargeIntOptimizationTest tester(
        argv.items, static_cast<uint32_t>(argv.start));

    // 运行测试并打印结果
    double ratio = tester.runAndPrint("Large Integer Optimization",
                                      "NumberWriter::WriteUnsigned",
                                      "std::to_chars", argv.loop, 10);
    COUTF(std::isnan(ratio), false);

    // 检查验证结果
    if (std::isnan(ratio))
    {
        DESC("WARNING: Verification failed - methods produce different output");
    }
}

// 测试3: 小范围浮点数优化验证
DEF_TAST(design_small_float, "NumberWriter 小范围浮点数优化验证")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items,
         argv.loop);
    test::perf::SmallFloatOptimizationTest tester(
        argv.items, static_cast<uint32_t>(argv.start));

    const char *methodB_name = "std::to_chars";
    if constexpr (!wwjson::has_float_to_chars_v<double>)
    {
        methodB_name = "snprintf %.17g";
    }

    tester.runAndPrint("Small Float Optimization",
                       "NumberWriter::WriteSmall(double)", methodB_name,
                       argv.loop, 10);
}


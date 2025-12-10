#include "couttast/tinytast.hpp"

#include "argv.h"
#include "relative_perf.h"

#include "wwjson.hpp"

#include <string>
#include <vector>
#include <random>
#include <charconv>
#include <cstdio>

// 备用除100版本的 WriteUnsigned 实现
namespace wwjson {
namespace design_test {

/// 每次除100版本的 WriteUnsigned（不使用 WriteSmall）
template<typename stringT>
void WriteUnsignedDiv100(stringT& dst, uint32_t value) {
    constexpr int max_len = 15; // uint32_t 最大10位数字
    char buffer[max_len];
    char* const buffer_end = buffer + max_len;
    char* ptr = buffer_end;
    
    while (value >= 100) {
        uint32_t chunk = value % 100;
        value /= 100;
        
        // 使用 DigitPair 缓存表
        auto& pairs = NumberWriter<stringT>::kDigitPairs;
        const auto& pair = pairs[chunk];
        
        *(--ptr) = pair.low;
        *(--ptr) = pair.high;
    }
    
    // 剩余的单个或两位数直接添加
    if (value < 10) {
        dst.push_back(static_cast<char>('0' + value));
    } else {
        auto& pairs = NumberWriter<stringT>::kDigitPairs;
        const auto& pair = pairs[value];
        dst.push_back(pair.high);
        dst.push_back(pair.low);
    }
    
    dst.append(ptr, buffer_end - ptr);
}

} // namespace design_test
} // namespace wwjson

namespace test {
namespace perf {

// ========== 测试1: 小整数优化验证 ==========

/**
 * @brief 小整数优化验证测试
 * 对比 NumberWriter::WriteSmall vs std::to_chars
 */
class SmallIntOptimizationTest : public RelativeTimer<SmallIntOptimizationTest> {
public:
    int items;
    uint32_t seed;
    std::string result;
    std::vector<uint32_t> test_numbers;
    
    SmallIntOptimizationTest(int count, uint32_t s) : items(count), seed(s) {
        generateTestNumbers();
    }
    
    void generateTestNumbers() {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<uint32_t> dis(0, 9999);
        
        test_numbers.clear();
        test_numbers.reserve(items);
        for (int i = 0; i < items; ++i) {
            test_numbers.push_back(dis(gen));
        }
    }
    
    void methodA() {
        // 方法A: 使用当前实现的 NumberWriter::WriteSmall
        result.clear();
        for (uint32_t num : test_numbers) {
            wwjson::NumberWriter<std::string>::WriteSmall(result, num);
        }
    }
    
    void methodB() {
        // 方法B: 直接调用 std::to_chars，封装为相同接口
        result.clear();
        char buffer[12];
        for (uint32_t num : test_numbers) {
            auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), num);
            result.append(buffer, ptr - buffer);
        }
    }
};

// ========== 测试2: 小范围浮点数优化验证 ==========

/**
 * @brief 小范围浮点数优化验证测试
 * 对比 NumberWriter::WriteSmall(double) vs std::to_chars/snprintf
 */
class SmallFloatOptimizationTest : public RelativeTimer<SmallFloatOptimizationTest> {
public:
    int items;
    uint32_t seed;
    std::string result;
    std::vector<double> test_numbers;
    
    SmallFloatOptimizationTest(int count, uint32_t s) : items(count), seed(s) {
        generateTestNumbers();
    }
    
    void generateTestNumbers() {
        std::mt19937 gen(seed);
        //std::uniform_real_distribution<double> dis(0.0, 9999.9999);
        std::uniform_int_distribution<uint32_t> dis(0, 9999);
        
        test_numbers.clear();
        test_numbers.reserve(items);
        for (int i = 0; i < items; ++i) {
            // Generate double f = m + n/10000
            int m = dis(gen);
            int n = dis(gen);
            double f = static_cast<double>(m) + n / 10000.0;
            test_numbers.push_back(f);
            //test_numbers.push_back(dis(gen));
        }
    }
    
    void methodA() {
        // 方法A: 调用 double 参数的 NumberWriter::WriteSmall
        result.clear();
        for (double num : test_numbers) {
            wwjson::NumberWriter<std::string>::WriteSmall(result, num);
        }
    }
    
    template <typename floatT>
    void to_chars(floatT num, char* buffer) {
        if constexpr (wwjson::has_float_to_chars_v<double>) {
            auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), num);
            result.append(buffer, ptr - buffer);
        }
        else {
            int len = std::snprintf(buffer, sizeof(buffer), "%.17g", num);
            result.append(buffer, len);
        }
    }
    void methodB() {
        // 方法B: 根据环境支持情况选择 std::to_chars 或 snprintf %17.g
        result.clear();
        char buffer[64];
        
        // 尝试使用 std::to_chars，如果编译失败则回退到 snprintf
        for (double num : test_numbers) {
            to_chars(num, buffer);
        }
    }
};

// ========== 测试3: 大整数除法策略验证 ==========

/**
 * @brief 大整数除法策略验证测试
 * 对比除10000 vs 除100的策略
 */
class BigIntDivisionStrategyTest : public RelativeTimer<BigIntDivisionStrategyTest> {
public:
    int items;
    uint32_t seed;
    std::string result;
    std::vector<uint32_t> test_numbers;
    
    BigIntDivisionStrategyTest(int count, uint32_t s) : items(count), seed(s) {
        generateTestNumbers();
    }
    
    void generateTestNumbers() {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<uint32_t> dis(10001, UINT32_MAX);
        
        test_numbers.clear();
        test_numbers.reserve(items);
        for (int i = 0; i < items; ++i) {
            test_numbers.push_back(dis(gen));
        }
    }
    
    void methodA() {
        // 方法A: 调用当前实现 WriteUnsigned uint32 版（除10000）
        result.clear();
        for (uint32_t num : test_numbers) {
            wwjson::NumberWriter<std::string>::WriteUnsigned(result, num);
        }
    }
    
    void methodB() {
        // 方法B: 重写版本，每次除100（不使用 WriteSmall）
        result.clear();
        for (uint32_t num : test_numbers) {
            wwjson::design_test::WriteUnsignedDiv100<std::string>(result, num);
        }
    }
};

} // namespace perf
} // namespace test

// ========== 性能测试用例定义 ==========

// 测试1: 小整数优化验证
DEF_TAST(small_int_optimization, "验证 NumberWriter 的小整数优化是否有效")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    test::perf::SmallIntOptimizationTest tester(argv.items, static_cast<uint32_t>(argv.start));
    tester.runAndPrint("Small Integer Optimization", 
                      "NumberWriter::WriteSmall", "std::to_chars", 
                      argv.loop, 10);
}

// 测试2: 小范围浮点数优化验证
DEF_TAST(small_float_optimization, "验证 NumberWriter 的小范围浮点数优化是否有效")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    test::perf::SmallFloatOptimizationTest tester(argv.items, static_cast<uint32_t>(argv.start));
    
    const char* methodB_name = "std::to_chars";
    if constexpr (!wwjson::has_float_to_chars_v<double>) {
        methodB_name = "snprintf %.17g";
    }
    
    tester.runAndPrint("Small Float Optimization", 
                      "NumberWriter::WriteSmall(double)", methodB_name, 
                      argv.loop, 10);
}

// 测试3: 大整数除法策略验证
DEF_TAST(big_int_division_strategy, "验证大整数每次除10000快还是每次除100快")
{
    test::CArgv argv;
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    test::perf::BigIntDivisionStrategyTest tester(argv.items, static_cast<uint32_t>(argv.start));
    tester.runAndPrint("Big Integer Division Strategy", 
                      "Divide by 10000", "Divide by 100", 
                      argv.loop, 10);
}

// 工具命令：验证输出正确性
DEF_TOOL(verify_design_correctness, "验证设计测试的输出正确性")
{
    test::CArgv argv;
    
    // 验证小整数输出
    {
        test::perf::SmallIntOptimizationTest tester(1, 123); // 单个值测试
        tester.methodA();
        std::string result_a = tester.result;
        tester.methodB();
        std::string result_b = tester.result;
        COUT(result_a, result_b);
    }
    
    // 验证浮点数输出
    {
        test::perf::SmallFloatOptimizationTest tester(1, 456); // 单个值测试
        COUT(tester.test_numbers[0]);
        tester.methodA();
        std::string result_a = tester.result;
        tester.methodB();
        std::string result_b = tester.result;
        COUT(result_a, result_b);
    }
    
    // 验证大整数除法策略输出
    {
        test::perf::BigIntDivisionStrategyTest tester(1, 789); // 单个值测试
        tester.methodA();
        std::string result_a = tester.result;
        tester.methodB();
        std::string result_b = tester.result;
        COUT(result_a, result_b);
    }
}

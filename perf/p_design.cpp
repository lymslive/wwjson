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

// ========== 测试4: 字面量拷贝优化验证 ==========

/**
 * @brief 字符串字面量拷贝优化测试
 * 对比编译期长度获取 vs 运行期 strlen 的性能差异
 */
class StringLiteralOptimizationTest : public RelativeTimer<StringLiteralOptimizationTest>
{
  public:
    int items;
    int literal_length; // 字面量长度类型: 0=短, 1=中, 2=长
    std::string result;
    
    // 预定义的不同长度的字符串字面量
    static constexpr const char short_literal[] = "name";
    static constexpr const char medium_literal[] = "username";
    static constexpr const char long_literal[] = "authentication_token";

    StringLiteralOptimizationTest(int count, int length_type) : items(count), literal_length(length_type)
    {
    }

    // 方法A: 编译期获取长度
    template<size_t N>
    void append_literal_optimized(std::string& target, const char(&literal)[N])
    {
        target.append(literal, N-1); // N包含null终止符，所以长度为N-1
    }

    void methodA()
    {
        result.clear();
        for (int i = 0; i < items; ++i)
        {
            switch (literal_length)
            {
            case 0:
                append_literal_optimized(result, short_literal);
                break;
            case 1:
                append_literal_optimized(result, medium_literal);
                break;
            case 2:
                append_literal_optimized(result, long_literal);
                break;
            }
            result += ":"; // 模拟JSON键值分隔符
        }
    }

    // 方法B: 运行期strlen
    void append_literal_traditional(std::string& target, const char* str)
    {
        target.append(str, ::strlen(str));
    }

    void methodB()
    {
        result.clear();
        for (int i = 0; i < items; ++i)
        {
            switch (literal_length)
            {
            case 0:
                append_literal_traditional(result, short_literal);
                break;
            case 1:
                append_literal_traditional(result, medium_literal);
                break;
            case 2:
                append_literal_traditional(result, long_literal);
                break;
            }
            result += ":"; // 模拟JSON键值分隔符
        }
    }

    bool methodVerify()
    {
        methodA();
        std::string resultA = result;
        methodB();
        std::string resultB = result;
        return resultA == resultB;
    }

    const char* getLiteralTypeName() const
    {
        switch (literal_length)
        {
        case 0: return "Short (5 chars)";
        case 1: return "Medium (8 chars)";
        case 2: return "Long (21 chars)";
        default: return "Unknown";
        }
    }
};

// ========== 测试5: 字符串转义优化验证 ==========

/**
 * @brief 字符串转义优化测试
 * 对比两种字符串转义方法的性能差异
 */
class StringEscapeOptimizationTest : public RelativeTimer<StringEscapeOptimizationTest>
{
  public:
    int items;
    int size_multiplier;  // 字符串大小倍数
    uint32_t seed;       // 随机种子
    std::string result;
    std::string base_string;  // 基本样例字符串 {"key":"value"}
    std::vector<std::string> test_strings;

    StringEscapeOptimizationTest(int count, int size, uint32_t s) : items(count), size_multiplier(size), seed(s)
    {
        generateTestStrings();
    }

    void generateTestStrings()
    {
        // 基本样例字符串 {"key":"value"}
        base_string = R"({"key":"value"})";
        
        // 扩展字符串到指定大小
        std::string extended_string;
        extended_string = base_string;
        for (int i = 1; i < size_multiplier; ++i) {
            extended_string += base_string;
        }
        
        // 随机将部分字符改为需要转义的字符
        if (extended_string.size() > 10) {
            std::mt19937 gen(seed);  // 使用提供的种子以确保可重复性
            std::uniform_int_distribution<> pos_dis(0, extended_string.size() - 1);
            std::uniform_int_distribution<> char_dis(0, 5);
            
            // 随机替换一些字符为需要转义的字符
            for (size_t i = 0; i < extended_string.size() / 10; ++i) {
                size_t pos = pos_dis(gen);
                char replacement_chars[] = {'\n', '\t', '\r', '\\', '"', '\0'};
                char replacement = replacement_chars[char_dis(gen)];
                extended_string[pos] = replacement;
            }
        }
        
        test_strings.clear();
        test_strings.resize(items, extended_string);
    }

    // 方法A: 临时申请两倍长度的堆内存缓冲区，写入转义后的字符串，再一次性调用 string::append
    void methodA()
    {
        result.clear();
        for (const auto& str : test_strings)
        {
            // 预分配两倍长度的缓冲区
            std::vector<char> buffer;
            buffer.reserve(str.size() * 2);
            
            // 在缓冲区中写入转义后的字符串
            for (size_t i = 0; i < str.size(); ++i)
            {
                unsigned char c = static_cast<unsigned char>(str[i]);
                
                if (c >= 128) {
                    // 可能是UTF-8字节流，直接追加
                    buffer.push_back(c);
                } else {
                    // ASCII字符，检查转义表
                    uint8_t escape_char = wwjson::BasicConfig<std::string>::kEscapeTable[c];
                    if (escape_char != 0) {
                        buffer.push_back('\\');
                        buffer.push_back(escape_char);
                    } else {
                        buffer.push_back(c);
                    }
                }
            }
            
            // 一次性追加到目标字符串
            result.append(buffer.data(), buffer.size());
        }
    }

    // 方法B: 直接遍历每个字符判断是否需要转义，调用 string::push_back 写入目标字符串
    void methodB()
    {
        result.clear();
        for (const auto& str : test_strings)
        {
            // 预分配内存以减少重新分配
            result.reserve(result.size() + str.size() + str.size() / 4);
            
            // 直接写入目标字符串
            for (size_t i = 0; i < str.size(); ++i)
            {
                unsigned char c = static_cast<unsigned char>(str[i]);
                
                if (c >= 128) {
                    // 可能是UTF-8字节流，直接追加
                    result.push_back(c);
                } else {
                    // ASCII字符，检查转义表
                    uint8_t escape_char = wwjson::BasicConfig<std::string>::kEscapeTable[c];
                    if (escape_char != 0) {
                        result.push_back('\\');
                        result.push_back(escape_char);
                    } else {
                        result.push_back(c);
                    }
                }
            }
        }
    }

    bool methodVerify()
    {
        methodA();
        std::string resultA = result;
        methodB();
        std::string resultB = result;
        
        // 比较两个方法产生的输出是否一致
        return resultA == resultB;
    }

    const char* getSizeDescription() const
    {
        if (size_multiplier <= 1) return "Small (~15 chars)";
        if (size_multiplier <= 5) return "Medium (~75 chars)";
        return "Large (150+ chars)";
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

// 测试4: 字面量拷贝优化验证
DEF_TAST(design_string_literal, "字符串字面量拷贝优化验证")
{
    test::CArgv argv;
    DESC("Args: --items=%d --loop=%d", argv.items, argv.loop);
    
    // 短字符串测试
    {
        test::perf::StringLiteralOptimizationTest tester(argv.items, 0); // 0 = 短字符串
        std::string test_name = std::string("String Literal Optimization - ") + tester.getLiteralTypeName();
        tester.runAndPrint(test_name, "Compile-time Length", "Runtime strlen", argv.loop, 10);
    }
    
    // 中等长度字符串测试
    {
        test::perf::StringLiteralOptimizationTest tester(argv.items, 1); // 1 = 中等长度字符串
        std::string test_name = std::string("String Literal Optimization - ") + tester.getLiteralTypeName();
        tester.runAndPrint(test_name, "Compile-time Length", "Runtime strlen", argv.loop, 10);
    }
    
    // 长字符串测试
    {
        test::perf::StringLiteralOptimizationTest tester(argv.items, 2); // 2 = 长字符串
        std::string test_name = std::string("String Literal Optimization - ") + tester.getLiteralTypeName();
        tester.runAndPrint(test_name, "Compile-time Length", "Runtime strlen", argv.loop, 10);
    }
}

// 测试5: 字符串转义优化验证
DEF_TAST(design_string_escape, "字符串转义优化方案预测试")
{
    test::CArgv argv;
    
    DESC("Args: --items=%d --loop=%d --size=%d --start=%d", argv.items, argv.loop, argv.size, argv.start);
    
    test::perf::StringEscapeOptimizationTest tester(argv.items, argv.size, static_cast<uint32_t>(argv.start));
    std::string test_name = std::string("String Escape Optimization - ") + tester.getSizeDescription();
    
    // 运行测试并打印结果
    double ratio = tester.runAndPrint(test_name, "Method A: Buffer+Append", "Method B: Direct PushBack", argv.loop, 10);
    
    // 检查验证结果
    if (std::isnan(ratio))
    {
        DESC("WARNING: Verification failed - methods produce different output");
    }
}


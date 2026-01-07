#include "couttast/tastargv.hpp"
#include "couttast/tinytast.hpp"
#include "wwjson.hpp"

#include <bitset>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>

DEF_TOOL(ext_to_chars, "test std::to_chars")
{
    std::string json;
    std::array<char, 32> buffer;
    auto [ptr, ec] =
        std::to_chars(buffer.data(), buffer.data() + buffer.size(), 100);
    json.append(buffer.data(), ptr);
    COUT(json);

    // compile error for double?
    {
        std::string json;
        std::array<char, 32> buffer;
        auto [ptr, ec] =
            std::to_chars(buffer.data(), buffer.data() + buffer.size(), 500);
        json.append(buffer.data(), ptr);
        COUT(json);
    }

    {
        uint64_t maxInt = -1;
        std::string strMaxInt = std::to_string(maxInt);
        COUT(strMaxInt);
        COUT(strMaxInt.size());
    }

    COUT(wwjson::detail::has_float_to_chars_v<float>);
    COUT(wwjson::detail::has_float_to_chars_v<double>);
}

namespace tool
{
// 分析 double 类型的二进制编码
void analyze_double_binary(double value)
{
    // 使用 union 来访问 double 的二进制表示
    union
    {
        double d;
        uint64_t u;
    } double_union;
    double_union.d = value;

    uint64_t bits = double_union.u;

    // IEEE 754 double 格式：1位符号位，11位指数位，52位尾数位
    uint64_t sign = (bits >> 63) & 0x1;
    uint64_t exponent = (bits >> 52) & 0x7FF;
    uint64_t mantissa = bits & 0xFFFFFFFFFFFFF;

    std::cout << "Value: " << value << std::endl;
    std::cout << "Hex: 0x" << std::hex << bits << std::dec << std::endl;
    std::cout << "Binary: " << std::bitset<64>(bits) << std::endl;
    std::cout << "Sign: " << sign << " (" << (sign ? "negative" : "positive")
              << ")" << std::endl;
    std::cout << "Exponent: " << exponent << " (0x" << std::hex << exponent
              << std::dec << ")" << std::endl;
    std::cout << "Mantissa: " << mantissa << " (0x" << std::hex << mantissa
              << std::dec << ")" << std::endl;

    // 计算实际指数（偏移量为1023）
    if (exponent == 0)
    {
        if (mantissa == 0)
        {
            std::cout << "Special: Zero" << std::endl;
            std::cout << "Formula: 0" << std::endl;
        }
        else
        {
            std::cout << "Special: Denormalized number" << std::endl;
            std::cout << "Formula: " << (sign ? "-" : "") << "0."
                      << std::bitset<52>(mantissa) << " × 2^(-1022)"
                      << std::endl;
            std::cout << "         = " << (sign ? "-" : "")
                      << "mantissa × 2^(-1022-52)" << std::endl;
        }
    }
    else if (exponent == 0x7FF)
    {
        if (mantissa == 0)
        {
            std::cout << "Special: Infinity" << std::endl;
            std::cout << "Formula: " << (sign ? "-" : "") << "∞" << std::endl;
        }
        else
        {
            std::cout << "Special: NaN" << std::endl;
            std::cout << "Formula: NaN (Not a Number)" << std::endl;
        }
    }
    else
    {
        int actual_exponent = static_cast<int>(exponent) - 1023;
        std::cout << "Actual exponent: " << actual_exponent << std::endl;
        std::cout << "Formula: " << (sign ? "-" : "") << "1."
                  << std::bitset<52>(mantissa) << " × 2^(" << actual_exponent
                  << ")" << std::endl;
        std::cout << "         = " << (sign ? "-" : "")
                  << "(1 + mantissa/2^52) × 2^(" << actual_exponent << ")"
                  << std::endl;

        // 计算实际的尾数值
        double mantissa_value = static_cast<double>(mantissa) / (1ULL << 52);
        double calculated_value =
            (1.0 + mantissa_value) * std::pow(2.0, actual_exponent);
        if (sign)
            calculated_value = -calculated_value;

        std::cout << "         = " << (sign ? "-" : "") << "(1 + "
                  << mantissa_value << ") × 2^(" << actual_exponent << ")"
                  << std::endl;
        std::cout << "         = " << calculated_value << " (calculated)"
                  << std::endl;
    }
    std::cout << std::endl;
}

// 分析 float 类型的二进制编码
void analyze_float_binary(float value)
{
    // 使用 union 来访问 float 的二进制表示
    union
    {
        float f;
        uint32_t u;
    } float_union;
    float_union.f = value;

    uint32_t bits = float_union.u;

    // IEEE 754 float 格式：1位符号位，8位指数位，23位尾数位
    uint32_t sign = (bits >> 31) & 0x1;
    uint32_t exponent = (bits >> 23) & 0xFF;
    uint32_t mantissa = bits & 0x7FFFFF;

    std::cout << "Value: " << value << std::endl;
    std::cout << "Hex: 0x" << std::hex << bits << std::dec << std::endl;
    std::cout << "Binary: " << std::bitset<32>(bits) << std::endl;
    std::cout << "Sign: " << sign << " (" << (sign ? "negative" : "positive")
              << ")" << std::endl;
    std::cout << "Exponent: " << exponent << " (0x" << std::hex << exponent
              << std::dec << ")" << std::endl;
    std::cout << "Mantissa: " << mantissa << " (0x" << std::hex << mantissa
              << std::dec << ")" << std::endl;

    // 计算实际指数（偏移量为127）
    if (exponent == 0)
    {
        if (mantissa == 0)
        {
            std::cout << "Special: Zero" << std::endl;
            std::cout << "Formula: 0" << std::endl;
        }
        else
        {
            std::cout << "Special: Denormalized number" << std::endl;
            std::cout << "Formula: " << (sign ? "-" : "") << "0."
                      << std::bitset<23>(mantissa) << " × 2^(-126)"
                      << std::endl;
            std::cout << "         = " << (sign ? "-" : "")
                      << "mantissa × 2^(-126-23)" << std::endl;
        }
    }
    else if (exponent == 0xFF)
    {
        if (mantissa == 0)
        {
            std::cout << "Special: Infinity" << std::endl;
            std::cout << "Formula: " << (sign ? "-" : "") << "∞" << std::endl;
        }
        else
        {
            std::cout << "Special: NaN" << std::endl;
            std::cout << "Formula: NaN (Not a Number)" << std::endl;
        }
    }
    else
    {
        int actual_exponent = static_cast<int>(exponent) - 127;
        std::cout << "Actual exponent: " << actual_exponent << std::endl;
        std::cout << "Formula: " << (sign ? "-" : "") << "1."
                  << std::bitset<23>(mantissa) << " × 2^(" << actual_exponent
                  << ")" << std::endl;
        std::cout << "         = " << (sign ? "-" : "")
                  << "(1 + mantissa/2^23) × 2^(" << actual_exponent << ")"
                  << std::endl;

        // 计算实际的尾数值
        double mantissa_value = static_cast<double>(mantissa) / (1ULL << 23);
        double calculated_value =
            (1.0 + mantissa_value) * std::pow(2.0, actual_exponent);
        if (sign)
            calculated_value = -calculated_value;

        std::cout << "         = " << (sign ? "-" : "") << "(1 + "
                  << mantissa_value << ") × 2^(" << actual_exponent << ")"
                  << std::endl;
        std::cout << "         = " << calculated_value << " (calculated)"
                  << std::endl;
    }
    std::cout << std::endl;
}

// Fixed-point floating-point experimental tests
template <typename floatT> bool is_effectively_integer(floatT value)
{
    floatT int_part;
    return std::modf(value, &int_part) == 0.0;
}

template <typename floatT>
void test_decimal_precision(int scale, const char *type_name)
{
    DESC("=== Testing %s precision with scale %d ===", type_name, scale);

    // Test if scale*f is effectively integer
    DESC("Testing if %d*f is integer:", scale);
    int integer_errors = 0;
    int total_checks = 0;
    floatT max_int_error = 0.0f;
    // for (int i = 0; i < scale; i += (scale > 10000 ? scale/100 : 1))
    for (int i = 0; i < scale; ++i)
    {
        total_checks++;
        floatT f = static_cast<floatT>(i) / static_cast<floatT>(scale);
        floatT scaled = f * static_cast<floatT>(scale);
        if (!is_effectively_integer(scaled))
        {
            integer_errors++;
            floatT error = std::abs(scaled - std::round(scaled));
            max_int_error = std::max(max_int_error, error);
            //          if (integer_errors <= 50) // Show details for first 50
            //          errors
            {
                DESC("Error: %d * %g = %.*g is not integer (error: %.*g)", i, f,
                     std::numeric_limits<floatT>::max_digits10, scaled,
                     std::numeric_limits<floatT>::max_digits10, error);
            }
        }
    }
    floatT int_error_rate =
        static_cast<floatT>(integer_errors) / total_checks * 100.0f;
    DESC("Integer check failure rate: %.2f%% (%d/%d)", int_error_rate,
         integer_errors, total_checks);
    DESC("Maximum integer check error: %.*g",
         std::numeric_limits<floatT>::max_digits10, max_int_error);
    COUTF(integer_errors, 0);
    if (integer_errors > 0)
    {
        DESC("Total %d values failed integer check (showing first 50)",
             integer_errors);
    }

    // Test if scale.0*f/scale.0 equals original f
    DESC("Testing if %d.0*f/%d.0 equals original f:", scale, scale);
    int restore_errors = 0;
    total_checks = 0;
    floatT max_error = 0.0f;
    //  for (int i = 0; i < scale; i += (scale > 10000 ? scale/100 : 1))
    for (int i = 0; i < scale; ++i)
    {
        total_checks++;
        floatT f = static_cast<floatT>(i) / static_cast<floatT>(scale);
        floatT restored =
            (static_cast<floatT>(scale) * f) / static_cast<floatT>(scale);
        floatT error = std::abs(f - restored);
        if (error > max_error)
        {
            max_error = error;
        }
        if (error > std::numeric_limits<floatT>::epsilon())
        {
            restore_errors++;
            if (restore_errors <= 50) // Show details for first 50 errors
            {
                DESC("Error: %g -> %g (diff: %.*g)", f, restored,
                     std::numeric_limits<floatT>::max_digits10, f - restored);
            }
        }
    }
    floatT restore_error_rate =
        static_cast<floatT>(restore_errors) / total_checks * 100.0f;
    DESC("Restoration failure rate: %.2f%% (%d/%d)", restore_error_rate,
         restore_errors, total_checks);
    DESC("Maximum restoration error: %.*g",
         std::numeric_limits<floatT>::max_digits10, max_error);
    COUTF(restore_errors, 0);
    if (restore_errors > 0)
    {
        DESC("Total %d restoration errors (showing first 50)", restore_errors);
    }
}

template <typename floatT> int detect_decimal_places(floatT value)
{
    // Separate integer and fractional parts
    floatT int_part;
    floatT frac_part = std::modf(std::abs(value), &int_part);

    // If it's effectively an integer, return 0
    if (is_effectively_integer(value))
    {
        return 0;
    }

    // Count decimal places by multiplying by 10 until we get an integer
    int decimal_places = 0;
    floatT test_frac = frac_part;
    const floatT epsilon = std::numeric_limits<floatT>::epsilon() * 100.0f;

    while (decimal_places < 15) // Reasonable limit
    {
        test_frac *= 10.0f;
        decimal_places++;
        if (is_effectively_integer(test_frac))
        {
            break;
        }
    }

    return decimal_places;
}

template <typename floatT> void test_decimal_detection(floatT value)
{
    DESC("Testing decimal places detection for: %.*g",
         std::numeric_limits<floatT>::max_digits10, value);

    floatT int_part;
    floatT frac_part = std::modf(std::abs(value), &int_part);

    DESC("Integer part: %g, Fractional part: %.*g", int_part,
         std::numeric_limits<floatT>::max_digits10, frac_part);

    int decimal_places = detect_decimal_places(value);
    DESC("Detected decimal places: %d", decimal_places);

    // Test restoration - only scale and restore fractional part
    floatT scale = std::pow(10.0f, decimal_places);
    floatT scaled_frac = frac_part * scale;
    floatT restored_frac = std::round(scaled_frac) / scale;
    floatT restored = int_part + restored_frac;

    DESC("Original: %.*g", std::numeric_limits<floatT>::max_digits10, value);
    DESC("Scaled fractional part by %g: %.*g", scale,
         std::numeric_limits<floatT>::max_digits10, scaled_frac);
    DESC("Restored: %.*g", std::numeric_limits<floatT>::max_digits10, restored);
    COUTF(std::abs(value - restored) <
              std::numeric_limits<floatT>::epsilon() * 100.0f,
          true);
}

} // namespace tool

DEF_TOOL(ext_double_view, "分析 double 类型的二进制编码")
{
    double value = 0.0;
    BIND_ARGV(value);
    if (value != 0.0)
    {
        DESC("Analyzing provided double value:");
        tool::analyze_double_binary(value);
    }
    else
    {
        tool::analyze_double_binary(value);
        DESC("Analyzing sample double values (1/10 to 1/1):");
        // 分析从 1/10, 1/9 到 1/2, 1/1 这几个示例小数
        for (int denominator = 10; denominator >= 1; --denominator)
        {
            double sample = 1.0 / denominator;
            DESC("=== 1/%d ===", denominator);
            tool::analyze_double_binary(sample);
        }
    }
}

DEF_TOOL(ext_float_view, "分析 float 类型的二进制编码")
{
    float value = 0.0f;
    BIND_ARGV(value);
    if (value != 0.0f)
    {
        DESC("Analyzing provided float value:");
        tool::analyze_float_binary(value);
    }
    else
    {
        tool::analyze_double_binary(value);
        DESC("Analyzing sample float values (1/10 to 1/1):");
        // 分析从 1/10, 1/9 到 1/2, 1/1 这几个示例小数
        for (int denominator = 10; denominator >= 1; --denominator)
        {
            float sample = 1.0f / denominator;
            DESC("=== 1/%d ===", denominator);
            tool::analyze_float_binary(sample);
        }
    }
}

// 定点浮点数实验用例
DEF_TOOL(ext_2decimals, "测试两位小数精度")
{
    tool::test_decimal_precision<double>(100, "double");
}

DEF_TOOL(ext_4decimals, "测试四位小数精度")
{
    tool::test_decimal_precision<double>(10000, "double");
}

DEF_TOOL(ext_8decimals, "测试八位小数精度")
{
    tool::test_decimal_precision<double>(100000000, "double");
}

DEF_TOOL(ext_2decimals_float, "使用 float 测试两位小数精度")
{
    tool::test_decimal_precision<float>(100, "float");
}

DEF_TOOL(ext_4decimals_float, "使用 float 测试四位小数精度")
{
    tool::test_decimal_precision<float>(10000, "float");
}

DEF_TOOL(ext_8decimals_float, "使用 float 测试八位小数精度")
{
    tool::test_decimal_precision<float>(100000000, "float");
}

DEF_TOOL(ext_generic_float, "通用定点浮点数测试，支持命令行参数")
{
    std::string type_str = "double";
    int scale = 10000;

    BIND_ARGV(type_str, "type");
    BIND_ARGV(scale);

    DESC("Running generic fixed-point test with type=%s, scale=%d",
         type_str.c_str(), scale);

    if (type_str == "float")
    {
        tool::test_decimal_precision<float>(scale, "float");
    }
    else
    {
        tool::test_decimal_precision<double>(scale, "double");
    }
}

DEF_TOOL(ext_float_place, "检测浮点数的小数位数")
{
    std::string type_str = "double";
    double value = 0.0;

    BIND_ARGV(type_str, "type");
    BIND_ARGV(value);

    DESC("Detecting decimal places for value=%g with type=%s", value,
         type_str.c_str());

    if (type_str == "float")
    {
        tool::test_decimal_detection<float>(static_cast<float>(value));
    }
    else
    {
        tool::test_decimal_detection<double>(value);
    }
}

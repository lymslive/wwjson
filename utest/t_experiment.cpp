#include "couttast/tinytast.hpp"
#include "couttast/tastargv.hpp"
#include "wwjson.hpp"

#include <charconv>
#include <cstring>
#include <iomanip>
#include <bitset>
#include <iostream>
#include <cmath>

DEF_TOOL(to_chars, "test std::to_chars")
{
    std::string json;
    std::array<char, 32> buffer;
    auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), 100);
    json.append(buffer.data(), ptr);
    COUT(json);

    // compile error for double?
    {
        std::string json;
        std::array<char, 32> buffer;
        auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(),500);
        json.append(buffer.data(), ptr);
        COUT(json);
    }

    {
        uint64_t maxInt = -1;
        std::string strMaxInt = std::to_string(maxInt);
        COUT(strMaxInt);
        COUT(strMaxInt.size());
    }

    COUT(wwjson::has_float_to_chars_v<float>);
    COUT(wwjson::has_float_to_chars_v<double>);
}

// 分析 double 类型的二进制编码
void analyze_double_binary(double value) {
    // 使用 union 来访问 double 的二进制表示
    union {
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
    std::cout << "Sign: " << sign << " (" << (sign ? "negative" : "positive") << ")" << std::endl;
    std::cout << "Exponent: " << exponent << " (0x" << std::hex << exponent << std::dec << ")" << std::endl;
    std::cout << "Mantissa: " << mantissa << " (0x" << std::hex << mantissa << std::dec << ")" << std::endl;
    
    // 计算实际指数（偏移量为1023）
    if (exponent == 0) {
        if (mantissa == 0) {
            std::cout << "Special: Zero" << std::endl;
            std::cout << "Formula: 0" << std::endl;
        } else {
            std::cout << "Special: Denormalized number" << std::endl;
            std::cout << "Formula: " << (sign ? "-" : "") << "0." << std::bitset<52>(mantissa) << " × 2^(-1022)" << std::endl;
            std::cout << "         = " << (sign ? "-" : "") << "mantissa × 2^(-1022-52)" << std::endl;
        }
    } else if (exponent == 0x7FF) {
        if (mantissa == 0) {
            std::cout << "Special: Infinity" << std::endl;
            std::cout << "Formula: " << (sign ? "-" : "") << "∞" << std::endl;
        } else {
            std::cout << "Special: NaN" << std::endl;
            std::cout << "Formula: NaN (Not a Number)" << std::endl;
        }
    } else {
        int actual_exponent = static_cast<int>(exponent) - 1023;
        std::cout << "Actual exponent: " << actual_exponent << std::endl;
        std::cout << "Formula: " << (sign ? "-" : "") << "1." << std::bitset<52>(mantissa) << " × 2^(" << actual_exponent << ")" << std::endl;
        std::cout << "         = " << (sign ? "-" : "") << "(1 + mantissa/2^52) × 2^(" << actual_exponent << ")" << std::endl;
        
        // 计算实际的尾数值
        double mantissa_value = static_cast<double>(mantissa) / (1ULL << 52);
        double calculated_value = (1.0 + mantissa_value) * std::pow(2.0, actual_exponent);
        if (sign) calculated_value = -calculated_value;
        
        std::cout << "         = " << (sign ? "-" : "") << "(1 + " << mantissa_value << ") × 2^(" << actual_exponent << ")" << std::endl;
        std::cout << "         = " << calculated_value << " (calculated)" << std::endl;
    }
    std::cout << std::endl;
}

// 分析 float 类型的二进制编码
void analyze_float_binary(float value) {
    // 使用 union 来访问 float 的二进制表示
    union {
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
    std::cout << "Sign: " << sign << " (" << (sign ? "negative" : "positive") << ")" << std::endl;
    std::cout << "Exponent: " << exponent << " (0x" << std::hex << exponent << std::dec << ")" << std::endl;
    std::cout << "Mantissa: " << mantissa << " (0x" << std::hex << mantissa << std::dec << ")" << std::endl;
    
    // 计算实际指数（偏移量为127）
    if (exponent == 0) {
        if (mantissa == 0) {
            std::cout << "Special: Zero" << std::endl;
            std::cout << "Formula: 0" << std::endl;
        } else {
            std::cout << "Special: Denormalized number" << std::endl;
            std::cout << "Formula: " << (sign ? "-" : "") << "0." << std::bitset<23>(mantissa) << " × 2^(-126)" << std::endl;
            std::cout << "         = " << (sign ? "-" : "") << "mantissa × 2^(-126-23)" << std::endl;
        }
    } else if (exponent == 0xFF) {
        if (mantissa == 0) {
            std::cout << "Special: Infinity" << std::endl;
            std::cout << "Formula: " << (sign ? "-" : "") << "∞" << std::endl;
        } else {
            std::cout << "Special: NaN" << std::endl;
            std::cout << "Formula: NaN (Not a Number)" << std::endl;
        }
    } else {
        int actual_exponent = static_cast<int>(exponent) - 127;
        std::cout << "Actual exponent: " << actual_exponent << std::endl;
        std::cout << "Formula: " << (sign ? "-" : "") << "1." << std::bitset<23>(mantissa) << " × 2^(" << actual_exponent << ")" << std::endl;
        std::cout << "         = " << (sign ? "-" : "") << "(1 + mantissa/2^23) × 2^(" << actual_exponent << ")" << std::endl;
        
        // 计算实际的尾数值
        double mantissa_value = static_cast<double>(mantissa) / (1ULL << 23);
        double calculated_value = (1.0 + mantissa_value) * std::pow(2.0, actual_exponent);
        if (sign) calculated_value = -calculated_value;
        
        std::cout << "         = " << (sign ? "-" : "") << "(1 + " << mantissa_value << ") × 2^(" << actual_exponent << ")" << std::endl;
        std::cout << "         = " << calculated_value << " (calculated)" << std::endl;
    }
    std::cout << std::endl;
}

DEF_TOOL(double_view, "analyze double binary representation")
{
    double value = 0.0;
    BIND_ARGV(value);
    if (value != 0.0) {
        DESC("Analyzing provided double value:");
        analyze_double_binary(value);
    } else {
        analyze_double_binary(value);
        DESC("Analyzing sample double values (1/10 to 1/1):");
        // 分析从 1/10, 1/9 到 1/2, 1/1 这几个示例小数
        for (int denominator = 10; denominator >= 1; --denominator) {
            double sample = 1.0 / denominator;
            DESC("=== 1/%d ===", denominator);
            analyze_double_binary(sample);
        }
    }
}

DEF_TOOL(float_view, "analyze float binary representation")
{
    float value = 0.0f;
    BIND_ARGV(value);
    if (value != 0.0f) {
        DESC("Analyzing provided float value:");
        analyze_float_binary(value);
    } else {
        analyze_double_binary(value);
        DESC("Analyzing sample float values (1/10 to 1/1):");
        // 分析从 1/10, 1/9 到 1/2, 1/1 这几个示例小数
        for (int denominator = 10; denominator >= 1; --denominator) {
            float sample = 1.0f / denominator;
            DESC("=== 1/%d ===", denominator);
            analyze_float_binary(sample);
        }
    }
}


#include "couttast/tinytast.hpp"
#include "couttast/tastargv.hpp"

#include "wwjson.hpp"
#include "test_util.h"

#include <string>
#include <limits>
#include <iostream>

DEF_TAST(number_integer_member, "test serialization of 8 standard integer types")
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();

    // Test all signed integer types
    int8_t i8 = -128;
    int16_t i16 = -32768;
    int32_t i32 = -2147483648;
    int64_t i64 = -9223372036854775807LL;

    builder.AddMember("int8_t", i8);
    builder.AddMember("int16_t", i16);
    builder.AddMember("int32_t", i32);
    builder.AddMember("int64_t", i64);

    // Test all unsigned integer types
    uint8_t u8 = 255;
    uint16_t u16 = 65535;
    uint32_t u32 = 4294967295U;
    uint64_t u64 = 18446744073709551615ULL;

    builder.AddMember("uint8_t", u8);
    builder.AddMember("uint16_t", u16);
    builder.AddMember("uint32_t", u32);
    builder.AddMember("uint64_t", u64);

    // Test special values: zero
    builder.AddMember("zero", 0);

    // Test small integers (0-99) which should use digit pair optimization
    builder.AddMember("small_pos", 42);
    builder.AddMember("small_neg", -42);

    // Test boundary values around 100
    builder.AddMember("border_99", 99);
    builder.AddMember("border_100", 100);
    builder.AddMember("border_101", 101);

    // Test boundary values around 1000
    builder.AddMember("border_999", 999);
    builder.AddMember("border_1000", 1000);
    builder.AddMember("border_1001", 1001);

    // Test boundary values around 10000
    builder.AddMember("border_9999", 9999);
    builder.AddMember("border_10000", 10000);
    builder.AddMember("border_10001", 10001);

    builder.EndRoot();

    // Expected JSON string
    std::string expect = R"({"int8_t":-128,"int16_t":-32768,"int32_t":-2147483648,"int64_t":-9223372036854775807,"uint8_t":255,"uint16_t":65535,"uint32_t":4294967295,"uint64_t":18446744073709551615,"zero":0,"small_pos":42,"small_neg":-42,"border_99":99,"border_100":100,"border_101":101,"border_999":999,"border_1000":1000,"border_1001":1001,"border_9999":9999,"border_10000":10000,"border_10001":10001})";
    
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(number_integer_item, "test serialization of integers in arrays")
{
    wwjson::RawBuilder builder;
    builder.BeginRoot('[');

    // Add various integer values to array
    builder.AddItem(static_cast<int8_t>(-128));
    builder.AddItem(static_cast<uint8_t>(255));
    builder.AddItem(static_cast<int16_t>(-32768));
    builder.AddItem(static_cast<uint16_t>(65535));
    builder.AddItem(static_cast<int32_t>(-2147483648));
    builder.AddItem(static_cast<uint32_t>(4294967295U));
    builder.AddItem(static_cast<int64_t>(-9223372036854775807LL));
    builder.AddItem(static_cast<uint64_t>(18446744073709551615ULL));

    // Add small integers that should use optimization
    builder.AddItem(0);
    builder.AddItem(1);
    builder.AddItem(42);
    builder.AddItem(99);
    builder.AddItem(-1);
    builder.AddItem(-42);

    // Add boundary values
    builder.AddItem(100);
    builder.AddItem(101);
    builder.AddItem(999);
    builder.AddItem(1000);
    builder.AddItem(1001);
    builder.AddItem(9999);
    builder.AddItem(10000);
    builder.AddItem(10001);

    builder.EndRoot(']');

    std::string expect = R"([-128,255,-32768,65535,-2147483648,4294967295,-9223372036854775807,18446744073709551615,0,1,42,99,-1,-42,100,101,999,1000,1001,9999,10000,10001])";
    
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(number_float_serialization, "test various floating-point serialization scenarios")
{
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    
    // Test basic floating-point values
    builder.AddMember("zero", 0.0);
    builder.AddMember("positive", 3.14159);
    builder.AddMember("negative", -2.71828);
    builder.AddMember("small", 0.00123);
    builder.AddMember("large", 1234567.89);
    
    // Test special values - NaN and Inf should become "null" for valid JSON
    float positive_inf = std::numeric_limits<float>::infinity();
    float negative_inf = -std::numeric_limits<float>::infinity();
    float nan_val = std::numeric_limits<float>::quiet_NaN();
    
    builder.AddMember("pos_inf", positive_inf);
    builder.AddMember("neg_inf", negative_inf);
    builder.AddMember("nan_val", nan_val);
    
    // Test precision for different floating-point types
    builder.AddMember("float_val", 3.14159f);
    builder.AddMember("double_val", 2.718281828459045);
    
    // Test very precise values
    builder.AddMember("precise_float", 1.23456789f);
    builder.AddMember("precise_double", 1.23456789012345);
    
    builder.EndRoot();
    
    // Expected output based on to_chars support and format choice
    std::string expect;
    
    // Default to high precision expectations
    expect = R"({"zero":0,"positive":3.14159,"negative":-2.71828,"small":0.00123,"large":1234567.89,"pos_inf":null,"neg_inf":null,"nan_val":null,"float_val":3.14159,"double_val":2.718281828459045,"precise_float":1.2345679,"precise_double":1.23456789012345})";
    
    // If no to_chars support, adjust expectations based on format choice
    if (::wwjson::use_simple_float_format) {
    // Simple format expectations %g
    expect = R"({"zero":0,"positive":3.14159,"negative":-2.71828,"small":0.00123,"large":1.23457e+06,"pos_inf":null,"neg_inf":null,"nan_val":null,"float_val":3.14159,"double_val":2.71828,"precise_float":1.23457,"precise_double":1.23457})";
    }
    
    COUT(builder.json, expect);
    COUT(test::IsJsonValid(builder.json), true);
}

DEF_TAST(number_std_support, "check current runtime support for std::to_chars")
{
    // Use std::cout instead of COUT or DESC to ensure output even with --cout=silent
    std::cout << "=== std::to_chars Support Check ===" << std::endl;
    std::cout << "has_float_to_chars_v<double>: " << ::wwjson::has_float_to_chars_v<double> << std::endl;
    std::cout << "WWJSON_USE_SIMPLE_FLOAT_FORMAT: " << (::wwjson::use_simple_float_format ? "1 (enabled)" : "0 (disabled)") << std::endl;
    
    // Test actual float serialization with 1/3 and 1/4
    double third = 1.0/3.0;
    double quarter = 1.0/4.0;
    
    std::cout << "1/3 value: " << third << std::endl;
    std::cout << "1/4 value: " << quarter << std::endl;
    
    // Build a simple test to see actual serialization
    wwjson::RawBuilder builder;
    builder.BeginRoot();
    builder.AddMember("third", third);
    builder.AddMember("quarter", quarter);
    builder.EndRoot();
    
    std::cout << "Actual JSON output: " << builder.json << std::endl;
    std::cout << "=== End Support Check ===" << std::endl;
}

DEF_TAST(float_opt_basic, "test basic floating-point optimization")
{
    // Test integer values
    wwjson::RawBuilder b1;
    b1.BeginObject();
    b1.AddMember("value", 123.0);
    b1.EndObject();
    COUT(b1.GetResult(), R"({"value":123})");
    
    // Test simple decimal values
    wwjson::RawBuilder b2;
    b2.BeginObject();
    b2.AddMember("value", 123.5);
    b2.EndObject();
    COUT(b2.GetResult(), R"({"value":123.5})");
    
    // Test values with trailing zeros that should be removed
    wwjson::RawBuilder b3;
    b3.BeginObject();
    b3.AddMember("value", 123.5000);
    b3.EndObject();
    COUT(b3.GetResult(), R"({"value":123.5})");
}

DEF_TAST(float_opt_edge_cases, "test edge cases for floating-point optimization")
{
    // Test small values
    wwjson::RawBuilder b1;
    b1.BeginObject();
    b1.AddMember("value", 0.1);
    b1.EndObject();
    COUT(b1.GetResult(), R"({"value":0.1})");
    
    // Test values at boundary
    wwjson::RawBuilder b2;
    b2.BeginObject();
    b2.AddMember("value", 9999.9999);
    b2.EndObject();
    COUT(b2.GetResult(), R"({"value":9999.9999})");
//  if (::wwjson::use_simple_float_format) {
//      COUT(b2.GetResult(), R"({"value":10000})");
//  }
//  else {
//      COUT(b2.GetResult(), R"({"value":9999.9999})");
//  }
    
    // Test large integer that should use the optimization
    wwjson::RawBuilder b3;
    b3.BeginObject();
    b3.AddMember("value", 123456789.0);
    b3.EndObject();
    COUT(b3.GetResult(), R"({"value":123456789})");
}

DEF_TAST(float_opt_negative_numbers, "test negative numbers")
{
    // Test negative integers
    wwjson::RawBuilder b1;
    b1.BeginObject();
    b1.AddMember("value", -123.0);
    b1.EndObject();
    COUT(b1.GetResult(), R"({"value":-123})");
    
    // Test negative decimals
    wwjson::RawBuilder b2;
    b2.BeginObject();
    b2.AddMember("value", -123.5);
    b2.EndObject();
    COUT(b2.GetResult(), R"({"value":-123.5})");
    
    // Test -0.0 should serialize as 0
    wwjson::RawBuilder b3;
    b3.BeginObject();
    b3.AddMember("value", -0.0);
    b3.EndObject();
    COUT(b3.GetResult(), R"({"value":0})");
}

DEF_TAST(float_opt_special_values, "test special floating-point values")
{
    // Test NaN and infinity
    wwjson::RawBuilder b1;
    b1.BeginObject();
    b1.AddMember("nan", std::numeric_limits<double>::quiet_NaN());
    b1.AddMember("inf", std::numeric_limits<double>::infinity());
    b1.AddMember("neg_inf", -std::numeric_limits<double>::infinity());
    b1.EndObject();
    COUT(b1.GetResult(), R"({"nan":null,"inf":null,"neg_inf":null})");
}

// --value=0
DEF_TOOL(check_fast_double, "test a double value to WriteSmall path")
{
    double value = 0.0;
    BIND_ARGV(value);
    if (value != 0.0)
    {
        std::string dst;
        bool fast = wwjson::NumberWriter<std::string>::WriteSmall(dst, value);
        COUT(dst);
        COUT(fast);
        return;
    }

    auto smallPath = [](double val) {
        std::string dst;
        return wwjson::NumberWriter<std::string>::WriteSmall(dst, val);
    };

    COUT(smallPath(9999.9999), true);
    COUT(smallPath(1.0), true);
    COUT(smallPath(0.1), true);
    COUT(smallPath(0.01), true);
    COUT(smallPath(0.001), true);
    COUT(smallPath(0.0001), true);
    COUT(smallPath(0.00001), false);
    COUT(smallPath(0.1234), true);
    COUT(smallPath(0.12341), false);
    COUT(smallPath(0.123401), false);
    COUT(smallPath(0.1234001), false);
    COUT(smallPath(0.12340001), false);
    COUT(smallPath(0.123400001), false);
    COUT(smallPath(0.1234000001), false);
    COUT(smallPath(9999.999901), false);
    COUT(smallPath(9999.9999001), false);
    COUT(smallPath(9999.99990001), false);
    COUT(smallPath(9999.999900001), false);
    COUT(smallPath(9999.999900001), false);
    COUT(smallPath(9999.9999000001), false);
}

// --ipart=0
// when set tolerance=e-8, all double number 9999.xxxx pass fast path.
DEF_TAST(rate_fast_double, "check the rate of miss WriteSmall path")
{
    int ipart = 9999;
    BIND_ARGV(ipart);

    int scale = 10000;
    double fscale = 10000.0;
    int miss = 0;
    for (int i = 0; i < scale; ++i)
    {
        double value = ipart + i / fscale;
        std::string dst;
        bool fast = wwjson::NumberWriter<std::string>::WriteSmall(dst, value);
        if (!fast)
        {
            ++miss;
        }
    }
    COUT(miss, 0);
    DESC("%d.xxxx miss WriteSmall %d/%d", ipart, miss, scale);
}

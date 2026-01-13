/**
 * @file t_itoa.cpp
 * @author lymslive
 * @date 2026-01-14
 * @brief Unit test for IntegerWriter (itoa.hpp)
 */

#include "couttast/tastargv.hpp"
#include "couttast/tinytast.hpp"

#include "test_util.h"
#include "jbuilder.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <limits>

DEF_TAST(itoa_unsigned, "IntegerWriter 无符号整数测试")
{
    wwjson::JString js;
    js.reserve(50);

    // Test all unsigned types with key values
    // uint8_t
    std::vector<uint8_t> u8_values = {0, 1, 9, 10, 99, 100, 101, 255};
    for (uint8_t val : u8_values) {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    // uint16_t
    std::vector<uint16_t> u16_values = {
        0, 1, 9, 10, 99, 100, 101, 999, 1000, 1001,
        9999, 10000, 10001, 65534, 65535
    };
    for (uint16_t val : u16_values) {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    // uint32_t
    std::vector<uint32_t> u32_values = {
        0, 1, 9, 10, 99, 100, 101, 999, 1000, 1001,
        9999, 10000, 10001, 99999, 100000, 100001,
        999999, 1000000, 1000001,
        9999999, 10000000, 10000001,
        99999999, 100000000, 100000001,
        999999999, 1000000000, 1000000001,
        4294967294U, 4294967295U
    };
    for (uint32_t val : u32_values) {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    // uint64_t - including very large values
    std::vector<uint64_t> u64_values = {
        0ULL, 1ULL, 9ULL, 10ULL, 99ULL, 100ULL, 101ULL,
        999ULL, 1000ULL, 1001ULL,
        9999ULL, 10000ULL, 10001ULL,
        99999ULL, 100000ULL, 100001ULL,
        999999ULL, 1000000ULL, 1000001ULL,
        9999999ULL, 10000000ULL, 10000001ULL,
        99999999ULL, 100000000ULL, 100000001ULL,
        999999999ULL, 1000000000ULL, 1000000001ULL,
        9999999999ULL, 10000000000ULL, 10000000001ULL,
        99999999999ULL, 100000000000ULL, 100000000001ULL,
        999999999999ULL, 1000000000000ULL, 1000000000001ULL,
        9999999999999ULL, 10000000000000ULL, 10000000000001ULL,
        99999999999999ULL, 100000000000000ULL, 100000000000001ULL,
        999999999999999ULL, 1000000000000000ULL, 1000000000000001ULL,
        9999999999999999ULL, 10000000000000000ULL, 10000000000000001ULL,
        18446744073709551615ULL  // UINT64_MAX
    };
    for (uint64_t val : u64_values) {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    // Random tests (use COUTF to avoid excessive output)
    for (int i = 0; i < 500; ++i) {
        uint8_t val8 = static_cast<uint8_t>(std::rand());
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val8);
        COUTF(js.str(), std::to_string(val8));
    }

    for (int i = 0; i < 500; ++i) {
        uint16_t val16 = static_cast<uint16_t>(std::rand());
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val16);
        COUTF(js.str(), std::to_string(val16));
    }

    for (int i = 0; i < 500; ++i) {
        uint32_t val32 = static_cast<uint32_t>(std::rand()) << 16;
        val32 ^= static_cast<uint32_t>(std::rand());
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val32);
        COUTF(js.str(), std::to_string(val32));
    }

    for (int i = 0; i < 500; ++i) {
        uint64_t val64 = static_cast<uint64_t>(std::rand()) << 32;
        val64 ^= static_cast<uint64_t>(std::rand()) << 16;
        val64 ^= static_cast<uint64_t>(std::rand());
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val64);
        COUTF(js.str(), std::to_string(val64));
    }
}

DEF_TAST(itoa_signed, "IntegerWriter 有符号整数测试")
{
    wwjson::JString js;
    js.reserve(50);

    // int8_t - full range test
    for (int i = -128; i <= 127; ++i) {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, static_cast<int8_t>(i));
        COUT(js.str(), std::to_string(i));
    }

    // int16_t
    std::vector<int16_t> s16_values = {
        -32768, -10000, -1000, -100, -99, -10, -1, 0, 1, 10, 99, 100, 1000, 10000, 32767
    };
    for (int16_t val : s16_values) {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    // int32_t
    std::vector<int32_t> s32_values = {
        -2147483648LL, -1000000000, -1000000, -1000, -1, 0, 1, 1000, 1000000, 1000000000, 2147483647
    };
    for (int32_t val : s32_values) {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    // int64_t
    std::vector<int64_t> s64_values = {
        -9223372036854775807LL, -1000000000000LL, -1000000000LL, -1000000LL, -1000LL, -1, 0, 1,
        1000LL, 1000000LL, 1000000000LL, 1000000000000LL, 9223372036854775807LL
    };
    for (int64_t val : s64_values) {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    // Random tests for signed types (use COUTF to avoid excessive output)
    for (int i = 0; i < 500; ++i) {
        int16_t val16 = static_cast<int16_t>(std::rand());
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val16);
        COUTF(js.str(), std::to_string(val16));
    }

    for (int i = 0; i < 500; ++i) {
        int32_t val32 = static_cast<int32_t>(std::rand()) << 16;
        val32 ^= static_cast<int32_t>(std::rand());
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val32);
        COUTF(js.str(), std::to_string(val32));
    }

    for (int i = 0; i < 500; ++i) {
        int64_t val64 = static_cast<int64_t>(std::rand()) << 32;
        val64 ^= static_cast<int64_t>(std::rand()) << 16;
        val64 ^= static_cast<int64_t>(std::rand());
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val64);
        COUTF(js.str(), std::to_string(val64));
    }
}

DEF_TAST(itoa_edge_cases, "IntegerWriter 边界情况测试")
{
    wwjson::JString js;
    js.reserve(50);

    // Powers of 10
    std::vector<int> powers = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
    for (int p : powers) {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, p);
        COUT(js.str(), std::to_string(p));
    }

    // Just below powers of 10
    std::vector<int> below_powers = {9, 99, 999, 9999, 99999, 999999, 9999999, 99999999, 999999999};
    for (int p : below_powers) {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, p);
        COUT(js.str(), std::to_string(p));
    }

    // Type MIN and MAX values
    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<int8_t>::min());
    COUT(js.str(), std::to_string(std::numeric_limits<int8_t>::min()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<int8_t>::max());
    COUT(js.str(), std::to_string(std::numeric_limits<int8_t>::max()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<int16_t>::min());
    COUT(js.str(), std::to_string(std::numeric_limits<int16_t>::min()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<int16_t>::max());
    COUT(js.str(), std::to_string(std::numeric_limits<int16_t>::max()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<int32_t>::min());
    COUT(js.str(), std::to_string(std::numeric_limits<int32_t>::min()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<int32_t>::max());
    COUT(js.str(), std::to_string(std::numeric_limits<int32_t>::max()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<int64_t>::min());
    COUT(js.str(), std::to_string(std::numeric_limits<int64_t>::min()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<int64_t>::max());
    COUT(js.str(), std::to_string(std::numeric_limits<int64_t>::max()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<uint8_t>::max());
    COUT(js.str(), std::to_string(std::numeric_limits<uint8_t>::max()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<uint16_t>::max());
    COUT(js.str(), std::to_string(std::numeric_limits<uint16_t>::max()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<uint32_t>::max());
    COUT(js.str(), std::to_string(std::numeric_limits<uint32_t>::max()));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, std::numeric_limits<uint64_t>::max());
    COUT(js.str(), std::to_string(std::numeric_limits<uint64_t>::max()));
}

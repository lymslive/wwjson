/**
 * @file t_itoa.cpp
 * @author lymslive
 * @date 2026-01-13
 * @brief Unit test for IntegerWriter (itoa.hpp)
 */

#include "couttast/tastargv.hpp"
#include "couttast/tinytast.hpp"

#include "test_util.h"
#include "jbuilder.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>

DEF_TAST(itoa_uint8, "IntegerWriter uint8_t test")
{
    wwjson::JString js;
    js.reserve(10);

    for (uint8_t i = 0; i < 255; ++i)
    {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, i);
        COUT(js.str(), std::to_string(i));
    }

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, 0);
    COUT(js.str(), std::string("0"));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, 255);
    COUT(js.str(), std::string("255"));
}

DEF_TAST(itoa_uint16, "IntegerWriter uint16_t test")
{
    wwjson::JString js;
    js.reserve(20);

    std::vector<uint16_t> test_values = {
        0, 1, 9, 10, 99, 100, 101, 999, 1000, 1001,
        9999, 10000, 10001, 65534, 65535
    };

    for (uint16_t val : test_values)
    {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    for (int i = 0; i < 1000; ++i)
    {
        uint16_t val = static_cast<uint16_t>(std::rand() % 65536);
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }
}

DEF_TAST(itoa_uint32, "IntegerWriter uint32_t test")
{
    wwjson::JString js;
    js.reserve(30);

    std::vector<uint32_t> test_values = {
        0, 1, 9, 10, 99, 100, 101, 999, 1000, 1001,
        9999, 10000, 10001, 99999, 100000, 100001,
        999999, 1000000, 1000001,
        9999999, 10000000, 10000001,
        99999999, 100000000, 100000001,
        999999999, 1000000000, 1000000001,
        4294967294U, 4294967295U
    };

    for (uint32_t val : test_values)
    {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    for (int i = 0; i < 1000; ++i)
    {
        uint32_t val = static_cast<uint32_t>(std::rand()) << 16;
        val ^= static_cast<uint32_t>(std::rand());
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }
}

DEF_TAST(itoa_int8, "IntegerWriter int8_t test")
{
    wwjson::JString js;
    js.reserve(10);

    for (int i = -128; i <= 127; ++i)
    {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, static_cast<int8_t>(i));
        COUT(js.str(), std::to_string(i));
    }
}

DEF_TAST(itoa_int16, "IntegerWriter int16_t test")
{
    wwjson::JString js;
    js.reserve(20);

    std::vector<int16_t> test_values = {
        -32768, -10000, -1000, -100, -99, -10, -1, 0, 1, 10, 99, 100, 1000, 10000, 32767
    };

    for (int16_t val : test_values)
    {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    for (int i = 0; i < 1000; ++i)
    {
        int16_t val = static_cast<int16_t>(std::rand());
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }
}

DEF_TAST(itoa_int32, "IntegerWriter int32_t test")
{
    wwjson::JString js;
    js.reserve(30);

    std::vector<int32_t> test_values = {
        -2147483648LL, -1000000000, -1000000, -1000, -1, 0, 1, 1000, 1000000, 1000000000, 2147483647
    };

    for (int32_t val : test_values)
    {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }

    for (int i = 0; i < 1000; ++i)
    {
        int32_t val = static_cast<int32_t>(std::rand()) << 16;
        val ^= static_cast<int32_t>(std::rand());
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }
}

DEF_TAST(itoa_int64, "IntegerWriter int64_t test")
{
    wwjson::JString js;
    js.reserve(40);

    std::vector<int64_t> test_values = {
        -9223372036854775807LL, -1000000000000LL, -1000000LL, -1000LL, -1, 0, 1, 1000LL, 1000000LL, 1000000000000LL, 9223372036854775807LL
    };

    for (int64_t val : test_values)
    {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, val);
        COUT(js.str(), std::to_string(val));
    }
}

DEF_TAST(itoa_edge_cases, "IntegerWriter edge cases test")
{
    wwjson::JString js;
    js.reserve(50);

    // INT_MIN equivalent
    wwjson::IntegerWriter<wwjson::JString>::Output(js, static_cast<int8_t>(-128));
    COUT(js.str(), std::string("-128"));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, static_cast<int16_t>(-32768));
    COUT(js.str(), std::string("-32768"));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, static_cast<int32_t>(-2147483648LL));
    COUT(js.str(), std::string("-2147483648"));

    // UINT_MAX equivalent
    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, static_cast<uint8_t>(255));
    COUT(js.str(), std::string("255"));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, static_cast<uint16_t>(65535));
    COUT(js.str(), std::string("65535"));

    js.clear();
    wwjson::IntegerWriter<wwjson::JString>::Output(js, static_cast<uint32_t>(4294967295U));
    COUT(js.str(), std::string("4294967295"));

    // Powers of 10
    std::vector<int> powers = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
    for (int p : powers)
    {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, p);
        COUT(js.str(), std::to_string(p));
    }

    // Just below powers of 10
    std::vector<int> below_powers = {9, 99, 999, 9999, 99999, 999999, 9999999, 99999999, 999999999};
    for (int p : below_powers)
    {
        js.clear();
        wwjson::IntegerWriter<wwjson::JString>::Output(js, p);
        COUT(js.str(), std::to_string(p));
    }
}

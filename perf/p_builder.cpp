#include "couttast/tinytast.hpp"
#include "test_data.h"
#include "argv.h"
#include <string>

/**
 * @brief Performance test suite for comparing wwjson and yyjson builders
 * 
 * This test suite creates multiple test cases that measure the performance
 * of wwjson::RawBuilder versus yyjson's mutable document API for generating
 * JSON data of various sizes. The test cases use integer parameters (n) to
 * control the complexity of generated JSON, based on size mapping:
 * - ~0.5k JSON: n=6
 * - ~1k JSON: n=12
 * - ~10k JSON: n=120
 * - ~100k JSON: n=1200
 */

// Performance test for wwjson builder with ~0.5k JSON (n=6)
DEF_TAST(build_0_5k_wwjson, "Performance test for wwjson builder (~0.5k JSON, n=6)")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::wwjson::BuildJson(json_data, 6, argv.size);
    }
    TIME_TOC;
    
    DESC("Loop count: %d", argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for yyjson builder with ~0.5k JSON (n=6)
DEF_TAST(build_0_5k_yyjson, "Performance test for yyjson builder (~0.5k JSON, n=6)")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildJson(json_data, 6);
    }
    TIME_TOC;
    
    DESC("Loop count: %d", argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for wwjson builder with ~1k JSON (n=12)
DEF_TAST(build_1k_wwjson, "Performance test for wwjson builder (~1k JSON, n=12)")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::wwjson::BuildJson(json_data, 12, argv.size);
    }
    TIME_TOC;
    
    DESC("Loop count: %d", argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for yyjson builder with ~1k JSON (n=12)
DEF_TAST(build_1k_yyjson, "Performance test for yyjson builder (~1k JSON, n=12)")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildJson(json_data, 12);
    }
    TIME_TOC;
    
    DESC("Loop count: %d", argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for wwjson builder with ~10k JSON (n=120)
DEF_TAST(build_10k_wwjson, "Performance test for wwjson builder (~10k JSON, n=120)")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::wwjson::BuildJson(json_data, 120, argv.size);
    }
    TIME_TOC;
    
    DESC("Loop count: %d", argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for yyjson builder with ~10k JSON (n=120)
DEF_TAST(build_10k_yyjson, "Performance test for yyjson builder (~10k JSON, n=120)")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildJson(json_data, 120);
    }
    TIME_TOC;
    
    DESC("Loop count: %d", argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for wwjson builder with ~100k JSON (n=1200)
DEF_TAST(build_100k_wwjson, "Performance test for wwjson builder (~100k JSON, n=1200)")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::wwjson::BuildJson(json_data, 1200, argv.size);
    }
    TIME_TOC;
    
    DESC("Loop count: %d", argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for yyjson builder with ~100k JSON (n=1200)
DEF_TAST(build_100k_yyjson, "Performance test for yyjson builder (~100k JSON, n=1200)")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildJson(json_data, 1200);
    }
    TIME_TOC;
    
    DESC("Loop count: %d", argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// default args: --loop=1000 --items=1000
DEF_TAST(builder_ex_wwjson, "Performance test for wwjson builder(custom size)")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::wwjson::BuildJson(json_data, argv.items, argv.size);
    }
    TIME_TOC;

    DESC("Args: --loop=%d --items=%d", argv.loop, argv.items);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

DEF_TAST(builder_ex_yyjson, "Performance test for yyjson builder(custom size)")
{
    test::CArgv argv;
    std::string json_data;

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildJson(json_data, argv.items);
    }
    TIME_TOC;

    DESC("Loop count: %d", argv.loop);
    DESC("Args: --loop=%d --items=%d", argv.loop, argv.items);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

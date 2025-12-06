#include "couttast/tinytast.hpp"
#include "test_data.h"
#include "argv.h"
#include <string>

/**
 * @brief Performance test suite for comparing integer array serialization
 * 
 * This test suite creates multiple test cases that measure the performance
 * of wwjson::RawBuilder versus yyjson's mutable document API for generating
 * JSON integer arrays of different sizes and types.
 * 
 * The test cases use --start= parameter for the starting value and 
 * --items= parameter for the count of integer pairs to generate.
 */

// Performance test for wwjson int8 array building
DEF_TAST(array_int8_wwjson, "Performance test for wwjson int8 array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::BuildTinyIntArray(json_data, static_cast<uint8_t>(argv.start), argv.items);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for yyjson int8 array building
DEF_TAST(array_int8_yyjson, "Performance test for yyjson int8 array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildTinyIntArray(json_data, static_cast<uint8_t>(argv.start), argv.items);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for wwjson int16 array building
DEF_TAST(array_int16_wwjson, "Performance test for wwjson int16 array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::BuildShortIntArray(json_data, static_cast<uint16_t>(argv.start), argv.items);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for yyjson int16 array building
DEF_TAST(array_int16_yyjson, "Performance test for yyjson int16 array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildShortIntArray(json_data, static_cast<uint16_t>(argv.start), argv.items);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for wwjson int32 array building
DEF_TAST(array_int32_wwjson, "Performance test for wwjson int32 array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::BuildIntArray(json_data, static_cast<uint32_t>(argv.start), argv.items);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for yyjson int32 array building
DEF_TAST(array_int32_yyjson, "Performance test for yyjson int32 array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildIntArray(json_data, static_cast<uint32_t>(argv.start), argv.items);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for wwjson int64 array building
DEF_TAST(array_int64_wwjson, "Performance test for wwjson int64 array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::BuildBigIntArray(json_data, static_cast<uint64_t>(argv.start), argv.items);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for yyjson int64 array building
DEF_TAST(array_int64_yyjson, "Performance test for yyjson int64 array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildBigIntArray(json_data, static_cast<uint64_t>(argv.start), argv.items);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (2*items)", argv.items * 2);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}
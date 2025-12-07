#include "couttast/tinytast.hpp"

#include "test_data.h"
#include "argv.h"

#include "wwjson.hpp"
#include "yyjson.h"

#include <string>
#include <type_traits>

// Template-based BuildIntArray functions for performance testing
namespace test
{
namespace wwjson
{

/**
 * @brief Template function to build JSON arrays of integer pairs
 * 
 * @tparam uintT Unsigned integer type (uint8_t, uint16_t, uint32_t, uint64_t)
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integer pairs to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
template <typename uintT>
void BuildIntArray(std::string& dst, uintT start, int count, int size_k = 1) {
    using sintT = std::make_signed_t<uintT>;
    
    ::wwjson::RawBuilder builder(size_k << 10); //  * 1024
    builder.BeginArray();
    
    uintT current = start;
    
    for (int i = 0; i < count; i++) {
        uintT positive = current;
        sintT negative = -static_cast<sintT>(positive);
        
        builder.AddItem(positive);
        builder.AddItem(negative);
        
        current++;
    }
    
    builder.EndArray();
    dst = builder.MoveResult();
}

} // namespace wwjson

namespace yyjson
{

/**
 * @brief Template function to build JSON arrays of integer pairs using yyjson
 * 
 * @tparam uintT Unsigned integer type (uint8_t, uint16_t, uint32_t, uint64_t)
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integer pairs to generate
 */
template <typename uintT>
void BuildIntArray(std::string& dst, uintT start, int count) {
    using sintT = std::make_signed_t<uintT>;
    
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc) {
        dst = "[]";
        return;
    }
    
    yyjson_mut_val *root = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, root);
    
    uintT current = start;
    
    for (int i = 0; i < count; i++) {
        uintT positive = current;
        sintT negative = -static_cast<sintT>(positive);
        
        yyjson_mut_arr_add_uint(doc, root, positive);
        yyjson_mut_arr_add_sint(doc, root, negative);
        
        current++;
    }
    
    char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
    if (json_str) {
        dst = json_str;
        free(json_str);
    } else {
        dst = "[]";
    }
    
    yyjson_mut_doc_free(doc);
}

} // namespace yyjson
} // namespace test

// Float array functions for performance testing
namespace test
{
namespace wwjson
{

/**
 * @brief Function to build JSON arrays of float values
 * 
 * For each integer, generates 4 floating point values: +0.0, +1/5, +1/3, +1/2
 * 
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integers to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildFloatArray(std::string& dst, int start, int count, int size_k = 1) {
    ::wwjson::RawBuilder builder(size_k << 10); //  * 1024
    builder.BeginArray();
    
    for (int i = 0; i < count; i++) {
        int value = start + i;
        
        // Generate 4 floating point values for each integer
        builder.AddItem(static_cast<float>(value + 0.0f));
        builder.AddItem(static_cast<float>(value + 1.0f/5.0f));
        builder.AddItem(static_cast<float>(value + 1.0f/3.0f));
        builder.AddItem(static_cast<float>(value + 1.0f/2.0f));
    }
    
    builder.EndArray();
    dst = builder.MoveResult();
}

/**
 * @brief Function to build JSON arrays of double values
 * 
 * For each integer, generates 4 floating point values: +0.0, +1/5, +1/3, +1/2
 * 
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integers to generate
 * @param size_k Estimated size in kilobytes (default 1)
 */
void BuildDoubleArray(std::string& dst, int start, int count, int size_k = 1) {
    ::wwjson::RawBuilder builder(size_k << 10); //  * 1024
    builder.BeginArray();
    
    for (int i = 0; i < count; i++) {
        int value = start + i;
        
        // Generate 4 floating point values for each integer
        builder.AddItem(static_cast<double>(value + 0.0));
        builder.AddItem(static_cast<double>(value + 1.0/5.0));
        builder.AddItem(static_cast<double>(value + 1.0/3.0));
        builder.AddItem(static_cast<double>(value + 1.0/2.0));
    }
    
    builder.EndArray();
    dst = builder.MoveResult();
}

} // namespace wwjson

namespace yyjson
{

/**
 * @brief Function to build JSON arrays of float values using yyjson
 * 
 * For each integer, generates 4 floating point values: +0.0, +1/5, +1/3, +1/2
 * 
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integers to generate
 */
void BuildFloatArray(std::string& dst, int start, int count) {
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc) {
        dst = "[]";
        return;
    }
    
    yyjson_mut_val *root = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, root);
    
    for (int i = 0; i < count; i++) {
        int value = start + i;
        
        // Generate 4 floating point values for each integer using yyjson_mut_arr_add_float
        yyjson_mut_arr_add_float(doc, root, static_cast<float>(value + 0.0f));
        yyjson_mut_arr_add_float(doc, root, static_cast<float>(value + 1.0f/5.0f));
        yyjson_mut_arr_add_float(doc, root, static_cast<float>(value + 1.0f/3.0f));
        yyjson_mut_arr_add_float(doc, root, static_cast<float>(value + 1.0f/2.0f));
    }
    
    char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
    if (json_str) {
        dst = json_str;
        free(json_str);
    } else {
        dst = "[]";
    }
    
    yyjson_mut_doc_free(doc);
}

/**
 * @brief Function to build JSON arrays of double values using yyjson
 * 
 * For each integer, generates 4 floating point values: +0.0, +1/5, +1/3, +1/2
 * 
 * @param dst Output string to store generated JSON
 * @param start Starting value for the sequence
 * @param count Number of integers to generate
 */
void BuildDoubleArray(std::string& dst, int start, int count) {
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc) {
        dst = "[]";
        return;
    }
    
    yyjson_mut_val *root = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, root);
    
    for (int i = 0; i < count; i++) {
        int value = start + i;
        
        // Generate 4 floating point values for each integer using yyjson_mut_arr_add_double
        yyjson_mut_arr_add_double(doc, root, static_cast<double>(value + 0.0));
        yyjson_mut_arr_add_double(doc, root, static_cast<double>(value + 1.0/5.0));
        yyjson_mut_arr_add_double(doc, root, static_cast<double>(value + 1.0/3.0));
        yyjson_mut_arr_add_double(doc, root, static_cast<double>(value + 1.0/2.0));
    }
    
    char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
    if (json_str) {
        dst = json_str;
        free(json_str);
    } else {
        dst = "[]";
    }
    
    yyjson_mut_doc_free(doc);
}

} // namespace yyjson
} // namespace test

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
        test::wwjson::BuildIntArray(json_data, static_cast<uint8_t>(argv.start), argv.items, argv.size);
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
        test::yyjson::BuildIntArray(json_data, static_cast<uint8_t>(argv.start), argv.items);
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
        test::wwjson::BuildIntArray(json_data, static_cast<uint16_t>(argv.start), argv.items, argv.size);
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
        test::yyjson::BuildIntArray(json_data, static_cast<uint16_t>(argv.start), argv.items);
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
        test::wwjson::BuildIntArray(json_data, static_cast<uint32_t>(argv.start), argv.items, argv.size);
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
        test::wwjson::BuildIntArray(json_data, static_cast<uint64_t>(argv.start), argv.items, argv.size);
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
        test::yyjson::BuildIntArray(json_data, static_cast<uint64_t>(argv.start), argv.items);
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

// Comparison test for wwjson vs yyjson output
DEF_TOOL(compare_array_output, "Compare wwjson and yyjson BuildIntArray output")
{
    test::CArgv argv;
    int test_count = 10;
    
    // Test uint8_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(wwjson_output, static_cast<uint8_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(yyjson_output, static_cast<uint8_t>(argv.start), test_count);
        COUT(wwjson_output, yyjson_output);
    }
    
    // Test uint16_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(wwjson_output, static_cast<uint16_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(yyjson_output, static_cast<uint16_t>(argv.start), test_count);
        COUT(wwjson_output, yyjson_output);
    }
    
    // Test uint32_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(wwjson_output, static_cast<uint32_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(yyjson_output, static_cast<uint32_t>(argv.start), test_count);
        COUT(wwjson_output, yyjson_output);
    }
    
    // Test uint64_t
    {
        std::string wwjson_output, yyjson_output;
        test::wwjson::BuildIntArray(wwjson_output, static_cast<uint64_t>(argv.start), test_count);
        test::yyjson::BuildIntArray(yyjson_output, static_cast<uint64_t>(argv.start), test_count);
        COUT(wwjson_output, yyjson_output);
    }
}

// Performance test for wwjson float array building
DEF_TAST(array_float_wwjson, "Performance test for wwjson float array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::wwjson::BuildFloatArray(json_data, argv.start, argv.items, argv.size);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (4*items)", argv.items * 4);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for yyjson float array building
DEF_TAST(array_float_yyjson, "Performance test for yyjson float array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildFloatArray(json_data, argv.start, argv.items);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (4*items)", argv.items * 4);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for wwjson double array building
DEF_TAST(array_double_wwjson, "Performance test for wwjson double array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::wwjson::BuildDoubleArray(json_data, argv.start, argv.items, argv.size);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (4*items)", argv.items * 4);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

// Performance test for yyjson double array building
DEF_TAST(array_double_yyjson, "Performance test for yyjson double array building")
{
    test::CArgv argv;
    std::string json_data;
    
    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::yyjson::BuildDoubleArray(json_data, argv.start, argv.items);
    }
    TIME_TOC;
    
    DESC("Args: --start=%d --items=%d --loop=%d", argv.start, argv.items, argv.loop);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    DESC("Array elements: %d (4*items)", argv.items * 4);
    
    // Print JSON content for single iteration
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

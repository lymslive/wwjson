#include "couttast/tinytast.hpp"
#include "argv.h"
#include "wwjson.hpp"
#include "yyjson.h"

#include <string>
#include <fstream>
#include <filesystem>

namespace test::wwjson
{

/**
 * @brief Generate JSON data of specified approximate size using RawBuilder
 * 
 * @param dst Output string to store generated JSON
 * @param size Target size in kilobytes (k)
 */
void BuildJson(std::string& dst, double size)
{
    size_t target_size = static_cast<size_t>(size * 1024);
    ::wwjson::RawBuilder builder(target_size);
    
    // Start with an object
    builder.BeginObject();
    
    // Add a base object to reach target size
    // We'll create an array of objects with increasing complexity
    size_t current_size = 0;
    int item_count = 0;
    
    while (current_size < target_size) {
        // Add a member with a unique key
        std::string key = "item_" + std::to_string(item_count++);
        
        // Begin an array for this item
        builder.BeginArray(key.c_str());
        
        // Add multiple values to array to increase size
        for (int i = 0; i < 10 && current_size < target_size; i++) {
            builder.AddItem(item_count + i);
            builder.AddItem((item_count + i) * 1.5);
            builder.AddItem(std::to_string(item_count + i).c_str());
            
            current_size = builder.Size();
            if (current_size >= target_size) break;
        }
        
        // End the array
        builder.EndArray();
        
        // Check if we've reached the target size
        current_size = builder.Size();
        if (current_size >= target_size) break;
        
        // Add a nested object for more complexity
        builder.BeginObject("nested_" + std::to_string(item_count));
        builder.AddMember("id", item_count);
        builder.AddMember("name", "Test Item");
        builder.AddMember("value", item_count * 2.5);
        builder.EndObject();
        
        current_size = builder.Size();
    }
    
    // End the root object
    builder.EndObject();
    
    dst = builder.MoveResult();
}

/**
 * @brief Generate JSON data with specified number of items
 * 
 * The generated JSON structure is as follows:
 * {
 *   "item_0": [1, 1.5, "1"],
 *   "nested_0": {"id": 0, "name": "Test Item", "value": 0.0},
 *   "item_1": [2, 3.0, "2"],
 *   "nested_1": {"id": 1, "name": "Test Item", "value": 2.5},
 *   ...
 * }
 * 
 * @param dst Output string to store generated JSON
 * @param n Number of items to generate (each item = array + nested object)
 * @param size Estimated size in kilobytes (default 1)
 */
void BuildJson(std::string& dst, int n, int size = 1)
{
    ::wwjson::RawBuilder builder(size * 1024);
    
    // Start with an object
    builder.BeginObject();
    
    // Add n items with arrays and nested objects
    for (int i = 0; i < n; i++) {
        // Add array item
        std::string array_key = "item_" + std::to_string(i);
        builder.BeginArray(array_key.c_str());
        
        // Add 3 values to each array
        builder.AddItem(i + 1);
        builder.AddItem((i + 1) * 1.5);
        builder.AddItem(std::to_string(i + 1).c_str());
        
        builder.EndArray();
        
        // Add nested object
        std::string nested_key = "nested_" + std::to_string(i);
        builder.BeginObject(nested_key.c_str());
        builder.AddMember("id", i);
        builder.AddMember("name", "Test Item");
        builder.AddMember("value", i * 2.5);
        builder.EndObject();
    }
    
    // End the root object
    builder.EndObject();
    
    dst = builder.MoveResult();
}

} // namespace test::wwjson

namespace test::yyjson
{

/**
 * @brief Generate JSON data with specified number of items using yyjson.
 *
 * The generated JSON structure is like above test::wwjson::BuildJson .
 * 
 * @param dst Output string to store generated JSON
 * @param n Number of items to generate (each item = array + nested object)
 */
void BuildJson(std::string& dst, int n)
{
    // Create a new mutable document
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc) {
        dst = "{}";
        return;
    }
    
    // Create root object
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);
    
    // Add n items with arrays and nested objects
    for (int i = 0; i < n; i++) {
        // Add array item
        std::string array_key = "item_" + std::to_string(i);
        yyjson_mut_val *key = yyjson_mut_strcpy(doc, array_key.c_str());
        yyjson_mut_val *arr = yyjson_mut_arr(doc);
        if (!key || !arr || !yyjson_mut_obj_add(root, key, arr)) {
            yyjson_mut_doc_free(doc);
            dst = "{}";
            return;
        }
        
        // Add 3 values to each array
        yyjson_mut_arr_add_int(doc, arr, i + 1);
        yyjson_mut_arr_add_real(doc, arr, (i + 1) * 1.5);
        yyjson_mut_arr_add_strcpy(doc, arr, std::to_string(i + 1).c_str());
        
        // Add nested object
        std::string nested_key = "nested_" + std::to_string(i);
        yyjson_mut_val *nested_key_val = yyjson_mut_strcpy(doc, nested_key.c_str());
        yyjson_mut_val *nested_obj = yyjson_mut_obj(doc);
        if (!nested_key_val || !nested_obj || !yyjson_mut_obj_add(root, nested_key_val, nested_obj)) {
            yyjson_mut_doc_free(doc);
            dst = "{}";
            return;
        }
        
        yyjson_mut_obj_add_int(doc, nested_obj, "id", i);
        yyjson_mut_obj_add_str(doc, nested_obj, "name", "Test Item");
        yyjson_mut_obj_add_real(doc, nested_obj, "value", i * 2.5);
    }
    
    // Write the document to JSON string
    char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
    if (json_str) {
        dst = json_str;
        free(json_str);
    } else {
        dst = "{}";
    }
    
    // Free the document
    yyjson_mut_doc_free(doc);
}

} // namespace test::yyjson

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
DEF_TAST(build_0_5k_wwjson, "wwjson 构建器性能测试（约 0.5k JSON，n=6）")
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
DEF_TAST(build_0_5k_yyjson, "yyjson 构建器性能测试（约 0.5k JSON，n=6）")
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
DEF_TAST(build_1k_wwjson, "wwjson 构建器性能测试（约 1k JSON，n=12）")
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
DEF_TAST(build_1k_yyjson, "yyjson 构建器性能测试（约 1k JSON，n=12）")
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
DEF_TAST(build_10k_wwjson, "wwjson 构建器性能测试（约 10k JSON，n=120）")
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
DEF_TAST(build_10k_yyjson, "yyjson 构建器性能测试（约 10k JSON，n=120）")
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
DEF_TAST(build_100k_wwjson, "wwjson 构建器性能测试（约 100k JSON，n=1200）")
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
DEF_TAST(build_100k_yyjson, "yyjson 构建器性能测试（约 100k JSON，n=1200）")
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
DEF_TAST(build_ex_wwjson, "wwjson 构建器性能测试（自定义大小）")
{
    test::CArgv argv;
    std::string json_data;

    // Auto-estimate capacity only when argv.size is default (1)
    if (argv.size == 1) {
        // Run once to estimate capacity
        test::wwjson::BuildJson(json_data, argv.items, 1);
        int estimated_size = (json_data.size() / 1024) + 1;  // Convert to KB, round up
        argv.size = estimated_size;
    }

    TIME_TIC;
    for (int i = 0; i < argv.loop; i++) {
        test::wwjson::BuildJson(json_data, argv.items, argv.size);
    }
    TIME_TOC;

    DESC("Args: --loop=%d --items=%d --size=%d", argv.loop, argv.items, argv.size);
    DESC("Generated JSON size: %zu bytes", json_data.size());
    if (argv.loop == 1) {
        COUT(json_data);
    }
}

DEF_TAST(build_ex_yyjson, "yyjson 构建器性能测试（自定义大小）")
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

DEF_TOOL(build_sample, "生成不同大小的 JSON 样本用于性能测试")
{
    // Create output directory
    std::filesystem::create_directories("perf/test_data.tmp");
    
    // Test sizes: 0.5k, 1k, 10k, 100k, 500k, 1M
    double sizes[] = {0.5, 1.0, 10.0, 100.0, 500.0, 1000.0};
    
    for (double size : sizes) {
        std::string json_data;
        test::wwjson::BuildJson(json_data, size);
        
        // Calculate expected size in bytes
        double expect_size_bytes = size * 1024.0;
        double actual_size_bytes = static_cast<double>(json_data.size());
        
        // Validate size with tolerance
//      COUT(actual_size_bytes, expect_size_bytes, 10.0);
        COUT(actual_size_bytes / expect_size_bytes, 1.0, 0.02);
        
        // Write to file
        std::string size_str;
        if (size == static_cast<int>(size)) {
            size_str = std::to_string(static_cast<int>(size));
        } else {
            size_str = std::to_string(size);
            size_str.erase(size_str.find_last_not_of('0') + 1, std::string::npos);
            if (size_str.back() == '.') {
                size_str.pop_back();
            }
        }
        std::string filename = "perf/test_data.tmp/" + size_str + "k.json";
        std::ofstream file(filename);
        if (file.is_open()) {
            file << json_data;
            file.close();
        }
        
        DESC("Generated %g k JSON", size);
        DESC("Actual size: %.0f bytes", actual_size_bytes);
        DESC("Expected size: %.0f bytes", expect_size_bytes);
        DESC("File saved: %s", filename.c_str());
    }
}

// Verification tool to compare wwjson and yyjson outputs
DEF_TOOL(build_verify, "验证 wwjson 和 yyjson 生成相同的 JSON 结构")
{
    int n = 3;
    std::string wwjson_result;
    std::string yyjson_result;
    
    // Generate JSON using wwjson
    test::wwjson::BuildJson(wwjson_result, n);
    COUT(wwjson_result);
    DESC("wwjson generated JSON for %d items", n);
    DESC("wwjson result length: %zu bytes", wwjson_result.size());
    
    // Generate JSON using yyjson
    test::yyjson::BuildJson(yyjson_result, n);
    COUT(yyjson_result);
    DESC("yyjson generated JSON for %d items", n);
    DESC("yyjson result length: %zu bytes", yyjson_result.size());
    
    COUT(wwjson_result == yyjson_result, true);

    COUT(wwjson::has_float_to_chars_v<float>);
    COUT(wwjson::has_float_to_chars_v<double>);
}

// Tool to test different n values and their corresponding JSON sizes
DEF_TOOL(build_size, "测试不同 n 值对应的 JSON 大小")
{
    DESC("Testing JSON sizes for different item counts (each item has array + nested object):");
    DESC("");
    
    int test_values[] = {1, 2, 5, 10, 20, 50, 100};
    
    for (int n : test_values) {
        std::string wwjson_result;
        std::string yyjson_result;
        
        test::wwjson::BuildJson(wwjson_result, n);
        test::yyjson::BuildJson(yyjson_result, n);
        
        DESC("n = %d:", n);
        DESC("  wwjson size: %zu bytes (%.2f KB)", wwjson_result.size(), wwjson_result.size() / 1024.0);
        DESC("  yyjson size: %zu bytes (%.2f KB)", yyjson_result.size(), yyjson_result.size() / 1024.0);
        DESC("  sizes match: %s", wwjson_result.size() == yyjson_result.size() ? "✓" : "✗");
        DESC("");
    }
    
    DESC("Note: Each item consists of an array with 3 values and a nested object with 3 members");
}

#include "couttast/tinytast.hpp"
#include "wwjson.hpp"
#include "test_data.h"
#include <string>
#include <fstream>
#include <filesystem>
#include "yyjson.h"

namespace test
{

/**
 * @brief Generate JSON data of specified size using RawBuilder
 * 
 * The generated JSON structure is as follows:
 * {
 *   "item_0": [1, 1.5, "1", 2, 3.0, "2", ...],
 *   "nested_0": {"id": 0, "name": "Test Item", "value": 0.0},
 *   "item_1": [2, 3.0, "2", 3, 4.5, "3", ...],
 *   "nested_1": {"id": 1, "name": "Test Item", "value": 2.5},
 *   ...
 * }
 * 
 * @param dst Output string to store generated JSON
 * @param size Target size in kilobytes (k)
 */
void BuildJson(std::string& dst, double size)
{
    // Calculate target size in bytes
    size_t target_size = static_cast<size_t>(size * 1024);
    
    wwjson::RawBuilder builder;
    
    // Start with an object
    builder.BeginObject();
    
    // Add a base object to reach the target size
    // We'll create an array of objects with increasing complexity
    size_t current_size = 0;
    int item_count = 0;
    
    while (current_size < target_size) {
        // Add a member with a unique key
        std::string key = "item_" + std::to_string(item_count++);
        
        // Begin an array for this item
        builder.BeginArray(key.c_str());
        
        // Add multiple values to the array to increase size
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
 */
void BuildJson(std::string& dst, int n)
{
    wwjson::RawBuilder builder;
    
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

// Integer array building functions for performance testing
void BuildTinyIntArray(std::string& dst, uint8_t start, int count)
{
    wwjson::RawBuilder builder;
    builder.BeginArray();
    
    uint8_t current = start;
    
    for (int i = 0; i < count; i++) {
        uint8_t positive = current;
        int8_t negative = -static_cast<int8_t>(positive);
        
        builder.AddItem(positive);
        builder.AddItem(negative);
        
        current++;
    }
    
    builder.EndArray();
    dst = builder.MoveResult();
}

void BuildShortIntArray(std::string& dst, uint16_t start, int count)
{
    wwjson::RawBuilder builder;
    builder.BeginArray();
    
    uint16_t current = start;
    
    for (int i = 0; i < count; i++) {
        uint16_t positive = current;
        int16_t negative = -static_cast<int16_t>(positive);
        
        builder.AddItem(positive);
        builder.AddItem(negative);
        
        current++;
    }
    
    builder.EndArray();
    dst = builder.MoveResult();
}

void BuildIntArray(std::string& dst, uint32_t start, int count)
{
    wwjson::RawBuilder builder;
    builder.BeginArray();
    
    uint32_t current = start;
    
    for (int i = 0; i < count; i++) {
        uint32_t positive = current;
        int32_t negative = -static_cast<int32_t>(positive);
        
        builder.AddItem(positive);
        builder.AddItem(negative);
        
        current++;
    }
    
    builder.EndArray();
    dst = builder.MoveResult();
}

void BuildBigIntArray(std::string& dst, uint64_t start, int count)
{
    wwjson::RawBuilder builder;
    builder.BeginArray();
    
    uint64_t current = start;
    
    for (int i = 0; i < count; i++) {
        uint64_t positive = current;
        int64_t negative = -static_cast<int64_t>(positive);
        
        builder.AddItem(positive);
        builder.AddItem(negative);
        
        current++;
    }
    
    builder.EndArray();
    dst = builder.MoveResult();
}

} // end of namespace test::

// yyjson implementation namespace
namespace test::yyjson
{

/**
 * @brief Generate JSON data with specified number of items using yyjson
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
        yyjson_mut_val *arr = yyjson_mut_obj_add_arr(doc, root, array_key.c_str());
        if (!arr) {
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
        yyjson_mut_val *nested_obj = yyjson_mut_obj_add_obj(doc, root, nested_key.c_str());
        if (!nested_obj) {
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

// Integer array building functions for performance testing using yyjson
void BuildTinyIntArray(std::string& dst, uint8_t start, int count)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc) {
        dst = "[]";
        return;
    }
    
    yyjson_mut_val *root = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, root);
    
    uint8_t current = start;
    
    for (int i = 0; i < count; i++) {
        uint8_t positive = current;
        int8_t negative = -static_cast<int8_t>(positive);
        
        yyjson_mut_arr_add_int(doc, root, positive);
        yyjson_mut_arr_add_int(doc, root, negative);
        
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

void BuildShortIntArray(std::string& dst, uint16_t start, int count)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc) {
        dst = "[]";
        return;
    }
    
    yyjson_mut_val *root = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, root);
    
    uint16_t current = start;
    
    for (int i = 0; i < count; i++) {
        uint16_t positive = current;
        int16_t negative = -static_cast<int16_t>(positive);
        
        yyjson_mut_arr_add_int(doc, root, positive);
        yyjson_mut_arr_add_int(doc, root, negative);
        
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

void BuildIntArray(std::string& dst, uint32_t start, int count)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc) {
        dst = "[]";
        return;
    }
    
    yyjson_mut_val *root = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, root);
    
    uint32_t current = start;
    
    for (int i = 0; i < count; i++) {
        uint32_t positive = current;
        int32_t negative = -static_cast<int32_t>(positive);
        
        yyjson_mut_arr_add_int(doc, root, positive);
        yyjson_mut_arr_add_int(doc, root, negative);
        
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

void BuildBigIntArray(std::string& dst, uint64_t start, int count)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc) {
        dst = "[]";
        return;
    }
    
    yyjson_mut_val *root = yyjson_mut_arr(doc);
    yyjson_mut_doc_set_root(doc, root);
    
    uint64_t current = start;
    
    for (int i = 0; i < count; i++) {
        uint64_t positive = current;
        int64_t negative = -static_cast<int64_t>(positive);
        
        yyjson_mut_arr_add_int(doc, root, positive);
        yyjson_mut_arr_add_int(doc, root, negative);
        
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

} // namespace test::yyjson

DEF_TOOL(data_sample, "Generate JSON samples of different sizes for performance testing")
{
    // Create output directory
    std::filesystem::create_directories("perf/test_data.tmp");
    
    // Test sizes: 0.5k, 1k, 10k, 100k, 500k, 1M
    double sizes[] = {0.5, 1.0, 10.0, 100.0, 500.0, 1000.0};
    
    for (double size : sizes) {
        std::string json_data;
        test::BuildJson(json_data, size);
        
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
DEF_TOOL(verify_json_builders, "Verify wwjson and yyjson generate identical JSON structures")
{
    int n = 3;
    std::string wwjson_result;
    std::string yyjson_result;
    
    // Generate JSON using wwjson
    test::BuildJson(wwjson_result, n);
    COUT(wwjson_result);
    DESC("wwjson generated JSON for %d items", n);
    DESC("wwjson result length: %zu bytes", wwjson_result.size());
    
    // Generate JSON using yyjson
    test::yyjson::BuildJson(yyjson_result, n);
    COUT(yyjson_result);
    DESC("yyjson generated JSON for %d items", n);
    DESC("yyjson result length: %zu bytes", yyjson_result.size());
    
    // Compare the two results
    if (wwjson_result == yyjson_result) {
        DESC("✓ Both libraries generated identical JSON");
    } else {
        DESC("✗ JSON outputs differ between libraries");
        DESC("wwjson size: %zu", wwjson_result.size());
        DESC("yyjson size: %zu", yyjson_result.size());
    }
}

// Tool to test different n values and their corresponding JSON sizes
DEF_TOOL(test_json_sizes, "Test JSON sizes for different n values")
{
    DESC("Testing JSON sizes for different item counts (each item has array + nested object):");
    DESC("");
    
    int test_values[] = {1, 2, 5, 10, 20, 50, 100};
    
    for (int n : test_values) {
        std::string wwjson_result;
        std::string yyjson_result;
        
        test::BuildJson(wwjson_result, n);
        test::yyjson::BuildJson(yyjson_result, n);
        
        DESC("n = %d:", n);
        DESC("  wwjson size: %zu bytes (%.2f KB)", wwjson_result.size(), wwjson_result.size() / 1024.0);
        DESC("  yyjson size: %zu bytes (%.2f KB)", yyjson_result.size(), yyjson_result.size() / 1024.0);
        DESC("  sizes match: %s", wwjson_result.size() == yyjson_result.size() ? "✓" : "✗");
        DESC("");
    }
    
    DESC("Note: Each item consists of an array with 3 values and a nested object with 3 members");
}

#include "couttast/tinytast.hpp"
#include "wwjson.hpp"
#include "test_data.h"
#include <string>
#include <fstream>
#include <filesystem>

namespace test {

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
    
    dst = builder.GetResult();
}

} // namespace test

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
#ifndef WWJSON_PERF_TEST_DATA_H
#define WWJSON_PERF_TEST_DATA_H

#include <string>

namespace test
{

// wwjson implementation namespace
namespace wwjson {

/**
 * @brief Generate JSON data of specified approximate size using RawBuilder
 * 
 * @param dst Output string to store generated JSON
 * @param size Target size in kilobytes (k)
 */
void BuildJson(std::string& dst, double size);

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
void BuildJson(std::string& dst, int n, int size = 1);

} // namespace wwjson

// yyjson implementation namespace
namespace yyjson {

/**
 * @brief Generate JSON data of specified size using yyjson
 * 
 * @param dst Output string to store generated JSON
 * @param size Target size in kilobytes (k)
 * @note hardly to estimate the size, so not implemented.
 */
void BuildJson(std::string& dst, double size);

/**
 * @brief Generate JSON data with specified number of items using yyjson.
 *
 * The generated JSON structure is like above test::wwjson::BuildJson .
 * 
 * @param dst Output string to store generated JSON
 * @param n Number of items to generate (each item = array + nested object)
 */
void BuildJson(std::string& dst, int n);

} // namespace yyjson

} // namespace test

#endif // WWJSON_PERF_TEST_DATA_H

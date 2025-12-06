#ifndef WWJSON_PERF_TEST_DATA_H
#define WWJSON_PERF_TEST_DATA_H

#include <string>

namespace test
{

/**
 * @brief Generate JSON data of specified size using RawBuilder
 * 
 * @param dst Output string to store generated JSON
 * @param size Target size in kilobytes (k)
 */
void BuildJson(std::string& dst, double size);

/**
 * @brief Generate JSON data with specified number of items
 * 
 * @param dst Output string to store generated JSON
 * @param n Number of items to generate (each item = array + nested object)
 */
void BuildJson(std::string& dst, int n);

// Integer array building functions for performance testing
void BuildTinyIntArray(std::string& dst, uint8_t start, int count);
void BuildShortIntArray(std::string& dst, uint16_t start, int count);
void BuildIntArray(std::string& dst, uint32_t start, int count);
void BuildBigIntArray(std::string& dst, uint64_t start, int count);

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
 * @brief Generate JSON data with specified number of items using yyjson
 * 
 * @param dst Output string to store generated JSON
 * @param n Number of items to generate (each item = array + nested object)
 */
void BuildJson(std::string& dst, int n);

// Integer array building functions for performance testing using yyjson
void BuildTinyIntArray(std::string& dst, uint8_t start, int count);
void BuildShortIntArray(std::string& dst, uint16_t start, int count);
void BuildIntArray(std::string& dst, uint32_t start, int count);
void BuildBigIntArray(std::string& dst, uint64_t start, int count);

} // namespace yyjson

} // namespace test

#endif // WWJSON_PERF_TEST_DATA_H

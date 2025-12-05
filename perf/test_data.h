#ifndef WWJSON_PERF_TEST_DATA_H
#define WWJSON_PERF_TEST_DATA_H

#include <string>

namespace test {

/**
 * @brief Generate JSON data of specified size using RawBuilder
 * 
 * @param dst Output string to store generated JSON
 * @param size Target size in kilobytes (k)
 */
void BuildJson(std::string& dst, double size);

} // namespace test

#endif // WWJSON_PERF_TEST_DATA_H
#ifndef WWJSON_PERF_ARGV_H
#define WWJSON_PERF_ARGV_H

#include "couttast/tastargv.hpp"

namespace test {

/**
 * @brief Command line argument handler for performance tests
 * 
 * Handles the --loop=n parameter to control the number of iterations
 * in performance test cases.
 */
struct CArgv
{
    int loop = 1000; // Default loop count
    int items = 1000; // count of json items, effect it's size
    uint64_t start = 0;   // the start index for json generation
    int size = 1; // Estimated size in kilobytes for pre-allocation
    
    CArgv() {
        // Bind the --loop command line argument
        BIND_ARGV(loop);
        BIND_ARGV(items);
        BIND_ARGV(start);
        BIND_ARGV(size);
    }
};

} // namespace test

#endif // WWJSON_PERF_ARGV_H

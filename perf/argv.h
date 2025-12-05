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
struct CArgv {
    int loop = 1000; // Default loop count
    
    CArgv() {
        // Bind the --loop command line argument
        BIND_ARGV(loop);
    }
};

} // namespace test

#endif // WWJSON_PERF_ARGV_H
#ifndef WWJSON_PERF_RELATIVE_PERF_H
#define WWJSON_PERF_RELATIVE_PERF_H

#include <chrono>
#include <iostream>
#include <string>

namespace test {
namespace perf {

/**
 * @brief CRTP template class for relative performance testing
 * 
 * This class uses the Curiously Recurring Template Pattern (CRTP) to provide
 * a framework for comparing the performance of two methods (methodA and methodB)
 * in a derived class. It runs the methods alternately in batches and calculates
 * the relative performance ratio.
 * 
 * @tparam Derived The derived class that implements methodA and methodB
 */
template <typename Derived>
class RelativeTimer {
public:
    /**
     * @brief Run relative performance test
     *
     * Executes methodA and methodB alternately in batches for the specified number
     * of loops. Returns the performance ratio (timeA/timeB).
     *
     * @param loop Total number of iterations for each method
     * @param batch Number of batches to divide the loops into (default: 10)
     * @param timeA_ms Optional pointer to store methodA's total execution time in milliseconds
     * @param timeB_ms Optional pointer to store methodB's total execution time in milliseconds
     * @return double Performance ratio (timeA/timeB). Values < 1 mean A is faster,
     *               values > 1 mean B is faster, value = 1 means equal performance
     */
    double run(int loop, int batch = 10, double* timeA_ms = nullptr, double* timeB_ms = nullptr) {
        // Handle special values for safety
        if (batch <= 0) batch = 1;
        if (loop <= 0) loop = 1;
        
        int inner_loop = loop / batch;
        if (inner_loop <= 0) inner_loop = 1;
        
        auto& derived = static_cast<Derived&>(*this);
        
        // Warm up
        derived.methodA();
        derived.methodB();
        
        // Time accumulation
        double timeA = 0.0;
        double timeB = 0.0;
        
        // Run in alternating batches
        for (int i = 0; i < batch; ++i) {
            // Time methodA
            auto startA = std::chrono::high_resolution_clock::now();
            for (int j = 0; j < inner_loop; ++j) {
                derived.methodA();
            }
            auto endA = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedA = endA - startA;
            timeA += elapsedA.count();
            
            // Time methodB
            auto startB = std::chrono::high_resolution_clock::now();
            for (int j = 0; j < inner_loop; ++j) {
                derived.methodB();
            }
            auto endB = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedB = endB - startB;
            timeB += elapsedB.count();
        }
        
        // Convert to milliseconds for output if requested
        if (timeA_ms) *timeA_ms = timeA * 1000.0;
        if (timeB_ms) *timeB_ms = timeB * 1000.0;
        
        // Calculate and return ratio
        if (timeB == 0.0) {
            // Avoid division by zero
            return timeA == 0.0 ? 1.0 : 1000.0;  // Large value indicating A is much slower
        }
        return timeA / timeB;
    }
    
    /**
     * @brief Run performance test and print results
     * 
     * @param testName Name of the test for display
     * @param methodALabel Label for method A
     * @param methodBLabel Label for method B
     * @param loop Total number of iterations for each method
     * @param batch Number of batches to divide the loops into
     */
    double runAndPrint(const std::string& testName,
                      const std::string& methodALabel,
                      const std::string& methodBLabel,
                      int loop, int batch = 10) {
        double timeA_ms, timeB_ms;
        double ratio = run(loop, batch, &timeA_ms, &timeB_ms);
        
        std::cout << "=== " << testName << " ===" << std::endl;
        std::cout << "Loops: " << loop << ", Batches: " << batch << std::endl;
        std::cout << "Performance ratio (" << methodALabel << "/" << methodBLabel << ") "
                  << timeA_ms << " ms / " << timeB_ms << " ms = " << ratio << std::endl;
        
        if (ratio < 0.95) {
            std::cout << methodALabel << " is " << (1.0/ratio - 1.0) * 100 << "% faster" << std::endl;
        } else if (ratio > 1.05) {
            std::cout << methodBLabel << " is " << (ratio - 1.0) * 100 << "% faster" << std::endl;
        } else {
            std::cout << "Performance is approximately equal" << std::endl;
        }
        
        return ratio;
    }
};

} // namespace perf
} // namespace test

#endif // WWJSON_PERF_RELATIVE_PERF_H
#ifndef WWJSON_PERF_RELATIVE_PERF_H
#define WWJSON_PERF_RELATIVE_PERF_H

#include <chrono>
#include <iostream>
#include <string>
#include <limits>
#include <cmath>

namespace test {
namespace perf {

/**
 * @brief Concept structure for documenting RelativeTimer requirements
 * 
 * This struct serves as documentation for the expected interface of classes
 * that inherit from RelativeTimer. In C++17, concepts are not supported, but
 * this struct provides a clear reference for the expected methods.
 */
struct RelativeTimerConcept {
    void methodA();        // Required: First method to compare
    void methodB();        // Required: Second method to compare
    bool methodVerify();   // Optional: Verification method (recommended to override)
    std::string testName;  // Optional: Test scenario description
    std::string labelA;    // Optional: Description for method A
    std::string labelB;    // Optional: Description for method B
};

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
     * @brief Verification method for functional correctness
     * 
     * This method should be overridden by derived classes to verify that
     * methodA and methodB produce equivalent results. The default implementation
     * returns true (no verification).
     * 
     * @return bool True if methods are functionally equivalent, false otherwise
     */
    bool methodVerify() {
        return true;
    }
    
    /**
     * @brief Run relative performance test with verification
     *
     * Executes methodA and methodB alternately in batches for the specified number
     * of loops. Returns the performance ratio (timeA/timeB). Includes a verification
     * phase to ensure functional correctness before performance measurement.
     *
     * @param loop Total number of iterations for each method
     * @param batch Number of batches to divide the loops into (default: 10)
     * @param timeA_ms Optional pointer to store methodA's total execution time in milliseconds
     * @param timeB_ms Optional pointer to store methodB's total execution time in milliseconds
     * @return double Performance ratio (timeA/timeB). Returns NaN if verification fails
     */
    double run(int loop, int batch = 10, double* timeA_ms = nullptr, double* timeB_ms = nullptr) {
        // Handle special values for safety
        if (batch <= 0) batch = 1;
        if (loop <= 0) loop = 1;
        
        int inner_loop = loop / batch;
        if (inner_loop <= 0) inner_loop = 1;
        
        auto& derived = static_cast<Derived&>(*this);
        
        // Verification phase - check functional correctness before performance testing
        if (!derived.methodVerify()) {
            // Return NaN to indicate verification failure
            if (timeA_ms) *timeA_ms = 0.0;
            if (timeB_ms) *timeB_ms = 0.0;
            return std::numeric_limits<double>::quiet_NaN();
        }
        
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
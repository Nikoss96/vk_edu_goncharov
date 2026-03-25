#include "process_pool.h"
#include "future.h"

#include <iostream>
#include <chrono>
#include <vector>

int main() {
    std::cout << "=== ProcessPool Demo ===" << std::endl;
    std::cout << "Parent Process ID: " << GetCurrentProcessId() << std::endl;
    std::cout << std::endl;
    
    try {
        ProcessPool pool(4);
        std::cout << "ProcessPool created with " << pool.ProcessCount() << " worker processes" << std::endl;
        
        // Выводим ID всех worker процессов
        auto worker_pids = pool.GetWorkerProcessIds();
        std::cout << "Worker Process IDs: ";
        for (size_t i = 0; i < worker_pids.size(); ++i) {
            std::cout << worker_pids[i];
            if (i < worker_pids.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl << std::endl;
        
        // Test 1: Simple calculations
        std::cout << "Test 1: Simple calculations in separate processes" << std::endl;
        auto f1 = pool.SubmitAdd(10, 20);
        auto f2 = pool.SubmitMultiply(5, 7);
        auto f3 = pool.SubmitAdd(100, 200);
        
        std::cout << "10 + 20 = " << f1.get() << std::endl;
        std::cout << "5 * 7 = " << f2.get() << std::endl;
        std::cout << "100 + 200 = " << f3.get() << std::endl;
        std::cout << std::endl;
        
        // Test 2: Fibonacci calculations (CPU intensive) in separate processes
        std::cout << "Test 2: CPU intensive tasks (Fibonacci) in separate processes" << std::endl;
        std::vector<Future<int>> futures;
        std::vector<int> inputs = {35, 36, 37, 38};
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int n : inputs) {
            futures.push_back(pool.SubmitFibonacci(n));
        }
        
        for (size_t i = 0; i < futures.size(); ++i) {
            std::cout << "fibonacci(" << inputs[i] << ") = " << futures[i].get() << std::endl;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Time: " << duration.count() << " ms" << std::endl;
        std::cout << std::endl;
        
        // Test 3: Exception handling across processes
        std::cout << "Test 3: Exception handling across processes" << std::endl;
        auto f4 = pool.SubmitException();
        
        try {
            f4.get();
        } catch (const std::exception& e) {
            std::cout << "Caught exception from worker process: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        // Test 4: wait() and is_ready() with process
        std::cout << "Test 4: wait() and is_ready() with worker process" << std::endl;
        auto f5 = pool.SubmitSleepReturn(500, 42);
        
        std::cout << "Is ready before wait: " << std::boolalpha << f5.is_ready() << std::endl;
        f5.wait();
        std::cout << "Is ready after wait: " << std::boolalpha << f5.is_ready() << std::endl;
        std::cout << "Result: " << f5.get() << std::endl;
        std::cout << std::endl;
        
        // Test 5: Multiple tasks to show process reuse
        std::cout << "Test 5: Multiple tasks showing process pool reuse" << std::endl;
        std::vector<Future<int>> many_futures;
        for (int i = 0; i < 12; ++i) {
            many_futures.push_back(pool.SubmitMultiply(i, i));
        }
        
        std::cout << "Results: ";
        for (size_t i = 0; i < many_futures.size(); ++i) {
            std::cout << many_futures[i].get();
            if (i < many_futures.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl << std::endl;
        
        std::cout << "=== All tests completed ===" << std::endl;
        std::cout << "Note: Tasks were executed in separate OS processes!" << std::endl;
        std::cout << "You can verify this in Task Manager during execution." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

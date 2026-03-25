#include "thread_pool.h"
#include "future.h"

#include <iostream>
#include <chrono>
#include <vector>
#include <string>

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void print_result(const std::string& name, int result) {
    std::cout << name << " = " << result << std::endl;
}

int main() {
    std::cout << "=== ThreadPool Demo ===" << std::endl;
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << std::endl;
    
    ThreadPool pool(4);
    std::cout << "ThreadPool created with " << pool.ThreadCount() << " threads" << std::endl;
    std::cout << std::endl;

    // Test 1: Simple calculations
    std::cout << "Test 1: Simple calculations" << std::endl;
    auto f1 = pool.Submit([]() { return 10 + 20; });
    auto f2 = pool.Submit([](int x, int y) { return x * y; }, 5, 7);
    auto f3 = pool.Submit([]() { return std::string("Hello from thread!"); });
    
    std::cout << "Result 1: " << f1.get() << std::endl;
    std::cout << "Result 2: " << f2.get() << std::endl;
    std::cout << "Result 3: " << f3.get() << std::endl;
    std::cout << std::endl;

    // Test 2: Fibonacci calculations (CPU intensive)
    std::cout << "Test 2: CPU intensive tasks (Fibonacci)" << std::endl;
    std::vector<Future<int>> futures;
    std::vector<int> inputs = {35, 36, 37, 38};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int n : inputs) {
        futures.push_back(pool.Submit(fibonacci, n));
    }
    
    for (size_t i = 0; i < futures.size(); ++i) {
        std::cout << "fibonacci(" << inputs[i] << ") = " << futures[i].get() << std::endl;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Time: " << duration.count() << " ms" << std::endl;
    std::cout << std::endl;

    // Test 3: void functions
    std::cout << "Test 3: Void functions" << std::endl;
    auto f4 = pool.Submit(print_result, "Task 1", 100);
    auto f5 = pool.Submit(print_result, "Task 2", 200);
    auto f6 = pool.Submit(print_result, "Task 3", 300);
    
    f4.get();
    f5.get();
    f6.get();
    std::cout << std::endl;

    // Test 4: Exception handling
    std::cout << "Test 4: Exception handling" << std::endl;
    auto f7 = pool.Submit([]() -> int {
        throw std::runtime_error("Test exception");
        return 0;
    });
    
    try {
        f7.get();
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    std::cout << std::endl;

    // Test 5: wait() and is_ready()
    std::cout << "Test 5: wait() and is_ready()" << std::endl;
    auto f8 = pool.Submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return 42;
    });
    
    std::cout << "Is ready before wait: " << std::boolalpha << f8.is_ready() << std::endl;
    f8.wait();
    std::cout << "Is ready after wait: " << std::boolalpha << f8.is_ready() << std::endl;
    std::cout << "Result: " << f8.get() << std::endl;
    std::cout << std::endl;

    std::cout << "=== All tests completed ===" << std::endl;
    
    return 0;
}

#include "apply_function.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <cmath>

static void BM_SmallVector_SimpleOp_SingleThread(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> data(100);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<int>(i);
        }
        
        std::function<void(int&)> transform = [](int& x) { x = x * 2 + 1; };
        ApplyFunction(data, transform, 1);
        
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_SmallVector_SimpleOp_SingleThread);

static void BM_SmallVector_SimpleOp_MultiThread(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> data(100);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<int>(i);
        }
        
        std::function<void(int&)> transform = [](int& x) { x = x * 2 + 1; };
        ApplyFunction(data, transform, 4);
        
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_SmallVector_SimpleOp_MultiThread);

static void BM_TinyVector_SingleThread(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> data(10);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<int>(i);
        }
        
        std::function<void(int&)> transform = [](int& x) { x += 5; };
        ApplyFunction(data, transform, 1);
        
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_TinyVector_SingleThread);

static void BM_TinyVector_MultiThread(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> data(10);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<int>(i);
        }
        
        std::function<void(int&)> transform = [](int& x) { x += 5; };
        ApplyFunction(data, transform, 8);
        
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_TinyVector_MultiThread);

static void BM_LargeVector_ComplexOp_SingleThread(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<double> data(1000000);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<double>(i);
        }
        
        std::function<void(double&)> transform = [](double& x) {
            x = std::sqrt(x) + std::sin(x) * std::cos(x) + std::log(x + 1.0);
        };
        ApplyFunction(data, transform, 1);
        
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_LargeVector_ComplexOp_SingleThread);

static void BM_LargeVector_ComplexOp_MultiThread(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<double> data(1000000);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<double>(i);
        }
        
        std::function<void(double&)> transform = [](double& x) {
            x = std::sqrt(x) + std::sin(x) * std::cos(x) + std::log(x + 1.0);
        };
        ApplyFunction(data, transform, 8);
        
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_LargeVector_ComplexOp_MultiThread);

static void BM_ComputeIntensive_SingleThread(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> data(50000);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<int>(i) + 1;
        }
        
        std::function<void(int&)> transform = [](int& x) {
            int sum = 0;
            for (int i = 1; i <= x; ++i) {
                if (x % i == 0) {
                    sum += i;
                }
            }
            x = sum;
        };
        ApplyFunction(data, transform, 1);
        
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_ComputeIntensive_SingleThread);

static void BM_ComputeIntensive_MultiThread(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> data(50000);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<int>(i) + 1;
        }
        
        std::function<void(int&)> transform = [](int& x) {
            int sum = 0;
            for (int i = 1; i <= x; ++i) {
                if (x % i == 0) {
                    sum += i;
                }
            }
            x = sum;
        };
        ApplyFunction(data, transform, 8);
        
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_ComputeIntensive_MultiThread);

static void BM_LargeVector_ModerateOp_SingleThread(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<long long> data(5000000);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<long long>(i);
        }
        
        std::function<void(long long&)> transform = [](long long& x) {
            x = (x * x + x * 3 + 7) % 1000000007;
        };
        ApplyFunction(data, transform, 1);
        
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_LargeVector_ModerateOp_SingleThread);

static void BM_LargeVector_ModerateOp_MultiThread(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<long long> data(5000000);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<long long>(i);
        }
        
        std::function<void(long long&)> transform = [](long long& x) {
            x = (x * x + x * 3 + 7) % 1000000007;
        };
        ApplyFunction(data, transform, 8);
        
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_LargeVector_ModerateOp_MultiThread);

BENCHMARK_MAIN();

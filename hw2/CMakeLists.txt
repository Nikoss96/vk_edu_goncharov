cmake_minimum_required(VERSION 3.14)
project(hw2_processes_and_threads)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent для загрузки gtest и benchmark
include(FetchContent)

# Google Test
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Google Benchmark
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# Заголовочный файл с реализацией
include_directories(${CMAKE_SOURCE_DIR})

# Тесты
add_executable(tests test.cpp)
target_link_libraries(tests gtest gtest_main)

# Бенчмарки
add_executable(benchmarks benchmark.cpp)
target_link_libraries(benchmarks benchmark::benchmark)

enable_testing()
add_test(NAME tests COMMAND tests)

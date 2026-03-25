cmake_minimum_required(VERSION 3.16)

project(thread_pool_future LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(thread_pool_compile_check compile_check.cpp)
target_link_libraries(thread_pool_compile_check PRIVATE Threads::Threads)

add_executable(thread_pool_demo demo.cpp)
target_link_libraries(thread_pool_demo PRIVATE Threads::Threads)

# ProcessPool executables
add_executable(process_worker process_worker.cpp)
target_link_libraries(process_worker PRIVATE Threads::Threads)

add_executable(process_pool_demo process_demo.cpp)
target_link_libraries(process_pool_demo PRIVATE Threads::Threads)

# Comparison demo
add_executable(comparison comparison.cpp)
target_link_libraries(comparison PRIVATE Threads::Threads)

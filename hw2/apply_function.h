#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <algorithm>

template <typename T>
void ApplyFunction(std::vector<T>& data,
                   const std::function<void(T&)>& transform,
                   const int threadCount = 1) {
    if (data.empty()) {
        return;
    }

    // Если потоков больше чем элементов, используем количество элементов
    int actualThreadCount = std::min(threadCount, static_cast<int>(data.size()));
    
    // Если только один поток, выполняем последовательно
    if (actualThreadCount <= 1) {
        for (auto& element : data) {
            transform(element);
        }
        return;
    }

    // Разбиваем работу между потоками
    std::vector<std::thread> threads;
    threads.reserve(actualThreadCount);
    
    size_t chunkSize = data.size() / actualThreadCount;
    size_t remainder = data.size() % actualThreadCount;

    size_t startIdx = 0;
    for (int i = 0; i < actualThreadCount; ++i) {
        // Распределяем остаток по первым потокам
        size_t currentChunkSize = chunkSize + (i < remainder ? 1 : 0);
        size_t endIdx = startIdx + currentChunkSize;

        threads.emplace_back([&data, &transform, startIdx, endIdx]() {
            for (size_t j = startIdx; j < endIdx; ++j) {
                transform(data[j]);
            }
        });

        startIdx = endIdx;
    }

    // Ждем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
}

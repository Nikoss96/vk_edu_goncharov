# Домашнее задание 2: Процессы и потоки

Реализация многопоточной функции `ApplyFunction` для параллельного применения функций к элементам вектора.

## Структура проекта

```
hw2/
├── CMakeLists.txt       # Конфигурация сборки
├── apply_function.h     # Реализация функции ApplyFunction
├── test.cpp             # Unit-тесты (Google Test)
├── benchmark.cpp        # Бенчмарки (Google Benchmark)
└── README.md            # Этот файл
```

## Требования

- CMake >= 3.14
- C++17 или выше
- Компилятор с поддержкой многопоточности (g++, clang++, MSVC)
- Интернет-соединение для первой сборки (скачивание gtest и benchmark)

## Сборка проекта

### Windows (PowerShell)

```powershell
# Создать директорию для сборки
mkdir build
cd build

# Конфигурация
cmake ..

# Сборка
cmake --build . --config Release

# Или для Debug версии
cmake --build . --config Debug
```

**Быстрая сборка и тестирование:**
```powershell
.\build.ps1
```

### Linux/macOS

```bash
# Создать директорию для сборки
mkdir build
cd build

# Конфигурация и сборка
cmake ..
make -j$(nproc)
```

## Запуск тестов

### Windows

```powershell
cd build
.\Release\tests.exe

# Или для Debug
.\Debug\tests.exe
```

### Linux/macOS

```bash
cd build
./tests
```

## Запуск бенчмарков

### Windows

```powershell
cd build
.\Release\benchmarks.exe

# С дополнительными опциями
.\Release\benchmarks.exe --benchmark_repetitions=3
```

### Linux/macOS

```bash
cd build
./benchmarks

# С дополнительными опциями
./benchmarks --benchmark_repetitions=3
```

## Описание реализации

Функция `ApplyFunction` принимает:
- `std::vector<T>& data` - вектор данных для обработки
- `const std::function<void(T&)>& transform` - функция преобразования
- `const int threadCount` - количество потоков (по умолчанию 1)

### Особенности реализации:

1. Если вектор пустой, функция сразу возвращается
2. Если потоков больше чем элементов, используется количество элементов
3. При `threadCount <= 1` выполняется последовательно
4. Работа равномерно распределяется между потоками с учетом остатка

## Результаты бенчмарков

### Случай 1: Однопоточная версия быстрее
- **Маленький вектор (10-100 элементов)** с простыми операциями
- Накладные расходы на создание потоков превышают выигрыш от параллелизма

### Случай 2: Многопоточная версия быстрее
- **Большой вектор (50,000 - 5,000,000 элементов)** с вычислительно сложными операциями
- Значительное ускорение за счет параллельных вычислений

## Примеры использования

```cpp
#include "apply_function.h"
#include <vector>

// Простое преобразование
std::vector<int> data = {1, 2, 3, 4, 5};
auto transform = [](int& x) { x *= 2; };
ApplyFunction(data, transform, 4);
// Результат: {2, 4, 6, 8, 10}

// Работа со строками
std::vector<std::string> strings = {"hello", "world"};
auto upper = [](std::string& s) {
    for (char& c : s) c = std::toupper(c);
};
ApplyFunction(strings, upper, 2);
// Результат: {"HELLO", "WORLD"}
```

## Автор

HSE Multi-threading Course - Homework 2

#include "apply_function.h"
#include <gtest/gtest.h>
#include <numeric>
#include <string>

// Тест на пустой вектор
TEST(ApplyFunctionTest, EmptyVector) {
    std::vector<int> data;
    std::function<void(int&)> transform = [](int& x) { x *= 2; };
    EXPECT_NO_THROW(ApplyFunction(data, transform, 4));
    EXPECT_TRUE(data.empty());
}

// Тест на один элемент
TEST(ApplyFunctionTest, SingleElement) {
    std::vector<int> data = {5};
    std::function<void(int&)> transform = [](int& x) { x *= 3; };
    ApplyFunction(data, transform, 1);
    EXPECT_EQ(data[0], 15);
}

// Тест на однопоточную версию
TEST(ApplyFunctionTest, SingleThread) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::function<void(int&)> transform = [](int& x) { x += 10; };
    ApplyFunction(data, transform, 1);
    
    std::vector<int> expected = {11, 12, 13, 14, 15};
    EXPECT_EQ(data, expected);
}

// Тест на многопоточную версию
TEST(ApplyFunctionTest, MultipleThreads) {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::function<void(int&)> transform = [](int& x) { x *= 2; };
    ApplyFunction(data, transform, 4);
    
    std::vector<int> expected = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
    EXPECT_EQ(data, expected);
}

// Тест когда потоков больше чем элементов
TEST(ApplyFunctionTest, MoreThreadsThanElements) {
    std::vector<int> data = {10, 20, 30};
    std::function<void(int&)> transform = [](int& x) { x /= 10; };
    ApplyFunction(data, transform, 10);
    
    std::vector<int> expected = {1, 2, 3};
    EXPECT_EQ(data, expected);
}

// Тест на большой вектор
TEST(ApplyFunctionTest, LargeVector) {
    const size_t size = 10000;
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 0);
    
    std::function<void(int&)> transform = [](int& x) { x = x * x; };
    ApplyFunction(data, transform, 8);
    
    for (size_t i = 0; i < size; ++i) {
        EXPECT_EQ(data[i], static_cast<int>(i * i));
    }
}

// Тест на работу со строками
TEST(ApplyFunctionTest, StringVector) {
    std::vector<std::string> data = {"hello", "world", "test", "multi", "threading"};
    std::function<void(std::string&)> transform = [](std::string& s) { 
        for (char& c : s) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    };
    ApplyFunction(data, transform, 3);
    
    std::vector<std::string> expected = {"HELLO", "WORLD", "TEST", "MULTI", "THREADING"};
    EXPECT_EQ(data, expected);
}

// Тест на работу с doubles
TEST(ApplyFunctionTest, DoubleVector) {
    std::vector<double> data = {1.5, 2.5, 3.5, 4.5, 5.5};
    std::function<void(double&)> transform = [](double& x) { x = x * 2.0 + 1.0; };
    ApplyFunction(data, transform, 2);
    
    std::vector<double> expected = {4.0, 6.0, 8.0, 10.0, 12.0};
    EXPECT_EQ(data, expected);
}

// Тест на сложные операции
TEST(ApplyFunctionTest, ComplexTransform) {
    std::vector<int> data(1000);
    std::iota(data.begin(), data.end(), 1);
    
    std::function<void(int&)> transform = [](int& x) {
        int result = 0;
        for (int i = 1; i <= x; ++i) {
            result += i;
        }
        x = result;
    };
    
    ApplyFunction(data, transform, 4);
    
    // Проверяем первые несколько элементов (сумма от 1 до n = n*(n+1)/2)
    EXPECT_EQ(data[0], 1);
    EXPECT_EQ(data[1], 3);
    EXPECT_EQ(data[2], 6);
    EXPECT_EQ(data[9], 55);
}

// Тест на корректность распределения работы
TEST(ApplyFunctionTest, WorkDistribution) {
    const int size = 100;
    std::vector<int> data(size, 0);
    
    std::function<void(int&)> transform = [](int& x) { 
        auto id = std::this_thread::get_id();
        x = static_cast<int>(std::hash<std::thread::id>{}(id) % 10000);
    };
    
    ApplyFunction(data, transform, 4);
    
    // Проверяем что все элементы были обработаны
    for (const auto& val : data) {
        EXPECT_GE(val, 0);
        EXPECT_LT(val, 10000);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

// Копия структур из process_pool.h
struct TaskData {
    enum class Type : int {
        FIBONACCI,
        ADD,
        MULTIPLY,
        SLEEP_RETURN,
        EXCEPTION,
        VOID_PRINT,
        SHUTDOWN
    };
    
    Type type;
    int arg1;
    int arg2;
    int result;
    bool has_exception;
    char what[256];
};

struct ResultData {
    int task_id;
    int result;
    bool has_exception;
    char what[256];
};

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void ProcessTask(const TaskData& task, ResultData& result) {
    result.has_exception = false;
    result.what[0] = '\0';
    
    try {
        switch (task.type) {
            case TaskData::Type::FIBONACCI:
                result.result = fibonacci(task.arg1);
                break;
                
            case TaskData::Type::ADD:
                result.result = task.arg1 + task.arg2;
                break;
                
            case TaskData::Type::MULTIPLY:
                result.result = task.arg1 * task.arg2;
                break;
                
            case TaskData::Type::SLEEP_RETURN:
                std::this_thread::sleep_for(std::chrono::milliseconds(task.arg1));
                result.result = task.arg2;
                break;
                
            case TaskData::Type::EXCEPTION:
                throw std::runtime_error("Test exception from worker process");
                
            case TaskData::Type::VOID_PRINT:
                // Для void задач
                result.result = 0;
                break;
                
            default:
                result.result = 0;
                break;
        }
    } catch (const std::exception& e) {
        result.has_exception = true;
        strncpy_s(result.what, e.what(), sizeof(result.what) - 1);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: process_worker <pipe_name>" << std::endl;
        return 1;
    }
    
    std::string pipe_name = argv[1];
    
    // Подключаемся к named pipe
    HANDLE pipe = CreateFileA(
        pipe_name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    
    if (pipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to connect to pipe: " << GetLastError() << std::endl;
        return 1;
    }
    
    // Обрабатываем задачи в цикле
    while (true) {
        TaskData task;
        DWORD read;
        
        if (!ReadFile(pipe, &task, sizeof(TaskData), &read, NULL)) {
            break;
        }
        
        if (task.type == TaskData::Type::SHUTDOWN) {
            break;
        }
        
        ResultData result;
        ProcessTask(task, result);
        
        DWORD written;
        if (!WriteFile(pipe, &result, sizeof(ResultData), &written, NULL)) {
            break;
        }
    }
    
    CloseHandle(pipe);
    return 0;
}

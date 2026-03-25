#pragma once

#include "future.h"

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>
#include <sstream>
#include <iostream>

namespace detail {

// Структура для передачи задачи через shared memory
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

// Структура для передачи результата
struct ResultData {
    int task_id;
    int result;
    bool has_exception;
    char what[256];
};

class WorkerProcess {
public:
    WorkerProcess(int worker_id, const std::string& pipe_name) 
        : worker_id_(worker_id), pipe_name_(pipe_name) {
        
        // Создаем named pipe для коммуникации с worker процессом
        std::string full_pipe_name = "\\\\.\\pipe\\" + pipe_name_ + "_" + std::to_string(worker_id_);
        
        pipe_handle_ = CreateNamedPipeA(
            full_pipe_name.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1,
            sizeof(TaskData),
            sizeof(ResultData),
            0,
            NULL
        );
        
        if (pipe_handle_ == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Failed to create named pipe");
        }
        
        // Запускаем worker процесс
        StartWorkerProcess(full_pipe_name);
    }
    
    ~WorkerProcess() {
        // Отправляем команду завершения
        TaskData shutdown_task;
        shutdown_task.type = TaskData::Type::SHUTDOWN;
        DWORD written;
        WriteFile(pipe_handle_, &shutdown_task, sizeof(TaskData), &written, NULL);
        
        if (process_handle_ != INVALID_HANDLE_VALUE) {
            WaitForSingleObject(process_handle_, 5000);
            CloseHandle(process_handle_);
        }
        if (pipe_handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(pipe_handle_);
        }
    }
    
    void ExecuteTask(const TaskData& task, ResultData& result) {
        // Отправляем задачу в worker процесс
        DWORD written;
        if (!WriteFile(pipe_handle_, &task, sizeof(TaskData), &written, NULL)) {
            throw std::runtime_error("Failed to write to pipe");
        }
        
        // Читаем результат
        DWORD read;
        if (!ReadFile(pipe_handle_, &result, sizeof(ResultData), &read, NULL)) {
            throw std::runtime_error("Failed to read from pipe");
        }
    }
    
    DWORD GetProcessId() const {
        return process_id_;
    }
    
private:
    void StartWorkerProcess(const std::string& pipe_name) {
        // Получаем путь к текущему исполняемому файлу
        char exe_path[MAX_PATH];
        GetModuleFileNameA(NULL, exe_path, MAX_PATH);
        
        // Извлекаем директорию
        std::string exe_dir = exe_path;
        size_t last_slash = exe_dir.find_last_of("\\/");
        if (last_slash != std::string::npos) {
            exe_dir = exe_dir.substr(0, last_slash);
        }
        
        // Формируем команду для запуска worker процесса
        std::string worker_exe = exe_dir + "\\process_worker.exe";
        std::string cmd_line = worker_exe + " " + pipe_name;
        
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        
        if (!CreateProcessA(
            worker_exe.c_str(),
            const_cast<char*>(cmd_line.c_str()),
            NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            throw std::runtime_error("Failed to create worker process: " + std::to_string(GetLastError()));
        }
        
        process_handle_ = pi.hProcess;
        process_id_ = pi.dwProcessId;
        CloseHandle(pi.hThread);
        
        // Ждем подключения к pipe
        if (!ConnectNamedPipe(pipe_handle_, NULL)) {
            DWORD error = GetLastError();
            if (error != ERROR_PIPE_CONNECTED) {
                throw std::runtime_error("Failed to connect named pipe");
            }
        }
    }
    
    int worker_id_;
    std::string pipe_name_;
    HANDLE pipe_handle_ = INVALID_HANDLE_VALUE;
    HANDLE process_handle_ = INVALID_HANDLE_VALUE;
    DWORD process_id_ = 0;
};

} // namespace detail

class ProcessPool {
public:
    explicit ProcessPool(std::size_t process_count = std::thread::hardware_concurrency())
        : pipe_base_name_("ProcessPool_" + std::to_string(GetCurrentProcessId())) {
        
        if (process_count == 0) {
            process_count = 1;
        }
        
        // Создаем worker процессы
        for (std::size_t i = 0; i < process_count; ++i) {
            workers_.emplace_back(std::make_unique<detail::WorkerProcess>(i, pipe_base_name_));
        }
        
        // Запускаем поток диспетчера задач
        dispatcher_thread_ = std::thread([this] { DispatcherLoop(); });
    }
    
    ProcessPool(const ProcessPool&) = delete;
    ProcessPool& operator=(const ProcessPool&) = delete;
    
    ProcessPool(ProcessPool&&) = delete;
    ProcessPool& operator=(ProcessPool&&) = delete;
    
    ~ProcessPool() {
        Stop();
    }
    
    void Stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopping_) {
                return;
            }
            stopping_ = true;
        }
        
        cv_.notify_all();
        
        if (dispatcher_thread_.joinable()) {
            dispatcher_thread_.join();
        }
        
        workers_.clear();
    }
    
    // Специализированные методы Submit для поддерживаемых типов задач
    
    Future<int> SubmitFibonacci(int n) {
        auto state = std::make_shared<detail::SharedState<int>>();
        
        detail::TaskData task;
        task.type = detail::TaskData::Type::FIBONACCI;
        task.arg1 = n;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopping_) {
                throw std::runtime_error("submit on stopped ProcessPool");
            }
            pending_tasks_.push_back({task, state});
        }
        
        cv_.notify_one();
        return Future<int>(std::move(state));
    }
    
    Future<int> SubmitAdd(int a, int b) {
        auto state = std::make_shared<detail::SharedState<int>>();
        
        detail::TaskData task;
        task.type = detail::TaskData::Type::ADD;
        task.arg1 = a;
        task.arg2 = b;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopping_) {
                throw std::runtime_error("submit on stopped ProcessPool");
            }
            pending_tasks_.push_back({task, state});
        }
        
        cv_.notify_one();
        return Future<int>(std::move(state));
    }
    
    Future<int> SubmitMultiply(int a, int b) {
        auto state = std::make_shared<detail::SharedState<int>>();
        
        detail::TaskData task;
        task.type = detail::TaskData::Type::MULTIPLY;
        task.arg1 = a;
        task.arg2 = b;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopping_) {
                throw std::runtime_error("submit on stopped ProcessPool");
            }
            pending_tasks_.push_back({task, state});
        }
        
        cv_.notify_one();
        return Future<int>(std::move(state));
    }
    
    Future<int> SubmitSleepReturn(int ms, int value) {
        auto state = std::make_shared<detail::SharedState<int>>();
        
        detail::TaskData task;
        task.type = detail::TaskData::Type::SLEEP_RETURN;
        task.arg1 = ms;
        task.arg2 = value;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopping_) {
                throw std::runtime_error("submit on stopped ProcessPool");
            }
            pending_tasks_.push_back({task, state});
        }
        
        cv_.notify_one();
        return Future<int>(std::move(state));
    }
    
    Future<int> SubmitException() {
        auto state = std::make_shared<detail::SharedState<int>>();
        
        detail::TaskData task;
        task.type = detail::TaskData::Type::EXCEPTION;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopping_) {
                throw std::runtime_error("submit on stopped ProcessPool");
            }
            pending_tasks_.push_back({task, state});
        }
        
        cv_.notify_one();
        return Future<int>(std::move(state));
    }
    
    std::size_t ProcessCount() const noexcept {
        return workers_.size();
    }
    
    std::vector<DWORD> GetWorkerProcessIds() const {
        std::vector<DWORD> pids;
        for (const auto& worker : workers_) {
            pids.push_back(worker->GetProcessId());
        }
        return pids;
    }
    
private:
    struct PendingTask {
        detail::TaskData task;
        std::shared_ptr<detail::SharedState<int>> state;
    };
    
    void DispatcherLoop() {
        std::size_t next_worker = 0;
        
        while (true) {
            PendingTask pending;
            
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] {
                    return stopping_ || !pending_tasks_.empty();
                });
                
                if (stopping_ && pending_tasks_.empty()) {
                    return;
                }
                
                pending = std::move(pending_tasks_.front());
                pending_tasks_.pop_front();
            }
            
            // Выбираем worker процесс
            auto& worker = workers_[next_worker];
            next_worker = (next_worker + 1) % workers_.size();
            
            // Выполняем задачу в worker процессе
            try {
                detail::ResultData result;
                worker->ExecuteTask(pending.task, result);
                
                if (result.has_exception) {
                    pending.state->SetException(
                        std::make_exception_ptr(std::runtime_error(result.what))
                    );
                } else {
                    pending.state->SetValue(result.result);
                }
            } catch (const std::exception& e) {
                pending.state->SetException(std::current_exception());
            }
        }
    }
    
    std::string pipe_base_name_;
    std::vector<std::unique_ptr<detail::WorkerProcess>> workers_;
    std::thread dispatcher_thread_;
    
    std::deque<PendingTask> pending_tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopping_ = false;
};

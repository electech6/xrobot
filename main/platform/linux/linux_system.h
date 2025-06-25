#pragma once

#include "../system_interface.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>
#include <functional>
#include <cstdarg>
#include <iostream>

namespace platform {
namespace linux_platform {

class LinuxThreadConfig : public ThreadConfig {
public:
    LinuxThreadConfig() {
        default_config_.name = "linux_thread";
        default_config_.stack_size = 8192;
        default_config_.priority = 0;
    }
    
    void SetConfig(const Config& config) override {
        current_config_ = config;
    }
    
    Config GetDefaultConfig() override {
        return default_config_;
    }
    
private:
    Config default_config_;
    Config current_config_;
};

class LinuxLogger : public Logger {
public:
    void Log(LogLevel level, const char* tag, const char* format, ...) override {
        // 获取日志级别前缀
        const char* level_str = "";
        switch (level) {
            case LogLevel::kError:
                level_str = "E";
                break;
            case LogLevel::kWarning:
                level_str = "W";
                break;
            case LogLevel::kInfo:
                level_str = "I";
                break;
            case LogLevel::kDebug:
                level_str = "D";
                break;
            case LogLevel::kVerbose:
                level_str = "V";
                break;
            default:
                level_str = "I";
                break;
        }
        
        // 格式化消息
        va_list args;
        va_start(args, format);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        // 输出到标准输出
        std::cout << "[" << level_str << "][" << tag << "] " << buffer << std::endl;
    }
};

class LinuxEventGroup : public EventGroup {
public:
    LinuxEventGroup();
    ~LinuxEventGroup() override = default;
    
    SystemError SetBits(uint32_t bits) override;
    SystemError ClearBits(uint32_t bits) override;
    uint32_t WaitBits(uint32_t bits, bool clear_on_exit, bool wait_for_all, uint32_t timeout_ms) override;
    
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    uint32_t bits_ = 0;
};

class LinuxTimer : public Timer {
public:
    LinuxTimer();
    ~LinuxTimer() override;
    
    SystemError Start(uint64_t period_us, bool periodic = true) override;
    SystemError Stop() override;
    bool IsActive() const override;
    void SetCallback(Callback callback) override;
    
private:
    std::unique_ptr<std::thread> timer_thread_;
    std::atomic<bool> is_active_{false};
    std::atomic<bool> should_stop_{false};
    Callback callback_;
    uint64_t period_us_ = 0;
    bool periodic_ = true;
    
    void TimerLoop();
};

class LinuxEventLoop : public EventLoop {
public:
    LinuxEventLoop();
    ~LinuxEventLoop() override;
    
    SystemError Initialize() override;
    SystemError PostEvent(std::function<void()> event) override;
    SystemError Run() override;
    SystemError Stop() override;
    
private:
    std::queue<std::function<void()>> event_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> is_running_{false};
    std::atomic<bool> should_stop_{false};
};

class LinuxTaskManager : public TaskManager {
public:
    SystemError CreateTask(const TaskConfig& config, 
                         TaskFunction task_func, 
                         TaskHandle* handle) override;
    SystemError DeleteTask(TaskHandle handle) override;
    SystemError Delay(uint32_t ms) override;
    uint32_t GetTickCount() override;
    
private:
    std::vector<std::unique_ptr<std::thread>> tasks_;
    std::mutex tasks_mutex_;
    std::chrono::steady_clock::time_point start_time_;
};

class LinuxSystemInfo : public SystemInfo {
public:
    LinuxSystemInfo();
    std::string GetChipModel() override;
    uint32_t GetFreeHeapSize() override;
    uint32_t GetMinFreeHeapSize() override;
    std::string GetMacAddress() override;
    void Restart() override;
    
    // 实现新增方法
    uint32_t GetFlashSize() override;
    bool PrintTaskCpuUsage(uint32_t wait_ms) override;
    bool PrintTaskList() override;
    
    // 实现应用程序描述相关方法
    std::string GetAppName() override;
    std::string GetAppVersion() override;
    std::string GetAppDescription() override;
};

} // namespace linux_platform
} // namespace platform
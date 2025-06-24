#pragma once

#include "../system_interface.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>

namespace platform {
namespace linux_platform {

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
};

} // namespace linux_platform
} // namespace platform
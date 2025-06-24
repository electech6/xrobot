#pragma once

#include <functional>
#include <memory>
#include <string>
#include <cstdint>

namespace platform {

// 平台无关的错误码定义
enum class SystemError {
    kSuccess = 0,
    kFailed = -1,
    kInvalidParameter = -2,
    kOutOfMemory = -3,
    kTimeout = -4,
    kNotSupported = -5
};

// 存储接口前向声明
class Storage;

// 定时器接口
class Timer {
public:
    virtual ~Timer() = default;
    virtual SystemError Start(uint64_t period_us, bool periodic = true) = 0;
    virtual SystemError Stop() = 0;
    virtual bool IsActive() const = 0;
    
    using Callback = std::function<void()>;
    virtual void SetCallback(Callback callback) = 0;
};

// 事件循环接口
class EventLoop {
public:
    virtual ~EventLoop() = default;
    virtual SystemError Initialize() = 0;
    virtual SystemError PostEvent(std::function<void()> event) = 0;
    virtual SystemError Run() = 0;
    virtual SystemError Stop() = 0;
};

// 任务管理接口
class TaskManager {
public:
    virtual ~TaskManager() = default;
    
    struct TaskConfig {
        std::string name;
        uint32_t stack_size = 4096;
        int priority = 5;
    };
    
    using TaskFunction = std::function<void()>;
    using TaskHandle = void*;
    
    virtual SystemError CreateTask(const TaskConfig& config, 
                                 TaskFunction task_func, 
                                 TaskHandle* handle) = 0;
    virtual SystemError DeleteTask(TaskHandle handle) = 0;
    virtual SystemError Delay(uint32_t ms) = 0;
    virtual uint32_t GetTickCount() = 0;
};

// 系统信息接口
class SystemInfo {
public:
    virtual ~SystemInfo() = default;
    virtual std::string GetChipModel() = 0;
    virtual uint32_t GetFreeHeapSize() = 0;
    virtual uint32_t GetMinFreeHeapSize() = 0;
    virtual std::string GetMacAddress() = 0;
    virtual void Restart() = 0;
    
    // 新增方法 - 所有平台都必须实现
    virtual uint32_t GetFlashSize() = 0;
    virtual bool PrintTaskCpuUsage(uint32_t wait_ms) = 0;
    virtual bool PrintTaskList() = 0;
};

// 系统工厂类
class SystemFactory {
public:
    static std::unique_ptr<Timer> CreateTimer();
    static std::unique_ptr<EventLoop> CreateEventLoop();
    static std::unique_ptr<TaskManager> CreateTaskManager();
    static std::unique_ptr<SystemInfo> CreateSystemInfo();
    static std::unique_ptr<Storage> CreateStorage();
};

} // namespace platform
#pragma once

#include "../system_interface.h"
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <esp_flash.h>
#include <esp_mac.h>
#include <esp_system.h>
#include <esp_partition.h>
#include <esp_app_desc.h>
#include <esp_ota_ops.h>
#if CONFIG_IDF_TARGET_ESP32P4
#include "esp_wifi_remote.h"
#endif
#include <memory>

namespace platform {
namespace esp32 {

class Esp32EventGroup : public EventGroup {
public:
    Esp32EventGroup();
    ~Esp32EventGroup() override;
    
    SystemError SetBits(uint32_t bits) override;
    SystemError ClearBits(uint32_t bits) override;
    uint32_t WaitBits(uint32_t bits, bool clear_on_exit, bool wait_for_all, uint32_t timeout_ms) override;
    
private:
    EventGroupHandle_t event_group_ = nullptr;
};

class Esp32Timer : public Timer {
public:
    Esp32Timer();
    ~Esp32Timer() override;
    
    SystemError Start(uint64_t period_us, bool periodic = true) override;
    SystemError Stop() override;
    bool IsActive() const override;
    void SetCallback(Callback callback) override;
    
private:
    esp_timer_handle_t timer_handle_ = nullptr;
    Callback callback_;
    bool is_active_ = false;
    
    static void TimerCallback(void* arg);
};

class Esp32EventLoop : public EventLoop {
public:
    Esp32EventLoop();
    ~Esp32EventLoop() override;
    
    SystemError Initialize() override;
    SystemError PostEvent(std::function<void()> event) override;
    SystemError Run() override;
    SystemError Stop() override;
    
private:
    EventGroupHandle_t event_group_ = nullptr;
    bool is_running_ = false;
};

class Esp32TaskManager : public TaskManager {
public:
    SystemError CreateTask(const TaskConfig& config, 
                         TaskFunction task_func, 
                         TaskHandle* handle) override;
    SystemError DeleteTask(TaskHandle handle) override;
    SystemError Delay(uint32_t ms) override;
    uint32_t GetTickCount() override;
    
private:
    struct TaskContext {
        TaskFunction function;
    };
    
    static void TaskWrapper(void* param);
};

class Esp32SystemInfo : public SystemInfo {
public:
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

} // namespace esp32
} // namespace platform
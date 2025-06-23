#include "esp32_system.h"
#include <esp_log.h>
#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_mac.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <sstream>
#include <iomanip>

namespace platform {
namespace esp32 {

// Esp32Timer 实现
Esp32Timer::Esp32Timer() {
    esp_timer_create_args_t timer_args = {
        .callback = &Esp32Timer::TimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "platform_timer"
    };
    esp_timer_create(&timer_args, &timer_handle_);
}

Esp32Timer::~Esp32Timer() {
    if (timer_handle_) {
        Stop();
        esp_timer_delete(timer_handle_);
    }
}

SystemError Esp32Timer::Start(uint64_t period_us, bool periodic) {
    if (!timer_handle_ || !callback_) {
        return SystemError::kInvalidParameter;
    }
    
    esp_err_t err;
    if (periodic) {
        err = esp_timer_start_periodic(timer_handle_, period_us);
    } else {
        err = esp_timer_start_once(timer_handle_, period_us);
    }
    
    if (err == ESP_OK) {
        is_active_ = true;
        return SystemError::kSuccess;
    }
    return SystemError::kFailed;
}

SystemError Esp32Timer::Stop() {
    if (!timer_handle_) {
        return SystemError::kInvalidParameter;
    }
    
    esp_err_t err = esp_timer_stop(timer_handle_);
    is_active_ = false;
    return (err == ESP_OK) ? SystemError::kSuccess : SystemError::kFailed;
}

bool Esp32Timer::IsActive() const {
    return is_active_;
}

void Esp32Timer::SetCallback(Callback callback) {
    callback_ = std::move(callback);
}

void Esp32Timer::TimerCallback(void* arg) {
    auto* timer = static_cast<Esp32Timer*>(arg);
    if (timer && timer->callback_) {
        timer->callback_();
    }
}

// Esp32EventLoop 实现
Esp32EventLoop::Esp32EventLoop() {
    event_group_ = xEventGroupCreate();
}

Esp32EventLoop::~Esp32EventLoop() {
    Stop();
    if (event_group_) {
        vEventGroupDelete(event_group_);
    }
}

SystemError Esp32EventLoop::Initialize() {
    return event_group_ ? SystemError::kSuccess : SystemError::kFailed;
}

SystemError Esp32EventLoop::PostEvent(std::function<void()> event) {
    // 在ESP32上，我们使用任务通知机制来实现事件投递
    // 这里简化实现，直接在当前上下文执行
    if (event) {
        event();
        return SystemError::kSuccess;
    }
    return SystemError::kInvalidParameter;
}

SystemError Esp32EventLoop::Run() {
    is_running_ = true;
    return SystemError::kSuccess;
}

SystemError Esp32EventLoop::Stop() {
    is_running_ = false;
    return SystemError::kSuccess;
}

// Esp32TaskManager 实现
SystemError Esp32TaskManager::CreateTask(const TaskConfig& config, 
                                       TaskFunction task_func, 
                                       TaskHandle* handle) {
    if (!task_func || !handle) {
        return SystemError::kInvalidParameter;
    }
    
    auto* context = new TaskContext{std::move(task_func)};
    
    BaseType_t result = xTaskCreate(
        TaskWrapper,
        config.name.c_str(),
        config.stack_size,
        context,
        config.priority,
        reinterpret_cast<TaskHandle_t*>(handle)
    );
    
    if (result == pdPASS) {
        return SystemError::kSuccess;
    }
    
    delete context;
    return SystemError::kFailed;
}

SystemError Esp32TaskManager::DeleteTask(TaskHandle handle) {
    if (!handle) {
        return SystemError::kInvalidParameter;
    }
    
    vTaskDelete(static_cast<TaskHandle_t>(handle));
    return SystemError::kSuccess;
}

SystemError Esp32TaskManager::Delay(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
    return SystemError::kSuccess;
}

uint32_t Esp32TaskManager::GetTickCount() {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void Esp32TaskManager::TaskWrapper(void* param) {
    auto* context = static_cast<TaskContext*>(param);
    if (context && context->function) {
        context->function();
    }
    delete context;
    vTaskDelete(nullptr);
}

// Esp32SystemInfo 实现
std::string Esp32SystemInfo::GetChipModel() {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    std::string model;
    switch (chip_info.model) {
        case CHIP_ESP32:
            model = "ESP32";
            break;
        case CHIP_ESP32S2:
            model = "ESP32-S2";
            break;
        case CHIP_ESP32S3:
            model = "ESP32-S3";
            break;
        case CHIP_ESP32C3:
            model = "ESP32-C3";
            break;
        case CHIP_ESP32C6:
            model = "ESP32-C6";
            break;
        default:
            model = "Unknown ESP32";
            break;
    }
    
    return model + " Rev " + std::to_string(chip_info.revision);
}

uint32_t Esp32SystemInfo::GetFreeHeapSize() {
    return esp_get_free_heap_size();
}

uint32_t Esp32SystemInfo::GetMinFreeHeapSize() {
    return esp_get_minimum_free_heap_size();
}

std::string Esp32SystemInfo::GetMacAddress() {
    uint8_t mac[6];
    esp_err_t err = esp_read_mac(mac, ESP_MAC_WIFI_STA);
    if (err != ESP_OK) {
        return "00:00:00:00:00:00";
    }
    
    std::stringstream ss;
    for (int i = 0; i < 6; ++i) {
        if (i > 0) ss << ":";
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mac[i]);
    }
    return ss.str();
}

void Esp32SystemInfo::Restart() {
    esp_restart();
}

} // namespace esp32
} // namespace platform
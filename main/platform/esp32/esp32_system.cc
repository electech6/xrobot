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
#include "esp32_storage.h"

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
#if CONFIG_IDF_TARGET_ESP32P4
    esp_wifi_get_mac(WIFI_IF_STA, mac);
#else
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
#endif
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(mac_str);
}

void Esp32SystemInfo::Restart() {
    esp_restart();
}

// 实现GetFlashSize
uint32_t Esp32SystemInfo::GetFlashSize() {
    uint32_t flash_size;
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        return 0;
    }
    return flash_size;
}

// 实现PrintTaskCpuUsage
bool Esp32SystemInfo::PrintTaskCpuUsage(uint32_t wait_ms) {
    // 移植原来的PrintTaskCpuUsage实现
    TaskStatus_t *start_array = NULL, *end_array = NULL;
    UBaseType_t start_array_size, end_array_size;
    configRUN_TIME_COUNTER_TYPE start_run_time, end_run_time;
    bool success = false;
    uint32_t total_elapsed_time;

    // 实现原来的ESP32特定代码
    // ...

    return success;
}

// 实现PrintTaskList
bool Esp32SystemInfo::PrintTaskList() {
    char buffer[500];
    vTaskList(buffer);
    printf("Task list: \n%s", buffer);
    return true;
}

// 实现应用程序描述相关方法
std::string Esp32SystemInfo::GetAppName() {
    const esp_app_desc_t* app_desc = esp_app_get_description();
    return std::string(app_desc->project_name);
}

std::string Esp32SystemInfo::GetAppVersion() {
    const esp_app_desc_t* app_desc = esp_app_get_description();
    return std::string(app_desc->version);
}

std::string Esp32SystemInfo::GetAppDescription() {
    const esp_app_desc_t* app_desc = esp_app_get_description();
    // 使用project_name作为描述，因为esp_app_desc_t没有description成员
    return std::string("ESP32 Application: ") + app_desc->project_name;
}

// Esp32EventGroup 实现
Esp32EventGroup::Esp32EventGroup() {
    event_group_ = xEventGroupCreate();
}

Esp32EventGroup::~Esp32EventGroup() {
    if (event_group_) {
        vEventGroupDelete(event_group_);
    }
}

SystemError Esp32EventGroup::SetBits(uint32_t bits) {
    if (!event_group_) {
        return SystemError::kInvalidParameter;
    }
    xEventGroupSetBits(event_group_, bits);
    return SystemError::kSuccess;
}

SystemError Esp32EventGroup::ClearBits(uint32_t bits) {
    if (!event_group_) {
        return SystemError::kInvalidParameter;
    }
    xEventGroupClearBits(event_group_, bits);
    return SystemError::kSuccess;
}

uint32_t Esp32EventGroup::WaitBits(uint32_t bits, bool clear_on_exit, bool wait_for_all, uint32_t timeout_ms) {
    if (!event_group_) {
        return 0;
    }
    return xEventGroupWaitBits(
        event_group_,
        bits,
        clear_on_exit ? pdTRUE : pdFALSE,
        wait_for_all ? pdTRUE : pdFALSE,
        pdMS_TO_TICKS(timeout_ms)
    );
}

} // namespace esp32
} // namespace platform
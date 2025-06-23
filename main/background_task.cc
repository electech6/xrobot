#include "background_task.h"
#include "platform/system_interface.h"  // 改为包含 system_interface.h


#define TAG "BackgroundTask"

BackgroundTask::BackgroundTask(uint32_t stack_size) {
    task_manager_ = platform::SystemFactory::CreateTaskManager();
    
    platform::TaskManager::TaskConfig config;
    config.name = "background_task";
    config.stack_size = stack_size;
    config.priority = 2;
    
    auto task_func = [this]() {
        this->BackgroundTaskLoop();
    };
    
    task_manager_->CreateTask(config, task_func, &background_task_handle_);
}

BackgroundTask::~BackgroundTask() {
    if (background_task_handle_ != nullptr && task_manager_) {
        task_manager_->DeleteTask(background_task_handle_);
    }
}

void BackgroundTask::Schedule(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (active_tasks_ >= 30) {
        // 注意：这里需要使用平台抽象的系统信息接口
        auto system_info = platform::SystemFactory::CreateSystemInfo();
        uint32_t free_sram = system_info->GetFreeHeapSize();
        if (free_sram < 10000) {
            // 使用平台抽象的日志接口（后续在日志抽象层任务中实现）
            printf("WARN: active_tasks_ == %lu, free_sram == %lu\n", (unsigned long)active_tasks_.load(), (unsigned long)free_sram);
        }
    }
    active_tasks_++;
    background_tasks_.emplace_back([this, cb = std::move(callback)]() {
        cb();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            active_tasks_--;
            if (background_tasks_.empty() && active_tasks_ == 0) {
                condition_variable_.notify_all();
            }
        }
    });
    condition_variable_.notify_all();
}

void BackgroundTask::WaitForCompletion() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_variable_.wait(lock, [this]() {
        return background_tasks_.empty() && active_tasks_ == 0;
    });
}

void BackgroundTask::BackgroundTaskLoop() {
    printf("INFO: background_task started\n");
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_variable_.wait(lock, [this]() { return !background_tasks_.empty(); });
        
        std::list<std::function<void()>> tasks = std::move(background_tasks_);
        lock.unlock();

        for (auto& task : tasks) {
            task();
        }
    }
}

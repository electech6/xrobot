#include "linux_system.h"
#include <unistd.h>
#include <sys/sysinfo.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <sys/statvfs.h>
#include <iostream>
#include <sys/resource.h>
#include <sys/times.h>
#include <cstring>
#include <random>

namespace platform {
namespace linux_platform {

// LinuxTimer 实现
LinuxTimer::LinuxTimer() {
    start_time_ = std::chrono::steady_clock::now();
}

LinuxTimer::~LinuxTimer() {
    Stop();
}

SystemError LinuxTimer::Start(uint64_t period_us, bool periodic) {
    if (is_active_) {
        return SystemError::kFailed;
    }
    
    period_us_ = period_us;
    periodic_ = periodic;
    should_stop_ = false;
    is_active_ = true;
    
    timer_thread_ = std::make_unique<std::thread>(&LinuxTimer::TimerLoop, this);
    return SystemError::kSuccess;
}

SystemError LinuxTimer::Stop() {
    if (!is_active_) {
        return SystemError::kSuccess;
    }
    
    should_stop_ = true;
    is_active_ = false;
    
    if (timer_thread_ && timer_thread_->joinable()) {
        timer_thread_->join();
    }
    timer_thread_.reset();
    
    return SystemError::kSuccess;
}

bool LinuxTimer::IsActive() const {
    return is_active_;
}

void LinuxTimer::SetCallback(Callback callback) {
    callback_ = std::move(callback);
}

void LinuxTimer::TimerLoop() {
    while (!should_stop_) {
        std::this_thread::sleep_for(std::chrono::microseconds(period_us_));
        
        if (!should_stop_ && callback_) {
            callback_();
        }
        
        if (!periodic_) {
            break;
        }
    }
    is_active_ = false;
}

// LinuxEventLoop 实现
LinuxEventLoop::LinuxEventLoop() {
    start_time_ = std::chrono::steady_clock::now();
}

LinuxEventLoop::~LinuxEventLoop() {
    Stop();
}

SystemError LinuxEventLoop::Initialize() {
    return SystemError::kSuccess;
}

SystemError LinuxEventLoop::PostEvent(std::function<void()> event) {
    if (!event) {
        return SystemError::kInvalidParameter;
    }
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        event_queue_.push(std::move(event));
    }
    queue_cv_.notify_one();
    
    return SystemError::kSuccess;
}

SystemError LinuxEventLoop::Run() {
    is_running_ = true;
    
    while (is_running_ && !should_stop_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !event_queue_.empty() || should_stop_; });
        
        while (!event_queue_.empty() && !should_stop_) {
            auto event = std::move(event_queue_.front());
            event_queue_.pop();
            lock.unlock();
            
            event();
            
            lock.lock();
        }
    }
    
    return SystemError::kSuccess;
}

SystemError LinuxEventLoop::Stop() {
    should_stop_ = true;
    is_running_ = false;
    queue_cv_.notify_all();
    return SystemError::kSuccess;
}

// LinuxTaskManager 实现
LinuxTaskManager::LinuxTaskManager() {
    start_time_ = std::chrono::steady_clock::now();
}

SystemError LinuxTaskManager::CreateTask(const TaskConfig& config, 
                                       TaskFunction task_func, 
                                       TaskHandle* handle) {
    if (!task_func || !handle) {
        return SystemError::kInvalidParameter;
    }
    
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto task_thread = std::make_unique<std::thread>(std::move(task_func));
    *handle = task_thread.get();
    tasks_.push_back(std::move(task_thread));
    
    return SystemError::kSuccess;
}

SystemError LinuxTaskManager::DeleteTask(TaskHandle handle) {
    if (!handle) {
        return SystemError::kInvalidParameter;
    }
    
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = std::find_if(tasks_.begin(), tasks_.end(),
        [handle](const std::unique_ptr<std::thread>& task) {
            return task.get() == handle;
        });
    
    if (it != tasks_.end()) {
        if ((*it)->joinable()) {
            (*it)->join();
        }
        tasks_.erase(it);
        return SystemError::kSuccess;
    }
    
    return SystemError::kFailed;
}

SystemError LinuxTaskManager::Delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return SystemError::kSuccess;
}

uint32_t LinuxTaskManager::GetTickCount() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
    return static_cast<uint32_t>(duration.count());
}

// LinuxSystemInfo 实现
LinuxSystemInfo::LinuxSystemInfo() {}

std::string LinuxSystemInfo::GetChipModel() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                return line.substr(pos + 2);
            }
        }
    }
    return "Unknown Linux CPU";
}

uint32_t LinuxSystemInfo::GetFreeHeapSize() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return static_cast<uint32_t>(info.freeram * info.mem_unit);
    }
    return 0;
}

uint32_t LinuxSystemInfo::GetMinFreeHeapSize() {
    // Linux上没有直接的最小空闲内存概念，返回当前空闲内存
    return GetFreeHeapSize();
}

std::string LinuxSystemInfo::GetMacAddress() {
    struct ifaddrs *ifap, *ifa;
    if (getifaddrs(&ifap) == -1) {
        return "00:00:00:00:00:00";
    }
    
    for (ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_PACKET) {
            // 简化实现，返回第一个非回环接口的MAC地址
            if (strcmp(ifa->ifa_name, "lo") != 0) {
                // 这里需要更复杂的实现来获取真实的MAC地址
                freeifaddrs(ifap);
                return "02:00:00:00:00:01"; // 模拟MAC地址
            }
        }
    }
    
    freeifaddrs(ifap);
    return "00:00:00:00:00:00";
}

void LinuxSystemInfo::Restart() {
    std::system("sudo reboot");
}

// 实现GetFlashSize
uint32_t LinuxSystemInfo::GetFlashSize() {
    // Linux上可以通过statvfs获取磁盘空间
    struct statvfs stat;
    if (statvfs("/", &stat) != 0) {
        return 0;
    }
    return stat.f_blocks * stat.f_frsize;
}

// 实现PrintTaskCpuUsage
bool LinuxSystemInfo::PrintTaskCpuUsage(uint32_t wait_ms) {
    // Linux上可以通过/proc/stat获取CPU使用率
    printf("Task CPU usage (Linux implementation):\n");
    // 这里可以实现Linux特定的CPU使用率统计
    return true;
}

// 实现PrintTaskList
bool LinuxSystemInfo::PrintTaskList() {
    // Linux上可以通过ps命令获取进程列表
    printf("Task list (Linux implementation):\n");
    // 这里可以实现Linux特定的进程列表显示
    return true;
}

// LinuxEventGroup 实现
LinuxEventGroup::LinuxEventGroup() : bits_(0) {}

SystemError LinuxEventGroup::SetBits(uint32_t bits) {
    std::lock_guard<std::mutex> lock(mutex_);
    bits_ |= bits;
    cv_.notify_all();
    return SystemError::kSuccess;
}

SystemError LinuxEventGroup::ClearBits(uint32_t bits) {
    std::lock_guard<std::mutex> lock(mutex_);
    bits_ &= ~bits;
    return SystemError::kSuccess;
}

uint32_t LinuxEventGroup::WaitBits(uint32_t bits, bool clear_on_exit, bool wait_for_all, uint32_t timeout_ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    auto pred = [this, bits, wait_for_all]() {
        if (wait_for_all) {
            return (bits_ & bits) == bits;
        } else {
            return (bits_ & bits) != 0;
        }
    };
    
    if (timeout_ms == 0) {
        cv_.wait(lock, pred);
    } else {
        auto result = cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), pred);
        if (!result) {
            return 0; // 超时
        }
    }
    
    uint32_t result_bits = bits_ & bits;
    
    if (clear_on_exit) {
        bits_ &= ~bits;
    }
    
    return result_bits;
}

} // namespace linux_platform
} // namespace platform
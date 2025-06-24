#include "system_info.h"
#include <iostream>

#define TAG "SystemInfo"

// 单例实现
SystemInfo& SystemInfo::GetInstance() {
    static SystemInfo instance;
    return instance;
}

SystemInfo::SystemInfo() {
    // 使用工厂创建平台特定实现
    platform_system_info_ = platform::SystemFactory::CreateSystemInfo();
}

// 实例方法实现
size_t SystemInfo::GetFlashSizeInstance() {
    return platform_system_info_->GetFlashSize();
}

// 静态方法实现
size_t SystemInfo::GetFlashSize() {
    return GetInstance().GetFlashSizeInstance();
}

// 实例方法实现
size_t SystemInfo::GetMinimumFreeHeapSizeInstance() {
    return platform_system_info_->GetMinFreeHeapSize();
}

// 静态方法实现
size_t SystemInfo::GetMinimumFreeHeapSize() {
    return GetInstance().GetMinimumFreeHeapSizeInstance();
}

// 实例方法实现
size_t SystemInfo::GetFreeHeapSizeInstance() {
    return platform_system_info_->GetFreeHeapSize();
}

// 静态方法实现
size_t SystemInfo::GetFreeHeapSize() {
    return GetInstance().GetFreeHeapSizeInstance();
}

// 实例方法实现
std::string SystemInfo::GetMacAddressInstance() {
    return platform_system_info_->GetMacAddress();
}

// 静态方法实现
std::string SystemInfo::GetMacAddress() {
    return GetInstance().GetMacAddressInstance();
}

// 实例方法实现
std::string SystemInfo::GetChipModelNameInstance() {
    return platform_system_info_->GetChipModel();
}

// 静态方法实现
std::string SystemInfo::GetChipModelName() {
    return GetInstance().GetChipModelNameInstance();
}

// 实例方法实现
bool SystemInfo::PrintTaskCpuUsageInstance(uint32_t wait_ms) {
    return platform_system_info_->PrintTaskCpuUsage(wait_ms);
}

// 静态方法实现
bool SystemInfo::PrintTaskCpuUsage(uint32_t wait_ms) {
    return GetInstance().PrintTaskCpuUsageInstance(wait_ms);
}

// 实例方法实现
bool SystemInfo::PrintTaskListInstance() {
    return platform_system_info_->PrintTaskList();
}

// 静态方法实现
bool SystemInfo::PrintTaskList() {
    return GetInstance().PrintTaskListInstance();
}

// 静态方法实现
void SystemInfo::PrintHeapStats() {
    std::cout << "Free heap: " << GetFreeHeapSize() 
              << " bytes, Minimum free heap: " << GetMinimumFreeHeapSize() 
              << " bytes" << std::endl;
}

// 实例方法实现
void SystemInfo::RestartSystemInstance() {
    platform_system_info_->Restart();
}

// 静态方法实现
void SystemInfo::RestartSystem() {
    GetInstance().RestartSystemInstance();
}

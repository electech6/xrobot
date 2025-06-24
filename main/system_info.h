#ifndef _SYSTEM_INFO_H_
#define _SYSTEM_INFO_H_

#include <string>
#include <memory>
#include "platform/system_interface.h"

class SystemInfo {
public:
    // 单例模式获取实例
    static SystemInfo& GetInstance();
    
    // 平台无关的系统信息方法 - 全部为静态方法
    static size_t GetFlashSize();
    static size_t GetMinimumFreeHeapSize();
    static size_t GetFreeHeapSize();
    static std::string GetMacAddress();
    static std::string GetChipModelName();
    
    // 打印任务和内存信息的方法 - 全部为静态方法
    static bool PrintTaskCpuUsage(uint32_t wait_ms);
    static bool PrintTaskList();
    static void PrintHeapStats();
    
    // 系统重启 - 静态方法
    static void RestartSystem();

private:
    SystemInfo();
    ~SystemInfo() = default;
    
    // 禁止拷贝和赋值
    SystemInfo(const SystemInfo&) = delete;
    SystemInfo& operator=(const SystemInfo&) = delete;
    
    // 平台抽象层系统信息接口
    std::unique_ptr<platform::SystemInfo> platform_system_info_;
    
    // 实例方法，供静态方法内部调用
    size_t GetFlashSizeInstance();
    size_t GetMinimumFreeHeapSizeInstance();
    size_t GetFreeHeapSizeInstance();
    std::string GetMacAddressInstance();
    std::string GetChipModelNameInstance();
    bool PrintTaskCpuUsageInstance(uint32_t wait_ms);
    bool PrintTaskListInstance();
    void RestartSystemInstance();
};

#endif // _SYSTEM_INFO_H_

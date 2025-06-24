#pragma once

#include <string>
#include "system_interface.h"

namespace platform {

// 存储接口抽象类
class Storage {
public:
    virtual ~Storage() = default;
    
    // 打开存储命名空间
    virtual SystemError Open(const std::string& namespace_name, bool read_write) = 0;
    
    // 关闭存储
    virtual SystemError Close() = 0;
    
    // 提交更改
    virtual SystemError Commit() = 0;
    
    // 字符串操作
    virtual SystemError GetString(const std::string& key, std::string& value) = 0;
    virtual SystemError SetString(const std::string& key, const std::string& value) = 0;
    
    // 整数操作
    virtual SystemError GetInt(const std::string& key, int32_t& value) = 0;
    virtual SystemError SetInt(const std::string& key, int32_t value) = 0;
    
    // 删除操作
    virtual SystemError EraseKey(const std::string& key) = 0;
    virtual SystemError EraseAll() = 0;
    
    // 检查键是否存在
    virtual bool KeyExists(const std::string& key) = 0;
};

// 存储工厂类
class StorageFactory {
public:
    static std::unique_ptr<Storage> CreateStorage();
};

} // namespace platform

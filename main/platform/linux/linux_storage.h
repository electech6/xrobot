#pragma once

#include "../storage_interface.h"
#include <map>
#include <mutex>
#include <fstream>

namespace platform {
namespace linux_platform {

class LinuxStorage : public Storage {
public:
    LinuxStorage();
    ~LinuxStorage() override;
    
    SystemError Open(const std::string& namespace_name, bool read_write) override;
    SystemError Close() override;
    SystemError Commit() override;
    
    SystemError GetString(const std::string& key, std::string& value) override;
    SystemError SetString(const std::string& key, const std::string& value) override;
    
    SystemError GetInt(const std::string& key, int32_t& value) override;
    SystemError SetInt(const std::string& key, int32_t value) override;
    
    SystemError EraseKey(const std::string& key) override;
    SystemError EraseAll() override;
    
    bool KeyExists(const std::string& key) override;
    
private:
    std::string namespace_name_;
    bool is_open_ = false;
    bool read_write_ = false;
    bool dirty_ = false;
    
    std::map<std::string, std::string> string_values_;
    std::map<std::string, int32_t> int_values_;
    std::mutex mutex_;
    
    // 文件路径
    std::string GetStoragePath() const;
    
    // 加载和保存数据
    SystemError LoadData();
    SystemError SaveData();
};

} // namespace linux_platform
} // namespace platform

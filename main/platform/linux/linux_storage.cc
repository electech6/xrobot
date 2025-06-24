#include "linux_storage.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace platform {
namespace linux_platform {

LinuxStorage::LinuxStorage() {}

LinuxStorage::~LinuxStorage() {
    Close();
}

std::string LinuxStorage::GetStoragePath() const {
    // 在Linux上使用用户主目录下的.config目录存储数据
    std::string home_dir = std::getenv("HOME") ? std::getenv("HOME") : ".";
    std::string config_dir = home_dir + "/.config/xrobot";
    
    // 确保目录存在
    std::filesystem::create_directories(config_dir);
    
    return config_dir + "/" + namespace_name_ + ".conf";
}

SystemError LinuxStorage::Open(const std::string& namespace_name, bool read_write) {
    if (is_open_) {
        Close();
    }
    
    namespace_name_ = namespace_name;
    read_write_ = read_write;
    
    SystemError err = LoadData();
    if (err != SystemError::kSuccess && err != SystemError::kNotSupported) {
        return err;
    }
    
    is_open_ = true;
    return SystemError::kSuccess;
}

SystemError LinuxStorage::Close() {
    if (is_open_ && read_write_ && dirty_) {
        SaveData();
    }
    
    string_values_.clear();
    int_values_.clear();
    is_open_ = false;
    dirty_ = false;
    
    return SystemError::kSuccess;
}

SystemError LinuxStorage::Commit() {
    if (!is_open_ || !read_write_ || !dirty_) {
        return SystemError::kFailed;
    }
    
    return SaveData();
}

SystemError LinuxStorage::LoadData() {
    std::string filepath = GetStoragePath();
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        return SystemError::kNotSupported; // 文件不存在，不是错误
    }
    
    string_values_.clear();
    int_values_.clear();
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type, key, value;
        
        if (std::getline(iss, type, ':') && std::getline(iss, key, ':') && std::getline(iss, value)) {
            if (type == "s") {
                string_values_[key] = value;
            } else if (type == "i") {
                try {
                    int_values_[key] = std::stoi(value);
                } catch (...) {
                    std::cerr << "Error parsing int value for key: " << key << std::endl;
                }
            }
        }
    }
    
    return SystemError::kSuccess;
}

SystemError LinuxStorage::SaveData() {
    if (!read_write_) {
        return SystemError::kFailed;
    }
    
    std::string filepath = GetStoragePath();
    std::ofstream file(filepath);
    
    if (!file.is_open()) {
        return SystemError::kFailed;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 写入字符串值
    for (const auto& pair : string_values_) {
        file << "s:" << pair.first << ":" << pair.second << std::endl;
    }
    
    // 写入整数值
    for (const auto& pair : int_values_) {
        file << "i:" << pair.first << ":" << pair.second << std::endl;
    }
    
    dirty_ = false;
    return SystemError::kSuccess;
}

SystemError LinuxStorage::GetString(const std::string& key, std::string& value) {
    if (!is_open_) {
        return SystemError::kFailed;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = string_values_.find(key);
    if (it == string_values_.end()) {
        return SystemError::kFailed;
    }
    
    value = it->second;
    return SystemError::kSuccess;
}

SystemError LinuxStorage::SetString(const std::string& key, const std::string& value) {
    if (!is_open_ || !read_write_) {
        return SystemError::kFailed;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    string_values_[key] = value;
    dirty_ = true;
    
    return SystemError::kSuccess;
}

SystemError LinuxStorage::GetInt(const std::string& key, int32_t& value) {
    if (!is_open_) {
        return SystemError::kFailed;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = int_values_.find(key);
    if (it == int_values_.end()) {
        return SystemError::kFailed;
    }
    
    value = it->second;
    return SystemError::kSuccess;
}

SystemError LinuxStorage::SetInt(const std::string& key, int32_t value) {
    if (!is_open_ || !read_write_) {
        return SystemError::kFailed;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    int_values_[key] = value;
    dirty_ = true;
    
    return SystemError::kSuccess;
}

SystemError LinuxStorage::EraseKey(const std::string& key) {
    if (!is_open_ || !read_write_) {
        return SystemError::kFailed;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    bool found = false;
    
    auto str_it = string_values_.find(key);
    if (str_it != string_values_.end()) {
        string_values_.erase(str_it);
        found = true;
    }
    
    auto int_it = int_values_.find(key);
    if (int_it != int_values_.end()) {
        int_values_.erase(int_it);
        found = true;
    }
    
    if (found) {
        dirty_ = true;
        return SystemError::kSuccess;
    }
    
    return SystemError::kNotSupported;
}

SystemError LinuxStorage::EraseAll() {
    if (!is_open_ || !read_write_) {
        return SystemError::kFailed;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    string_values_.clear();
    int_values_.clear();
    dirty_ = true;
    
    return SystemError::kSuccess;
}

bool LinuxStorage::KeyExists(const std::string& key) {
    if (!is_open_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return (string_values_.find(key) != string_values_.end() || 
            int_values_.find(key) != int_values_.end());
}

} // namespace linux_platform
} // namespace platform

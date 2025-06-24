#include "esp32_storage.h"
#include <esp_log.h>

namespace platform {
namespace esp32 {

Esp32Storage::Esp32Storage() {}

Esp32Storage::~Esp32Storage() {
    Close();
}

SystemError Esp32Storage::Open(const std::string& namespace_name, bool read_write) {
    if (is_open_) {
        Close();
    }
    
    read_write_ = read_write;
    esp_err_t err = nvs_open(namespace_name.c_str(), 
                            read_write_ ? NVS_READWRITE : NVS_READONLY, 
                            &nvs_handle_);
    
    if (err != ESP_OK) {
        return SystemError::kFailed;
    }
    
    is_open_ = true;
    return SystemError::kSuccess;
}

SystemError Esp32Storage::Close() {
    if (is_open_) {
        nvs_close(nvs_handle_);
        is_open_ = false;
    }
    return SystemError::kSuccess;
}

SystemError Esp32Storage::Commit() {
    if (!is_open_ || !read_write_) {
        return SystemError::kFailed;
    }
    
    esp_err_t err = nvs_commit(nvs_handle_);
    return (err == ESP_OK) ? SystemError::kSuccess : SystemError::kFailed;
}

SystemError Esp32Storage::GetString(const std::string& key, std::string& value) {
    if (!is_open_) {
        return SystemError::kFailed;
    }
    
    size_t length = 0;
    esp_err_t err = nvs_get_str(nvs_handle_, key.c_str(), nullptr, &length);
    if (err != ESP_OK) {
        return SystemError::kFailed;
    }
    
    value.resize(length);
    err = nvs_get_str(nvs_handle_, key.c_str(), value.data(), &length);
    if (err != ESP_OK) {
        return SystemError::kFailed;
    }
    
    // 移除字符串末尾的空字符
    while (!value.empty() && value.back() == '\0') {
        value.pop_back();
    }
    
    return SystemError::kSuccess;
}

SystemError Esp32Storage::SetString(const std::string& key, const std::string& value) {
    if (!is_open_ || !read_write_) {
        return SystemError::kFailed;
    }
    
    esp_err_t err = nvs_set_str(nvs_handle_, key.c_str(), value.c_str());
    return (err == ESP_OK) ? SystemError::kSuccess : SystemError::kFailed;
}

SystemError Esp32Storage::GetInt(const std::string& key, int32_t& value) {
    if (!is_open_) {
        return SystemError::kFailed;
    }
    
    esp_err_t err = nvs_get_i32(nvs_handle_, key.c_str(), &value);
    return (err == ESP_OK) ? SystemError::kSuccess : SystemError::kFailed;
}

SystemError Esp32Storage::SetInt(const std::string& key, int32_t value) {
    if (!is_open_ || !read_write_) {
        return SystemError::kFailed;
    }
    
    esp_err_t err = nvs_set_i32(nvs_handle_, key.c_str(), value);
    return (err == ESP_OK) ? SystemError::kSuccess : SystemError::kFailed;
}

SystemError Esp32Storage::EraseKey(const std::string& key) {
    if (!is_open_ || !read_write_) {
        return SystemError::kFailed;
    }
    
    esp_err_t err = nvs_erase_key(nvs_handle_, key.c_str());
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return SystemError::kNotSupported;
    }
    return (err == ESP_OK) ? SystemError::kSuccess : SystemError::kFailed;
}

SystemError Esp32Storage::EraseAll() {
    if (!is_open_ || !read_write_) {
        return SystemError::kFailed;
    }
    
    esp_err_t err = nvs_erase_all(nvs_handle_);
    return (err == ESP_OK) ? SystemError::kSuccess : SystemError::kFailed;
}

bool Esp32Storage::KeyExists(const std::string& key) {
    if (!is_open_) {
        return false;
    }
    
    int32_t dummy;
    esp_err_t err = nvs_get_i32(nvs_handle_, key.c_str(), &dummy);
    if (err == ESP_OK ) {
        return true;
    }
    return false;
}

} // namespace esp32
} // namespace platform

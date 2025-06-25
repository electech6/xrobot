#include "settings.h"
#include "logging/logger.h"

#define TAG "Settings"

Settings::Settings(const std::string& ns, bool read_write) : ns_(ns), read_write_(read_write) {
    storage_ = platform::StorageFactory::CreateStorage();
    platform::SystemError err = storage_->Open(ns, read_write);
    if (err != platform::SystemError::kSuccess) {
        logging::Logger::GetInstance()->Log(logging::LogLevel::kError, TAG, "Failed to open storage namespace: %s", ns.c_str());
    }
}

Settings::~Settings() {
    if (storage_ && read_write_ && dirty_) {
        storage_->Commit();
    }
    if (storage_) {
        storage_->Close();
    }
}

std::string Settings::GetString(const std::string& key, const std::string& default_value) {
    if (!storage_) {
        return default_value;
    }

    std::string value;
    if (storage_->GetString(key, value) != platform::SystemError::kSuccess) {
        return default_value;
    }

    return value;
}

void Settings::SetString(const std::string& key, const std::string& value) {
    if (storage_ && read_write_) {
        if (storage_->SetString(key, value) == platform::SystemError::kSuccess) {
            dirty_ = true;
        } else {
            logging::Logger::GetInstance()->Log(logging::LogLevel::kError, TAG, "Failed to set string for key: %s", key.c_str());
        }
    } else {
        logging::Logger::GetInstance()->Log(logging::LogLevel::kError, TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}

int32_t Settings::GetInt(const std::string& key, int32_t default_value) {
    if (!storage_) {
        return default_value;
    }

    int32_t value;
    if (storage_->GetInt(key, value) != platform::SystemError::kSuccess) {
        return default_value;
    }
    return value;
}

void Settings::SetInt(const std::string& key, int32_t value) {
    if (storage_ && read_write_) {
        if (storage_->SetInt(key, value) == platform::SystemError::kSuccess) {
            dirty_ = true;
        } else {
            logging::Logger::GetInstance()->Log(logging::LogLevel::kError, TAG, "Failed to set int for key: %s", key.c_str());
        }
    } else {
        logging::Logger::GetInstance()->Log(logging::LogLevel::kError, TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}

void Settings::EraseKey(const std::string& key) {
    if (storage_ && read_write_) {
        platform::SystemError err = storage_->EraseKey(key);
        if (err != platform::SystemError::kSuccess && 
            err != platform::SystemError::kNotSupported) {
            logging::Logger::GetInstance()->Log(logging::LogLevel::kError, TAG, "Failed to erase key: %s", key.c_str());
        } else {
            dirty_ = true;
        }
    } else {
        logging::Logger::GetInstance()->Log(logging::LogLevel::kError, TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}

void Settings::EraseAll() {
    if (storage_ && read_write_) {
        if (storage_->EraseAll() == platform::SystemError::kSuccess) {
            dirty_ = true;
        } else {
            logging::Logger::GetInstance()->Log(logging::LogLevel::kError, TAG, "Failed to erase all keys in namespace: %s", ns_.c_str());
        }
    } else {
        logging::Logger::GetInstance()->Log(logging::LogLevel::kError, TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}

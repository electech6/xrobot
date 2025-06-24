#pragma once

#include "../storage_interface.h"
#include <nvs_flash.h>

namespace platform {
namespace esp32 {

class Esp32Storage : public Storage {
public:
    Esp32Storage();
    ~Esp32Storage() override;
    
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
    nvs_handle_t nvs_handle_ = 0;
    bool is_open_ = false;
    bool read_write_ = false;
};

} // namespace esp32
} // namespace platform

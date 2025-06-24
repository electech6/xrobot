#include "storage_interface.h"

#ifdef ESP_PLATFORM
#include "esp32/esp32_storage.h"
using namespace platform::esp32;
#else
#include "linux/linux_storage.h"
using namespace platform::linux_platform;
#endif

namespace platform {

std::unique_ptr<Storage> StorageFactory::CreateStorage() {
#ifdef ESP_PLATFORM
    return std::make_unique<Esp32Storage>();
#else
    return std::make_unique<LinuxStorage>();
#endif
}

} // namespace platform 
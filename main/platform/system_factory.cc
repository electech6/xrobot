#include "system_interface.h"

#ifdef ESP_PLATFORM
#include "esp32/esp32_system.h"
using namespace platform::esp32;
#else
#include "linux/linux_system.h"
using namespace platform::linux_platform;
#endif

namespace platform {

std::unique_ptr<Timer> SystemFactory::CreateTimer() {
#ifdef ESP_PLATFORM
    return std::make_unique<Esp32Timer>();
#else
    return std::make_unique<LinuxTimer>();
#endif
}

std::unique_ptr<EventLoop> SystemFactory::CreateEventLoop() {
#ifdef ESP_PLATFORM
    return std::make_unique<Esp32EventLoop>();
#else
    return std::make_unique<LinuxEventLoop>();
#endif
}

std::unique_ptr<TaskManager> SystemFactory::CreateTaskManager() {
#ifdef ESP_PLATFORM
    return std::make_unique<Esp32TaskManager>();
#else
    return std::make_unique<LinuxTaskManager>();
#endif
}

std::unique_ptr<SystemInfo> SystemFactory::CreateSystemInfo() {
#ifdef ESP_PLATFORM
    return std::make_unique<Esp32SystemInfo>();
#else
    return std::make_unique<LinuxSystemInfo>();
#endif
}

} // namespace platform
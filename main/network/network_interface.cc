#include "network_interface.h"

#ifdef ESP_PLATFORM
#include "esp32_network.h"
#else
#include "linux_network.h"
#endif

namespace network {

std::unique_ptr<WebSocketInterface> NetworkFactory::CreateWebSocket() {
#ifdef ESP_PLATFORM
    return std::make_unique<esp32::Esp32WebSocket>();
#else
    return std::make_unique<linux::LinuxWebSocket>();
#endif
}

std::unique_ptr<MqttInterface> NetworkFactory::CreateMqtt() {
#ifdef ESP_PLATFORM
    return std::make_unique<esp32::Esp32Mqtt>();
#else
    return std::make_unique<linux::LinuxMqtt>();
#endif
}

std::unique_ptr<HttpInterface> NetworkFactory::CreateHttp() {
#ifdef ESP_PLATFORM
    return std::make_unique<esp32::Esp32Http>();
#else
    return std::make_unique<linux::LinuxHttp>();
#endif
}

std::unique_ptr<UdpInterface> NetworkFactory::CreateUdp() {
#ifdef ESP_PLATFORM
    return std::make_unique<esp32::Esp32Udp>();
#else
    return std::make_unique<linux::LinuxUdp>();
#endif
}

} // namespace network 
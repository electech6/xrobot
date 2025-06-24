#pragma once

#include "network_interface.h"
#include <web_socket.h>
#include <mqtt.h>
#include <http.h>
#include <udp.h>

namespace network {
namespace esp32 {

// ESP32 WebSocket实现
class Esp32WebSocket : public WebSocketInterface {
public:
    Esp32WebSocket();
    ~Esp32WebSocket() override;
    
    bool Connect(const char* url) override;
    bool IsConnected() const override;
    bool Send(const char* data) override;
    bool Send(const void* data, size_t len, bool binary = false) override;
    void SetHeader(const char* key, const char* value) override;
    void OnData(std::function<void(const char* data, size_t len, bool binary)> callback) override;
    void OnDisconnected(std::function<void()> callback) override;
    
private:
    WebSocket* websocket_ = nullptr;
};

// ESP32 MQTT实现
class Esp32Mqtt : public MqttInterface {
public:
    Esp32Mqtt();
    ~Esp32Mqtt() override;
    
    bool Connect(const std::string& broker_address, int broker_port, 
                const std::string& client_id,
                const std::string& username = "",
                const std::string& password = "") override;
    bool IsConnected() const override;
    bool Publish(const std::string& topic, const std::string& payload, int qos = 0) override;
    bool Subscribe(const std::string& topic, int qos = 0) override;
    void SetKeepAlive(int seconds) override;
    void OnMessage(std::function<void(const std::string& topic, const std::string& payload)> callback) override;
    void OnDisconnected(std::function<void()> callback) override;
    
private:
    Mqtt* mqtt_ = nullptr;
};

// ESP32 HTTP实现
class Esp32Http : public HttpInterface {
public:
    Esp32Http();
    ~Esp32Http() override;
    
    bool Open(const char* method, const std::string& url) override;
    void Close() override;
    void SetHeader(const char* key, const char* value) override;
    void SetContent(std::string&& content) override;
    int GetStatusCode() const override;
    std::string ReadAll() override;
    
private:
    Http* http_ = nullptr;
};

// ESP32 UDP实现
class Esp32Udp : public UdpInterface {
public:
    Esp32Udp();
    ~Esp32Udp() override;
    
    bool Connect(const std::string& host, int port) override;
    bool IsConnected() const override;
    int Send(const std::string& data) override;
    int Send(const void* data, size_t len) override;
    int Receive(void* buffer, size_t len) override;
    void OnData(std::function<void(const void* data, size_t len)> callback) override;
    
private:
    Udp* udp_ = nullptr;
};

} // namespace esp32
} // namespace network

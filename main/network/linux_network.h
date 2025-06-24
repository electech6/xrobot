#pragma once

#include "network_interface.h"

namespace network {
namespace linux {

// Linux WebSocket实现（占位）
class LinuxWebSocket : public WebSocketInterface {
public:
    LinuxWebSocket() {}
    ~LinuxWebSocket() override {}
    
    bool Connect(const char* url) override { return false; }
    bool IsConnected() const override { return false; }
    bool Send(const char* data) override { return false; }
    bool Send(const void* data, size_t len, bool binary = false) override { return false; }
    void SetHeader(const char* key, const char* value) override {}
    void OnData(std::function<void(const char* data, size_t len, bool binary)> callback) override {}
    void OnDisconnected(std::function<void()> callback) override {}
};

// Linux MQTT实现（占位）
class LinuxMqtt : public MqttInterface {
public:
    LinuxMqtt() {}
    ~LinuxMqtt() override {}
    
    bool Connect(const std::string& broker_address, int broker_port, 
                const std::string& client_id,
                const std::string& username = "",
                const std::string& password = "") override { return false; }
    bool IsConnected() const override { return false; }
    bool Publish(const std::string& topic, const std::string& payload, int qos = 0) override { return false; }
    bool Subscribe(const std::string& topic, int qos = 0) override { return false; }
    void SetKeepAlive(int seconds) override {}
    void OnMessage(std::function<void(const std::string& topic, const std::string& payload)> callback) override {}
    void OnDisconnected(std::function<void()> callback) override {}
};

// Linux HTTP实现（占位）
class LinuxHttp : public HttpInterface {
public:
    LinuxHttp() {}
    ~LinuxHttp() override {}
    
    bool Open(const char* method, const std::string& url) override { return false; }
    void Close() override {}
    void SetHeader(const char* key, const char* value) override {}
    void SetContent(std::string&& content) override {}
    int GetStatusCode() const override { return 0; }
    std::string ReadAll() override { return ""; }
};

// Linux UDP实现（占位）
class LinuxUdp : public UdpInterface {
public:
    LinuxUdp() {}
    ~LinuxUdp() override {}
    
    bool Connect(const std::string& host, int port) override { return false; }
    bool IsConnected() const override { return false; }
    int Send(const std::string& data) override { return 0; }
    int Send(const void* data, size_t len) override { return 0; }
    int Receive(void* buffer, size_t len) override { return 0; }
    void OnData(std::function<void(const void* data, size_t len)> callback) override {}
};

} // namespace linux
} // namespace network

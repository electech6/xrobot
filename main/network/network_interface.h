#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <map>

namespace network {

// 网络错误码定义
enum class NetworkError {
    kSuccess = 0,
    kFailed = -1,
    kTimeout = -2,
    kInvalidParameter = -3,
    kConnectionFailed = -4,
    kDisconnected = -5,
    kNotSupported = -6
};

// 前向声明
class WebSocketInterface;
class MqttInterface;
class HttpInterface;
class UdpInterface;

// WebSocket接口
class WebSocketInterface {
public:
    virtual ~WebSocketInterface() = default;
    
    // 连接方法
    virtual bool Connect(const char* url) = 0;
    virtual bool IsConnected() const = 0;
    
    // 发送方法
    virtual bool Send(const char* data) = 0;
    virtual bool Send(const std::string& data) { return Send(data.c_str()); }
    virtual bool Send(const void* data, size_t len, bool binary = false) = 0;
    
    // 设置HTTP头
    virtual void SetHeader(const char* key, const char* value) = 0;
    
    // 回调设置
    virtual void OnData(std::function<void(const char* data, size_t len, bool binary)> callback) = 0;
    virtual void OnDisconnected(std::function<void()> callback) = 0;
};

// MQTT接口
class MqttInterface {
public:
    virtual ~MqttInterface() = default;
    
    // 连接方法
    virtual bool Connect(const std::string& broker_address, int broker_port, 
                        const std::string& client_id,
                        const std::string& username = "",
                        const std::string& password = "") = 0;
    virtual bool IsConnected() const = 0;
    
    // 发布和订阅
    virtual bool Publish(const std::string& topic, const std::string& payload, int qos = 0) = 0;
    virtual bool Subscribe(const std::string& topic, int qos = 0) = 0;
    
    // 设置保活间隔
    virtual void SetKeepAlive(int seconds) = 0;
    
    // 回调设置
    virtual void OnMessage(std::function<void(const std::string& topic, const std::string& payload)> callback) = 0;
    virtual void OnDisconnected(std::function<void()> callback) = 0;
};

// HTTP接口
class HttpInterface {
public:
    virtual ~HttpInterface() = default;
    
    // 打开连接
    virtual bool Open(const char* method, const std::string& url) = 0;
    virtual void Close() = 0;
    
    // 设置请求参数
    virtual void SetHeader(const char* key, const char* value) = 0;
    virtual void SetContent(std::string&& content) = 0;
    
    // 获取响应
    virtual int GetStatusCode() const = 0;
    virtual std::string ReadAll() = 0;
};

// UDP接口
class UdpInterface {
public:
    virtual ~UdpInterface() = default;
    
    // 连接方法
    virtual bool Connect(const std::string& host, int port) = 0;
    virtual bool IsConnected() const = 0;
    
    // 发送方法
    virtual int Send(const std::string& data) = 0;
    virtual int Send(const void* data, size_t len) = 0;
    
    // 接收方法
    virtual int Receive(void* buffer, size_t len) = 0;
    
    // 回调设置
    virtual void OnData(std::function<void(const void* data, size_t len)> callback) = 0;
};

// 网络工厂类
class NetworkFactory {
public:
    static std::unique_ptr<WebSocketInterface> CreateWebSocket();
    static std::unique_ptr<MqttInterface> CreateMqtt();
    static std::unique_ptr<HttpInterface> CreateHttp();
    static std::unique_ptr<UdpInterface> CreateUdp();
};

} // namespace network

#include "esp32_network.h"
#include "../boards/common/board.h"

namespace network {
namespace esp32 {

// ESP32 WebSocket实现
Esp32WebSocket::Esp32WebSocket() {
    websocket_ = Board::GetInstance().CreateWebSocket();
}

Esp32WebSocket::~Esp32WebSocket() {
    if (websocket_) {
        delete websocket_;
        websocket_ = nullptr;
    }
}

bool Esp32WebSocket::Connect(const char* url) {
    if (!websocket_) return false;
    return websocket_->Connect(url);
}

bool Esp32WebSocket::IsConnected() const {
    if (!websocket_) return false;
    return websocket_->IsConnected();
}

bool Esp32WebSocket::Send(const char* data) {
    if (!websocket_) return false;
    return websocket_->Send(data);
}

bool Esp32WebSocket::Send(const void* data, size_t len, bool binary) {
    if (!websocket_) return false;
    return websocket_->Send(data, len, binary);
}

void Esp32WebSocket::SetHeader(const char* key, const char* value) {
    if (!websocket_) return;
    websocket_->SetHeader(key, value);
}

void Esp32WebSocket::OnData(std::function<void(const char* data, size_t len, bool binary)> callback) {
    if (!websocket_) return;
    websocket_->OnData(callback);
}

void Esp32WebSocket::OnDisconnected(std::function<void()> callback) {
    if (!websocket_) return;
    websocket_->OnDisconnected(callback);
}

// ESP32 MQTT实现
Esp32Mqtt::Esp32Mqtt() {
    mqtt_ = Board::GetInstance().CreateMqtt();
}

Esp32Mqtt::~Esp32Mqtt() {
    if (mqtt_) {
        delete mqtt_;
        mqtt_ = nullptr;
    }
}

bool Esp32Mqtt::Connect(const std::string& broker_address, int broker_port, 
                       const std::string& client_id,
                       const std::string& username,
                       const std::string& password) {
    if (!mqtt_) return false;
    return mqtt_->Connect(broker_address, broker_port, client_id, username, password);
}

bool Esp32Mqtt::IsConnected() const {
    if (!mqtt_) return false;
    return mqtt_->IsConnected();
}

bool Esp32Mqtt::Publish(const std::string& topic, const std::string& payload, int qos) {
    if (!mqtt_) return false;
    return mqtt_->Publish(topic, payload);
}

bool Esp32Mqtt::Subscribe(const std::string& topic, int qos) {
    if (!mqtt_) return false;
    return mqtt_->Subscribe(topic);
}

void Esp32Mqtt::SetKeepAlive(int seconds) {
    if (!mqtt_) return;
    mqtt_->SetKeepAlive(seconds);
}

void Esp32Mqtt::OnMessage(std::function<void(const std::string& topic, const std::string& payload)> callback) {
    if (!mqtt_) return;
    mqtt_->OnMessage(callback);
}

void Esp32Mqtt::OnDisconnected(std::function<void()> callback) {
    if (!mqtt_) return;
    mqtt_->OnDisconnected(callback);
}

// ESP32 HTTP实现
Esp32Http::Esp32Http() {
    http_ = Board::GetInstance().CreateHttp();
}

Esp32Http::~Esp32Http() {
    if (http_) {
        delete http_;
        http_ = nullptr;
    }
}

bool Esp32Http::Open(const char* method, const std::string& url) {
    if (!http_) return false;
    return http_->Open(method, url);
}

void Esp32Http::Close() {
    if (!http_) return;
    http_->Close();
}

void Esp32Http::SetHeader(const char* key, const char* value) {
    if (!http_) return;
    http_->SetHeader(key, value);
}

void Esp32Http::SetContent(std::string&& content) {
    if (!http_) return;
    http_->SetContent(std::move(content));
}

int Esp32Http::GetStatusCode() const {
    if (!http_) return 0;
    return http_->GetStatusCode();
}

std::string Esp32Http::ReadAll() {
    if (!http_) return "";
    return http_->ReadAll();
}

// ESP32 UDP实现
Esp32Udp::Esp32Udp() {
    udp_ = Board::GetInstance().CreateUdp();
}

Esp32Udp::~Esp32Udp() {
    if (udp_) {
        delete udp_;
        udp_ = nullptr;
    }
}

bool Esp32Udp::Connect(const std::string& host, int port) {
    if (!udp_) return false;
    return udp_->Connect(host, port);
}

bool Esp32Udp::IsConnected() const {
    if (!udp_) return false;
    return udp_->connected();
}

int Esp32Udp::Send(const std::string& data) {
    if (!udp_) return 0;
    return udp_->Send(data);
}

int Esp32Udp::Send(const void* data, size_t len) {
    if (!udp_) return 0;
    // 将 void* 数据转换为 std::string
    std::string str_data(static_cast<const char*>(data), len);
    return udp_->Send(str_data);
}

int Esp32Udp::Receive(void* buffer, size_t len) {
    if (!udp_) return 0;
    // Udp 类没有 Receive 方法，返回 0 表示没有数据
    return 0;
}

void Esp32Udp::OnData(std::function<void(const void* data, size_t len)> callback) {
    if (!udp_) return;
    // 适配 Udp::OnMessage 到 UdpInterface::OnData
    udp_->OnMessage([callback](const std::string& data) {
        callback(data.data(), data.size());
    });
}

} // namespace esp32
} // namespace network

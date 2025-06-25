#include "esp32_network.h"
#include "../boards/common/board.h"

#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_app_format.h>
#include <esp_partition.h>
#include <esp_system.h>
#include <esp_app_desc.h>
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#ifdef SOC_HMAC_SUPPORTED
#include <esp_hmac.h>
#endif

#define TAG "ESP32Network"

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

int Esp32Http::Read(void* buffer, size_t len) {
    if (!http_) return 0;
    return http_->Read(static_cast<char*>(buffer), len);
}

size_t Esp32Http::GetBodyLength() const {
    if (!http_) return 0;
    return http_->GetBodyLength();
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

void Esp32Udp::OnMessage(std::function<void(const std::string& data)> callback) {
    if (!udp_) return;
    // 直接传递回调给底层Udp类
    udp_->OnMessage(callback);
}

// ESP32 OTA实现
Esp32Ota::Esp32Ota() {
}

Esp32Ota::~Esp32Ota() {
}

bool Esp32Ota::CheckVersion(const std::string& current_version, std::string& new_version, std::string& download_url) {
    // 这个方法通常需要与服务器通信来检查新版本
    // 这里简单返回false表示没有新版本
    ESP_LOGI(TAG, "Checking version: %s", current_version.c_str());
    return false;
}

bool Esp32Ota::UpgradeFirmware(const std::string& url, std::function<void(int progress, size_t speed)> callback) {
    ESP_LOGI(TAG, "Upgrading firmware from %s", url.c_str());
    esp_ota_handle_t update_handle = 0;
    auto update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "Failed to get update partition");
        return false;
    }

    ESP_LOGI(TAG, "Writing to partition %s at offset 0x%lx", update_partition->label, update_partition->address);
    bool image_header_checked = false;
    std::string image_header;

    auto http = std::unique_ptr<HttpInterface>(NetworkFactory::CreateHttp().release());
    if (!http->Open("GET", url)) {
        ESP_LOGE(TAG, "Failed to open HTTP connection");
        return false;
    }

    if (http->GetStatusCode() != 200) {
        ESP_LOGE(TAG, "Failed to get firmware, status code: %d", http->GetStatusCode());
        return false;
    }

    size_t content_length = http->GetBodyLength();
    if (content_length == 0) {
        ESP_LOGE(TAG, "Failed to get content length");
        return false;
    }

    char buffer[512];
    size_t total_read = 0, recent_read = 0;
    auto last_calc_time = esp_timer_get_time();
    while (true) {
        int ret = http->Read(buffer, sizeof(buffer));
        if (ret < 0) {
            ESP_LOGE(TAG, "Failed to read HTTP data");
            return false;
        }

        // Calculate speed and progress every second
        recent_read += ret;
        total_read += ret;
        if (esp_timer_get_time() - last_calc_time >= 1000000 || ret == 0) {
            size_t progress = total_read * 100 / content_length;
            ESP_LOGI(TAG, "Progress: %u%% (%u/%u), Speed: %uB/s", progress, total_read, content_length, recent_read);
            if (callback) {
                callback(progress, recent_read);
            }
            last_calc_time = esp_timer_get_time();
            recent_read = 0;
        }

        if (ret == 0) {
            break;
        }

        if (!image_header_checked) {
            image_header.append(buffer, ret);
            if (image_header.size() >= sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                esp_app_desc_t new_app_info;
                memcpy(&new_app_info, image_header.data() + sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t), sizeof(esp_app_desc_t));
                ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

                auto current_version = esp_app_get_description()->version;
                if (memcmp(new_app_info.version, current_version, sizeof(new_app_info.version)) == 0) {
                    ESP_LOGE(TAG, "Firmware version is the same, skipping upgrade");
                    return false;
                }

                if (esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle)) {
                    esp_ota_abort(update_handle);
                    ESP_LOGE(TAG, "Failed to begin OTA");
                    return false;
                }

                image_header_checked = true;
                std::string().swap(image_header);
            }
        }
        auto err = esp_ota_write(update_handle, buffer, ret);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write OTA data");
            esp_ota_abort(update_handle);
            return false;
        }
    }
    http->Close();

    esp_err_t err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        } else {
            ESP_LOGE(TAG, "Failed to end OTA");
        }
        return false;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set boot partition");
        return false;
    }

    ESP_LOGI(TAG, "Firmware upgrade successful, rebooting in 3 seconds...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
    return true;
}

bool Esp32Ota::MarkCurrentVersionValid() {
    auto partition = esp_ota_get_running_partition();
    if (strcmp(partition->label, "factory") == 0) {
        ESP_LOGI(TAG, "Running from factory partition, skipping");
        return true;
    }

    ESP_LOGI(TAG, "Running partition: %s", partition->label);
    esp_ota_img_states_t state;
    if (esp_ota_get_state_partition(partition, &state) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get state of partition");
        return false;
    }

    if (state == ESP_OTA_IMG_PENDING_VERIFY) {
        ESP_LOGI(TAG, "Marking firmware as valid");
        esp_ota_mark_app_valid_cancel_rollback();
    }
    return true;
}

std::string Esp32Ota::GetAppName() {
    const esp_app_desc_t* app_desc = esp_app_get_description();
    return std::string(app_desc->project_name);
}

std::string Esp32Ota::GetAppVersion() {
    const esp_app_desc_t* app_desc = esp_app_get_description();
    return std::string(app_desc->version);
}

std::string Esp32Ota::GetAppDescription() {
    const esp_app_desc_t* app_desc = esp_app_get_description();
    // 使用project_name作为描述，因为esp_app_desc_t没有description成员
    return std::string("ESP32 Application: ") + app_desc->project_name;
}

std::string Esp32Ota::CalculateHmac(const std::string& data) {
    std::string hmac_hex;
#ifdef SOC_HMAC_SUPPORTED
    uint8_t hmac_result[32]; // SHA-256 输出为32字节
    
    // 使用Key0计算HMAC
    esp_err_t ret = esp_hmac_calculate(HMAC_KEY0, (uint8_t*)data.data(), data.size(), hmac_result);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "HMAC calculation failed");
        return "";
    }

    for (size_t i = 0; i < sizeof(hmac_result); i++) {
        char buffer[3];
        sprintf(buffer, "%02x", hmac_result[i]);
        hmac_hex += buffer;
    }
#endif
    return hmac_hex;
}

} // namespace esp32
} // namespace network

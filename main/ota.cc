#include "ota.h"
#include "system_info.h"
#include "settings.h"
#include "assets/lang_config.h"
#include "platform/system_interface.h"
#include "network/network_interface.h"

#include <cJSON.h>
#include <algorithm>
#include <vector>
#include <sstream>
#include <cstring>

#define TAG "Ota"

// 使用平台无关的日志接口
#define LOG_E(...) platform::Logger::GetInstance()->Log(platform::LogLevel::kError, TAG, __VA_ARGS__)
#define LOG_W(...) platform::Logger::GetInstance()->Log(platform::LogLevel::kWarning, TAG, __VA_ARGS__)
#define LOG_I(...) platform::Logger::GetInstance()->Log(platform::LogLevel::kInfo, TAG, __VA_ARGS__)
#define LOG_D(...) platform::Logger::GetInstance()->Log(platform::LogLevel::kDebug, TAG, __VA_ARGS__)

Ota::Ota() {
    // 获取系统信息
    auto system_info = platform::SystemFactory::CreateSystemInfo();
    
    // 读取序列号（平台相关实现）
    // 这里假设序列号可以从系统信息中获取
    serial_number_ = system_info->GetMacAddress(); // 暂时使用MAC地址作为序列号
    has_serial_number_ = !serial_number_.empty();
}

Ota::~Ota() {
}

std::string Ota::GetCheckVersionUrl() {
    Settings settings("wifi", false);
    std::string url = settings.GetString("ota_url");
    if (url.empty()) {
        url = CONFIG_OTA_URL;
    }
    return url;
}

Http* Ota::SetupHttp() {
    auto& board = Board::GetInstance();
    auto system_info = platform::SystemFactory::CreateSystemInfo();

    auto http = board.CreateHttp();
    http->SetHeader("Activation-Version", has_serial_number_ ? "2" : "1");
    http->SetHeader("Device-Id", system_info->GetMacAddress().c_str());
    http->SetHeader("Client-Id", board.GetUuid());
    if (has_serial_number_) {
        http->SetHeader("Serial-Number", serial_number_.c_str());
    }
    http->SetHeader("User-Agent", std::string(BOARD_NAME "/") + system_info->GetAppVersion());
    http->SetHeader("Accept-Language", Lang::CODE);
    http->SetHeader("Content-Type", "application/json");

    return http;
}

/* 
 * Specification: https://ccnphfhqs21z.feishu.cn/wiki/FjW6wZmisimNBBkov6OcmfvknVd
 */
bool Ota::CheckVersion() {
    auto system_info = platform::SystemFactory::CreateSystemInfo();

    // Check if there is a new firmware version available
    current_version_ = system_info->GetAppVersion();
    LOG_I("Current version: %s", current_version_.c_str());

    std::string url = GetCheckVersionUrl();
    if (url.length() < 10) {
        LOG_E("Check version URL is not properly set");
        return false;
    }

    auto http = std::unique_ptr<Http>(SetupHttp());

    auto& board = Board::GetInstance();
    std::string data = board.GetJson();
    std::string method = data.length() > 0 ? "POST" : "GET";
    http->SetContent(std::move(data));

    if (!http->Open(method, url)) {
        LOG_E("Failed to open HTTP connection");
        return false;
    }

    auto status_code = http->GetStatusCode();
    if (status_code != 200) {
        LOG_E("Failed to check version, status code: %d", status_code);
        return false;
    }

    data = http->ReadAll();
    http->Close();

    // Response: { "firmware": { "version": "1.0.0", "url": "http://" } }
    // Parse the JSON response and check if the version is newer
    // If it is, set has_new_version_ to true and store the new version and URL
    
    cJSON *root = cJSON_Parse(data.c_str());
    if (root == NULL) {
        LOG_E("Failed to parse JSON response");
        return false;
    }

    has_activation_code_ = false;
    has_activation_challenge_ = false;
    cJSON *activation = cJSON_GetObjectItem(root, "activation");
    if (cJSON_IsObject(activation)) {
        cJSON* message = cJSON_GetObjectItem(activation, "message");
        if (cJSON_IsString(message)) {
            activation_message_ = message->valuestring;
        }
        cJSON* code = cJSON_GetObjectItem(activation, "code");
        if (cJSON_IsString(code)) {
            activation_code_ = code->valuestring;
            has_activation_code_ = true;
        }
        cJSON* challenge = cJSON_GetObjectItem(activation, "challenge");
        if (cJSON_IsString(challenge)) {
            activation_challenge_ = challenge->valuestring;
            has_activation_challenge_ = true;
        }
        cJSON* timeout_ms = cJSON_GetObjectItem(activation, "timeout_ms");
        if (cJSON_IsNumber(timeout_ms)) {
            activation_timeout_ms_ = timeout_ms->valueint;
        }
    }

    has_mqtt_config_ = false;
    cJSON *mqtt = cJSON_GetObjectItem(root, "mqtt");
    if (cJSON_IsObject(mqtt)) {
        Settings settings("mqtt", true);
        cJSON *item = NULL;
        cJSON_ArrayForEach(item, mqtt) {
            if (cJSON_IsString(item)) {
                if (settings.GetString(item->string) != item->valuestring) {
                    settings.SetString(item->string, item->valuestring);
                }
            } else if (cJSON_IsNumber(item)) {
                if (settings.GetInt(item->string) != item->valueint) {
                    settings.SetInt(item->string, item->valueint);
                }
            }
        }
        has_mqtt_config_ = true;
    } else {
        LOG_I("No mqtt section found !");
    }

    has_websocket_config_ = false;
    cJSON *websocket = cJSON_GetObjectItem(root, "websocket");
    if (cJSON_IsObject(websocket)) {
        Settings settings("websocket", true);
        cJSON *item = NULL;
        cJSON_ArrayForEach(item, websocket) {
            if (cJSON_IsString(item)) {
                if (settings.GetString(item->string) != item->valuestring) {
                    settings.SetString(item->string, item->valuestring);
                }
            } else if (cJSON_IsNumber(item)) {
                if (settings.GetInt(item->string) != item->valueint) {
                    settings.SetInt(item->string, item->valueint);
                }
            }
        }
        has_websocket_config_ = true;
    } else {
        LOG_I("No websocket section found!");
    }

    has_server_time_ = false;
    cJSON *server_time = cJSON_GetObjectItem(root, "server_time");
    if (cJSON_IsObject(server_time)) {
        cJSON *timestamp = cJSON_GetObjectItem(server_time, "timestamp");
        cJSON *timezone_offset = cJSON_GetObjectItem(server_time, "timezone_offset");
        
        if (cJSON_IsNumber(timestamp)) {
            // 设置系统时间 - 这部分需要平台相关实现
            // 暂时保留时间戳信息，但不设置系统时间
            has_server_time_ = true;
        }
    } else {
        LOG_W("No server_time section found!");
    }

    has_new_version_ = false;
    cJSON *firmware = cJSON_GetObjectItem(root, "firmware");
    if (cJSON_IsObject(firmware)) {
        cJSON *version = cJSON_GetObjectItem(firmware, "version");
        if (cJSON_IsString(version)) {
            firmware_version_ = version->valuestring;
        }
        cJSON *url = cJSON_GetObjectItem(firmware, "url");
        if (cJSON_IsString(url)) {
            firmware_url_ = url->valuestring;
        }

        if (cJSON_IsString(version) && cJSON_IsString(url)) {
            // Check if the version is newer, for example, 0.1.0 is newer than 0.0.1
            has_new_version_ = IsNewVersionAvailable(current_version_, firmware_version_);
            if (has_new_version_) {
                LOG_I("New version available: %s", firmware_version_.c_str());
            } else {
                LOG_I("Current is the latest version");
            }
            // If the force flag is set to 1, the given version is forced to be installed
            cJSON *force = cJSON_GetObjectItem(firmware, "force");
            if (cJSON_IsNumber(force) && force->valueint == 1) {
                has_new_version_ = true;
            }
        }
    } else {
        LOG_W("No firmware section found!");
    }

    cJSON_Delete(root);
    return true;
}

void Ota::MarkCurrentVersionValid() {
    // 使用OtaInterface来标记当前版本为有效
    auto ota = network::NetworkFactory::CreateOta();
    if (ota) {
        ota->MarkCurrentVersionValid();
    }
}

void Ota::Upgrade(const std::string& firmware_url) {
    // 使用OtaInterface来升级固件
    auto ota = network::NetworkFactory::CreateOta();
    if (ota) {
        ota->UpgradeFirmware(firmware_url, upgrade_callback_);
    } else {
        LOG_E("Failed to create OTA interface");
    }
}

void Ota::StartUpgrade(std::function<void(int progress, size_t speed)> callback) {
    upgrade_callback_ = callback;
    Upgrade(firmware_url_);
}

std::vector<int> Ota::ParseVersion(const std::string& version) {
    std::vector<int> versionNumbers;
    std::stringstream ss(version);
    std::string segment;
    
    while (std::getline(ss, segment, '.')) {
        versionNumbers.push_back(std::stoi(segment));
    }
    
    return versionNumbers;
}

bool Ota::IsNewVersionAvailable(const std::string& currentVersion, const std::string& newVersion) {
    std::vector<int> current = ParseVersion(currentVersion);
    std::vector<int> newer = ParseVersion(newVersion);
    
    for (size_t i = 0; i < std::min(current.size(), newer.size()); ++i) {
        if (newer[i] > current[i]) {
            return true;
        } else if (newer[i] < current[i]) {
            return false;
        }
    }
    
    return newer.size() > current.size();
}

std::string Ota::GetActivationPayload() {
    if (!has_serial_number_) {
        return "{}";
    }

    // 使用OtaInterface来计算HMAC
    std::string hmac_hex;
    auto ota = network::NetworkFactory::CreateOta();
    if (ota) {
        hmac_hex = ota->CalculateHmac(activation_challenge_);
    }
    
    cJSON *payload = cJSON_CreateObject();
    cJSON_AddStringToObject(payload, "algorithm", "hmac-sha256");
    cJSON_AddStringToObject(payload, "serial_number", serial_number_.c_str());
    cJSON_AddStringToObject(payload, "challenge", activation_challenge_.c_str());
    cJSON_AddStringToObject(payload, "hmac", hmac_hex.c_str());
    auto json_str = cJSON_PrintUnformatted(payload);
    std::string json(json_str);
    cJSON_free(json_str);
    cJSON_Delete(payload);

    LOG_I("Activation payload: %s", json.c_str());
    return json;
}

platform::SystemError Ota::Activate() {
    if (!has_activation_challenge_) {
        LOG_W("No activation challenge found");
        return platform::SystemError::kFailed;
    }

    std::string url = GetCheckVersionUrl();
    if (url.back() != '/') {
        url += "/activate";
    } else {
        url += "activate";
    }

    auto http = std::unique_ptr<Http>(SetupHttp());

    std::string data = GetActivationPayload();
    http->SetContent(std::move(data));

    if (!http->Open("POST", url)) {
        LOG_E("Failed to open HTTP connection");
        return platform::SystemError::kFailed;
    }
    
    auto status_code = http->GetStatusCode();
    if (status_code == 202) {
        return platform::SystemError::kTimeout;
    }
    if (status_code != 200) {
        LOG_E("Failed to activate, code: %d, body: %s", status_code, http->ReadAll().c_str());
        return platform::SystemError::kFailed;
    }

    LOG_I("Activation successful");
    return platform::SystemError::kSuccess;
}

# XRobot 系统依赖关系图

## 架构层次
```plaintext
应用层 (Application Layer)
├── Application.cc
├── MCP Server
├── IoT Thing Manager
└── Background Tasks

协议层 (Protocol Layer)
├── MQTT Protocol
├── WebSocket Protocol
└── HTTP/HTTPS

服务层 (Service Layer)
├── Audio Processing (ESP-SR)
├── Display Management
├── LED Control
├── Button Handling
└── OTA Updates

硬件抽象层 (HAL)
├── Audio Codecs
├── Display Drivers
├── GPIO Control
├── SPI/I2C Bus
└── Network Interface

ESP-IDF 平台层
├── FreeRTOS
├── WiFi Stack
├── Driver Layer
├── NVS Storage
└── Event System

硬件层 (Hardware)
├── ESP32/ESP32-S3/ESP32-C3/ESP32-P4
├── Audio Codec Chips
├── Display Panels
├── Sensors
└── Actuators
```

## 关键依赖关系

1. **Application → ESP-IDF Event System**
2. **Audio Processing → ESP-SR + Audio Codecs**
3. **Display → ESP-LCD + SPI/I2C**
4. **Network → ESP-WiFi + LwIP**
5. **Storage → NVS Flash**
6. **Threading → FreeRTOS**
# 阶段1：平台抽象层重构

## 项目背景
本项目基于ESP32 IDF开发，代码库位于workspace的`main/`目录。本阶段将创建平台无关的抽象层，移除ESP32 IDF特定依赖。

## 目标
创建平台无关的抽象层，移除ESP32 IDF特定依赖，确保代码可以在ESP32和Linux上编译

### 1.1 系统抽象层
- [x] 创建 `main/platform/` 目录结构
- [x] 实现 `main/platform/system_interface.h` 系统接口抽象
- [x] 创建 `main/platform/esp32/esp32_system.h/cc` ESP32平台实现
- [x] 创建 `main/platform/linux/linux_system.h/cc` Linux平台实现
- [x] 抽象定时器接口（Timer类），替代ESP32 IDF定时器
- [x] 抽象事件循环接口（EventLoop类），替代ESP32事件循环
- [x] 抽象任务管理接口（TaskManager类），替代FreeRTOS任务
- [x] **修改现有代码**：将 `main/background_task.h/cc` 修改为使用新的TaskManager接口，移除FreeRTOS直接调用
- [x] **修改CMakeLists.txt**：添加平台检测和条件编译支持，兼容ESP-IDF构建系统
- [x] **编译验证**：确保代码没有ESP32 IDF特定依赖且ESP32和Linux都能正常编译
- [x] **功能验证**：ESP32硬件测试功能正常

### 1.2 存储抽象层
- [ ] 创建 `main/storage/storage_interface.h`
- [ ] 实现 `main/storage/nvs_storage.h/cc` (ESP32 NVS)
- [ ] 实现 `main/storage/file_storage.h/cc` (Linux文件系统)
- [ ] 创建 `main/storage/storage_factory.h/cc` 工厂类
- [ ] **修改现有代码**：将 `main/settings.h/cc` 修改为使用新的存储抽象层接口，移除ESP32 NVS直接调用
- [ ] **修改现有代码**：将 `main/ota.h/cc` 中的存储操作修改为使用新接口，移除ESP32分区操作
- [ ] **修改CMakeLists.txt**：添加存储模块的条件编译配置
- [ ] **编译验证**：确保代码没有ESP32特定依赖且ESP32和Linux都能正常编译
- [ ] **功能验证**：ESP32 NVS存储功能正常

### 1.3 网络抽象层
- [ ] 创建 `main/network/network_interface.h`
- [ ] 实现 `main/network/esp32_network.h/cc` (基于ESP-IDF WiFi)
- [ ] 实现 `main/network/linux_network.h/cc` (基于标准socket)
- [ ] 创建网络工厂类
- [ ] **修改现有代码**：将 `main/protocols/` 目录下的网络相关代码修改为使用新的网络抽象层，移除ESP-IDF网络组件依赖
- [ ] **修改现有代码**：将 `main/mcp_server.h/cc` 修改为使用新的网络接口，移除ESP32 WiFi直接调用
- [ ] **修改现有代码**：将 `main/ota.h/cc` 中的网络操作修改为使用新接口
- [ ] **修改CMakeLists.txt**：添加网络模块的条件编译和依赖管理
- [ ] **编译验证**：确保代码没有ESP32特定依赖且ESP32和Linux都能正常编译
- [ ] **功能验证**：ESP32 WiFi功能正常

### 1.4 日志系统抽象
- [ ] 创建 `main/logging/logger.h`
- [ ] 实现 `main/logging/esp_logger.cc` (包装ESP-IDF esp_log)
- [ ] 实现 `main/logging/console_logger.cc` (Linux标准输出)
- [ ] 添加日志宏定义兼容层，替代ESP_LOG*宏
- [ ] **修改现有代码**：将所有源文件中的ESP_LOG*宏调用修改为使用新的日志抽象层
- [ ] **修改现有代码**：将 `main/audio_processing/` 目录下的日志调用修改为使用新接口
- [ ] **修改现有代码**：将 `main/display/` 目录下的日志调用修改为使用新接口
- [ ] **修改现有代码**：将 `main/iot/` 目录下的日志调用修改为使用新接口
- [ ] **修改CMakeLists.txt**：添加日志系统的条件编译配置
- [ ] **编译验证**：确保代码没有ESP32特定依赖且ESP32和Linux都能正常编译
- [ ] **功能验证**：ESP32日志输出正常

### 1.5 硬件抽象层
- [ ] 创建 `main/hardware/hardware_interface.h`
- [ ] 实现 `main/hardware/esp32_hardware.h/cc` (基于ESP-IDF GPIO/I2C/SPI)
- [ ] 实现 `main/hardware/linux_hardware.h/cc` (模拟实现)
- [ ] **修改现有代码**：将 `main/audio_codecs/` 目录下的硬件操作修改为使用新的硬件抽象层，移除ESP32 GPIO/I2S直接调用
- [ ] **修改现有代码**：将 `main/display/` 目录下的硬件操作修改为使用新接口，移除SPI直接调用
- [ ] **修改现有代码**：将 `main/led/` 目录下的硬件操作修改为使用新接口，移除GPIO直接调用
- [ ] **修改现有代码**：将各个 `main/boards/` 配置文件修改为使用新的硬件抽象层
- [ ] **修改CMakeLists.txt**：添加硬件抽象层的条件编译配置
- [ ] **编译验证**：确保代码没有ESP32特定依赖且ESP32和Linux都能正常编译
- [ ] **功能验证**：ESP32硬件控制正常

### 1.6 集成测试与验证
- [ ] **完整编译验证**：确保所有修改后的代码完全没有ESP32 IDF特定依赖
- [ ] **完整编译验证**：确保ESP32平台使用ESP-IDF编译成功且功能完整
- [ ] **完整编译验证**：确保Linux平台编译成功
- [ ] **功能验证**：ESP32硬件测试所有功能正常
- [ ] **回归测试**：确保重构后ESP32版本与原ESP-IDF版本功能完全一致
- [ ] **性能验证**：确保抽象层不会显著影响ESP32系统性能
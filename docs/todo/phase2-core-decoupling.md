# 阶段2：核心业务逻辑解耦

## 项目背景
本项目是基于ESP32 IDF开发的xrobot项目，代码库位于workspace的`main/`目录下。本阶段目标是将核心业务逻辑从ESP32特定代码中完全分离，使用阶段1的抽象层，确保代码完全平台无关。

## 目标
将核心业务逻辑从ESP32特定代码中完全分离，使用阶段1的抽象层，确保代码完全平台无关

### 2.1 应用层重构
- [ ] 创建 `main/core/application_core.h/cc` (完全平台无关逻辑)
- [ ] 重构 `main/application.h/cc` 中的 `Application` 类使用平台抽象层，移除所有ESP32特定代码
- [ ] 实现依赖注入容器
- [ ] 分离硬件初始化和业务逻辑
- [ ] **Git提交**：提交应用核心抽象层框架
- [ ] **修改现有代码**：将 `main/application.h/cc` 修改为使用新的核心抽象层，移除ESP32依赖
- [ ] **Git提交**：提交application核心重构
- [ ] **修改现有代码**：将 `main/main.cc` 修改为使用新的应用层接口
- [ ] **Git提交**：提交main应用层重构
- [ ] **修改CMakeLists.txt**：添加应用核心模块的条件编译配置
- [ ] **Git提交**：提交应用层CMake配置
- [ ] **编译验证**：确保代码没有ESP32特定依赖且ESP32和Linux都能正常编译
- [ ] **功能验证**：ESP32版本应用逻辑正常
- [ ] **Git提交**：提交应用层重构完整实现

### 2.2 音频处理抽象
- [ ] 创建 `main/audio/audio_interface.h`
- [ ] 重构现有 `main/audio_codecs/` 目录下的音频编解码器实现接口，移除ESP32特定依赖
- [ ] 实现 `main/audio/null_audio_codec.cc` (测试用)
- [ ] 实现 `main/audio/alsa_audio_codec.cc` (Linux)
- [ ] 创建音频工厂类
- [ ] **Git提交**：提交音频抽象层框架
- [ ] **修改现有代码**：将 `main/audio_codecs/` 目录下所有编解码器修改为使用新的音频接口，移除I2S直接调用
- [ ] **Git提交**：提交audio_codecs重构
- [ ] **修改现有代码**：将 `main/audio_processing/` 目录下的处理器修改为使用新接口
- [ ] **Git提交**：提交audio_processing重构
- [ ] **修改CMakeLists.txt**：添加音频模块的条件编译和依赖管理
- [ ] **Git提交**：提交音频模块CMake配置
- [ ] **编译验证**：确保代码没有ESP32特定依赖且ESP32和Linux都能正常编译
- [ ] **功能验证**：ESP32音频功能正常
- [ ] **Git提交**：提交音频处理抽象完整实现

### 2.3 显示系统抽象
- [ ] 创建 `main/display/display_interface.h`
- [ ] 重构现有 `main/display/` 目录下的显示类实现接口，移除ESP32特定依赖
- [ ] 实现 `main/display/console_display.cc` (Linux终端)
- [ ] 实现 `main/display/null_display.cc` (无显示模式)
- [ ] **Git提交**：提交显示抽象层框架
- [ ] **修改现有代码**：将 `main/display/lcd_display.cc` 修改为使用新的显示接口，移除SPI直接调用
- [ ] **Git提交**：提交lcd_display重构
- [ ] **修改现有代码**：将 `main/display/oled_display.cc` 修改为使用新的显示接口
- [ ] **Git提交**：提交oled_display重构
- [ ] **修改现有代码**：将所有使用显示功能的模块修改为使用新接口
- [ ] **Git提交**：提交显示功能模块重构
- [ ] **修改CMakeLists.txt**：添加显示模块的条件编译配置
- [ ] **Git提交**：提交显示模块CMake配置
- [ ] **编译验证**：确保代码没有ESP32特定依赖且ESP32和Linux都能正常编译
- [ ] **功能验证**：ESP32显示功能正常
- [ ] **Git提交**：提交显示系统抽象完整实现

### 2.4 LED控制抽象
- [ ] 创建 `main/led/led_interface.h`
- [ ] 重构现有 `main/led/` 目录下的LED类实现接口，移除ESP32特定依赖
- [ ] 实现 `main/led/console_led.cc` (终端输出)
- [ ] 实现 `main/led/null_led.cc` (无LED模式)
- [ ] **Git提交**：提交LED抽象层框架
- [ ] **修改现有代码**：将 `main/led/gpio_led.cc` 修改为使用新的LED接口，移除GPIO直接调用
- [ ] **Git提交**：提交gpio_led重构
- [ ] **修改现有代码**：将 `main/led/single_led.cc` 修改为使用新的LED接口
- [ ] **Git提交**：提交single_led重构
- [ ] **修改现有代码**：将 `main/led/circular_strip.cc` 修改为使用新的LED接口
- [ ] **Git提交**：提交circular_strip重构
- [ ] **修改现有代码**：将所有使用LED功能的模块修改为使用新接口
- [ ] **Git提交**：提交LED功能模块重构
- [ ] **修改CMakeLists.txt**：添加LED模块的条件编译配置
- [ ] **Git提交**：提交LED模块CMake配置
- [ ] **编译验证**：确保代码没有ESP32特定依赖且ESP32和Linux都能正常编译
- [ ] **功能验证**：ESP32 LED控制正常
- [ ] **Git提交**：提交LED控制抽象完整实现

### 2.5 系统信息抽象
- [ ] 创建 `main/system/system_info_interface.h`
- [ ] 实现 `main/system/esp32_system_info.cc`
- [ ] 实现 `main/system/linux_system_info.cc`
- [ ] **Git提交**：提交系统信息抽象层框架
- [ ] **修改现有代码**：将 `main/system_info.h/cc` 修改为使用新的系统信息接口，移除ESP32特定API调用
- [ ] **Git提交**：提交system_info重构
- [ ] **修改现有代码**：将所有获取系统信息的模块修改为使用新接口
- [ ] **Git提交**：提交系统信息模块重构
- [ ] **修改CMakeLists.txt**：添加系统信息模块的条件编译配置
- [ ] **Git提交**：提交系统信息模块CMake配置
- [ ] **编译验证**：确保代码没有ESP32特定依赖且ESP32和Linux都能正常编译
- [ ] **功能验证**：ESP32系统信息获取正常
- [ ] **Git提交**：提交系统信息抽象完整实现

### 2.6 集成测试与验证
- [ ] **完整编译验证**：确保所有修改后的代码完全没有ESP32特定依赖
- [ ] **完整编译验证**：确保ESP32平台编译成功且功能完整
- [ ] **完整编译验证**：确保Linux平台编译成功
- [ ] **功能验证**：ESP32硬件测试所有核心功能正常
- [ ] **Git提交**：提交阶段2完整验证结果
- [ ] **功能验证**：Linux环境测试所有核心功能正常
- [ ] **一致性验证**：确保ESP32和Linux版本核心业务逻辑完全一致
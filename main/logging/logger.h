#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace logging {

// 日志级别定义
enum class LogLevel {
    kNone,
    kError,
    kWarning,
    kInfo,
    kDebug,
    kVerbose
};

// 日志接口
class Logger {
public:
    virtual ~Logger() = default;
    
    // 记录日志的主要方法
    virtual void Log(LogLevel level, const char* tag, const char* format, ...) = 0;
    
    // 获取日志实例的工厂方法
    static std::unique_ptr<Logger>& GetInstance();
    
    // 设置全局日志级别
    virtual void SetLogLevel(LogLevel level) = 0;
    
    // 获取当前日志级别
    virtual LogLevel GetLogLevel() const = 0;
    
    // 检查指定级别的日志是否会被输出
    virtual bool IsLogLevelEnabled(LogLevel level) const = 0;
};

// 便捷的日志宏定义
#define LOG_E(tag, ...) logging::Logger::GetInstance()->Log(logging::LogLevel::kError, tag, __VA_ARGS__)
#define LOG_W(tag, ...) logging::Logger::GetInstance()->Log(logging::LogLevel::kWarning, tag, __VA_ARGS__)
#define LOG_I(tag, ...) logging::Logger::GetInstance()->Log(logging::LogLevel::kInfo, tag, __VA_ARGS__)
#define LOG_D(tag, ...) logging::Logger::GetInstance()->Log(logging::LogLevel::kDebug, tag, __VA_ARGS__)
#define LOG_V(tag, ...) logging::Logger::GetInstance()->Log(logging::LogLevel::kVerbose, tag, __VA_ARGS__)

// TAG宏简化版，可在文件开头定义TAG后使用
#define TAG_LOG_E(...) logging::Logger::GetInstance()->Log(logging::LogLevel::kError, TAG, __VA_ARGS__)
#define TAG_LOG_W(...) logging::Logger::GetInstance()->Log(logging::LogLevel::kWarning, TAG, __VA_ARGS__)
#define TAG_LOG_I(...) logging::Logger::GetInstance()->Log(logging::LogLevel::kInfo, TAG, __VA_ARGS__)
#define TAG_LOG_D(...) logging::Logger::GetInstance()->Log(logging::LogLevel::kDebug, TAG, __VA_ARGS__)
#define TAG_LOG_V(...) logging::Logger::GetInstance()->Log(logging::LogLevel::kVerbose, TAG, __VA_ARGS__)

// 兼容ESP-IDF日志宏的定义，便于迁移现有代码
#ifndef ESP_LOGE
#define ESP_LOGE(tag, ...) logging::Logger::GetInstance()->Log(logging::LogLevel::kError, tag, __VA_ARGS__)
#endif
#ifndef ESP_LOGW
#define ESP_LOGW(tag, ...) logging::Logger::GetInstance()->Log(logging::LogLevel::kWarning, tag, __VA_ARGS__)
#endif
#ifndef ESP_LOGI
#define ESP_LOGI(tag, ...) logging::Logger::GetInstance()->Log(logging::LogLevel::kInfo, tag, __VA_ARGS__)
#endif
#ifndef ESP_LOGD
#define ESP_LOGD(tag, ...) logging::Logger::GetInstance()->Log(logging::LogLevel::kDebug, tag, __VA_ARGS__)
#endif
#ifndef ESP_LOGV
#define ESP_LOGV(tag, ...) logging::Logger::GetInstance()->Log(logging::LogLevel::kVerbose, tag, __VA_ARGS__)
#endif

} // namespace logging

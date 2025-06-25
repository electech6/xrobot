#include "logger.h"

#include <esp_log.h>
#include <cstdarg>

namespace logging {

class EspLogger : public Logger {
public:
    EspLogger() : log_level_(LogLevel::kInfo) {
        // 默认使用INFO级别
        esp_log_level_set("*", ESP_LOG_INFO);
    }
    
    void Log(LogLevel level, const char* tag, const char* format, ...) override {
        // 检查日志级别是否启用
        if (!IsLogLevelEnabled(level)) {
            return;
        }
        
        va_list args;
        va_start(args, format);
        
        // 将日志级别转换为ESP-IDF的日志级别
        esp_log_level_t esp_level = ConvertToEspLogLevel(level);
        
        // 使用ESP-IDF的日志函数输出日志
        esp_log_write(esp_level, tag, format, args);
        
        va_end(args);
    }
    
    void SetLogLevel(LogLevel level) override {
        log_level_ = level;
        
        // 同时设置ESP-IDF的全局日志级别
        esp_log_level_set("*", ConvertToEspLogLevel(level));
    }
    
    LogLevel GetLogLevel() const override {
        return log_level_;
    }
    
    bool IsLogLevelEnabled(LogLevel level) const override {
        return static_cast<int>(level) <= static_cast<int>(log_level_);
    }
    
private:
    LogLevel log_level_;
    
    // 将日志级别转换为ESP-IDF的日志级别
    esp_log_level_t ConvertToEspLogLevel(LogLevel level) const {
        switch (level) {
            case LogLevel::kError:
                return ESP_LOG_ERROR;
            case LogLevel::kWarning:
                return ESP_LOG_WARN;
            case LogLevel::kInfo:
                return ESP_LOG_INFO;
            case LogLevel::kDebug:
                return ESP_LOG_DEBUG;
            case LogLevel::kVerbose:
                return ESP_LOG_VERBOSE;
            default:
                return ESP_LOG_INFO;
        }
    }
};

// 实现Logger::GetInstance()工厂方法
std::unique_ptr<Logger>& Logger::GetInstance() {
    static std::unique_ptr<Logger> instance = std::make_unique<EspLogger>();
    return instance;
}

} // namespace logging

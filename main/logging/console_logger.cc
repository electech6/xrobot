#include "logger.h"

#ifndef ESP_PLATFORM
#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace logging {

class ConsoleLogger : public Logger {
public:
    ConsoleLogger() : log_level_(LogLevel::kInfo) {
        // 默认使用INFO级别
    }
    
    void Log(LogLevel level, const char* tag, const char* format, ...) override {
        // 检查日志级别是否启用
        if (!IsLogLevelEnabled(level)) {
            return;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_point = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream time_str;
        time_str << std::put_time(std::localtime(&time_point), "%Y-%m-%d %H:%M:%S")
                 << '.' << std::setfill('0') << std::setw(3) << ms.count();
        
        // 获取日志级别字符串
        const char* level_str = GetLogLevelString(level);
        
        // 格式化消息
        va_list args;
        va_start(args, format);
        char buffer[2048];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        // 输出到标准输出
        std::cout << time_str.str() << " [" << level_str << "][" << tag << "] " 
                  << buffer << std::endl;
    }
    
    void SetLogLevel(LogLevel level) override {
        log_level_ = level;
    }
    
    LogLevel GetLogLevel() const override {
        return log_level_;
    }
    
    bool IsLogLevelEnabled(LogLevel level) const override {
        return static_cast<int>(level) <= static_cast<int>(log_level_);
    }
    
private:
    LogLevel log_level_;
    
    // 获取日志级别的字符串表示
    const char* GetLogLevelString(LogLevel level) const {
        switch (level) {
            case LogLevel::kError:
                return "E";
            case LogLevel::kWarning:
                return "W";
            case LogLevel::kInfo:
                return "I";
            case LogLevel::kDebug:
                return "D";
            case LogLevel::kVerbose:
                return "V";
            default:
                return "I";
        }
    }
};

// 实现Logger::GetInstance()工厂方法
std::unique_ptr<Logger>& Logger::GetInstance() {
    static std::unique_ptr<Logger> instance = std::make_unique<ConsoleLogger>();
    return instance;
}

} // namespace logging

#endif // !ESP_PLATFORM

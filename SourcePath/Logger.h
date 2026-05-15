#pragma once
#include "Common.h"
#include <fstream>
#include <mutex>
#include <queue>

namespace MIT {
    enum class LogLevel {
        Info,
        Warning,
        Error,
        Critical
    };

    class Logger {
    private:
        std::ofstream logFile;
        std::mutex logMutex;
        std::queue<std::string> logQueue;
        std::thread logThread;
        std::atomic<bool> running;
        
        std::string GetTimestamp();
        std::string LevelToString(LogLevel level);
        void ProcessLogQueue();

    public:
        Logger();
        ~Logger();
        
        void Log(LogLevel level, const std::string& message);
        void LogInfo(const std::string& message);
        void LogWarning(const std::string& message);
        void LogError(const std::string& message);
        void LogCritical(const std::string& message);
        
        void Flush();
        void Close();
    };

    // Global logger instance
    extern std::unique_ptr<Logger> gLogger;
    
    // Convenience macros
    #define LOG_INFO(msg) if (MIT::gLogger) MIT::gLogger->LogInfo(msg)
    #define LOG_WARNING(msg) if (MIT::gLogger) MIT::gLogger->LogWarning(msg)
    #define LOG_ERROR(msg) if (MIT::gLogger) MIT::gLogger->LogError(msg)
    #define LOG_CRITICAL(msg) if (MIT::gLogger) MIT::gLogger->LogCritical(msg)
}

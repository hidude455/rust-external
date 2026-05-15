#include "Logger.h"
#include <iomanip>
#include <chrono>

namespace MIT {
    std::unique_ptr<Logger> gLogger = nullptr;

    Logger::Logger() : running(true) {
        // Create log file with timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::stringstream filename;
        filename << LOGS_PATH << "/MITMethod_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".log";
        
        logFile.open(filename.str(), std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            // Fallback to default location
            logFile.open("MITMethod.log", std::ios::out | std::ios::app);
        }
        
        // Start background logging thread
        logThread = std::thread(&Logger::ProcessLogQueue, this);
    }

    Logger::~Logger() {
        running = false;
        if (logThread.joinable()) {
            logThread.join();
        }
        Close();
    }

    std::string Logger::GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::stringstream ss;
        ss << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S]");
        return ss.str();
    }

    std::string Logger::LevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Info: return "[INFO]";
            case LogLevel::Warning: return "[WARNING]";
            case LogLevel::Error: return "[ERROR]";
            case LogLevel::Critical: return "[CRITICAL]";
            default: return "[UNKNOWN]";
        }
    }

    void Logger::ProcessLogQueue() {
        while (running || !logQueue.empty()) {
            if (!logQueue.empty()) {
                std::lock_guard<std::mutex> lock(logMutex);
                if (logFile.is_open()) {
                    logFile << logQueue.front() << std::endl;
                    logFile.flush();
                }
                logQueue.pop();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void Logger::Log(LogLevel level, const std::string& message) {
        std::stringstream logEntry;
        logEntry << GetTimestamp() << " " << LevelToString(level) << " " << message;
        
        std::lock_guard<std::mutex> lock(logMutex);
        logQueue.push(logEntry.str());
        
        // Also output to console for critical errors
        if (level == LogLevel::Critical || level == LogLevel::Error) {
            std::cout << logEntry.str() << std::endl;
        }
    }

    void Logger::LogInfo(const std::string& message) {
        Log(LogLevel::Info, message);
    }

    void Logger::LogWarning(const std::string& message) {
        Log(LogLevel::Warning, message);
    }

    void Logger::LogError(const std::string& message) {
        Log(LogLevel::Error, message);
    }

    void Logger::LogCritical(const std::string& message) {
        Log(LogLevel::Critical, message);
    }

    void Logger::Flush() {
        if (logFile.is_open()) {
            logFile.flush();
        }
    }

    void Logger::Close() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
}

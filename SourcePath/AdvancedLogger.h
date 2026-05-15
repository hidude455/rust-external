/*
 * Advanced Logging System for Rust Anti-Cheat Evasion System
 * Multiple output formats and log rotation
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <memory>
#include <functional>

namespace Logging {
    
    // Log levels
    enum class LogLevel {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Critical = 5
    };
    
    // Log output types
    enum class LogOutput {
        File,
        Console,
        Network,
        Database,
        EventLog
    };
    
    // Log entry structure
    struct LogEntry {
        LogLevel level;
        std::string message;
        std::string category;
        std::string function;
        std::string file;
        int line;
        uint64_t timestamp;
        uint32_t threadId;
        std::string module;
        std::map<std::string, std::string> metadata;
    };
    
    // Log configuration
    struct LogConfig {
        LogLevel minLevel;
        LogLevel maxLevel;
        std::vector<LogOutput> outputs;
        std::string logFilePath;
        std::string logFormat;
        bool enableTimestamp;
        bool enableThreadId;
        bool enableFunction;
        bool enableFileLine;
        bool enableRotation;
        size_t maxFileSize;
        int maxFileCount;
        bool enableCompression;
        bool enableNetworkLogging;
        std::string networkEndpoint;
        int networkPort;
        bool enableDatabase;
        std::string databasePath;
        bool enableEventLog;
        std::string eventLogSource;
        bool enableFiltering;
        std::vector<std::string> allowedCategories;
        std::vector<std::string> blockedCategories;
        bool enablePerformanceLogging;
        int performanceThresholdMs;
    };
    
    // Log filter
    struct LogFilter {
        std::string category;
        LogLevel minLevel;
        LogLevel maxLevel;
        std::string pattern;
        bool enabled;
    };
    
    // Log statistics
    struct LogStatistics {
        uint64_t totalEntries;
        uint64_t entriesByLevel[6];
        uint64_t entriesByCategory[100];
        uint64_t entriesPerHour[24];
        uint64_t averageEntriesPerSecond;
        uint64_t peakEntriesPerSecond;
        uint64_t totalFileSize;
        uint64_t lastRotationTime;
        std::string mostActiveCategory;
        std::string mostActiveLevel;
    };
    
    class CAdvancedLogger {
    private:
        // Configuration
        LogConfig m_config;
        std::vector<LogFilter> m_filters;
        
        // State
        bool m_isInitialized;
        std::mutex m_logMutex;
        std::vector<LogEntry> m_logBuffer;
        size_t m_bufferSize;
        uint64_t m_totalEntries;
        
        // File handling
        std::unique_ptr<std::ofstream> m_logFile;
        std::string m_currentLogFile;
        size_t m_currentFileSize;
        int m_currentFileIndex;
        
        // Network handling
        std::unique_ptr<class NetworkLogger> m_networkLogger;
        
        // Database handling
        std::unique_ptr<class DatabaseLogger> m_databaseLogger;
        
        // Statistics
        LogStatistics m_statistics;
        uint64_t m_lastStatsUpdate;
        
        // Callbacks
        std::function<void(const LogEntry&)> m_logCallback;
        std::function<void(const LogStatistics&)> m_statsCallback;
        
        // Private methods
        void WriteToFile(const LogEntry& entry);
        void WriteToConsole(const LogEntry& entry);
        void WriteToNetwork(const LogEntry& entry);
        void WriteToDatabase(const LogEntry& entry);
        void WriteToEventLog(const LogEntry& entry);
        void RotateLogFile();
        void CompressOldLogs();
        void UpdateStatistics(const LogEntry& entry);
        std::string FormatLogEntry(const LogEntry& entry) const;
        std::string GetLogLevelString(LogLevel level) const;
        std::string GetCurrentTimestamp() const;
        bool ShouldLog(const LogEntry& entry) const;
        void FlushBuffer();
        void InitializeFileLogging();
        void InitializeNetworkLogging();
        void InitializeDatabaseLogging();
        void InitializeEventLogging();
        void CleanupOldLogs();
        
    public:
        CAdvancedLogger();
        ~CAdvancedLogger();
        
        // Initialization
        bool Initialize(const LogConfig& config);
        void Shutdown();
        bool IsInitialized() const;
        
        // Configuration
        bool LoadConfiguration(const std::string& configFile = "rust_logging.ini");
        bool SaveConfiguration(const std::string& configFile = "rust_logging.ini");
        LogConfig GetConfig() const;
        void SetConfig(const LogConfig& config);
        
        // Logging methods
        void Log(LogLevel level, const std::string& message, 
                const std::string& category = "General",
                const std::string& function = "",
                const std::string& file = "",
                int line = 0);
        
        void LogTrace(const std::string& message, const std::string& category = "General");
        void LogDebug(const std::string& message, const std::string& category = "General");
        void LogInfo(const std::string& message, const std::string& category = "General");
        void LogWarning(const std::string& message, const std::string& category = "General");
        void LogError(const std::string& message, const std::string& category = "General");
        void LogCritical(const std::string& message, const std::string& category = "General");
        
        // Performance logging
        void LogPerformance(const std::string& operation, int durationMs, 
                         const std::string& category = "Performance");
        void LogMemoryUsage(size_t usedMemory, size_t totalMemory);
        void LogCpuUsage(float cpuUsage);
        void LogNetworkActivity(const std::string& endpoint, int bytesTransferred);
        
        // Filtering
        bool AddFilter(const LogFilter& filter);
        bool RemoveFilter(const std::string& category);
        void ClearFilters();
        std::vector<LogFilter> GetFilters() const;
        bool EnableFilter(const std::string& category, bool enable);
        
        // Buffer management
        void SetBufferSize(size_t size);
        size_t GetBufferSize() const;
        void Flush();
        void ClearBuffer();
        
        // File management
        bool SetLogFilePath(const std::string& filePath);
        std::string GetLogFilePath() const;
        void ForceRotation();
        std::vector<std::string> GetLogFiles() const;
        bool DeleteOldLogs(int daysToKeep);
        
        // Statistics
        LogStatistics GetStatistics() const;
        void ResetStatistics();
        void EnableStatistics(bool enable);
        bool IsStatisticsEnabled() const;
        
        // Callbacks
        void SetLogCallback(std::function<void(const LogEntry&)> callback);
        void SetStatisticsCallback(std::function<void(const LogStatistics&)> callback);
        
        // Advanced features
        bool EnableRemoteLogging(const std::string& endpoint, int port);
        bool DisableRemoteLogging();
        bool IsRemoteLoggingEnabled() const;
        
        bool EnableDatabaseLogging(const std::string& databasePath);
        bool DisableDatabaseLogging();
        bool IsDatabaseLoggingEnabled() const;
        
        bool EnableEventLogging(const std::string& sourceName);
        bool DisableEventLogging();
        bool IsEventLoggingEnabled() const;
        
        // Search and filtering
        std::vector<LogEntry> SearchLogs(const std::string& searchTerm, 
                                       LogLevel minLevel = LogLevel::Trace,
                                       LogLevel maxLevel = LogLevel::Critical,
                                       const std::string& category = "",
                                       uint64_t startTime = 0,
                                       uint64_t endTime = 0) const;
        
        std::vector<LogEntry> GetRecentLogs(int count = 100) const;
        std::vector<LogEntry> GetLogsByLevel(LogLevel level, int maxCount = 1000) const;
        std::vector<LogEntry> GetLogsByCategory(const std::string& category, int maxCount = 1000) const;
        
        // Export and import
        bool ExportLogs(const std::string& filePath, const std::string& format = "json");
        bool ImportLogs(const std::string& filePath, const std::string& format = "json");
        
        // Real-time monitoring
        bool EnableRealTimeMonitoring(bool enable);
        bool IsRealTimeMonitoringEnabled() const;
        void StartRealTimeMonitoring();
        void StopRealTimeMonitoring();
        
        // Debug features
        void EnableDebugMode(bool enable);
        bool IsDebugModeEnabled() const;
        void DumpBuffer() const;
        void TestLogging();
    };
    
    // Convenience macros
    #define LOG_TRACE(msg, ...) CAdvancedLogger::GetInstance()->LogTrace(msg, ##__VA_ARGS__)
    #define LOG_DEBUG(msg, ...) CAdvancedLogger::GetInstance()->LogDebug(msg, ##__VA_ARGS__)
    #define LOG_INFO(msg, ...) CAdvancedLogger::GetInstance()->LogInfo(msg, ##__VA_ARGS__)
    #define LOG_WARNING(msg, ...) CAdvancedLogger::GetInstance()->LogWarning(msg, ##__VA_ARGS__)
    #define LOG_ERROR(msg, ...) CAdvancedLogger::GetInstance()->LogError(msg, ##__VA_ARGS__)
    #define LOG_CRITICAL(msg, ...) CAdvancedLogger::GetInstance()->LogCritical(msg, ##__VA_ARGS__)
    
    #define LOG_PERFORMANCE(op, duration) CAdvancedLogger::GetInstance()->LogPerformance(op, duration)
    #define LOG_MEMORY(used, total) CAdvancedLogger::GetInstance()->LogMemoryUsage(used, total)
    #define LOG_CPU(usage) CAdvancedLogger::GetInstance()->LogCpuUsage(usage)
    #define LOG_NETWORK(endpoint, bytes) CAdvancedLogger::GetInstance()->LogNetworkActivity(endpoint, bytes)
}

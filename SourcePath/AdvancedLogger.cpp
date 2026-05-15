/*
 * Advanced Logger Implementation for Rust Anti-Cheat Evasion System
 * Multiple output formats and log rotation
 * Author: Anonymous
 * Last Modified: 2026
 */

#include "AdvancedLogger.h"
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>

namespace Logging {
    
    // Singleton instance
    static std::unique_ptr<CAdvancedLogger> g_instance;
    static std::mutex g_instanceMutex;
    
    CAdvancedLogger* CAdvancedLogger::GetInstance() {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        if (!g_instance) {
            g_instance = std::make_unique<CAdvancedLogger>();
        }
        return g_instance.get();
    }
    
    CAdvancedLogger::CAdvancedLogger() 
        : m_isInitialized(false), m_bufferSize(1000), m_totalEntries(0),
          m_currentFileSize(0), m_currentFileIndex(0), m_lastStatsUpdate(0) {
        
        // Initialize default configuration
        m_config.minLevel = LogLevel::Info;
        m_config.maxLevel = LogLevel::Critical;
        m_config.outputs = {LogOutput::File, LogOutput::Console};
        m_config.logFilePath = "logs/rust_toolkit.log";
        m_config.logFormat = "[{timestamp}] [{level}] [{category}] [{thread}] {message}";
        m_config.enableTimestamp = true;
        m_config.enableThreadId = true;
        m_config.enableFunction = false;
        m_config.enableFileLine = false;
        m_config.enableRotation = true;
        m_config.maxFileSize = 10 * 1024 * 1024; // 10MB
        m_config.maxFileCount = 10;
        m_config.enableCompression = true;
        m_config.enableNetworkLogging = false;
        m_config.networkEndpoint = "127.0.0.1";
        m_config.networkPort = 514;
        m_config.enableDatabase = false;
        m_config.databasePath = "logs/rust_toolkit.db";
        m_config.enableEventLog = false;
        m_config.eventLogSource = "RustToolkit";
        m_config.enableFiltering = false;
        m_config.enablePerformanceLogging = true;
        m_config.performanceThresholdMs = 100;
        
        // Initialize statistics
        memset(&m_statistics, 0, sizeof(m_statistics));
        
        // Create log directory
        std::filesystem::create_directories("logs");
    }
    
    CAdvancedLogger::~CAdvancedLogger() {
        if (m_isInitialized) {
            Shutdown();
        }
    }
    
    bool CAdvancedLogger::Initialize(const LogConfig& config) {
        std::cout << "Initializing Advanced Logger..." << std::endl;
        
        m_config = config;
        
        // Create log directory
        std::filesystem::create_directories(std::filesystem::path(m_config.logFilePath).parent_path());
        
        // Initialize file logging
        if (std::find(m_config.outputs.begin(), m_config.outputs.end(), LogOutput::File) != m_config.outputs.end()) {
            InitializeFileLogging();
        }
        
        // Initialize network logging
        if (std::find(m_config.outputs.begin(), m_config.outputs.end(), LogOutput::Network) != m_config.outputs.end()) {
            InitializeNetworkLogging();
        }
        
        // Initialize database logging
        if (std::find(m_config.outputs.begin(), m_config.outputs.end(), LogOutput::Database) != m_config.outputs.end()) {
            InitializeDatabaseLogging();
        }
        
        // Initialize event logging
        if (std::find(m_config.outputs.begin(), m_config.outputs.end(), LogOutput::EventLog) != m_config.outputs.end()) {
            InitializeEventLogging();
        }
        
        m_isInitialized = true;
        
        LogInfo("Advanced Logger initialized successfully", "Logging");
        
        std::cout << "[SUCCESS] Advanced Logger initialized" << std::endl;
        std::cout << "Log file: " << m_config.logFilePath << std::endl;
        std::cout << "Min level: " << GetLogLevelString(m_config.minLevel) << std::endl;
        std::cout << "Outputs: " << m_config.outputs.size() << std::endl;
        
        return true;
    }
    
    void CAdvancedLogger::Shutdown() {
        std::cout << "Shutting down Advanced Logger..." << std::endl;
        
        // Flush buffer
        Flush();
        
        // Close file
        if (m_logFile && m_logFile->is_open()) {
            m_logFile->close();
        }
        
        // Cleanup network logger
        if (m_networkLogger) {
            m_networkLogger->Shutdown();
        }
        
        // Cleanup database logger
        if (m_databaseLogger) {
            m_databaseLogger->Shutdown();
        }
        
        m_isInitialized = false;
        std::cout << "[SUCCESS] Advanced Logger shutdown complete" << std::endl;
    }
    
    bool CAdvancedLogger::IsInitialized() const {
        return m_isInitialized;
    }
    
    void CAdvancedLogger::Log(LogLevel level, const std::string& message, 
                          const std::string& category, const std::string& function, 
                          const std::string& file, int line) {
        if (!m_isInitialized) {
            return;
        }
        
        // Check level filter
        if (level < m_config.minLevel || level > m_config.maxLevel) {
            return;
        }
        
        // Create log entry
        LogEntry entry;
        entry.level = level;
        entry.message = message;
        entry.category = category;
        entry.function = function;
        entry.file = file;
        entry.line = line;
        entry.timestamp = GetTickCount64();
        entry.threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
        entry.module = "RustToolkit";
        
        // Check filters
        if (!ShouldLog(entry)) {
            return;
        }
        
        // Add to buffer
        {
            std::lock_guard<std::mutex> lock(m_logMutex);
            m_logBuffer.push_back(entry);
            m_totalEntries++;
            
            // Update statistics
            UpdateStatistics(entry);
            
            // Check buffer size
            if (m_logBuffer.size() >= m_bufferSize) {
                FlushBuffer();
            }
        }
        
        // Trigger callback
        if (m_logCallback) {
            m_logCallback(entry);
        }
    }
    
    void CAdvancedLogger::LogTrace(const std::string& message, const std::string& category) {
        Log(LogLevel::Trace, message, category, "", "", 0);
    }
    
    void CAdvancedLogger::LogDebug(const std::string& message, const std::string& category) {
        Log(LogLevel::Debug, message, category, "", "", 0);
    }
    
    void CAdvancedLogger::LogInfo(const std::string& message, const std::string& category) {
        Log(LogLevel::Info, message, category, "", "", 0);
    }
    
    void CAdvancedLogger::LogWarning(const std::string& message, const std::string& category) {
        Log(LogLevel::Warning, message, category, "", "", 0);
    }
    
    void CAdvancedLogger::LogError(const std::string& message, const std::string& category) {
        Log(LogLevel::Error, message, category, "", "", 0);
    }
    
    void CAdvancedLogger::LogCritical(const std::string& message, const std::string& category) {
        Log(LogLevel::Critical, message, category, "", "", 0);
    }
    
    void CAdvancedLogger::LogPerformance(const std::string& operation, int durationMs, const std::string& category) {
        if (!m_config.enablePerformanceLogging) {
            return;
        }
        
        // Only log if performance threshold is exceeded
        if (durationMs >= m_config.performanceThresholdMs) {
            std::string message = "Performance: " + operation + " took " + std::to_string(durationMs) + "ms";
            Log(LogLevel::Warning, message, category, "", "", 0);
        }
    }
    
    void CAdvancedLogger::LogMemoryUsage(size_t usedMemory, size_t totalMemory) {
        float usagePercent = (static_cast<float>(usedMemory) / static_cast<float>(totalMemory)) * 100.0f;
        std::string message = "Memory Usage: " + std::to_string(usedMemory / 1024 / 1024) + "MB / " + 
                            std::to_string(totalMemory / 1024 / 1024) + "MB (" + 
                            std::to_string(usagePercent) + "%)";
        Log(LogLevel::Info, message, "Memory", "", "", 0);
    }
    
    void CAdvancedLogger::LogCpuUsage(float cpuUsage) {
        std::string message = "CPU Usage: " + std::to_string(cpuUsage) + "%";
        Log(LogLevel::Info, message, "CPU", "", "", 0);
    }
    
    void CAdvancedLogger::LogNetworkActivity(const std::string& endpoint, int bytesTransferred) {
        std::string message = "Network: " + endpoint + " transferred " + std::to_string(bytesTransferred) + " bytes";
        Log(LogLevel::Info, message, "Network", "", "", 0);
    }
    
    void CAdvancedLogger::Flush() {
        std::lock_guard<std::mutex> lock(m_logMutex);
        FlushBuffer();
    }
    
    void CAdvancedLogger::FlushBuffer() {
        for (const auto& entry : m_logBuffer) {
            // Write to file
            if (std::find(m_config.outputs.begin(), m_config.outputs.end(), LogOutput::File) != m_config.outputs.end()) {
                WriteToFile(entry);
            }
            
            // Write to console
            if (std::find(m_config.outputs.begin(), m_config.outputs.end(), LogOutput::Console) != m_config.outputs.end()) {
                WriteToConsole(entry);
            }
            
            // Write to network
            if (std::find(m_config.outputs.begin(), m_config.outputs.end(), LogOutput::Network) != m_config.outputs.end()) {
                WriteToNetwork(entry);
            }
            
            // Write to database
            if (std::find(m_config.outputs.begin(), m_config.outputs.end(), LogOutput::Database) != m_config.outputs.end()) {
                WriteToDatabase(entry);
            }
            
            // Write to event log
            if (std::find(m_config.outputs.begin(), m_config.outputs.end(), LogOutput::EventLog) != m_config.outputs.end()) {
                WriteToEventLog(entry);
            }
        }
        
        m_logBuffer.clear();
    }
    
    void CAdvancedLogger::WriteToFile(const LogEntry& entry) {
        if (!m_logFile || !m_logFile->is_open()) {
            InitializeFileLogging();
        }
        
        if (m_logFile && m_logFile->is_open()) {
            std::string formatted = FormatLogEntry(entry);
            *m_logFile << formatted << std::endl;
            m_logFile->flush();
            
            // Update file size
            m_currentFileSize += formatted.length() + 1;
            
            // Check rotation
            if (m_config.enableRotation && m_currentFileSize >= m_config.maxFileSize) {
                RotateLogFile();
            }
        }
    }
    
    void CAdvancedLogger::WriteToConsole(const LogEntry& entry) {
        std::string formatted = FormatLogEntry(entry);
        
        // Color code for different levels
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        
        WORD color = consoleInfo.wAttributes;
        
        switch (entry.level) {
            case LogLevel::Trace:
            case LogLevel::Debug:
                color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                break;
            case LogLevel::Info:
                color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                break;
            case LogLevel::Warning:
                color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                break;
            case LogLevel::Error:
            case LogLevel::Critical:
                color = FOREGROUND_RED | FOREGROUND_INTENSITY;
                break;
        }
        
        SetConsoleTextAttribute(hConsole, color);
        std::cout << formatted << std::endl;
        SetConsoleTextAttribute(hConsole, consoleInfo.wAttributes);
    }
    
    void CAdvancedLogger::WriteToNetwork(const LogEntry& entry) {
        if (m_networkLogger) {
            m_networkLogger->WriteLog(entry);
        }
    }
    
    void CAdvancedLogger::WriteToDatabase(const LogEntry& entry) {
        if (m_databaseLogger) {
            m_databaseLogger->WriteLog(entry);
        }
    }
    
    void CAdvancedLogger::WriteToEventLog(const LogEntry& entry) {
        // This would write to Windows Event Log
        // For now, we'll just write to console
        std::string message = "Event Log: " + FormatLogEntry(entry);
        std::cout << message << std::endl;
    }
    
    void CAdvancedLogger::RotateLogFile() {
        if (!m_logFile || !m_logFile->is_open()) {
            return;
        }
        
        // Close current file
        m_logFile->close();
        
        // Create backup filename with timestamp
        std::filesystem::path logPath(m_config.logFilePath);
        std::string backupPath = logPath.parent_path().string() + "/" + 
                               logPath.stem().string() + "_" + 
                               GetCurrentTimestamp() + 
                               logPath.extension().string();
        
        // Rename current file
        std::filesystem::rename(m_config.logFilePath, backupPath);
        
        // Compress old logs if enabled
        if (m_config.enableCompression) {
            CompressOldLogs();
        }
        
        // Clean up old logs
        CleanupOldLogs();
        
        // Create new log file
        InitializeFileLogging();
        
        std::cout << "[INFO] Log file rotated: " << backupPath << std::endl;
    }
    
    void CAdvancedLogger::CompressOldLogs() {
        // This would compress old log files
        // For now, we'll just log the action
        std::cout << "[INFO] Compressing old log files..." << std::endl;
    }
    
    void CAdvancedLogger::CleanupOldLogs() {
        std::filesystem::path logDir = std::filesystem::path(m_config.logFilePath).parent_path();
        std::vector<std::filesystem::path> logFiles;
        
        // Find all log files
        for (const auto& entry : std::filesystem::directory_iterator(logDir)) {
            if (entry.path().extension() == ".log" || 
                entry.path().filename().string().find("rust_toolkit") == 0) {
                logFiles.push_back(entry.path());
            }
        }
        
        // Sort by creation time
        std::sort(logFiles.begin(), logFiles.end(), 
            [](const std::filesystem::path& a, const std::filesystem::path& b) {
                return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
            });
        
        // Keep only the most recent files
        size_t filesToKeep = m_config.maxFileCount;
        if (logFiles.size() > filesToKeep) {
            for (size_t i = filesToKeep; i < logFiles.size(); i++) {
                try {
                    std::filesystem::remove(logFiles[i]);
                    std::cout << "[INFO] Removed old log file: " << logFiles[i].filename().string() << std::endl;
                } catch (const std::exception& e) {
                    std::cout << "[ERROR] Failed to remove old log file: " << e.what() << std::endl;
                }
            }
        }
    }
    
    void CAdvancedLogger::UpdateStatistics(const LogEntry& entry) {
        m_statistics.totalEntries++;
        m_statistics.entriesByLevel[static_cast<int>(entry.level)]++;
        
        // Update category statistics
        size_t categoryHash = std::hash<std::string>{}(entry.category) % 100;
        m_statistics.entriesByCategory[categoryHash]++;
        
        // Update hourly statistics
        auto now = std::chrono::system_clock::now();
        auto hour = std::chrono::duration_cast<std::chrono::hours>(now.time_since_epoch()).count() % 24;
        m_statistics.entriesPerHour[hour]++;
        
        // Update most active category
        if (m_statistics.entriesByCategory[categoryHash] > 
            m_statistics.entriesByCategory[std::hash<std::string>{}(m_statistics.mostActiveCategory) % 100]) {
            m_statistics.mostActiveCategory = entry.category;
        }
        
        // Update most active level
        if (m_statistics.entriesByLevel[static_cast<int>(entry.level)] > 
            m_statistics.entriesByLevel[static_cast<int>(GetLogLevelFromString(m_statistics.mostActiveLevel))]) {
            m_statistics.mostActiveLevel = GetLogLevelString(entry.level);
        }
        
        // Update file size
        m_statistics.totalFileSize += m_currentFileSize;
        
        // Update peak entries per second (simplified calculation)
        auto currentTime = GetTickCount64();
        if (currentTime - m_lastStatsUpdate > 1000) { // 1 second
            uint64_t entriesInSecond = m_statistics.totalEntries - m_lastStatsUpdate;
            if (entriesInSecond > m_statistics.peakEntriesPerSecond) {
                m_statistics.peakEntriesPerSecond = entriesInSecond;
            }
            m_statistics.averageEntriesPerSecond = m_statistics.totalEntries / (currentTime / 1000);
            m_lastStatsUpdate = currentTime;
        }
    }
    
    std::string CAdvancedLogger::FormatLogEntry(const LogEntry& entry) const {
        std::string formatted = m_config.logFormat;
        
        // Replace placeholders
        if (m_config.enableTimestamp) {
            size_t pos = formatted.find("{timestamp}");
            if (pos != std::string::npos) {
                formatted.replace(pos, 11, GetCurrentTimestamp());
            }
        }
        
        size_t pos = formatted.find("{level}");
        if (pos != std::string::npos) {
            formatted.replace(pos, 7, GetLogLevelString(entry.level));
        }
        
        pos = formatted.find("{category}");
        if (pos != std::string::npos) {
            formatted.replace(pos, 10, entry.category);
        }
        
        if (m_config.enableThreadId) {
            pos = formatted.find("{thread}");
            if (pos != std::string::npos) {
                formatted.replace(pos, 8, std::to_string(entry.threadId));
            }
        }
        
        if (m_config.enableFunction) {
            pos = formatted.find("{function}");
            if (pos != std::string::npos) {
                formatted.replace(pos, 10, entry.function);
            }
        }
        
        if (m_config.enableFileLine) {
            pos = formatted.find("{file}");
            if (pos != std::string::npos) {
                formatted.replace(pos, 6, entry.file);
            }
            
            pos = formatted.find("{line}");
            if (pos != std::string::npos) {
                formatted.replace(pos, 6, std::to_string(entry.line));
            }
        }
        
        pos = formatted.find("{message}");
        if (pos != std::string::npos) {
            formatted.replace(pos, 9, entry.message);
        }
        
        return formatted;
    }
    
    std::string CAdvancedLogger::GetLogLevelString(LogLevel level) const {
        switch (level) {
            case LogLevel::Trace: return "TRACE";
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info: return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Critical: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
    LogLevel CAdvancedLogger::GetLogLevelFromString(const std::string& level) const {
        if (level == "TRACE") return LogLevel::Trace;
        if (level == "DEBUG") return LogLevel::Debug;
        if (level == "INFO") return LogLevel::Info;
        if (level == "WARN") return LogLevel::Warning;
        if (level == "ERROR") return LogLevel::Error;
        if (level == "CRITICAL") return LogLevel::Critical;
        return LogLevel::Info;
    }
    
    std::string CAdvancedLogger::GetCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        
        return ss.str();
    }
    
    bool CAdvancedLogger::ShouldLog(const LogEntry& entry) const {
        if (!m_config.enableFiltering) {
            return true;
        }
        
        // Check category filters
        for (const auto& filter : m_filters) {
            if (!filter.enabled) {
                continue;
            }
            
            if (filter.category == entry.category || filter.category == "*") {
                if (entry.level < filter.minLevel || entry.level > filter.maxLevel) {
                    return false;
                }
                
                if (!filter.pattern.empty() && entry.message.find(filter.pattern) == std::string::npos) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    void CAdvancedLogger::InitializeFileLogging() {
        if (!m_logFile) {
            m_logFile = std::make_unique<std::ofstream>();
        }
        
        if (!m_logFile->is_open()) {
            m_logFile->open(m_config.logFilePath, std::ios::app);
            
            if (m_logFile->is_open()) {
                // Write header
                *m_logFile << "# Rust Anti-Cheat Evasion System Log" << std::endl;
                *m_logFile << "# Started: " << GetCurrentTimestamp() << std::endl;
                *m_logFile << "# Min Level: " << GetLogLevelString(m_config.minLevel) << std::endl;
                *m_logFile << std::endl;
                
                // Get current file size
                m_logFile->seekp(0, std::ios::end);
                m_currentFileSize = m_logFile->tellp();
                m_logFile->seekp(0, std::ios::beg);
            }
        }
    }
    
    void CAdvancedLogger::InitializeNetworkLogging() {
        // This would initialize network logger
        // For now, we'll just log the initialization
        std::cout << "[INFO] Network logging initialized" << std::endl;
    }
    
    void CAdvancedLogger::InitializeDatabaseLogging() {
        // This would initialize database logger
        // For now, we'll just log the initialization
        std::cout << "[INFO] Database logging initialized" << std::endl;
    }
    
    void CAdvancedLogger::InitializeEventLogging() {
        // This would initialize Windows Event Log
        // For now, we'll just log the initialization
        std::cout << "[INFO] Event logging initialized" << std::endl;
    }
    
    LogStatistics CAdvancedLogger::GetStatistics() const {
        return m_statistics;
    }
    
    void CAdvancedLogger::ResetStatistics() {
        memset(&m_statistics, 0, sizeof(m_statistics));
        m_totalEntries = 0;
        std::cout << "[INFO] Logging statistics reset" << std::endl;
    }
    
    bool CAdvancedLogger::LoadConfiguration(const std::string& configFile) {
        std::cout << "Loading logging configuration from: " << configFile << std::endl;
        
        std::ifstream file(configFile);
        if (!file.is_open()) {
            std::cout << "[WARNING] Logging config file not found, using defaults" << std::endl;
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                // Trim whitespace
                key.erase(0, key.find_first_not_of(" \t\n\r"));
                key.erase(key.find_last_not_of(" \t\n\r") + 1);
                
                value.erase(0, value.find_first_not_of(" \t\n\r"));
                value.erase(value.find_last_not_of(" \t\n\r") + 1);
                
                // Apply configuration
                if (key == "minLevel") {
                    m_config.minLevel = GetLogLevelFromString(value);
                } else if (key == "maxLevel") {
                    m_config.maxLevel = GetLogLevelFromString(value);
                } else if (key == "logFilePath") {
                    m_config.logFilePath = value;
                } else if (key == "enableTimestamp") {
                    m_config.enableTimestamp = (value == "true");
                } else if (key == "enableThreadId") {
                    m_config.enableThreadId = (value == "true");
                } else if (key == "enableRotation") {
                    m_config.enableRotation = (value == "true");
                } else if (key == "maxFileSize") {
                    m_config.maxFileSize = std::stoull(value);
                } else if (key == "maxFileCount") {
                    m_config.maxFileCount = std::stoi(value);
                } else if (key == "enableCompression") {
                    m_config.enableCompression = (value == "true");
                } else if (key == "enablePerformanceLogging") {
                    m_config.enablePerformanceLogging = (value == "true");
                } else if (key == "performanceThresholdMs") {
                    m_config.performanceThresholdMs = std::stoi(value);
                }
            }
        }
        
        file.close();
        std::cout << "[SUCCESS] Logging configuration loaded" << std::endl;
        return true;
    }
    
    bool CAdvancedLogger::SaveConfiguration(const std::string& configFile) {
        std::cout << "Saving logging configuration to: " << configFile << std::endl;
        
        std::ofstream file(configFile);
        if (!file.is_open()) {
            std::cout << "[ERROR] Failed to save logging configuration" << std::endl;
            return false;
        }
        
        file << "# Rust Anti-Cheat Evasion System Logging Configuration" << std::endl;
        file << "# Generated: " << GetCurrentTimestamp() << std::endl;
        file << std::endl;
        
        file << "minLevel=" << GetLogLevelString(m_config.minLevel) << std::endl;
        file << "maxLevel=" << GetLogLevelString(m_config.maxLevel) << std::endl;
        file << "logFilePath=" << m_config.logFilePath << std::endl;
        file << "enableTimestamp=" << (m_config.enableTimestamp ? "true" : "false") << std::endl;
        file << "enableThreadId=" << (m_config.enableThreadId ? "true" : "false") << std::endl;
        file << "enableRotation=" << (m_config.enableRotation ? "true" : "false") << std::endl;
        file << "maxFileSize=" << m_config.maxFileSize << std::endl;
        file << "maxFileCount=" << m_config.maxFileCount << std::endl;
        file << "enableCompression=" << (m_config.enableCompression ? "true" : "false") << std::endl;
        file << "enablePerformanceLogging=" << (m_config.enablePerformanceLogging ? "true" : "false") << std::endl;
        file << "performanceThresholdMs=" << m_config.performanceThresholdMs << std::endl;
        
        file.close();
        std::cout << "[SUCCESS] Logging configuration saved" << std::endl;
        return true;
    }
    
    std::vector<LogEntry> CAdvancedLogger::GetRecentLogs(int count) const {
        std::lock_guard<std::mutex> lock(m_logMutex);
        
        std::vector<LogEntry> recentLogs;
        int startIdx = std::max(0, static_cast<int>(m_logBuffer.size()) - count);
        
        for (int i = startIdx; i < m_logBuffer.size(); i++) {
            recentLogs.push_back(m_logBuffer[i]);
        }
        
        return recentLogs;
    }
    
    void CAdvancedLogger::SetLogCallback(std::function<void(const LogEntry&)> callback) {
        m_logCallback = callback;
    }
    
    void CAdvancedLogger::SetStatisticsCallback(std::function<void(const LogStatistics&)> callback) {
        m_statsCallback = callback;
    }
}

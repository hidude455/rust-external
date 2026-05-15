/*
 * Statistics and Analytics Manager for Rust Anti-Cheat Evasion System
 * Usage analytics and performance monitoring
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <memory>
#include <functional>

namespace Statistics {
    
    // Statistics categories
    enum class StatCategory {
        Usage,
        Performance,
        Detection,
        Network,
        Memory,
        CPU,
        Errors,
        Events,
        Custom
    };
    
    // Statistics types
    enum class StatType {
        Counter,
        Timer,
        Gauge,
        Histogram,
        Average,
        Percentage
    };
    
    // Statistics entry
    struct StatEntry {
        std::string name;
        StatCategory category;
        StatType type;
        double value;
        std::string unit;
        uint64_t timestamp;
        std::map<std::string, std::string> metadata;
        std::string description;
    };
    
    // Performance metrics
    struct PerformanceMetrics {
        double averageFPS;
        double minFPS;
        double maxFPS;
        double frameTime;
        double cpuUsage;
        size_t memoryUsage;
        size_t peakMemoryUsage;
        uint64_t totalFrames;
        uint64_t droppedFrames;
        double renderTime;
        double updateTime;
        double networkLatency;
        uint64_t packetsPerSecond;
    };
    
    // Usage statistics
    struct UsageStatistics {
        uint64_t totalRuntime;
        uint64_t sessionCount;
        uint64_t lastSessionTime;
        std::map<std::string, uint64_t> featureUsage;
        std::map<std::string, uint64_t> featureToggleCount;
        std::map<std::string, double> featureUsageTime;
        std::vector<std::string> mostUsedFeatures;
        std::vector<std::string> leastUsedFeatures;
    };
    
    // Detection statistics
    struct DetectionStatistics {
        uint64_t detectionAttempts;
        uint64_t successfulEvasions;
        uint64_t failedEvasions;
        std::map<std::string, uint64_t> detectionMethods;
        std::map<std::string, uint64_t> evasionTechniques;
        std::vector<std::string> recentDetections;
        double stealthScore;
        uint64_t lastDetectionTime;
        std::string lastDetectionMethod;
    };
    
    // Analytics configuration
    struct AnalyticsConfig {
        bool enableAnalytics;
        bool enablePerformanceMonitoring;
        bool enableUsageTracking;
        bool enableDetectionTracking;
        bool enableNetworkMonitoring;
        bool enableMemoryMonitoring;
        bool enableCPUMonitoring;
        int sampleIntervalMs;
        int historyRetentionDays;
        bool enableRealTimeUpdates;
        bool enableDataExport;
        std::string exportFormat;
        std::string exportPath;
        bool enableRemoteAnalytics;
        std::string remoteEndpoint;
        int remotePort;
        bool enableCompression;
        std::vector<std::string> trackedFeatures;
        std::vector<std::string> excludedFeatures;
    };
    
    // Analytics report
    struct AnalyticsReport {
        std::string reportType;
        std::string timeRange;
        uint64_t generatedTime;
        PerformanceMetrics performance;
        UsageStatistics usage;
        DetectionStatistics detection;
        std::map<std::string, double> customMetrics;
        std::vector<std::string> insights;
        std::vector<std::string> recommendations;
    };
    
    class CStatisticsManager {
    private:
        // Configuration
        AnalyticsConfig m_config;
        
        // State
        bool m_isInitialized;
        bool m_isMonitoring;
        uint64_t m_startTime;
        
        // Statistics storage
        std::map<std::string, std::vector<StatEntry>> m_statistics;
        std::vector<PerformanceMetrics> m_performanceHistory;
        std::vector<UsageStatistics> m_usageHistory;
        std::vector<DetectionStatistics> m_detectionHistory;
        
        // Current metrics
        PerformanceMetrics m_currentPerformance;
        UsageStatistics m_currentUsage;
        DetectionStatistics m_currentDetection;
        
        // Monitoring
        std::unique_ptr<std::thread> m_monitoringThread;
        std::atomic<bool> m_shouldStopMonitoring;
        
        // Callbacks
        std::function<void(const PerformanceMetrics&)> m_performanceCallback;
        std::function<void(const UsageStatistics&)> m_usageCallback;
        std::function<void(const DetectionStatistics&)> m_detectionCallback;
        std::function<void(const AnalyticsReport&)> m_reportCallback;
        
        // Private methods
        void StartMonitoring();
        void StopMonitoring();
        void MonitoringLoop();
        void CollectPerformanceMetrics();
        void CollectUsageStatistics();
        void CollectDetectionStatistics();
        void UpdatePerformanceMetrics();
        void UpdateUsageStatistics();
        void UpdateDetectionStatistics();
        void CleanupOldData();
        void GenerateInsights();
        void SaveStatistics();
        void LoadStatistics();
        void ExportStatistics();
        void SendToRemoteAnalytics();
        std::string GetCurrentTimestamp() const;
        void AddStatistic(const std::string& name, StatCategory category, StatType type, 
                          double value, const std::string& unit = "");
        
    public:
        CStatisticsManager();
        ~CStatisticsManager();
        
        // Initialization
        bool Initialize(const AnalyticsConfig& config);
        void Shutdown();
        bool IsInitialized() const;
        
        // Configuration
        bool LoadConfiguration(const std::string& configFile = "rust_analytics.ini");
        bool SaveConfiguration(const std::string& configFile = "rust_analytics.ini");
        AnalyticsConfig GetConfig() const;
        void SetConfig(const AnalyticsConfig& config);
        
        // Statistics collection
        void RecordUsage(const std::string& feature, bool enabled);
        void RecordPerformance(const std::string& operation, double durationMs);
        void RecordDetection(const std::string& method, bool evaded);
        void RecordNetworkActivity(const std::string& endpoint, int bytes, double latency);
        void RecordMemoryUsage(size_t used, size_t total);
        void RecordCPUUsage(double usage);
        void RecordError(const std::string& error, const std::string& context);
        void RecordEvent(const std::string& eventType, const std::map<std::string, std::string>& data);
        
        // Custom metrics
        void RecordCustomMetric(const std::string& name, double value, const std::string& unit = "");
        void IncrementCounter(const std::string& name, double increment = 1.0);
        void SetGauge(const std::string& name, double value);
        void RecordTimer(const std::string& name, double durationMs);
        void RecordHistogram(const std::string& name, double value);
        
        // Statistics retrieval
        PerformanceMetrics GetCurrentPerformanceMetrics() const;
        UsageStatistics GetCurrentUsageStatistics() const;
        DetectionStatistics GetCurrentDetectionStatistics() const;
        std::vector<StatEntry> GetStatistics(const std::string& name = "", 
                                               StatCategory category = StatCategory::Custom,
                                               uint64_t startTime = 0,
                                               uint64_t endTime = 0) const;
        
        // History and trends
        std::vector<PerformanceMetrics> GetPerformanceHistory(int days = 7) const;
        std::vector<UsageStatistics> GetUsageHistory(int days = 30) const;
        std::vector<DetectionStatistics> GetDetectionHistory(int days = 30) const;
        std::map<std::string, double> GetTrends(const std::string& metric, int days = 30) const;
        
        // Analytics and reporting
        AnalyticsReport GenerateReport(const std::string& reportType = "summary", 
                                   const std::string& timeRange = "last_7_days");
        std::vector<std::string> GetInsights() const;
        std::vector<std::string> GetRecommendations() const;
        double GetStealthScore() const;
        
        // Export and import
        bool ExportStatistics(const std::string& filePath, const std::string& format = "json");
        bool ImportStatistics(const std::string& filePath, const std::string& format = "json");
        bool ExportToCSV(const std::string& filePath);
        bool ExportToJSON(const std::string& filePath);
        bool ExportToXML(const std::string& filePath);
        
        // Real-time monitoring
        bool EnableRealTimeMonitoring(bool enable);
        bool IsRealTimeMonitoringEnabled() const;
        void StartRealTimeUpdates();
        void StopRealTimeUpdates();
        
        // Callbacks
        void SetPerformanceCallback(std::function<void(const PerformanceMetrics&)> callback);
        void SetUsageCallback(std::function<void(const UsageStatistics&)> callback);
        void SetDetectionCallback(std::function<void(const DetectionStatistics&)> callback);
        void SetReportCallback(std::function<void(const AnalyticsReport&)> callback);
        
        // Advanced features
        bool EnableRemoteAnalytics(const std::string& endpoint, int port);
        bool DisableRemoteAnalytics();
        bool IsRemoteAnalyticsEnabled() const;
        
        bool EnableDataCompression(bool enable);
        bool IsDataCompressionEnabled() const;
        
        void ResetStatistics();
        void ResetPerformanceMetrics();
        void ResetUsageStatistics();
        void ResetDetectionStatistics();
        
        // Debugging
        void EnableDebugMode(bool enable);
        bool IsDebugModeEnabled() const;
        void DumpStatistics() const;
        void TestStatisticsCollection();
        
        // Performance optimization
        void OptimizeDataCollection();
        void SetSampleInterval(int intervalMs);
        int GetSampleInterval() const;
        void SetHistoryRetention(int days);
        int GetHistoryRetention() const;
        
        // Privacy and security
        void AnonymizeData(bool enable);
        bool IsDataAnonymized() const;
        void EnableDataEncryption(bool enable);
        bool IsDataEncryptionEnabled() const;
        void ClearSensitiveData();
    };
}

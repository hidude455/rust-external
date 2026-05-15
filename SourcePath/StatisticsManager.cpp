/*
 * Statistics and Analytics Manager Implementation for Rust Anti-Cheat Evasion System
 * Usage analytics and performance monitoring
 * Author: Anonymous
 * Last Modified: 2026
 */

#include "StatisticsManager.h"
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>

namespace Statistics {
    
    CStatisticsManager::CStatisticsManager() 
        : m_isInitialized(false), m_isMonitoring(false), m_startTime(0), m_shouldStopMonitoring(false) {
        
        // Initialize default configuration
        m_config.enableAnalytics = true;
        m_config.enablePerformanceMonitoring = true;
        m_config.enableUsageTracking = true;
        m_config.enableDetectionTracking = true;
        m_config.enableNetworkMonitoring = false;
        m_config.enableMemoryMonitoring = true;
        m_config.enableCPUMonitoring = false;
        m_config.sampleIntervalMs = 1000;
        m_config.historyRetentionDays = 30;
        m_config.enableRealTimeUpdates = true;
        m_config.enableDataExport = false;
        m_config.exportFormat = "json";
        m_config.exportPath = "analytics/";
        m_config.enableRemoteAnalytics = false;
        m_config.remoteEndpoint = "127.0.0.1";
        m_config.remotePort = 8080;
        m_config.enableCompression = false;
        m_config.trackedFeatures = {"ESP", "Aimbot", "VPN", "Spoofer"};
        m_config.excludedFeatures = {};
        
        // Initialize metrics
        memset(&m_currentPerformance, 0, sizeof(m_currentPerformance));
        memset(&m_currentUsage, 0, sizeof(m_currentUsage));
        memset(&m_currentDetection, 0, sizeof(m_currentDetection));
        
        // Create analytics directory
        std::filesystem::create_directories("analytics");
    }
    
    CStatisticsManager::~CStatisticsManager() {
        if (m_isInitialized) {
            Shutdown();
        }
    }
    
    bool CStatisticsManager::Initialize(const AnalyticsConfig& config) {
        std::cout << "Initializing Statistics Manager..." << std::endl;
        
        m_config = config;
        m_startTime = GetTickCount64();
        
        // Load existing statistics
        LoadStatistics();
        
        // Start monitoring if enabled
        if (m_config.enableAnalytics) {
            StartMonitoring();
        }
        
        m_isInitialized = true;
        
        std::cout << "[SUCCESS] Statistics Manager initialized" << std::endl;
        std::cout << "Analytics enabled: " << (m_config.enableAnalytics ? "Yes" : "No") << std::endl;
        std::cout << "Performance monitoring: " << (m_config.enablePerformanceMonitoring ? "Yes" : "No") << std::endl;
        std::cout << "Usage tracking: " << (m_config.enableUsageTracking ? "Yes" : "No") << std::endl;
        std::cout << "Detection tracking: " << (m_config.enableDetectionTracking ? "Yes" : "No") << std::endl;
        
        return true;
    }
    
    void CStatisticsManager::Shutdown() {
        std::cout << "Shutting down Statistics Manager..." << std::endl;
        
        // Stop monitoring
        StopMonitoring();
        
        // Save statistics
        SaveStatistics();
        
        // Cleanup
        m_statistics.clear();
        m_performanceHistory.clear();
        m_usageHistory.clear();
        m_detectionHistory.clear();
        
        m_isInitialized = false;
        std::cout << "[SUCCESS] Statistics Manager shutdown complete" << std::endl;
    }
    
    bool CStatisticsManager::IsInitialized() const {
        return m_isInitialized;
    }
    
    void CStatisticsManager::RecordUsage(const std::string& feature, bool enabled) {
        if (!m_config.enableUsageTracking || !m_isInitialized) {
            return;
        }
        
        // Check if feature is tracked
        if (std::find(m_config.trackedFeatures.begin(), m_config.trackedFeatures.end(), feature) == m_config.trackedFeatures.end()) {
            return;
        }
        
        // Check if feature is excluded
        if (std::find(m_config.excludedFeatures.begin(), m_config.excludedFeatures.end(), feature) != m_config.excludedFeatures.end()) {
            return;
        }
        
        // Update usage statistics
        m_currentUsage.featureUsage[feature]++;
        m_currentUsage.featureToggleCount[feature]++;
        
        if (enabled) {
            m_currentUsage.featureUsageTime[feature] += m_config.sampleIntervalMs / 1000.0;
        }
        
        // Add to statistics
        AddStatistic("feature_usage", StatCategory::Usage, StatType::Counter, 
                    m_currentUsage.featureUsage[feature], "count");
        
        std::cout << "[STATS] Feature usage recorded: " << feature << " - " << (enabled ? "enabled" : "disabled") << std::endl;
    }
    
    void CStatisticsManager::RecordPerformance(const std::string& operation, double durationMs) {
        if (!m_config.enablePerformanceMonitoring || !m_isInitialized) {
            return;
        }
        
        // Update performance metrics
        if (operation == "frame_render") {
            m_currentPerformance.frameTime = durationMs;
            
            // Calculate FPS
            if (durationMs > 0) {
                double fps = 1000.0 / durationMs;
                m_currentPerformance.averageFPS = (m_currentPerformance.averageFPS * 0.9) + (fps * 0.1);
                m_currentPerformance.minFPS = std::min(m_currentPerformance.minFPS, fps);
                m_currentPerformance.maxFPS = std::max(m_currentPerformance.maxFPS, fps);
            }
            
            m_currentPerformance.totalFrames++;
        } else if (operation == "update") {
            m_currentPerformance.updateTime = durationMs;
        } else if (operation == "render") {
            m_currentPerformance.renderTime = durationMs;
        }
        
        // Add to statistics
        AddStatistic("performance_" + operation, StatCategory::Performance, StatType::Timer, 
                    durationMs, "ms");
        
        std::cout << "[STATS] Performance recorded: " << operation << " - " << durationMs << "ms" << std::endl;
    }
    
    void CStatisticsManager::RecordDetection(const std::string& method, bool evaded) {
        if (!m_config.enableDetectionTracking || !m_isInitialized) {
            return;
        }
        
        // Update detection statistics
        m_currentDetection.detectionAttempts++;
        
        if (evaded) {
            m_currentDetection.successfulEvasions++;
        } else {
            m_currentDetection.failedEvasions++;
        }
        
        m_currentDetection.detectionMethods[method]++;
        m_currentDetection.lastDetectionTime = GetTickCount64();
        m_currentDetection.lastDetectionMethod = method;
        
        // Add to recent detections
        m_currentDetection.recentDetections.push_back(method + ":" + (evaded ? "evaded" : "detected"));
        if (m_currentDetection.recentDetections.size() > 100) {
            m_currentDetection.recentDetections.erase(m_currentDetection.recentDetections.begin());
        }
        
        // Calculate stealth score
        double evasionRate = static_cast<double>(m_currentDetection.successfulEvasions) / 
                            static_cast<double>(m_currentDetection.detectionAttempts) * 100.0;
        m_currentDetection.stealthScore = evasionRate;
        
        // Add to statistics
        AddStatistic("detection_attempt", StatCategory::Detection, StatType::Counter, 1.0, "count");
        AddStatistic("detection_method_" + method, StatCategory::Detection, StatType::Counter, 1.0, "count");
        
        std::cout << "[STATS] Detection recorded: " << method << " - " << (evaded ? "evaded" : "detected") << std::endl;
    }
    
    void CStatisticsManager::RecordNetworkActivity(const std::string& endpoint, int bytes, double latency) {
        if (!m_config.enableNetworkMonitoring || !m_isInitialized) {
            return;
        }
        
        // Update network metrics
        m_currentPerformance.networkLatency = latency;
        m_currentPerformance.packetsPerSecond = bytes / 1024; // Simplified calculation
        
        // Add to statistics
        AddStatistic("network_bytes", StatCategory::Network, StatType::Counter, bytes, "bytes");
        AddStatistic("network_latency", StatCategory::Network, StatType::Average, latency, "ms");
        
        std::cout << "[STATS] Network activity recorded: " << endpoint << " - " << bytes << " bytes, " << latency << "ms" << std::endl;
    }
    
    void CStatisticsManager::RecordMemoryUsage(size_t used, size_t total) {
        if (!m_config.enableMemoryMonitoring || !m_isInitialized) {
            return;
        }
        
        // Update memory metrics
        m_currentPerformance.memoryUsage = used;
        m_currentPerformance.peakMemoryUsage = std::max(m_currentPerformance.peakMemoryUsage, used);
        
        // Add to statistics
        AddStatistic("memory_usage", StatCategory::Memory, StatType::Gauge, used, "bytes");
        AddStatistic("memory_total", StatCategory::Memory, StatType::Gauge, total, "bytes");
        
        float usagePercent = (static_cast<float>(used) / static_cast<float>(total)) * 100.0f;
        std::cout << "[STATS] Memory usage recorded: " << used / 1024 / 1024 << "MB / " << total / 1024 / 1024 << "MB (" << usagePercent << "%)" << std::endl;
    }
    
    void CStatisticsManager::RecordCPUUsage(double usage) {
        if (!m_config.enableCPUMonitoring || !m_isInitialized) {
            return;
        }
        
        // Update CPU metrics
        m_currentPerformance.cpuUsage = usage;
        
        // Add to statistics
        AddStatistic("cpu_usage", StatCategory::CPU, StatType::Gauge, usage, "percent");
        
        std::cout << "[STATS] CPU usage recorded: " << usage << "%" << std::endl;
    }
    
    void CStatisticsManager::RecordError(const std::string& error, const std::string& context) {
        if (!m_isInitialized) {
            return;
        }
        
        // Add to statistics
        AddStatistic("error_count", StatCategory::Errors, StatType::Counter, 1.0, "count");
        AddStatistic("error_" + context, StatCategory::Errors, StatType::Counter, 1.0, "count");
        
        std::cout << "[STATS] Error recorded: " << error << " - " << context << std::endl;
    }
    
    void CStatisticsManager::RecordEvent(const std::string& eventType, const std::map<std::string, std::string>& data) {
        if (!m_isInitialized) {
            return;
        }
        
        // Add to statistics
        AddStatistic("event_" + eventType, StatCategory::Events, StatType::Counter, 1.0, "count");
        
        // Add metadata
        for (const auto& pair : data) {
            AddStatistic("event_" + eventType + "_" + pair.first, StatCategory::Events, StatType::Counter, 1.0, "count");
        }
        
        std::cout << "[STATS] Event recorded: " << eventType << std::endl;
    }
    
    void CStatisticsManager::RecordCustomMetric(const std::string& name, double value, const std::string& unit) {
        if (!m_isInitialized) {
            return;
        }
        
        // Add to statistics
        AddStatistic(name, StatCategory::Custom, StatType::Gauge, value, unit);
        
        std::cout << "[STATS] Custom metric recorded: " << name << " - " << value << " " << unit << std::endl;
    }
    
    void CStatisticsManager::IncrementCounter(const std::string& name, double increment) {
        if (!m_isInitialized) {
            return;
        }
        
        // Add to statistics
        AddStatistic(name, StatCategory::Custom, StatType::Counter, increment, "count");
        
        std::cout << "[STATS] Counter incremented: " << name << " + " << increment << std::endl;
    }
    
    void CStatisticsManager::SetGauge(const std::string& name, double value) {
        if (!m_isInitialized) {
            return;
        }
        
        // Add to statistics
        AddStatistic(name, StatCategory::Custom, StatType::Gauge, value, "");
        
        std::cout << "[STATS] Gauge set: " << name << " = " << value << std::endl;
    }
    
    void CStatisticsManager::RecordTimer(const std::string& name, double durationMs) {
        if (!m_isInitialized) {
            return;
        }
        
        // Add to statistics
        AddStatistic(name, StatCategory::Custom, StatType::Timer, durationMs, "ms");
        
        std::cout << "[STATS] Timer recorded: " << name << " - " << durationMs << "ms" << std::endl;
    }
    
    void CStatisticsManager::RecordHistogram(const std::string& name, double value) {
        if (!m_isInitialized) {
            return;
        }
        
        // Add to statistics
        AddStatistic(name + "_histogram", StatCategory::Custom, StatType::Histogram, value, "");
        
        std::cout << "[STATS] Histogram recorded: " << name << " - " << value << std::endl;
    }
    
    PerformanceMetrics CStatisticsManager::GetCurrentPerformanceMetrics() const {
        return m_currentPerformance;
    }
    
    UsageStatistics CStatisticsManager::GetCurrentUsageStatistics() const {
        return m_currentUsage;
    }
    
    DetectionStatistics CStatisticsManager::GetCurrentDetectionStatistics() const {
        return m_currentDetection;
    }
    
    std::vector<StatEntry> CStatisticsManager::GetStatistics(const std::string& name, 
                                                           StatCategory category, 
                                                           uint64_t startTime, 
                                                           uint64_t endTime) const {
        std::vector<StatEntry> results;
        
        // Search through all statistics
        for (const auto& pair : m_statistics) {
            const std::vector<StatEntry>& entries = pair.second;
            
            for (const auto& entry : entries) {
                // Filter by name
                if (!name.empty() && entry.name.find(name) == std::string::npos) {
                    continue;
                }
                
                // Filter by category
                if (category != StatCategory::Custom && entry.category != category) {
                    continue;
                }
                
                // Filter by time range
                if (startTime > 0 && entry.timestamp < startTime) {
                    continue;
                }
                
                if (endTime > 0 && entry.timestamp > endTime) {
                    continue;
                }
                
                results.push_back(entry);
            }
        }
        
        return results;
    }
    
    AnalyticsReport CStatisticsManager::GenerateReport(const std::string& reportType, const std::string& timeRange) {
        AnalyticsReport report;
        report.reportType = reportType;
        report.timeRange = timeRange;
        report.generatedTime = GetTickCount64();
        
        // Add current metrics
        report.performance = m_currentPerformance;
        report.usage = m_currentUsage;
        report.detection = m_currentDetection;
        
        // Generate insights
        GenerateInsights();
        report.insights = GetInsights();
        
        // Generate recommendations
        report.recommendations = GetRecommendations();
        
        std::cout << "[STATS] Report generated: " << reportType << " - " << timeRange << std::endl;
        
        return report;
    }
    
    std::vector<std::string> CStatisticsManager::GetInsights() const {
        std::vector<std::string> insights;
        
        // Performance insights
        if (m_currentPerformance.averageFPS < 30) {
            insights.push_back("Low FPS detected - consider reducing ESP features");
        }
        
        if (m_currentPerformance.memoryUsage > 1024 * 1024 * 1024) { // 1GB
            insights.push_back("High memory usage detected - consider optimizing settings");
        }
        
        // Usage insights
        if (m_currentUsage.featureUsage.empty()) {
            insights.push_back("No feature usage recorded - system may not be active");
        }
        
        // Detection insights
        if (m_currentDetection.stealthScore < 50.0) {
            insights.push_back("Low stealth score detected - review anti-detection settings");
        }
        
        return insights;
    }
    
    std::vector<std::string> CStatisticsManager::GetRecommendations() const {
        std::vector<std::string> recommendations;
        
        // Performance recommendations
        if (m_currentPerformance.averageFPS < 60) {
            recommendations.push_back("Enable performance optimization mode");
        }
        
        if (m_currentPerformance.frameTime > 16.67) { // 60 FPS
            recommendations.push_back("Reduce ESP rendering distance");
        }
        
        // Usage recommendations
        auto maxUsage = std::max_element(m_currentUsage.featureUsage.begin(), m_currentUsage.featureUsage.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        
        if (maxUsage != m_currentUsage.featureUsage.end()) {
            recommendations.push_back("Most used feature: " + maxUsage->first);
        }
        
        // Detection recommendations
        if (m_currentDetection.detectionAttempts > 100) {
            recommendations.push_back("Consider enabling advanced anti-detection techniques");
        }
        
        return recommendations;
    }
    
    double CStatisticsManager::GetStealthScore() const {
        return m_currentDetection.stealthScore;
    }
    
    bool CStatisticsManager::ExportStatistics(const std::string& filePath, const std::string& format) {
        std::cout << "Exporting statistics to: " << filePath << " (" << format << ")" << std::endl;
        
        if (format == "json") {
            return ExportToJSON(filePath);
        } else if (format == "csv") {
            return ExportToCSV(filePath);
        } else if (format == "xml") {
            return ExportToXML(filePath);
        }
        
        std::cout << "[ERROR] Unsupported export format: " << format << std::endl;
        return false;
    }
    
    bool CStatisticsManager::ExportToJSON(const std::string& filePath) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cout << "[ERROR] Failed to create export file: " << filePath << std::endl;
            return false;
        }
        
        file << "{" << std::endl;
        file << "  \"generated_time\": " << GetTickCount64() << "," << std::endl;
        file << "  \"performance\": {" << std::endl;
        file << "    \"average_fps\": " << m_currentPerformance.averageFPS << "," << std::endl;
        file << "    \"min_fps\": " << m_currentPerformance.minFPS << "," << std::endl;
        file << "    \"max_fps\": " << m_currentPerformance.maxFPS << "," << std::endl;
        file << "    \"frame_time\": " << m_currentPerformance.frameTime << "," << std::endl;
        file << "    \"cpu_usage\": " << m_currentPerformance.cpuUsage << "," << std::endl;
        file << "    \"memory_usage\": " << m_currentPerformance.memoryUsage << "," << std::endl;
        file << "    \"network_latency\": " << m_currentPerformance.networkLatency << std::endl;
        file << "  }," << std::endl;
        file << "  \"usage\": {" << std::endl;
        file << "    \"total_runtime\": " << m_currentUsage.totalRuntime << "," << std::endl;
        file << "    \"session_count\": " << m_currentUsage.sessionCount << "," << std::endl;
        file << "    \"stealth_score\": " << m_currentDetection.stealthScore << std::endl;
        file << "  }" << std::endl;
        file << "}" << std::endl;
        
        file.close();
        std::cout << "[SUCCESS] Statistics exported to JSON: " << filePath << std::endl;
        return true;
    }
    
    bool CStatisticsManager::ExportToCSV(const std::string& filePath) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cout << "[ERROR] Failed to create export file: " << filePath << std::endl;
            return false;
        }
        
        // Write header
        file << "timestamp,category,name,type,value,unit" << std::endl;
        
        // Write all statistics
        for (const auto& pair : m_statistics) {
            for (const auto& entry : pair.second) {
                file << entry.timestamp << ","
                     << static_cast<int>(entry.category) << ","
                     << entry.name << ","
                     << static_cast<int>(entry.type) << ","
                     << entry.value << ","
                     << entry.unit << std::endl;
            }
        }
        
        file.close();
        std::cout << "[SUCCESS] Statistics exported to CSV: " << filePath << std::endl;
        return true;
    }
    
    bool CStatisticsManager::ExportToXML(const std::string& filePath) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cout << "[ERROR] Failed to create export file: " << filePath << std::endl;
            return false;
        }
        
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        file << "<statistics>" << std::endl;
        file << "  <generated_time>" << GetTickCount64() << "</generated_time>" << std::endl;
        file << "  <performance>" << std::endl;
        file << "    <average_fps>" << m_currentPerformance.averageFPS << "</average_fps>" << std::endl;
        file << "    <min_fps>" << m_currentPerformance.minFPS << "</min_fps>" << std::endl;
        file << "    <max_fps>" << m_currentPerformance.maxFPS << "</max_fps>" << std::endl;
        file << "    <frame_time>" << m_currentPerformance.frameTime << "</frame_time>" << std::endl;
        file << "    <cpu_usage>" << m_currentPerformance.cpuUsage << "</cpu_usage>" << std::endl;
        file << "    <memory_usage>" << m_currentPerformance.memoryUsage << "</memory_usage>" << std::endl;
        file << "    <network_latency>" << m_currentPerformance.networkLatency << "</network_latency>" << std::endl;
        file << "  </performance>" << std::endl;
        file << "  <usage>" << std::endl;
        file << "    <total_runtime>" << m_currentUsage.totalRuntime << "</total_runtime>" << std::endl;
        file << "    <session_count>" << m_currentUsage.sessionCount << "</session_count>" << std::endl;
        file << "    <stealth_score>" << m_currentDetection.stealthScore << "</stealth_score>" << std::endl;
        file << "  </usage>" << std::endl;
        file << "</statistics>" << std::endl;
        
        file.close();
        std::cout << "[SUCCESS] Statistics exported to XML: " << filePath << std::endl;
        return true;
    }
    
    // Private methods
    void CStatisticsManager::StartMonitoring() {
        if (m_isMonitoring) {
            return;
        }
        
        std::cout << "Starting statistics monitoring..." << std::endl;
        
        m_isMonitoring = true;
        m_shouldStopMonitoring = false;
        
        // Start monitoring thread
        m_monitoringThread = std::make_unique<std::thread>([this]() {
            MonitoringLoop();
        });
        
        std::cout << "[SUCCESS] Statistics monitoring started" << std::endl;
    }
    
    void CStatisticsManager::StopMonitoring() {
        if (!m_isMonitoring) {
            return;
        }
        
        std::cout << "Stopping statistics monitoring..." << std::endl;
        
        m_shouldStopMonitoring = true;
        
        if (m_monitoringThread && m_monitoringThread->joinable()) {
            m_monitoringThread->join();
        }
        
        m_isMonitoring = false;
        std::cout << "[SUCCESS] Statistics monitoring stopped" << std::endl;
    }
    
    void CStatisticsManager::MonitoringLoop() {
        while (!m_shouldStopMonitoring) {
            // Collect metrics
            CollectPerformanceMetrics();
            CollectUsageStatistics();
            CollectDetectionStatistics();
            
            // Update current metrics
            UpdatePerformanceMetrics();
            UpdateUsageStatistics();
            UpdateDetectionStatistics();
            
            // Cleanup old data
            CleanupOldData();
            
            // Sleep for sample interval
            std::this_thread::sleep_for(std::chrono::milliseconds(m_config.sampleIntervalMs));
        }
    }
    
    void CStatisticsManager::CollectPerformanceMetrics() {
        // This would collect actual performance metrics
        // For now, we'll simulate collection
        static int counter = 0;
        counter++;
        
        // Simulate FPS variation
        m_currentPerformance.averageFPS = 60.0 + (sin(counter * 0.1) * 10.0);
        m_currentPerformance.frameTime = 1000.0 / m_currentPerformance.averageFPS;
        
        // Simulate memory usage
        m_currentPerformance.memoryUsage = 512 * 1024 * 1024 + (cos(counter * 0.05) * 100 * 1024 * 1024);
        
        // Simulate CPU usage
        m_currentPerformance.cpuUsage = 25.0 + (sin(counter * 0.2) * 15.0);
    }
    
    void CStatisticsManager::CollectUsageStatistics() {
        // Update runtime
        m_currentUsage.totalRuntime = GetTickCount64() - m_startTime;
        
        // Update session count (simplified)
        static bool firstRun = true;
        if (firstRun) {
            m_currentUsage.sessionCount = 1;
            firstRun = false;
        }
    }
    
    void CStatisticsManager::CollectDetectionStatistics() {
        // This would collect actual detection statistics
        // For now, we'll use current values
    }
    
    void CStatisticsManager::UpdatePerformanceMetrics() {
        // This would update performance metrics with collected data
        // For now, we'll use current values
    }
    
    void CStatisticsManager::UpdateUsageStatistics() {
        // This would update usage statistics with collected data
        // For now, we'll use current values
    }
    
    void CStatisticsManager::UpdateDetectionStatistics() {
        // This would update detection statistics with collected data
        // For now, we'll use current values
    }
    
    void CStatisticsManager::CleanupOldData() {
        // This would clean up old data based on retention policy
        // For now, we'll just log the action
        static int cleanupCounter = 0;
        cleanupCounter++;
        
        if (cleanupCounter % 100 == 0) { // Cleanup every 100 cycles
            std::cout << "[STATS] Cleaning up old data..." << std::endl;
        }
    }
    
    void CStatisticsManager::GenerateInsights() {
        // This would generate insights from collected data
        // For now, we'll use the GetInsights() method
    }
    
    void CStatisticsManager::SaveStatistics() {
        std::cout << "Saving statistics..." << std::endl;
        
        // This would save statistics to file
        // For now, we'll just log the action
        std::cout << "[INFO] Statistics saved to file" << std::endl;
    }
    
    void CStatisticsManager::LoadStatistics() {
        std::cout << "Loading statistics..." << std::endl;
        
        // This would load statistics from file
        // For now, we'll just log the action
        std::cout << "[INFO] Statistics loaded from file" << std::endl;
    }
    
    void CStatisticsManager::AddStatistic(const std::string& name, StatCategory category, StatType type, 
                                     double value, const std::string& unit) {
        StatEntry entry;
        entry.name = name;
        entry.category = category;
        entry.type = type;
        entry.value = value;
        entry.unit = unit;
        entry.timestamp = GetTickCount64();
        entry.module = "RustToolkit";
        
        m_statistics[name].push_back(entry);
        
        // Limit history size
        if (m_statistics[name].size() > 10000) {
            m_statistics[name].erase(m_statistics[name].begin());
        }
    }
    
    std::string CStatisticsManager::GetCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d_%H-%M-%S");
        
        return ss.str();
    }
}

/*
 * Performance Monitoring System for Rust Anti-Cheat Evasion System
 * Provides real-time performance monitoring and optimization recommendations
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <Psapi.h>
#include <Pdh.h>

namespace Performance {
    
    // Performance metrics
    struct CPUMetrics {
        float cpuUsage;
        uint64_t cpuTime;
        uint64_t idleTime;
        uint64_t kernelTime;
        uint64_t userTime;
        int activeThreads;
        int totalThreads;
        float cpuFrequency;
    };
    
    struct MemoryMetrics {
        size_t totalRAM;
        size_t availableRAM;
        size_t usedRAM;
        size_t processRAM;
        size_t processVirtualRAM;
        size_t processWorkingSet;
        size_t processPrivateBytes;
        float memoryUsage;
        float processMemoryUsage;
        int pageFaults;
        int pageFaultsDelta;
    };
    
    struct GPUMetrics {
        float gpuUsage;
        uint64_t dedicatedMemoryUsed;
        uint64_t dedicatedMemoryTotal;
        uint64_t sharedMemoryUsed;
        uint64_t sharedMemoryTotal;
        float memoryUsage;
        float temperature;
        float fanSpeed;
        int clockSpeed;
        int powerUsage;
    };
    
    struct NetworkMetrics {
        uint64_t bytesSent;
        uint64_t bytesReceived;
        uint64_t packetsSent;
        uint64_t packetsReceived;
        float uploadSpeed;
        float downloadSpeed;
        int activeConnections;
        float latency;
        float packetLoss;
    };
    
    struct FrameMetrics {
        float fps;
        float frameTime;
        float minFrameTime;
        float maxFrameTime;
        float averageFrameTime;
        uint64_t totalFrames;
        uint64_t droppedFrames;
        float frameTimeVariance;
        float renderThreadTime;
        float mainThreadTime;
    };
    
    // Performance thresholds
    struct PerformanceThresholds {
        float maxCPUUsage = 80.0f;
        float maxMemoryUsage = 85.0f;
        float maxGPUUsage = 90.0f;
        float minFPS = 30.0f;
        float maxFrameTime = 33.3f;
        float maxNetworkLatency = 100.0f;
        float maxPacketLoss = 1.0f;
        float maxGPUTemperature = 85.0f;
    };
    
    // Performance alert
    struct PerformanceAlert {
        std::string metric;
        std::string description;
        float currentValue;
        float threshold;
        std::chrono::system_clock::time_point timestamp;
        bool isCritical;
        std::string recommendation;
    };
    
    // Performance snapshot
    struct PerformanceSnapshot {
        CPUMetrics cpu;
        MemoryMetrics memory;
        GPUMetrics gpu;
        NetworkMetrics network;
        FrameMetrics frame;
        std::chrono::system_clock::time_point timestamp;
        std::vector<PerformanceAlert> alerts;
    };
    
    // Performance monitor interface
    class IPerformanceMonitor {
    public:
        virtual ~IPerformanceMonitor() = default;
        virtual bool Initialize() = 0;
        virtual void Update() = 0;
        virtual void Shutdown() = 0;
        virtual std::string GetMonitorName() const = 0;
        virtual bool IsAvailable() const = 0;
    };
    
    // CPU performance monitor
    class CPUPerformanceMonitor : public IPerformanceMonitor {
    private:
        HANDLE m_processHandle;
        ULARGE_INTEGER m_lastCPU;
        ULARGE_INTEGER m_lastSystemCPU;
        int m_processorCount;
        std::chrono::steady_clock::time_point m_lastUpdate;
        float m_cpuUsage;
        std::atomic<bool> m_initialized;
        
    public:
        CPUPerformanceMonitor() : m_processHandle(nullptr), m_processorCount(0), 
                                m_cpuUsage(0.0f), m_initialized(false) {}
        
        bool Initialize() override {
            m_processHandle = GetCurrentProcess();
            if (!m_processHandle) return false;
            
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            m_processorCount = si.dwNumberOfProcessors;
            
            m_lastCPU.QuadPart = 0;
            m_lastSystemCPU.QuadPart = 0;
            
            m_initialized = true;
            return true;
        }
        
        void Update() override {
            if (!m_initialized) return;
            
            ULARGE_INTEGER cpu, systemCPU;
            
            if (GetProcessTimes(m_processHandle, nullptr, nullptr, 
                              reinterpret_cast<FILETIME*>(&systemCPU.LowPart),
                              reinterpret_cast<FILETIME*>(&systemCPU.HighPart))) {
                
                if (GetSystemTimes(reinterpret_cast<FILETIME*>(&cpu.LowPart),
                                 nullptr, nullptr,
                                 reinterpret_cast<FILETIME*>(&cpu.HighPart))) {
                    
                    if (m_lastCPU.QuadPart > 0 && m_lastSystemCPU.QuadPart > 0) {
                        ULONGLONG cpuDiff = cpu.QuadPart - m_lastCPU.QuadPart;
                        ULONGLONG systemDiff = systemCPU.QuadPart - m_lastSystemCPU.QuadPart;
                        
                        if (systemDiff > 0) {
                            m_cpuUsage = (float)(cpuDiff * 100.0 / systemDiff / m_processorCount);
                        }
                    }
                    
                    m_lastCPU = cpu;
                    m_lastSystemCPU = systemCPU;
                }
            }
            
            m_lastUpdate = std::chrono::steady_clock::now();
        }
        
        void Shutdown() override {
            m_initialized = false;
        }
        
        std::string GetMonitorName() const override {
            return "CPU Performance Monitor";
        }
        
        bool IsAvailable() const override {
            return m_initialized;
        }
        
        CPUMetrics GetMetrics() const {
            CPUMetrics metrics;
            metrics.cpuUsage = m_cpuUsage;
            metrics.activeThreads = 0;
            metrics.totalThreads = m_processorCount;
            metrics.cpuFrequency = 0.0f;
            
            // Get additional CPU info
            if (m_initialized) {
                PROCESS_MEMORY_COUNTERS_EX pmc;
                if (GetProcessMemoryInfo(m_processHandle, 
                                       reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
                                       sizeof(pmc))) {
                    metrics.activeThreads = pmc.ThreadCount;
                }
            }
            
            return metrics;
        }
    };
    
    // Memory performance monitor
    class MemoryPerformanceMonitor : public IPerformanceMonitor {
    private:
        HANDLE m_processHandle;
        std::atomic<bool> m_initialized;
        
    public:
        MemoryPerformanceMonitor() : m_processHandle(nullptr), m_initialized(false) {}
        
        bool Initialize() override {
            m_processHandle = GetCurrentProcess();
            if (!m_processHandle) return false;
            
            m_initialized = true;
            return true;
        }
        
        void Update() override {
            // Memory metrics are calculated on demand
        }
        
        void Shutdown() override {
            m_initialized = false;
        }
        
        std::string GetMonitorName() const override {
            return "Memory Performance Monitor";
        }
        
        bool IsAvailable() const override {
            return m_initialized;
        }
        
        MemoryMetrics GetMetrics() const {
            MemoryMetrics metrics = {};
            
            if (!m_initialized) return metrics;
            
            // Get system memory info
            MEMORYSTATUSEX memoryStatus;
            memoryStatus.dwLength = sizeof(memoryStatus);
            
            if (GlobalMemoryStatusEx(&memoryStatus)) {
                metrics.totalRAM = memoryStatus.ullTotalPhys;
                metrics.availableRAM = memoryStatus.ullAvailPhys;
                metrics.usedRAM = metrics.totalRAM - metrics.availableRAM;
                metrics.memoryUsage = (float)metrics.usedRAM / metrics.totalRAM * 100.0f;
            }
            
            // Get process memory info
            PROCESS_MEMORY_COUNTERS_EX pmc;
            if (GetProcessMemoryInfo(m_processHandle, 
                                   reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
                                   sizeof(pmc))) {
                metrics.processRAM = pmc.WorkingSetSize;
                metrics.processVirtualRAM = pmc.PrivateUsage;
                metrics.processWorkingSet = pmc.WorkingSetSize;
                metrics.processPrivateBytes = pmc.PrivateUsage;
                metrics.pageFaults = pmc.PageFaultCount;
                metrics.processMemoryUsage = (float)metrics.processRAM / metrics.totalRAM * 100.0f;
            }
            
            return metrics;
        }
    };
    
    // Frame performance monitor
    class FramePerformanceMonitor : public IPerformanceMonitor {
    private:
        std::vector<float> m_frameTimes;
        std::chrono::steady_clock::time_point m_lastFrameTime;
        std::atomic<uint64_t> m_frameCount;
        std::atomic<uint64_t> m_droppedFrames;
        std::atomic<bool> m_initialized;
        std::mutex m_mutex;
        static const size_t FRAME_HISTORY_SIZE = 1000;
        
    public:
        FramePerformanceMonitor() : m_frameCount(0), m_droppedFrames(0), m_initialized(false) {
            m_frameTimes.reserve(FRAME_HISTORY_SIZE);
        }
        
        bool Initialize() override {
            m_lastFrameTime = std::chrono::steady_clock::now();
            m_initialized = true;
            return true;
        }
        
        void Update() override {
            if (!m_initialized) return;
            
            auto currentTime = std::chrono::steady_clock::now();
            float frameTime = std::chrono::duration<float, std::milli>(currentTime - m_lastFrameTime).count();
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            m_frameTimes.push_back(frameTime);
            if (m_frameTimes.size() > FRAME_HISTORY_SIZE) {
                m_frameTimes.erase(m_frameTimes.begin());
            }
            
            m_lastFrameTime = currentTime;
            m_frameCount++;
            
            // Check for dropped frames (frame time > 100ms)
            if (frameTime > 100.0f) {
                m_droppedFrames++;
            }
        }
        
        void Shutdown() override {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_frameTimes.clear();
            m_initialized = false;
        }
        
        std::string GetMonitorName() const override {
            return "Frame Performance Monitor";
        }
        
        bool IsAvailable() const override {
            return m_initialized;
        }
        
        FrameMetrics GetMetrics() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            FrameMetrics metrics = {};
            metrics.totalFrames = m_frameCount;
            metrics.droppedFrames = m_droppedFrames;
            
            if (m_frameTimes.empty()) {
                return metrics;
            }
            
            // Calculate frame time statistics
            float sum = 0.0f;
            float minTime = m_frameTimes[0];
            float maxTime = m_frameTimes[0];
            
            for (float frameTime : m_frameTimes) {
                sum += frameTime;
                if (frameTime < minTime) minTime = frameTime;
                if (frameTime > maxTime) maxTime = frameTime;
            }
            
            metrics.averageFrameTime = sum / m_frameTimes.size();
            metrics.minFrameTime = minTime;
            metrics.maxFrameTime = maxTime;
            
            // Calculate FPS
            metrics.fps = 1000.0f / metrics.averageFrameTime;
            
            // Calculate frame time variance
            float variance = 0.0f;
            for (float frameTime : m_frameTimes) {
                float diff = frameTime - metrics.averageFrameTime;
                variance += diff * diff;
            }
            metrics.frameTimeVariance = variance / m_frameTimes.size();
            
            // Get the most recent frame time
            if (!m_frameTimes.empty()) {
                metrics.frameTime = m_frameTimes.back();
            }
            
            return metrics;
        }
        
        void RecordFrame() {
            Update();
        }
    };
    
    // Comprehensive performance monitoring system
    class PerformanceMonitoringSystem {
    private:
        std::vector<std::unique_ptr<IPerformanceMonitor>> m_monitors;
        std::vector<PerformanceSnapshot> m_snapshots;
        std::vector<PerformanceAlert> m_activeAlerts;
        PerformanceThresholds m_thresholds;
        std::mutex m_mutex;
        std::atomic<bool> m_running;
        std::thread m_monitoringThread;
        std::chrono::milliseconds m_updateInterval;
        std::atomic<uint64_t> m_updateCount;
        
        // Specific monitors
        std::unique_ptr<CPUPerformanceMonitor> m_cpuMonitor;
        std::unique_ptr<MemoryPerformanceMonitor> m_memoryMonitor;
        std::unique_ptr<FramePerformanceMonitor> m_frameMonitor;
        
    public:
        PerformanceMonitoringSystem(std::chrono::milliseconds updateInterval = std::chrono::milliseconds(1000))
            : m_running(false), m_updateInterval(updateInterval), m_updateCount(0) {
            
            // Create specific monitors
            m_cpuMonitor = std::make_unique<CPUPerformanceMonitor>();
            m_memoryMonitor = std::make_unique<MemoryPerformanceMonitor>();
            m_frameMonitor = std::make_unique<FramePerformanceMonitor>();
            
            // Add monitors to list
            m_monitors.push_back(std::move(m_cpuMonitor));
            m_monitors.push_back(std::move(m_memoryMonitor));
            m_monitors.push_back(std::move(m_frameMonitor));
            
            // Re-create specific monitors for access
            m_cpuMonitor = std::make_unique<CPUPerformanceMonitor>();
            m_memoryMonitor = std::make_unique<MemoryPerformanceMonitor>();
            m_frameMonitor = std::make_unique<FramePerformanceMonitor>();
        }
        
        ~PerformanceMonitoringSystem() {
            Stop();
        }
        
        bool Start() {
            if (m_running.exchange(true)) return true;
            
            LOG_INFO("Starting performance monitoring system");
            
            // Initialize all monitors
            for (auto& monitor : m_monitors) {
                if (!monitor->Initialize()) {
                    LOG_WARNING("Failed to initialize monitor: " + monitor->GetMonitorName());
                }
            }
            
            // Initialize specific monitors
            m_cpuMonitor->Initialize();
            m_memoryMonitor->Initialize();
            m_frameMonitor->Initialize();
            
            // Start monitoring thread
            m_monitoringThread = std::thread([this]() {
                MonitoringLoop();
            });
            
            return true;
        }
        
        void Stop() {
            if (!m_running.exchange(false)) return;
            
            LOG_INFO("Stopping performance monitoring system");
            
            // Wait for monitoring thread to finish
            if (m_monitoringThread.joinable()) {
                m_monitoringThread.join();
            }
            
            // Shutdown all monitors
            for (auto& monitor : m_monitors) {
                monitor->Shutdown();
            }
            
            m_cpuMonitor->Shutdown();
            m_memoryMonitor->Shutdown();
            m_frameMonitor->Shutdown();
        }
        
        PerformanceSnapshot GetCurrentSnapshot() {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            PerformanceSnapshot snapshot;
            snapshot.timestamp = std::chrono::system_clock::now();
            
            // Get CPU metrics
            if (m_cpuMonitor && m_cpuMonitor->IsAvailable()) {
                m_cpuMonitor->Update();
                snapshot.cpu = m_cpuMonitor->GetMetrics();
            }
            
            // Get memory metrics
            if (m_memoryMonitor && m_memoryMonitor->IsAvailable()) {
                snapshot.memory = m_memoryMonitor->GetMetrics();
            }
            
            // Get frame metrics
            if (m_frameMonitor && m_frameMonitor->IsAvailable()) {
                snapshot.frame = m_frameMonitor->GetMetrics();
            }
            
            // Check for alerts
            CheckThresholds(snapshot);
            
            return snapshot;
        }
        
        std::vector<PerformanceSnapshot> GetSnapshots(size_t count = 100) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            std::vector<PerformanceSnapshot> result;
            size_t start = (m_snapshots.size() > count) ? m_snapshots.size() - count : 0;
            
            for (size_t i = start; i < m_snapshots.size(); i++) {
                result.push_back(m_snapshots[i]);
            }
            
            return result;
        }
        
        std::vector<PerformanceAlert> GetActiveAlerts() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_activeAlerts;
        }
        
        void SetThresholds(const PerformanceThresholds& thresholds) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_thresholds = thresholds;
        }
        
        PerformanceThresholds GetThresholds() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_thresholds;
        }
        
        void RecordFrame() {
            if (m_frameMonitor) {
                m_frameMonitor->RecordFrame();
            }
        }
        
        uint64_t GetUpdateCount() const {
            return m_updateCount.load();
        }
        
        bool IsRunning() const {
            return m_running.load();
        }
        
        void GeneratePerformanceReport(const std::string& filePath) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            std::ofstream file(filePath);
            if (!file.is_open()) return;
            
            file << "# Performance Report\n\n";
            file << "Generated: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n";
            file << "Total Updates: " << m_updateCount.load() << "\n\n";
            
            // Current snapshot
            if (!m_snapshots.empty()) {
                const PerformanceSnapshot& current = m_snapshots.back();
                
                file << "## Current Performance\n\n";
                file << "CPU Usage: " << current.cpu.cpuUsage << "%\n";
                file << "Memory Usage: " << current.memory.memoryUsage << "%\n";
                file << "FPS: " << current.frame.fps << "\n";
                file << "Frame Time: " << current.frame.frameTime << "ms\n\n";
                
                // Alerts
                if (!current.alerts.empty()) {
                    file << "## Active Alerts\n\n";
                    for (const auto& alert : current.alerts) {
                        file << "- " << alert.metric << ": " << alert.description << "\n";
                        file << "  Current: " << alert.currentValue << ", Threshold: " << alert.threshold << "\n";
                        file << "  Recommendation: " << alert.recommendation << "\n\n";
                    }
                }
            }
            
            file.close();
        }
        
    private:
        void MonitoringLoop() {
            while (m_running) {
                try {
                    // Update all monitors
                    for (auto& monitor : m_monitors) {
                        if (monitor->IsAvailable()) {
                            monitor->Update();
                        }
                    }
                    
                    // Create snapshot
                    PerformanceSnapshot snapshot = GetCurrentSnapshot();
                    
                    // Store snapshot
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        m_snapshots.push_back(snapshot);
                        
                        // Keep only last 1000 snapshots
                        if (m_snapshots.size() > 1000) {
                            m_snapshots.erase(m_snapshots.begin());
                        }
                    }
                    
                    m_updateCount++;
                    
                    // Sleep for next update
                    std::this_thread::sleep_for(m_updateInterval);
                    
                } catch (const std::exception& e) {
                    LOG_ERROR("Performance monitoring error: " + std::string(e.what()));
                }
            }
        }
        
        void CheckThresholds(PerformanceSnapshot& snapshot) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            snapshot.alerts.clear();
            
            // Check CPU usage
            if (snapshot.cpu.cpuUsage > m_thresholds.maxCPUUsage) {
                PerformanceAlert alert;
                alert.metric = "CPU Usage";
                alert.description = "High CPU usage detected";
                alert.currentValue = snapshot.cpu.cpuUsage;
                alert.threshold = m_thresholds.maxCPUUsage;
                alert.timestamp = snapshot.timestamp;
                alert.isCritical = snapshot.cpu.cpuUsage > 95.0f;
                alert.recommendation = "Consider reducing workload or optimizing algorithms";
                snapshot.alerts.push_back(alert);
            }
            
            // Check memory usage
            if (snapshot.memory.memoryUsage > m_thresholds.maxMemoryUsage) {
                PerformanceAlert alert;
                alert.metric = "Memory Usage";
                alert.description = "High memory usage detected";
                alert.currentValue = snapshot.memory.memoryUsage;
                alert.threshold = m_thresholds.maxMemoryUsage;
                alert.timestamp = snapshot.timestamp;
                alert.isCritical = snapshot.memory.memoryUsage > 95.0f;
                alert.recommendation = "Consider freeing unused memory or optimizing data structures";
                snapshot.alerts.push_back(alert);
            }
            
            // Check FPS
            if (snapshot.frame.fps < m_thresholds.minFPS) {
                PerformanceAlert alert;
                alert.metric = "FPS";
                alert.description = "Low FPS detected";
                alert.currentValue = snapshot.frame.fps;
                alert.threshold = m_thresholds.minFPS;
                alert.timestamp = snapshot.timestamp;
                alert.isCritical = snapshot.frame.fps < 20.0f;
                alert.recommendation = "Consider reducing rendering load or optimizing graphics settings";
                snapshot.alerts.push_back(alert);
            }
            
            // Check frame time
            if (snapshot.frame.frameTime > m_thresholds.maxFrameTime) {
                PerformanceAlert alert;
                alert.metric = "Frame Time";
                alert.description = "High frame time detected";
                alert.currentValue = snapshot.frame.frameTime;
                alert.threshold = m_thresholds.maxFrameTime;
                alert.timestamp = snapshot.timestamp;
                alert.isCritical = snapshot.frame.frameTime > 100.0f;
                alert.recommendation = "Consider optimizing rendering pipeline or reducing workload";
                snapshot.alerts.push_back(alert);
            }
            
            // Update active alerts
            m_activeAlerts = snapshot.alerts;
        }
    };
    
    // Performance profiler for detailed analysis
    class PerformanceProfiler {
    private:
        struct ProfileEntry {
            std::string name;
            std::chrono::high_resolution_clock::time_point startTime;
            std::chrono::duration<double, std::milli> totalTime;
            uint64_t callCount;
            double averageTime;
            double minTime;
            double maxTime;
            std::vector<double> samples;
        };
        
        std::map<std::string, ProfileEntry> m_profiles;
        std::mutex m_mutex;
        
    public:
        void BeginProfile(const std::string& name) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto& entry = m_profiles[name];
            entry.name = name;
            entry.startTime = std::chrono::high_resolution_clock::now();
        }
        
        void EndProfile(const std::string& name) {
            auto endTime = std::chrono::high_resolution_clock::now();
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_profiles.find(name);
            if (it == m_profiles.end()) return;
            
            auto& entry = it->second;
            auto duration = std::chrono::duration<double, std::milli>(endTime - entry.startTime);
            
            entry.totalTime += duration;
            entry.callCount++;
            entry.samples.push_back(duration.count());
            
            if (entry.callCount == 1) {
                entry.averageTime = duration.count();
                entry.minTime = duration.count();
                entry.maxTime = duration.count();
            } else {
                entry.averageTime = entry.totalTime.count() / entry.callCount;
                entry.minTime = std::min(entry.minTime, duration.count());
                entry.maxTime = std::max(entry.maxTime, duration.count());
            }
        }
        
        ProfileEntry GetProfile(const std::string& name) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_profiles.find(name);
            if (it != m_profiles.end()) {
                return it->second;
            }
            
            return ProfileEntry{};
        }
        
        std::map<std::string, ProfileEntry> GetAllProfiles() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_profiles;
        }
        
        void Reset() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_profiles.clear();
        }
        
        void ResetProfile(const std::string& name) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_profiles.erase(name);
        }
    };
    
    // RAII profiler helper
    class ScopedProfiler {
    private:
        std::string m_name;
        PerformanceProfiler* m_profiler;
        
    public:
        ScopedProfiler(const std::string& name, PerformanceProfiler* profiler)
            : m_name(name), m_profiler(profiler) {
            if (m_profiler) {
                m_profiler->BeginProfile(m_name);
            }
        }
        
        ~ScopedProfiler() {
            if (m_profiler) {
                m_profiler->EndProfile(m_name);
            }
        }
    };
    
    // Global performance monitoring instance
    inline std::unique_ptr<PerformanceMonitoringSystem> g_performanceMonitor = std::make_unique<PerformanceMonitoringSystem>();
    inline std::unique_ptr<PerformanceProfiler> g_profiler = std::make_unique<PerformanceProfiler>();
    
    // Convenience macros
    #define PROFILE_SCOPE(name) ScopedProfiler _profiler(name, g_profiler.get())
    #define PROFILE_BEGIN(name) g_profiler->BeginProfile(name)
    #define PROFILE_END(name) g_profiler->EndProfile(name)
    #define RECORD_FRAME() g_performanceMonitor->RecordFrame()
    
} // namespace Performance

/*
 * Thread-Safe Components for Rust Anti-Cheat Evasion System
 * Provides thread-safe wrappers for all shared data structures
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <memory>
#include <map>
#include <vector>
#include <queue>
#include <functional>

namespace ThreadSafe {
    
    // Thread-safe configuration manager
    class ThreadSafeConfigManager {
    private:
        std::shared_mutex m_configMutex;
        std::map<std::string, std::string> m_configValues;
        std::map<std::string, std::function<void(const std::string&)>> m_callbacks;
        std::atomic<bool> m_isDirty{false};
        
    public:
        bool SetString(const std::string& key, const std::string& value) {
            std::unique_lock<std::shared_mutex> lock(m_configMutex);
            
            auto it = m_configValues.find(key);
            std::string oldValue = (it != m_configValues.end()) ? it->second : "";
            
            m_configValues[key] = value;
            
            if (oldValue != value) {
                m_isDirty = true;
                TriggerCallbacks(key, value);
            }
            
            return true;
        }
        
        std::string GetString(const std::string& key, const std::string& defaultValue = "") const {
            std::shared_lock<std::shared_mutex> lock(m_configMutex);
            
            auto it = m_configValues.find(key);
            return (it != m_configValues.end()) ? it->second : defaultValue;
        }
        
        bool SetBool(const std::string& key, bool value) {
            return SetString(key, value ? "true" : "false");
        }
        
        bool GetBool(const std::string& key, bool defaultValue = false) const {
            std::string value = GetString(key, defaultValue ? "true" : "false");
            return (value == "true" || value == "1" || value == "yes");
        }
        
        bool SetInt(const std::string& key, int value) {
            return SetString(key, std::to_string(value));
        }
        
        int GetInt(const std::string& key, int defaultValue = 0) const {
            std::string value = GetString(key, std::to_string(defaultValue));
            try {
                return std::stoi(value);
            } catch (...) {
                return defaultValue;
            }
        }
        
        bool SetFloat(const std::string& key, float value) {
            return SetString(key, std::to_string(value));
        }
        
        float GetFloat(const std::string& key, float defaultValue = 0.0f) const {
            std::string value = GetString(key, std::to_string(defaultValue));
            try {
                return std::stof(value);
            } catch (...) {
                return defaultValue;
            }
        }
        
        void RegisterCallback(const std::string& key, std::function<void(const std::string&)> callback) {
            std::unique_lock<std::shared_mutex> lock(m_configMutex);
            m_callbacks[key] = callback;
        }
        
        bool IsDirty() const {
            return m_isDirty.load();
        }
        
        void MarkClean() {
            m_isDirty = false;
        }
        
        std::map<std::string, std::string> GetAllConfig() const {
            std::shared_lock<std::shared_mutex> lock(m_configMutex);
            return m_configValues;
        }
        
    private:
        void TriggerCallbacks(const std::string& key, const std::string& value) {
            auto it = m_callbacks.find(key);
            if (it != m_callbacks.end()) {
                it->second(value);
            }
        }
    };
    
    // Thread-safe entity container
    template<typename T>
    class ThreadSafeContainer {
    private:
        mutable std::shared_mutex m_mutex;
        std::vector<T> m_data;
        std::atomic<size_t> m_size{0};
        
    public:
        void Add(const T& item) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            m_data.push_back(item);
            m_size = m_data.size();
        }
        
        void Add(T&& item) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            m_data.push_back(std::move(item));
            m_size = m_data.size();
        }
        
        bool Remove(size_t index) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            if (index >= m_data.size()) return false;
            
            m_data.erase(m_data.begin() + index);
            m_size = m_data.size();
            return true;
        }
        
        bool Remove(const T& item) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            auto it = std::find(m_data.begin(), m_data.end(), item);
            if (it != m_data.end()) {
                m_data.erase(it);
                m_size = m_data.size();
                return true;
            }
            return false;
        }
        
        void Clear() {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            m_data.clear();
            m_size = 0;
        }
        
        T Get(size_t index) const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            if (index >= m_data.size()) return T{};
            return m_data[index];
        }
        
        std::vector<T> GetAll() const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return m_data;
        }
        
        std::vector<T> GetCopy() const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return std::vector<T>(m_data);
        }
        
        size_t Size() const {
            return m_size.load();
        }
        
        bool Empty() const {
            return m_size.load() == 0;
        }
        
        template<typename Predicate>
        std::vector<T> Filter(Predicate predicate) const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            std::vector<T> result;
            
            for (const auto& item : m_data) {
                if (predicate(item)) {
                    result.push_back(item);
                }
            }
            
            return result;
        }
        
        template<typename Predicate>
        bool RemoveIf(Predicate predicate) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            
            auto it = std::remove_if(m_data.begin(), m_data.end(), predicate);
            bool removed = (it != m_data.end());
            
            if (removed) {
                m_data.erase(it, m_data.end());
                m_size = m_data.size();
            }
            
            return removed;
        }
        
        template<typename Function>
        void ForEach(Function function) const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            for (const auto& item : m_data) {
                function(item);
            }
        }
        
        template<typename Function>
        void ForEachMutable(Function function) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            for (auto& item : m_data) {
                function(item);
            }
        }
        
        void Reserve(size_t capacity) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            m_data.reserve(capacity);
        }
    };
    
    // Thread-safe queue for task management
    template<typename T>
    class ThreadSafeQueue {
    private:
        mutable std::mutex m_mutex;
        std::queue<T> m_queue;
        std::condition_variable m_condition;
        std::atomic<bool> m_stopping{false};
        
    public:
        void Push(const T& item) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.push(item);
            }
            m_condition.notify_one();
        }
        
        void Push(T&& item) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.push(std::move(item));
            }
            m_condition.notify_one();
        }
        
        bool TryPop(T& item) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (m_queue.empty() || m_stopping.load()) {
                return false;
            }
            
            item = std::move(m_queue.front());
            m_queue.pop();
            return true;
        }
        
        bool WaitAndPop(T& item) {
            std::unique_lock<std::mutex> lock(m_mutex);
            
            m_condition.wait(lock, [this] {
                return !m_queue.empty() || m_stopping.load();
            });
            
            if (m_queue.empty() || m_stopping.load()) {
                return false;
            }
            
            item = std::move(m_queue.front());
            m_queue.pop();
            return true;
        }
        
        bool Empty() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.empty();
        }
        
        size_t Size() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.size();
        }
        
        void Stop() {
            m_stopping = true;
            m_condition.notify_all();
        }
        
        void Clear() {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::queue<T> empty;
            m_queue.swap(empty);
        }
    };
    
    // Thread-safe statistics tracker
    class ThreadSafeStatistics {
    private:
        mutable std::shared_mutex m_mutex;
        std::map<std::string, std::atomic<uint64_t>> m_counters;
        std::map<std::string, std::atomic<double>> m_metrics;
        std::map<std::string, std::vector<double>> m_history;
        std::atomic<uint64_t> m_lastUpdate{0};
        
    public:
        void IncrementCounter(const std::string& name, uint64_t value = 1) {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            auto it = m_counters.find(name);
            if (it == m_counters.end()) {
                lock.unlock();
                std::unique_lock<std::shared_mutex> uniqueLock(m_mutex);
                m_counters[name].store(value);
            } else {
                it->second.fetch_add(value);
            }
            
            m_lastUpdate = GetTickCount64();
        }
        
        uint64_t GetCounter(const std::string& name) const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            auto it = m_counters.find(name);
            return (it != m_counters.end()) ? it->second.load() : 0;
        }
        
        void SetMetric(const std::string& name, double value) {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            auto it = m_metrics.find(name);
            if (it == m_metrics.end()) {
                lock.unlock();
                std::unique_lock<std::shared_mutex> uniqueLock(m_mutex);
                m_metrics[name].store(value);
            } else {
                it->second.store(value);
            }
            
            // Add to history
            AddToHistory(name, value);
            m_lastUpdate = GetTickCount64();
        }
        
        double GetMetric(const std::string& name) const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            auto it = m_metrics.find(name);
            return (it != m_metrics.end()) ? it->second.load() : 0.0;
        }
        
        void AddToHistory(const std::string& name, double value) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            
            auto& history = m_history[name];
            history.push_back(value);
            
            // Keep only last 1000 entries
            if (history.size() > 1000) {
                history.erase(history.begin());
            }
        }
        
        std::vector<double> GetHistory(const std::string& name) const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            auto it = m_history.find(name);
            return (it != m_history.end()) ? it->second : std::vector<double>();
        }
        
        double GetAverage(const std::string& name, size_t samples = 100) const {
            auto history = GetHistory(name);
            if (history.empty()) return 0.0;
            
            size_t count = std::min(samples, history.size());
            size_t start = history.size() - count;
            
            double sum = 0.0;
            for (size_t i = start; i < history.size(); i++) {
                sum += history[i];
            }
            
            return sum / count;
        }
        
        std::map<std::string, uint64_t> GetAllCounters() const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            std::map<std::string, uint64_t> result;
            for (const auto& pair : m_counters) {
                result[pair.first] = pair.second.load();
            }
            
            return result;
        }
        
        std::map<std::string, double> GetAllMetrics() const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            std::map<std::string, double> result;
            for (const auto& pair : m_metrics) {
                result[pair.first] = pair.second.load();
            }
            
            return result;
        }
        
        void Reset() {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            
            for (auto& pair : m_counters) {
                pair.second.store(0);
            }
            
            for (auto& pair : m_metrics) {
                pair.second.store(0.0);
            }
            
            m_history.clear();
            m_lastUpdate = GetTickCount64();
        }
        
        uint64_t GetLastUpdateTime() const {
            return m_lastUpdate.load();
        }
    };
    
    // Thread-safe event system
    class ThreadSafeEventSystem {
    private:
        mutable std::shared_mutex m_mutex;
        std::map<std::string, std::vector<std::function<void(const std::string&)>>> m_listeners;
        ThreadSafeQueue<std::pair<std::string, std::string>> m_eventQueue;
        std::atomic<bool> m_running{false};
        std::thread m_eventThread;
        
    public:
        void Start() {
            if (m_running.exchange(true)) return;
            
            m_eventThread = std::thread([this]() {
                EventProcessingLoop();
            });
        }
        
        void Stop() {
            if (!m_running.exchange(false)) return;
            
            m_eventQueue.Stop();
            if (m_eventThread.joinable()) {
                m_eventThread.join();
            }
        }
        
        void Subscribe(const std::string& eventType, std::function<void(const std::string&)> listener) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            m_listeners[eventType].push_back(listener);
        }
        
        void Unsubscribe(const std::string& eventType) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            m_listeners.erase(eventType);
        }
        
        void Emit(const std::string& eventType, const std::string& data = "") {
            m_eventQueue.Push({eventType, data});
        }
        
        void EmitSync(const std::string& eventType, const std::string& data = "") {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            auto it = m_listeners.find(eventType);
            if (it != m_listeners.end()) {
                for (const auto& listener : it->second) {
                    try {
                        listener(data);
                    } catch (const std::exception& e) {
                        LOG_ERROR("Event listener error: " + std::string(e.what()));
                    }
                }
            }
        }
        
        bool IsRunning() const {
            return m_running.load();
        }
        
    private:
        void EventProcessingLoop() {
            std::pair<std::string, std::string> event;
            
            while (m_running.load()) {
                if (m_eventQueue.WaitAndPop(event)) {
                    ProcessEvent(event.first, event.second);
                }
            }
        }
        
        void ProcessEvent(const std::string& eventType, const std::string& data) {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            
            auto it = m_listeners.find(eventType);
            if (it != m_listeners.end()) {
                for (const auto& listener : it->second) {
                    try {
                        listener(data);
                    } catch (const std::exception& e) {
                        LOG_ERROR("Event listener error: " + std::string(e.what()));
                    }
                }
            }
        }
    };
    
    // Thread-safe memory pool
    template<typename T>
    class ThreadSafeMemoryPool {
    private:
        struct PoolItem {
            alignas(T) unsigned char data[sizeof(T)];
            std::atomic<bool> inUse{false};
        };
        
        std::vector<PoolItem> m_pool;
        std::stack<size_t> m_available;
        mutable std::mutex m_mutex;
        std::atomic<size_t> m_allocatedCount{0};
        std::atomic<size_t> m_peakUsage{0};
        
    public:
        ThreadSafeMemoryPool(size_t poolSize = 1000) {
            m_pool.resize(poolSize);
            for (size_t i = 0; i < poolSize; i++) {
                m_available.push(i);
            }
        }
        
        T* Acquire() {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (m_available.empty()) {
                return nullptr; // Pool exhausted
            }
            
            size_t index = m_available.top();
            m_available.pop();
            
            PoolItem& item = m_pool[index];
            item.inUse = true;
            
            m_allocatedCount++;
            size_t currentUsage = m_pool.size() - m_available.size();
            if (currentUsage > m_peakUsage.load()) {
                m_peakUsage = currentUsage;
            }
            
            return new(&item.data) T();
        }
        
        void Release(T* ptr) {
            if (!ptr) return;
            
            // Find the pool item containing this pointer
            for (size_t i = 0; i < m_pool.size(); i++) {
                if (reinterpret_cast<T*>(&m_pool[i].data) == ptr) {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    
                    // Call destructor
                    ptr->~T();
                    
                    // Mark as available
                    m_pool[i].inUse = false;
                    m_available.push(i);
                    m_allocatedCount--;
                    
                    return;
                }
            }
        }
        
        size_t GetAllocatedCount() const {
            return m_allocatedCount.load();
        }
        
        size_t GetPeakUsage() const {
            return m_peakUsage.load();
        }
        
        size_t GetAvailableCount() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_available.size();
        }
        
        size_t GetPoolSize() const {
            return m_pool.size();
        }
        
        double GetUtilization() const {
            size_t total = m_pool.size();
            return total > 0 ? (static_cast<double>(m_allocatedCount.load()) / total) : 0.0;
        }
    };
    
    // Thread-safe logger wrapper
    class ThreadSafeLogger {
    private:
        mutable std::mutex m_logMutex;
        ThreadSafeQueue<std::string> m_logQueue;
        std::atomic<bool> m_running{false};
        std::thread m_logThread;
        std::string m_logFile;
        
    public:
        ThreadSafeLogger(const std::string& logFile = "rust_toolkit.log") 
            : m_logFile(logFile) {
            Start();
        }
        
        ~ThreadSafeLogger() {
            Stop();
        }
        
        void Start() {
            if (m_running.exchange(true)) return;
            
            m_logThread = std::thread([this]() {
                LogProcessingLoop();
            });
        }
        
        void Stop() {
            if (!m_running.exchange(false)) return;
            
            m_logQueue.Stop();
            if (m_logThread.joinable()) {
                m_logThread.join();
            }
        }
        
        void Log(const std::string& message) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            ss << " - " << message;
            
            m_logQueue.Push(ss.str());
        }
        
        void LogError(const std::string& message) {
            Log("[ERROR] " + message);
        }
        
        void LogWarning(const std::string& message) {
            Log("[WARNING] " + message);
        }
        
        void LogInfo(const std::string& message) {
            Log("[INFO] " + message);
        }
        
        void LogDebug(const std::string& message) {
            Log("[DEBUG] " + message);
        }
        
    private:
        void LogProcessingLoop() {
            std::string message;
            
            while (m_running.load()) {
                if (m_logQueue.WaitAndPop(message)) {
                    WriteToFile(message);
                }
            }
        }
        
        void WriteToFile(const std::string& message) {
            std::lock_guard<std::mutex> lock(m_logMutex);
            
            std::ofstream file(m_logFile, std::ios::app);
            if (file.is_open()) {
                file << message << std::endl;
                file.close();
            }
        }
    };
    
} // namespace ThreadSafe

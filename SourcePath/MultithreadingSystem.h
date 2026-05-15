/*
 * Multithreading System for Rust Anti-Cheat Evasion System
 * Provides thread pool, job system, and concurrent processing
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <atomic>
#include <functional>
#include <vector>
#include <memory>
#include <chrono>
#include <unordered_map>

namespace Threading {
    
    // Job priority levels
    enum class JobPriority {
        Low = 0,
        Normal = 1,
        High = 2,
        Critical = 3
    };
    
    // Job status
    enum class JobStatus {
        Pending,
        Running,
        Completed,
        Failed,
        Cancelled
    };
    
    // Job wrapper
    struct Job {
        std::function<void()> function;
        JobPriority priority;
        JobStatus status;
        uint64_t id;
        std::chrono::steady_clock::time_point submitTime;
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point endTime;
        std::string name;
        std::string category;
        
        Job(uint64_t jobId, std::function<void()> func, JobPriority prio = JobPriority::Normal,
             const std::string& jobName = "", const std::string& jobCategory = "")
            : function(func), priority(prio), status(JobStatus::Pending), id(jobId),
              submitTime(std::chrono::steady_clock::now()), name(jobName), category(jobCategory) {}
    };
    
    // Thread pool implementation
    class ThreadPool {
    private:
        std::vector<std::thread> m_workers;
        std::priority_queue<std::shared_ptr<Job>, std::vector<std::shared_ptr<Job>>, JobComparator> m_jobQueue;
        std::mutex m_queueMutex;
        std::condition_variable m_condition;
        std::atomic<bool> m_running;
        std::atomic<uint64_t> m_jobCounter;
        std::unordered_map<uint64_t, std::shared_ptr<Job>> m_activeJobs;
        std::mutex m_activeJobsMutex;
        size_t m_threadCount;
        
        struct JobComparator {
            bool operator()(const std::shared_ptr<Job>& a, const std::shared_ptr<Job>& b) const {
                return a->priority < b->priority; // Higher priority jobs come first
            }
        };
        
    public:
        ThreadPool(size_t threadCount = std::thread::hardware_concurrency()) 
            : m_running(true), m_jobCounter(0), m_threadCount(threadCount) {
            
            for (size_t i = 0; i < m_threadCount; i++) {
                m_workers.emplace_back([this]() { WorkerLoop(); });
            }
        }
        
        ~ThreadPool() {
            Shutdown();
        }
        
        void Shutdown() {
            m_running = false;
            m_condition.notify_all();
            
            for (auto& worker : m_workers) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
            
            m_workers.clear();
        }
        
        uint64_t SubmitJob(std::function<void()> function, JobPriority priority = JobPriority::Normal,
                          const std::string& name = "", const std::string& category = "") {
            uint64_t jobId = m_jobCounter++;
            
            auto job = std::make_shared<Job>(jobId, function, priority, name, category);
            
            {
                std::lock_guard<std::mutex> lock(m_queueMutex);
                m_jobQueue.push(job);
                
                std::lock_guard<std::mutex> activeLock(m_activeJobsMutex);
                m_activeJobs[jobId] = job;
            }
            
            m_condition.notify_one();
            return jobId;
        }
        
        template<typename F, typename... Args>
        auto SubmitFuture(F&& function, Args&&... args) -> std::future<decltype(function(args...))> {
            using ReturnType = decltype(function(args...));
            
            auto promise = std::make_shared<std::promise<ReturnType>>();
            auto future = promise->get_future();
            
            auto wrappedFunction = [promise, function, args...]() {
                try {
                    if constexpr (std::is_void_v<ReturnType>) {
                        function(args...);
                        promise->set_value();
                    } else {
                        promise->set_value(function(args...));
                    }
                } catch (...) {
                    promise->set_exception(std::current_exception());
                }
            };
            
            SubmitJob(wrappedFunction);
            return future;
        }
        
        bool CancelJob(uint64_t jobId) {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            std::lock_guard<std::mutex> activeLock(m_activeJobsMutex);
            
            auto it = m_activeJobs.find(jobId);
            if (it != m_activeJobs.end() && it->second->status == JobStatus::Pending) {
                it->second->status = JobStatus::Cancelled;
                m_activeJobs.erase(it);
                return true;
            }
            
            return false;
        }
        
        JobStatus GetJobStatus(uint64_t jobId) {
            std::lock_guard<std::mutex> lock(m_activeJobsMutex);
            
            auto it = m_activeJobs.find(jobId);
            if (it != m_activeJobs.end()) {
                return it->second->status;
            }
            
            return JobStatus::Completed; // Assume completed if not found
        }
        
        std::vector<std::shared_ptr<Job>> GetActiveJobs() {
            std::lock_guard<std::mutex> lock(m_activeJobsMutex);
            
            std::vector<std::shared_ptr<Job>> jobs;
            for (const auto& pair : m_activeJobs) {
                jobs.push_back(pair.second);
            }
            
            return jobs;
        }
        
        size_t GetQueueSize() const {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            return m_jobQueue.size();
        }
        
        size_t GetThreadCount() const {
            return m_threadCount;
        }
        
        bool IsRunning() const {
            return m_running;
        }
        
    private:
        void WorkerLoop() {
            while (m_running) {
                std::shared_ptr<Job> job;
                
                {
                    std::unique_lock<std::mutex> lock(m_queueMutex);
                    
                    m_condition.wait(lock, [this]() {
                        return !m_jobQueue.empty() || !m_running;
                    });
                    
                    if (!m_running && m_jobQueue.empty()) {
                        break;
                    }
                    
                    if (!m_jobQueue.empty()) {
                        job = m_jobQueue.top();
                        m_jobQueue.pop();
                    }
                }
                
                if (job) {
                    ExecuteJob(job);
                }
            }
        }
        
        void ExecuteJob(std::shared_ptr<Job> job) {
            job->status = JobStatus::Running;
            job->startTime = std::chrono::steady_clock::now();
            
            try {
                job->function();
                job->status = JobStatus::Completed;
            } catch (const std::exception& e) {
                LOG_ERROR("Job execution failed: " + std::string(e.what()));
                job->status = JobStatus::Failed;
            } catch (...) {
                LOG_ERROR("Job execution failed with unknown exception");
                job->status = JobStatus::Failed;
            }
            
            job->endTime = std::chrono::steady_clock::now();
            
            // Remove from active jobs
            {
                std::lock_guard<std::mutex> lock(m_activeJobsMutex);
                m_activeJobs.erase(job->id);
            }
        }
    };
    
    // Parallel processing utilities
    class ParallelProcessor {
    private:
        std::unique_ptr<ThreadPool> m_threadPool;
        
    public:
        ParallelProcessor(size_t threadCount = std::thread::hardware_concurrency()) {
            m_threadPool = std::make_unique<ThreadPool>(threadCount);
        }
        
        template<typename Iterator, typename Function>
        void ParallelFor(Iterator begin, Iterator end, Function function, size_t chunkSize = 1) {
            const size_t totalSize = std::distance(begin, end);
            const size_t numChunks = (totalSize + chunkSize - 1) / chunkSize;
            
            std::vector<std::future<void>> futures;
            futures.reserve(numChunks);
            
            auto it = begin;
            for (size_t i = 0; i < numChunks; ++i) {
                auto chunkEnd = it;
                std::advance(chunkEnd, std::min(chunkSize, static_cast<size_t>(std::distance(it, end))));
                
                futures.push_back(m_threadPool->SubmitFuture([it, chunkEnd, function]() mutable {
                    for (auto current = it; current != chunkEnd; ++current) {
                        function(*current);
                    }
                }));
                
                it = chunkEnd;
            }
            
            // Wait for all jobs to complete
            for (auto& future : futures) {
                future.wait();
            }
        }
        
        template<typename Container, typename Function>
        void ParallelFor(Container& container, Function function, size_t chunkSize = 1) {
            ParallelFor(std::begin(container), std::end(container), function, chunkSize);
        }
        
        template<typename Iterator, typename Function>
        void ParallelForWithIndex(Iterator begin, Iterator end, Function function, size_t chunkSize = 1) {
            const size_t totalSize = std::distance(begin, end);
            const size_t numChunks = (totalSize + chunkSize - 1) / chunkSize;
            
            std::vector<std::future<void>> futures;
            futures.reserve(numChunks);
            
            auto it = begin;
            size_t startIndex = 0;
            
            for (size_t i = 0; i < numChunks; ++i) {
                auto chunkEnd = it;
                std::advance(chunkEnd, std::min(chunkSize, static_cast<size_t>(std::distance(it, end))));
                size_t chunkStartIndex = startIndex;
                
                futures.push_back(m_threadPool->SubmitFuture([it, chunkEnd, chunkStartIndex, function]() mutable {
                    size_t index = chunkStartIndex;
                    for (auto current = it; current != chunkEnd; ++current, ++index) {
                        function(*current, index);
                    }
                }));
                
                it = chunkEnd;
                startIndex += chunkSize;
            }
            
            // Wait for all jobs to complete
            for (auto& future : futures) {
                future.wait();
            }
        }
        
        template<typename Container, typename Function>
        void ParallelForWithIndex(Container& container, Function function, size_t chunkSize = 1) {
            ParallelForWithIndex(std::begin(container), std::end(container), function, chunkSize);
        }
        
        template<typename T, typename Function>
        std::vector<std::future<T>> ParallelMap(const std::vector<T>& input, Function function, size_t chunkSize = 1) {
            std::vector<std::future<T>> futures;
            futures.reserve(input.size());
            
            for (const auto& item : input) {
                futures.push_back(m_threadPool->SubmitFuture([function, item]() {
                    return function(item);
                }));
            }
            
            return futures;
        }
        
        template<typename T, typename Function>
        std::vector<T> ParallelReduce(const std::vector<T>& input, T identity, Function function, size_t chunkSize = 1) {
            if (input.empty()) return identity;
            
            // First, map in parallel
            auto futures = ParallelMap(input, [](const T& item) { return item; }, chunkSize);
            
            // Collect results
            std::vector<T> results;
            results.reserve(futures.size());
            
            for (auto& future : futures) {
                results.push_back(future.get());
            }
            
            // Then reduce sequentially (could be parallelized for large datasets)
            T result = identity;
            for (const auto& item : results) {
                result = function(result, item);
            }
            
            return result;
        }
        
        ThreadPool& GetThreadPool() {
            return *m_threadPool;
        }
    };
    
    // Thread-safe counter
    class ThreadSafeCounter {
    private:
        std::atomic<uint64_t> m_value;
        
    public:
        ThreadSafeCounter(uint64_t initialValue = 0) : m_value(initialValue) {}
        
        uint64_t Increment(uint64_t amount = 1) {
            return m_value.fetch_add(amount) + amount;
        }
        
        uint64_t Decrement(uint64_t amount = 1) {
            return m_value.fetch_sub(amount) - amount;
        }
        
        uint64_t Get() const {
            return m_value.load();
        }
        
        void Set(uint64_t value) {
            m_value.store(value);
        }
        
        void Reset() {
            m_value.store(0);
        }
    };
    
    // Thread-safe flag
    class ThreadSafeFlag {
    private:
        std::atomic<bool> m_value;
        
    public:
        ThreadSafeFlag(bool initialValue = false) : m_value(initialValue) {}
        
        void Set(bool value) {
            m_value.store(value);
        }
        
        bool Get() const {
            return m_value.load();
        }
        
        bool CompareAndSet(bool expected, bool desired) {
            return m_value.compare_exchange_strong(expected, desired);
        }
        
        void SetTrue() {
            Set(true);
        }
        
        void SetFalse() {
            Set(false);
        }
        
        bool IsTrue() const {
            return Get();
        }
        
        bool IsFalse() const {
            return !Get();
        }
    };
    
    // Thread-safe queue for producer-consumer pattern
    template<typename T>
    class ThreadSafeQueue {
    private:
        std::queue<T> m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable m_condition;
        std::atomic<bool> m_stopping;
        
    public:
        ThreadSafeQueue() : m_stopping(false) {}
        
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
    
    // RAII thread guard
    class ThreadGuard {
    private:
        std::thread m_thread;
        bool m_joinable;
        
    public:
        template<typename Function, typename... Args>
        explicit ThreadGuard(Function&& func, Args&&... args)
            : m_thread(std::forward<Function>(func), std::forward<Args>(args)...), m_joinable(true) {}
        
        ~ThreadGuard() {
            if (m_joinable && m_thread.joinable()) {
                m_thread.join();
            }
        }
        
        ThreadGuard(const ThreadGuard&) = delete;
        ThreadGuard& operator=(const ThreadGuard&) = delete;
        
        ThreadGuard(ThreadGuard&& other) noexcept
            : m_thread(std::move(other.m_thread)), m_joinable(other.m_joinable) {
            other.m_joinable = false;
        }
        
        ThreadGuard& operator=(ThreadGuard&& other) noexcept {
            if (this != &other) {
                if (m_joinable && m_thread.joinable()) {
                    m_thread.join();
                }
                
                m_thread = std::move(other.m_thread);
                m_joinable = other.m_joinable;
                other.m_joinable = false;
            }
            return *this;
        }
        
        std::thread& Get() {
            return m_thread;
        }
        
        const std::thread& Get() const {
            return m_thread;
        }
        
        bool Joinable() const {
            return m_thread.joinable();
        }
    };
    
    // Thread-safe singleton pattern
    template<typename T>
    class ThreadSafeSingleton {
    private:
        static std::atomic<T*> m_instance;
        static std::mutex m_mutex;
        
    public:
        static T& GetInstance() {
            T* instance = m_instance.load();
            if (!instance) {
                std::lock_guard<std::mutex> lock(m_mutex);
                instance = m_instance.load();
                if (!instance) {
                    instance = new T();
                    m_instance.store(instance);
                }
            }
            return *instance;
        }
        
        static void DestroyInstance() {
            T* instance = m_instance.load();
            if (instance) {
                std::lock_guard<std::mutex> lock(m_mutex);
                delete instance;
                m_instance.store(nullptr);
            }
        }
    };
    
    // Template static member definitions
    template<typename T>
    std::atomic<T*> ThreadSafeSingleton<T>::m_instance = nullptr;
    
    template<typename T>
    std::mutex ThreadSafeSingleton<T>::m_mutex;
    
} // namespace Threading

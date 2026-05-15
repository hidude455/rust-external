/*
 * Dependency Injection System for Rust Anti-Cheat Evasion System
 * Resolves circular dependencies and provides proper component management
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <typeinfo>
#include <mutex>
#include <stdexcept>

namespace DI {
    
    // Forward declarations
    class ServiceContainer;
    class ServiceLocator;
    
    // Service interface for dependency injection
    class IService {
    public:
        virtual ~IService() = default;
        virtual const char* GetServiceName() const = 0;
        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual bool IsInitialized() const = 0;
    };
    
    // Service registration information
    struct ServiceInfo {
        std::string name;
        std::string interfaceName;
        std::function<std::shared_ptr<IService>()> factory;
        bool singleton;
        bool initialized;
        std::shared_ptr<IService> instance;
        std::vector<std::string> dependencies;
        
        ServiceInfo(const std::string& n, const std::string& iface, 
                   std::function<std::shared_ptr<IService>()> f, bool single = true)
            : name(n), interfaceName(iface), factory(f), singleton(single), initialized(false) {}
    };
    
    // Dependency graph for resolving service dependencies
    class DependencyGraph {
    private:
        std::unordered_map<std::string, std::vector<std::string>> m_dependencies;
        std::unordered_map<std::string, std::vector<std::string>> m_dependents;
        mutable std::mutex m_mutex;
        
    public:
        void AddDependency(const std::string& service, const std::string& dependency) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_dependencies[service].push_back(dependency);
            m_dependents[dependency].push_back(service);
        }
        
        void RemoveDependency(const std::string& service, const std::string& dependency) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto& deps = m_dependencies[service];
            deps.erase(std::remove(deps.begin(), deps.end(), dependency), deps.end());
            
            auto& dependents = m_dependents[dependency];
            dependents.erase(std::remove(dependents.begin(), dependents.end(), service), dependents.end());
        }
        
        std::vector<std::string> GetDependencies(const std::string& service) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_dependencies.find(service);
            return (it != m_dependencies.end()) ? it->second : std::vector<std::string>();
        }
        
        std::vector<std::string> GetDependents(const std::string& service) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_dependents.find(service);
            return (it != m_dependents.end()) ? it->second : std::vector<std::string>();
        }
        
        bool HasCircularDependency() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            std::unordered_map<std::string, int> visited;
            std::unordered_map<std::string, int> recursionStack;
            
            for (const auto& pair : m_dependencies) {
                if (HasCircularDependencyUtil(pair.first, visited, recursionStack)) {
                    return true;
                }
            }
            
            return false;
        }
        
        std::vector<std::string> GetInitializationOrder() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            std::vector<std::string> result;
            std::unordered_map<std::string, int> visited;
            std::unordered_map<std::string, int> recursionStack;
            
            // Topological sort
            for (const auto& pair : m_dependencies) {
                if (visited[pair.first] == 0) {
                    TopologicalSortUtil(pair.first, visited, recursionStack, result);
                }
            }
            
            return result;
        }
        
    private:
        bool HasCircularDependencyUtil(const std::string& service,
                                     std::unordered_map<std::string, int>& visited,
                                     std::unordered_map<std::string, int>& recursionStack) const {
            visited[service] = 1;
            recursionStack[service] = 1;
            
            auto it = m_dependencies.find(service);
            if (it != m_dependencies.end()) {
                for (const auto& dep : it->second) {
                    if (recursionStack[dep] == 1) {
                        return true; // Circular dependency found
                    }
                    
                    if (visited[dep] == 0) {
                        if (HasCircularDependencyUtil(dep, visited, recursionStack)) {
                            return true;
                        }
                    }
                }
            }
            
            recursionStack[service] = 0;
            return false;
        }
        
        void TopologicalSortUtil(const std::string& service,
                                std::unordered_map<std::string, int>& visited,
                                std::unordered_map<std::string, int>& recursionStack,
                                std::vector<std::string>& result) const {
            visited[service] = 1;
            recursionStack[service] = 1;
            
            auto it = m_dependencies.find(service);
            if (it != m_dependencies.end()) {
                for (const auto& dep : it->second) {
                    if (visited[dep] == 0) {
                        TopologicalSortUtil(dep, visited, recursionStack, result);
                    }
                }
            }
            
            recursionStack[service] = 0;
            result.push_back(service);
        }
    };
    
    // Service container for managing service lifecycle
    class ServiceContainer {
    private:
        std::unordered_map<std::string, std::unique_ptr<ServiceInfo>> m_services;
        std::unordered_map<std::string, std::shared_ptr<IService>> m_instances;
        DependencyGraph m_dependencyGraph;
        mutable std::mutex m_mutex;
        bool m_initialized;
        
    public:
        ServiceContainer() : m_initialized(false) {}
        
        template<typename T>
        void RegisterService(const std::string& name, const std::vector<std::string>& dependencies = {}) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto factory = []() -> std::shared_ptr<IService> {
                return std::static_pointer_cast<IService>(std::make_shared<T>());
            };
            
            std::string interfaceName = typeid(T).name();
            auto info = std::make_unique<ServiceInfo>(name, interfaceName, factory, true);
            info->dependencies = dependencies;
            
            // Add dependencies to graph
            for (const auto& dep : dependencies) {
                m_dependencyGraph.AddDependency(name, dep);
            }
            
            // Check for circular dependencies
            if (m_dependencyGraph.HasCircularDependency()) {
                throw std::runtime_error("Circular dependency detected when registering service: " + name);
            }
            
            m_services[name] = std::move(info);
        }
        
        template<typename T>
        void RegisterServiceFactory(const std::string& name, 
                                  std::function<std::shared_ptr<T>()> factory,
                                  const std::vector<std::string>& dependencies = {}) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto serviceFactory = [factory]() -> std::shared_ptr<IService> {
                return std::static_pointer_cast<IService>(factory());
            };
            
            std::string interfaceName = typeid(T).name();
            auto info = std::make_unique<ServiceInfo>(name, interfaceName, serviceFactory, true);
            info->dependencies = dependencies;
            
            // Add dependencies to graph
            for (const auto& dep : dependencies) {
                m_dependencyGraph.AddDependency(name, dep);
            }
            
            // Check for circular dependencies
            if (m_dependencyGraph.HasCircularDependency()) {
                throw std::runtime_error("Circular dependency detected when registering service: " + name);
            }
            
            m_services[name] = std::move(info);
        }
        
        void RegisterInstance(const std::string& name, std::shared_ptr<IService> instance) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            std::string interfaceName = typeid(*instance).name();
            auto info = std::make_unique<ServiceInfo>(name, interfaceName, nullptr, true);
            info->instance = instance;
            info->initialized = instance->IsInitialized();
            
            m_services[name] = std::move(info);
            m_instances[name] = instance;
        }
        
        std::shared_ptr<IService> GetService(const std::string& name) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_services.find(name);
            if (it == m_services.end()) {
                throw std::runtime_error("Service not found: " + name);
            }
            
            ServiceInfo* info = it->second.get();
            
            if (!info->initialized) {
                throw std::runtime_error("Service not initialized: " + name);
            }
            
            if (info->singleton) {
                if (!info->instance) {
                    info->instance = info->factory();
                    m_instances[name] = info->instance;
                }
                return info->instance;
            }
            
            return info->factory();
        }
        
        template<typename T>
        std::shared_ptr<T> GetService(const std::string& name) {
            auto service = GetService(name);
            return std::dynamic_pointer_cast<T>(service);
        }
        
        bool InitializeServices() {
            if (m_initialized) return true;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            try {
                // Get initialization order from dependency graph
                auto initOrder = m_dependencyGraph.GetInitializationOrder();
                
                // Initialize services in dependency order
                for (const auto& serviceName : initOrder) {
                    auto it = m_services.find(serviceName);
                    if (it == m_services.end()) continue;
                    
                    ServiceInfo* info = it->second.get();
                    
                    if (!info->initialized && info->factory) {
                        // Create instance if singleton
                        if (info->singleton && !info->instance) {
                            info->instance = info->factory();
                            m_instances[serviceName] = info->instance;
                        }
                        
                        // Initialize service
                        std::shared_ptr<IService> service = info->singleton ? info->instance : info->factory();
                        if (service && !service->Initialize()) {
                            LOG_ERROR("Failed to initialize service: " + serviceName);
                            return false;
                        }
                        
                        info->initialized = true;
                    }
                }
                
                m_initialized = true;
                return true;
            }
            catch (const std::exception& e) {
                LOG_ERROR("Service initialization failed: " + std::string(e.what()));
                return false;
            }
        }
        
        void ShutdownServices() {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (!m_initialized) return;
            
            // Get reverse initialization order for shutdown
            auto initOrder = m_dependencyGraph.GetInitializationOrder();
            std::reverse(initOrder.begin(), initOrder.end());
            
            // Shutdown services in reverse dependency order
            for (const auto& serviceName : initOrder) {
                auto it = m_services.find(serviceName);
                if (it == m_services.end()) continue;
                
                ServiceInfo* info = it->second.get();
                
                if (info->initialized) {
                    std::shared_ptr<IService> service = info->singleton ? info->instance : nullptr;
                    if (service) {
                        service->Shutdown();
                    }
                    
                    info->initialized = false;
                }
            }
            
            // Clear instances
            m_instances.clear();
            m_initialized = false;
        }
        
        bool IsServiceRegistered(const std::string& name) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_services.find(name) != m_services.end();
        }
        
        bool IsServiceInitialized(const std::string& name) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_services.find(name);
            return (it != m_services.end()) && it->second->initialized;
        }
        
        std::vector<std::string> GetRegisteredServices() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            std::vector<std::string> services;
            for (const auto& pair : m_services) {
                services.push_back(pair.first);
            }
            
            return services;
        }
        
        std::vector<std::string> GetInitializationOrder() const {
            return m_dependencyGraph.GetInitializationOrder();
        }
        
        DependencyGraph& GetDependencyGraph() {
            return m_dependencyGraph;
        }
        
        bool IsInitialized() const {
            return m_initialized;
        }
    };
    
    // Service locator for easy access to services
    class ServiceLocator {
    private:
        static std::unique_ptr<ServiceContainer> s_container;
        static std::mutex s_mutex;
        
    public:
        static void Initialize(std::unique_ptr<ServiceContainer> container) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_container = std::move(container);
        }
        
        static ServiceContainer& GetContainer() {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_container) {
                throw std::runtime_error("Service container not initialized");
            }
            return *s_container;
        }
        
        static std::shared_ptr<IService> GetService(const std::string& name) {
            return GetContainer().GetService(name);
        }
        
        template<typename T>
        static std::shared_ptr<T> GetService(const std::string& name) {
            return GetContainer().GetService<T>(name);
        }
        
        static bool InitializeServices() {
            return GetContainer().InitializeServices();
        }
        
        static void ShutdownServices() {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_container) {
                s_container->ShutdownServices();
            }
        }
        
        static bool IsServiceRegistered(const std::string& name) {
            std::lock_guard<std::mutex> lock(s_mutex);
            return s_container && s_container->IsServiceRegistered(name);
        }
        
        static bool IsServiceInitialized(const std::string& name) {
            std::lock_guard<std::mutex> lock(s_mutex);
            return s_container && s_container->IsServiceInitialized(name);
        }
    };
    
    // Static member definitions
    std::unique_ptr<ServiceContainer> ServiceLocator::s_container = nullptr;
    std::mutex ServiceLocator::s_mutex;
    
    // Base service class for easy implementation
    class BaseService : public IService {
    protected:
        std::string m_name;
        bool m_initialized;
        
    public:
        BaseService(const std::string& name) : m_name(name), m_initialized(false) {}
        
        const char* GetServiceName() const override {
            return m_name.c_str();
        }
        
        bool IsInitialized() const override {
            return m_initialized;
        }
        
        bool Initialize() override {
            if (m_initialized) return true;
            
            try {
                if (OnInitialize()) {
                    m_initialized = true;
                    LOG_INFO("Service initialized: " + m_name);
                    return true;
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("Service initialization failed: " + m_name + " - " + e.what());
            }
            
            return false;
        }
        
        void Shutdown() override {
            if (!m_initialized) return;
            
            try {
                OnShutdown();
                m_initialized = false;
                LOG_INFO("Service shutdown: " + m_name);
            }
            catch (const std::exception& e) {
                LOG_ERROR("Service shutdown failed: " + m_name + " - " + e.what());
            }
        }
        
    protected:
        virtual bool OnInitialize() = 0;
        virtual void OnShutdown() = 0;
    };
    
    // Dependency-aware service base class
    class DependencyAwareService : public BaseService {
    protected:
        std::vector<std::string> m_dependencies;
        
    public:
        DependencyAwareService(const std::string& name, const std::vector<std::string>& dependencies = {})
            : BaseService(name), m_dependencies(dependencies) {}
        
        const std::vector<std::string>& GetDependencies() const {
            return m_dependencies;
        }
        
    protected:
        template<typename T>
        std::shared_ptr<T> GetDependency(const std::string& serviceName) {
            return ServiceLocator::GetService<T>(serviceName);
        }
        
        bool OnInitialize() override {
            // Check if all dependencies are available and initialized
            for (const auto& dep : m_dependencies) {
                if (!ServiceLocator::IsServiceRegistered(dep)) {
                    LOG_ERROR("Dependency not registered: " + dep + " for service: " + m_name);
                    return false;
                }
                
                if (!ServiceLocator::IsServiceInitialized(dep)) {
                    LOG_ERROR("Dependency not initialized: " + dep + " for service: " + m_name);
                    return false;
                }
            }
            
            return OnInitializeWithDependencies();
        }
        
        virtual bool OnInitializeWithDependencies() = 0;
    };
    
} // namespace DI

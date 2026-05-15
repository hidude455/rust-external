/*
 * Plugin Manager Implementation for Rust Anti-Cheat Evasion System
 * Modular architecture for extensibility
 * Author: Anonymous
 * Last Modified: 2026
 */

#include "PluginManager.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace Plugin {
    
    CPluginManager::CPluginManager() 
        : m_isInitialized(false), m_pluginsDirectory("plugins/"), 
          m_configDirectory("plugins/config/") {
        
        // Initialize default configuration
        m_config.pluginPath = "plugins/";
        m_config.configPath = "plugins/config/";
        m_config.autoLoad = true;
        m_config.autoUpdate = false;
        m_config.allowBeta = false;
        m_config.requireSignature = true;
        m_config.sandboxMode = false;
        
        // Create directories
        std::filesystem::create_directories(m_pluginsDirectory);
        std::filesystem::create_directories(m_configDirectory);
        
        // Register built-in plugins
        RegisterBuiltInPlugins();
    }
    
    CPluginManager::~CPluginManager() {
        if (m_isInitialized) {
            Shutdown();
        }
    }
    
    bool CPluginManager::Initialize(const PluginConfig& config) {
        std::cout << "Initializing Plugin Manager..." << std::endl;
        
        m_config = config;
        
        // Create directories
        std::filesystem::create_directories(m_pluginsDirectory);
        std::filesystem::create_directories(m_configDirectory);
        
        // Load plugin configuration
        LoadPluginConfiguration();
        
        // Scan for plugins
        ScanPluginsDirectory();
        
        // Load auto-load plugins
        if (m_config.autoLoad) {
            std::cout << "Loading auto-load plugins..." << std::endl;
            for (const auto& pair : m_availablePlugins) {
                const PluginInfo& info = pair.second;
                if (!IsPluginBlacklisted(info.name) && !IsPluginTrusted(info.name)) {
                    LoadPlugin(info.name);
                }
            }
        }
        
        // Register event handlers
        RegisterEventHandlers();
        
        m_isInitialized = true;
        std::cout << "[SUCCESS] Plugin Manager initialized with " << m_loadedPlugins.size() << " plugins loaded" << std::endl;
        return true;
    }
    
    void CPluginManager::Shutdown() {
        std::cout << "Shutting down Plugin Manager..." << std::endl;
        
        // Unload all plugins
        for (auto& pair : m_loadedPlugins) {
            UnloadPluginInternal(pair.first);
        }
        
        // Save configuration
        SavePluginConfiguration();
        
        // Cleanup
        m_loadedPlugins.clear();
        m_availablePlugins.clear();
        m_eventHistory.clear();
        m_eventHandlers.clear();
        
        m_isInitialized = false;
        std::cout << "[SUCCESS] Plugin Manager shutdown complete" << std::endl;
    }
    
    bool CPluginManager::IsInitialized() const {
        return m_isInitialized;
    }
    
    bool CPluginManager::LoadPlugin(const std::string& pluginName) {
        std::cout << "Loading plugin: " << pluginName << std::endl;
        
        // Check if plugin is already loaded
        if (m_loadedPlugins.find(pluginName) != m_loadedPlugins.end()) {
            std::cout << "[WARNING] Plugin already loaded: " << pluginName << std::endl;
            return true;
        }
        
        // Check if plugin is blacklisted
        if (IsPluginBlacklisted(pluginName)) {
            std::cout << "[ERROR] Plugin is blacklisted: " << pluginName << std::endl;
            return false;
        }
        
        // Find plugin info
        auto it = m_availablePlugins.find(pluginName);
        if (it == m_availablePlugins.end()) {
            std::cout << "[ERROR] Plugin not found: " << pluginName << std::endl;
            return false;
        }
        
        const PluginInfo& info = it->second;
        
        // Check dependencies
        if (!CheckDependencies(info.dependencies)) {
            std::cout << "[ERROR] Plugin dependencies not satisfied: " << pluginName << std::endl;
            return false;
        }
        
        // Load plugin
        if (!LoadPluginFromFile(info.filePath)) {
            std::cout << "[ERROR] Failed to load plugin: " << pluginName << std::endl;
            return false;
        }
        
        // Trigger event
        TriggerPluginEvent("PLUGIN_LOADED", pluginName);
        
        std::cout << "[SUCCESS] Plugin loaded: " << pluginName << std::endl;
        return true;
    }
    
    bool CPluginManager::LoadPluginFromFile(const std::string& pluginName, const std::string& filePath) {
        std::cout << "Loading plugin from file: " << filePath << std::endl;
        
        // Validate plugin
        if (!ValidatePlugin(filePath)) {
            std::cout << "[ERROR] Plugin validation failed: " << filePath << std::endl;
            return false;
        }
        
        // Verify signature if required
        if (m_config.requireSignature && !VerifyPluginSignature(filePath)) {
            std::cout << "[ERROR] Plugin signature verification failed: " << filePath << std::endl;
            return false;
        }
        
        // Load plugin library
        HMODULE hModule = LoadLibraryA(filePath.c_str());
        if (!hModule) {
            std::cout << "[ERROR] Failed to load plugin library: " << filePath << std::endl;
            return false;
        }
        
        // Get plugin creation function
        typedef IPlugin* (*CreatePluginFunc)();
        CreatePluginFunc createPlugin = reinterpret_cast<CreatePluginFunc>(GetProcAddress(hModule, "CreatePlugin"));
        
        if (!createPlugin) {
            std::cout << "[ERROR] Plugin creation function not found: " << filePath << std::endl;
            FreeLibrary(hModule);
            return false;
        }
        
        // Create plugin instance
        IPlugin* plugin = createPlugin();
        if (!plugin) {
            std::cout << "[ERROR] Failed to create plugin instance: " << filePath << std::endl;
            FreeLibrary(hModule);
            return false;
        }
        
        // Initialize plugin
        if (!plugin->Initialize()) {
            std::cout << "[ERROR] Plugin initialization failed: " << pluginName << std::endl;
            delete plugin;
            FreeLibrary(hModule);
            return false;
        }
        
        // Store plugin
        m_loadedPlugins[pluginName] = std::shared_ptr<IPlugin>(plugin);
        
        // Update plugin info
        auto it = m_availablePlugins.find(pluginName);
        if (it != m_availablePlugins.end()) {
            it->second.loadTime = GetTickCount64();
        }
        
        // Load plugin configuration
        LoadPluginConfig(pluginName);
        
        // Log plugin event
        LogPluginEvent(pluginName, "LOADED", "Plugin loaded successfully");
        
        std::cout << "[SUCCESS] Plugin loaded from file: " << filePath << std::endl;
        return true;
    }
    
    bool CPluginManager::UnloadPlugin(const std::string& pluginName) {
        std::cout << "Unloading plugin: " << pluginName << std::endl;
        
        if (UnloadPluginInternal(pluginName)) {
            // Trigger event
            TriggerPluginEvent("PLUGIN_UNLOADED", pluginName);
            
            // Log plugin event
            LogPluginEvent(pluginName, "UNLOADED", "Plugin unloaded successfully");
            
            std::cout << "[SUCCESS] Plugin unloaded: " << pluginName << std::endl;
            return true;
        }
        
        std::cout << "[ERROR] Failed to unload plugin: " << pluginName << std::endl;
        return false;
    }
    
    bool CPluginManager::ReloadPlugin(const std::string& pluginName) {
        std::cout << "Reloading plugin: " << pluginName << std::endl;
        
        if (UnloadPlugin(pluginName)) {
            return LoadPlugin(pluginName);
        }
        
        return false;
    }
    
    bool CPluginManager::EnablePlugin(const std::string& pluginName) {
        auto it = m_loadedPlugins.find(pluginName);
        if (it != m_loadedPlugins.end()) {
            if (it->second->Enable()) {
                TriggerPluginEvent("PLUGIN_ENABLED", pluginName);
                LogPluginEvent(pluginName, "ENABLED", "Plugin enabled successfully");
                std::cout << "[SUCCESS] Plugin enabled: " << pluginName << std::endl;
                return true;
            }
        }
        
        std::cout << "[ERROR] Failed to enable plugin: " << pluginName << std::endl;
        return false;
    }
    
    bool CPluginManager::DisablePlugin(const std::string& pluginName) {
        auto it = m_loadedPlugins.find(pluginName);
        if (it != m_loadedPlugins.end()) {
            if (it->second->Disable()) {
                TriggerPluginEvent("PLUGIN_DISABLED", pluginName);
                LogPluginEvent(pluginName, "DISABLED", "Plugin disabled successfully");
                std::cout << "[SUCCESS] Plugin disabled: " << pluginName << std::endl;
                return true;
            }
        }
        
        std::cout << "[ERROR] Failed to disable plugin: " << pluginName << std::endl;
        return false;
    }
    
    std::vector<std::string> CPluginManager::GetLoadedPlugins() const {
        std::vector<std::string> plugins;
        
        for (const auto& pair : m_loadedPlugins) {
            plugins.push_back(pair.first);
        }
        
        return plugins;
    }
    
    std::vector<std::string> CPluginManager::GetAvailablePlugins() const {
        std::vector<std::string> plugins;
        
        for (const auto& pair : m_availablePlugins) {
            plugins.push_back(pair.first);
        }
        
        return plugins;
    }
    
    std::vector<std::string> CPluginManager::GetEnabledPlugins() const {
        std::vector<std::string> enabledPlugins;
        
        for (const auto& pair : m_loadedPlugins) {
            if (pair.second->IsEnabled()) {
                enabledPlugins.push_back(pair.first);
            }
        }
        
        return enabledPlugins;
    }
    
    PluginInfo CPluginManager::GetPluginInfo(const std::string& pluginName) const {
        auto it = m_availablePlugins.find(pluginName);
        if (it != m_availablePlugins.end()) {
            return it->second;
        }
        
        return PluginInfo{};
    }
    
    PluginStatus CPluginManager::GetPluginStatus(const std::string& pluginName) const {
        auto it = m_loadedPlugins.find(pluginName);
        if (it != m_loadedPlugins.end()) {
            return it->second->GetStatus();
        }
        
        return PluginStatus::Unloaded;
    }
    
    std::string CPluginManager::GetLastError() const {
        return m_lastError;
    }
    
    bool CPluginManager::InstallPlugin(const std::string& pluginFile) {
        std::cout << "Installing plugin: " << pluginFile << std::endl;
        
        // Validate plugin file
        if (!ValidatePlugin(pluginFile)) {
            std::cout << "[ERROR] Plugin validation failed: " << pluginFile << std::endl;
            return false;
        }
        
        // Copy plugin to plugins directory
        std::filesystem::path sourcePath(pluginFile);
        std::string pluginName = sourcePath.filename().string();
        std::string destPath = m_pluginsDirectory + pluginName;
        
        try {
            std::filesystem::copy_file(pluginFile, destPath, std::filesystem::copy_options::overwrite_existing);
        } catch (const std::exception& e) {
            std::cout << "[ERROR] Failed to copy plugin: " << e.what() << std::endl;
            return false;
        }
        
        // Rescan plugins directory
        ScanPluginsDirectory();
        
        std::cout << "[SUCCESS] Plugin installed: " << pluginName << std::endl;
        return true;
    }
    
    bool CPluginManager::UninstallPlugin(const std::string& pluginName) {
        std::cout << "Uninstalling plugin: " << pluginName << std::endl;
        
        // Unload plugin if loaded
        if (m_loadedPlugins.find(pluginName) != m_loadedPlugins.end()) {
            UnloadPlugin(pluginName);
        }
        
        // Remove plugin file
        auto it = m_availablePlugins.find(pluginName);
        if (it != m_availablePlugins.end()) {
            try {
                std::filesystem::remove(it->second.filePath);
            } catch (const std::exception& e) {
                std::cout << "[ERROR] Failed to remove plugin file: " << e.what() << std::endl;
                return false;
            }
        }
        
        // Remove from available plugins
        m_availablePlugins.erase(pluginName);
        
        // Remove configuration
        std::string configPath = m_configDirectory + pluginName + ".ini";
        try {
            std::filesystem::remove(configPath);
        } catch (const std::exception& e) {
            std::cout << "[WARNING] Failed to remove plugin config: " << e.what() << std::endl;
        }
        
        std::cout << "[SUCCESS] Plugin uninstalled: " << pluginName << std::endl;
        return true;
    }
    
    void CPluginManager::UpdateAllPlugins() {
        for (auto& pair : m_loadedPlugins) {
            try {
                pair.second->Update();
            } catch (const std::exception& e) {
                std::cout << "[ERROR] Plugin update failed: " << pair.first << " - " << e.what() << std::endl;
                LogPluginEvent(pair.first, "UPDATE_ERROR", e.what());
            }
        }
    }
    
    void CPluginManager::RenderAllPlugins() {
        for (auto& pair : m_loadedPlugins) {
            try {
                if (pair.second->IsEnabled()) {
                    pair.second->Render();
                }
            } catch (const std::exception& e) {
                std::cout << "[ERROR] Plugin render failed: " << pair.first << " - " << e.what() << std::endl;
                LogPluginEvent(pair.first, "RENDER_ERROR", e.what());
            }
        }
    }
    
    void CPluginManager::RegisterEventHandler(const std::string& eventType, std::function<void(const PluginEvent&)> handler) {
        m_eventHandlers[eventType].push_back(handler);
    }
    
    void CPluginManager::TriggerPluginEvent(const std::string& eventType, const std::string& data) {
        PluginEvent event;
        event.eventType = eventType;
        event.data = data;
        event.timestamp = GetTickCount64();
        
        TriggerEvent(event);
    }
    
    std::vector<PluginEvent> CPluginManager::GetEventHistory() const {
        return m_eventHistory;
    }
    
    void CPluginManager::ClearEventHistory() {
        m_eventHistory.clear();
    }
    
    bool CPluginManager::VerifyPluginSignature(const std::string& pluginFile) {
        std::cout << "Verifying plugin signature: " << pluginFile << std::endl;
        
        // This would verify digital signature of plugin
        // For now, we'll just return true as a placeholder
        std::cout << "[INFO] Plugin signature verification not implemented" << std::endl;
        return true;
    }
    
    bool CPluginManager::IsPluginTrusted(const std::string& pluginName) const {
        return std::find(m_config.trustedPlugins.begin(), m_config.trustedPlugins.end(), pluginName) != m_config.trustedPlugins.end();
    }
    
    bool CPluginManager::IsPluginBlacklisted(const std::string& pluginName) const {
        return std::find(m_config.blacklistedPlugins.begin(), m_config.blacklistedPlugins.end(), pluginName) != m_config.blacklistedPlugins.end();
    }
    
    void CPluginManager::AddTrustedPlugin(const std::string& pluginName) {
        m_config.trustedPlugins.push_back(pluginName);
        SavePluginConfiguration();
    }
    
    void CPluginManager::RemoveTrustedPlugin(const std::string& pluginName) {
        auto it = std::find(m_config.trustedPlugins.begin(), m_config.trustedPlugins.end(), pluginName);
        if (it != m_config.trustedPlugins.end()) {
            m_config.trustedPlugins.erase(it);
            SavePluginConfiguration();
        }
    }
    
    // Private methods
    bool CPluginManager::UnloadPluginInternal(const std::string& pluginName) {
        auto it = m_loadedPlugins.find(pluginName);
        if (it != m_loadedPlugins.end()) {
            // Shutdown plugin
            it->second->Shutdown();
            
            // Remove from loaded plugins
            m_loadedPlugins.erase(it);
            
            return true;
        }
        
        return false;
    }
    
    bool CPluginManager::ValidatePlugin(const std::string& filePath) {
        std::cout << "Validating plugin: " << filePath << std::endl;
        
        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            std::cout << "[ERROR] Plugin file does not exist: " << filePath << std::endl;
            return false;
        }
        
        // Check file extension
        std::filesystem::path path(filePath);
        if (path.extension().string() != ".dll") {
            std::cout << "[ERROR] Invalid plugin file extension: " << filePath << std::endl;
            return false;
        }
        
        // Check file size
        uint64_t fileSize = std::filesystem::file_size(filePath);
        if (fileSize == 0) {
            std::cout << "[ERROR] Plugin file is empty: " << filePath << std::endl;
            return false;
        }
        
        // Check if it's a valid PE file
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "[ERROR] Cannot open plugin file: " << filePath << std::endl;
            return false;
        }
        
        // Check PE signature
        uint32_t signature;
        file.read(reinterpret_cast<char*>(&signature), 2);
        file.close();
        
        if (signature != 0x5A4D) { // MZ signature
            std::cout << "[ERROR] Invalid PE file format: " << filePath << std::endl;
            return false;
        }
        
        std::cout << "[SUCCESS] Plugin validation passed: " << filePath << std::endl;
        return true;
    }
    
    void CPluginManager::ScanPluginsDirectory() {
        std::cout << "Scanning plugins directory: " << m_pluginsDirectory << std::endl;
        
        m_availablePlugins.clear();
        
        try {
            for (const auto& entry : std::filesystem::directory_iterator(m_pluginsDirectory)) {
                if (entry.path().extension().string() == ".dll") {
                    std::string filePath = entry.path().string();
                    std::string fileName = entry.path().filename().string();
                    std::string pluginName = fileName.substr(0, fileName.find_last_of('.'));
                    
                    // Create plugin info
                    PluginInfo info;
                    info.name = pluginName;
                    info.version = "1.0.0";
                    info.author = "Unknown";
                    info.description = "Plugin loaded from file";
                    info.type = PluginType::Unknown;
                    info.filePath = filePath;
                    info.dependencies = "";
                    info.apiVersion = "1.0.0";
                    info.isBuiltIn = false;
                    info.isBeta = false;
                    info.loadTime = 0;
                    info.lastUsed = 0;
                    
                    m_availablePlugins[pluginName] = info;
                }
            }
        } catch (const std::exception& e) {
            std::cout << "[ERROR] Failed to scan plugins directory: " << e.what() << std::endl;
        }
        
        std::cout << "[SUCCESS] Found " << m_availablePlugins.size() << " plugins" << std::endl;
    }
    
    void CPluginManager::LoadPluginConfiguration() {
        std::cout << "Loading plugin configuration..." << std::endl;
        
        std::ifstream configFile("plugins.ini");
        if (!configFile.is_open()) {
            std::cout << "[WARNING] Plugin config file not found, using defaults" << std::endl;
            return;
        }
        
        std::string line;
        while (std::getline(configFile, line)) {
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
                if (key == "autoLoad") {
                    m_config.autoLoad = (value == "true");
                } else if (key == "autoUpdate") {
                    m_config.autoUpdate = (value == "true");
                } else if (key == "allowBeta") {
                    m_config.allowBeta = (value == "true");
                } else if (key == "requireSignature") {
                    m_config.requireSignature = (value == "true");
                } else if (key == "sandboxMode") {
                    m_config.sandboxMode = (value == "true");
                }
            }
        }
        
        configFile.close();
        std::cout << "[SUCCESS] Plugin configuration loaded" << std::endl;
    }
    
    void CPluginManager::SavePluginConfiguration() {
        std::cout << "Saving plugin configuration..." << std::endl;
        
        std::ofstream configFile("plugins.ini");
        if (!configFile.is_open()) {
            std::cout << "[ERROR] Failed to save plugin configuration" << std::endl;
            return;
        }
        
        configFile << "# Rust Anti-Cheat Evasion System Plugin Configuration" << std::endl;
        configFile << "# Generated: " << GetCurrentTimestamp() << std::endl;
        configFile << std::endl;
        
        configFile << "autoLoad=" << (m_config.autoLoad ? "true" : "false") << std::endl;
        configFile << "autoUpdate=" << (m_config.autoUpdate ? "true" : "false") << std::endl;
        configFile << "allowBeta=" << (m_config.allowBeta ? "true" : "false") << std::endl;
        configFile << "requireSignature=" << (m_config.requireSignature ? "true" : "false") << std::endl;
        configFile << "sandboxMode=" << (m_config.sandboxMode ? "true" : "false") << std::endl;
        
        // Save trusted plugins
        configFile << std::endl << "# Trusted Plugins" << std::endl;
        for (const auto& plugin : m_config.trustedPlugins) {
            configFile << "trusted=" << plugin << std::endl;
        }
        
        // Save blacklisted plugins
        configFile << std::endl << "# Blacklisted Plugins" << std::endl;
        for (const auto& plugin : m_config.blacklistedPlugins) {
            configFile << "blacklisted=" << plugin << std::endl;
        }
        
        configFile.close();
        std::cout << "[SUCCESS] Plugin configuration saved" << std::endl;
    }
    
    void CPluginManager::RegisterEventHandlers() {
        // Register internal event handlers
        RegisterEventHandler("PLUGIN_LOADED", [this](const PluginEvent& event) {
            std::cout << "[EVENT] Plugin loaded: " << event.data << std::endl;
        });
        
        RegisterEventHandler("PLUGIN_UNLOADED", [this](const PluginEvent& event) {
            std::cout << "[EVENT] Plugin unloaded: " << event.data << std::endl;
        });
        
        RegisterEventHandler("PLUGIN_ERROR", [this](const PluginEvent& event) {
            std::cout << "[EVENT] Plugin error: " << event.data << std::endl;
        });
    }
    
    void CPluginManager::TriggerEvent(const PluginEvent& event) {
        // Add to event history
        m_eventHistory.push_back(event);
        
        // Limit event history size
        if (m_eventHistory.size() > 1000) {
            m_eventHistory.erase(m_eventHistory.begin());
        }
        
        // Trigger event handlers
        auto it = m_eventHandlers.find(event.eventType);
        if (it != m_eventHandlers.end()) {
            for (const auto& handler : it->second) {
                try {
                    handler(event);
                } catch (const std::exception& e) {
                    std::cout << "[ERROR] Event handler error: " << e.what() << std::endl;
                }
            }
        }
    }
    
    bool CPluginManager::CheckDependencies(const std::string& dependencies) {
        if (dependencies.empty()) {
            return true;
        }
        
        std::stringstream ss(dependencies);
        std::string dependency;
        
        while (std::getline(ss, dependency, ',')) {
            // Trim whitespace
            dependency.erase(0, dependency.find_first_not_of(" \t\n\r"));
            dependency.erase(dependency.find_last_not_of(" \t\n\r") + 1);
            
            // Check if dependency is loaded
            if (m_loadedPlugins.find(dependency) == m_loadedPlugins.end()) {
                std::cout << "[ERROR] Missing dependency: " << dependency << std::endl;
                return false;
            }
        }
        
        return true;
    }
    
    void CPluginManager::LogPluginEvent(const std::string& pluginName, const std::string& event, const std::string& data) {
        std::cout << "[PLUGIN] " << pluginName << " - " << event;
        if (!data.empty()) {
            std::cout << " - " << data;
        }
        std::cout << std::endl;
    }
    
    void CPluginManager::RegisterBuiltInPlugins() {
        std::cout << "Registering built-in plugins..." << std::endl;
        
        // Register built-in ESP plugin
        PluginInfo espPlugin;
        espPlugin.name = "ESP";
        espPlugin.version = "1.0.0";
        espPlugin.author = "System";
        espPlugin.description = "Built-in ESP system";
        espPlugin.type = PluginType::ESP;
        espPlugin.filePath = "";
        espPlugin.dependencies = "";
        espPlugin.apiVersion = "1.0.0";
        espPlugin.isBuiltIn = true;
        espPlugin.isBeta = false;
        espPlugin.loadTime = 0;
        espPlugin.lastUsed = 0;
        
        m_availablePlugins["ESP"] = espPlugin;
        
        // Register built-in Aimbot plugin
        PluginInfo aimbotPlugin;
        aimbotPlugin.name = "Aimbot";
        aimbotPlugin.version = "1.0.0";
        aimbotPlugin.author = "System";
        aimbotPlugin.description = "Built-in aimbot system";
        aimbotPlugin.type = PluginType::Aimbot;
        aimbotPlugin.filePath = "";
        aimbotPlugin.dependencies = "";
        aimbotPlugin.apiVersion = "1.0.0";
        aimbotPlugin.isBuiltIn = true;
        aimbotPlugin.isBeta = false;
        aimbotPlugin.loadTime = 0;
        aimbotPlugin.lastUsed = 0;
        
        m_availablePlugins["Aimbot"] = aimbotPlugin;
        
        std::cout << "[SUCCESS] Built-in plugins registered" << std::endl;
    }
    
    std::string CPluginManager::GetCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d_%H-%M-%S");
        
        return ss.str();
    }
}

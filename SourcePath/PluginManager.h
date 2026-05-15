/*
 * Plugin Manager for Rust Anti-Cheat Evasion System
 * Modular architecture for extensibility
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace Plugin {
    
    // Plugin types
    enum class PluginType {
        ESP,
        Aimbot,
        VPN,
        HardwareSpoofer,
        NetworkBypass,
        Protection,
        UI,
        Utility,
        Unknown
    };
    
    // Plugin status
    enum class PluginStatus {
        Unloaded,
        Loading,
        Loaded,
        Initializing,
        Running,
        Error,
        Disabled
    };
    
    // Plugin information
    struct PluginInfo {
        std::string name;
        std::string version;
        std::string author;
        std::string description;
        PluginType type;
        std::string filePath;
        std::string dependencies;
        std::string apiVersion;
        bool isBuiltIn;
        bool isBeta;
        uint64_t loadTime;
        uint64_t lastUsed;
    };
    
    // Plugin interface
    class IPlugin {
    public:
        virtual ~IPlugin() = default;
        
        // Plugin lifecycle
        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual bool IsInitialized() const = 0;
        
        // Plugin information
        virtual PluginInfo GetInfo() const = 0;
        virtual PluginStatus GetStatus() const = 0;
        virtual std::string GetLastError() const = 0;
        
        // Plugin functionality
        virtual bool Enable() = 0;
        virtual bool Disable() = 0;
        virtual bool IsEnabled() const = 0;
        virtual void Update() = 0;
        virtual void Render() = 0;
        
        // Configuration
        virtual bool LoadConfiguration() = 0;
        virtual bool SaveConfiguration() = 0;
        virtual void ShowSettings() = 0;
        
        // Events
        virtual void OnGameStart() {}
        virtual void OnGameEnd() {}
        virtual void OnPlayerJoin(const std::string& playerName) {}
        virtual void OnPlayerLeave(const std::string& playerName) {}
        virtual void OnKeyPressed(int keyCode) {}
        virtual void OnConfigurationChanged(const std::string& key, const std::string& value) {}
    };
    
    // Plugin configuration
    struct PluginConfig {
        std::string pluginPath;
        std::string configPath;
        bool autoLoad;
        bool autoUpdate;
        bool allowBeta;
        std::vector<std::string> blacklistedPlugins;
        std::vector<std::string> trustedPlugins;
        bool requireSignature;
        bool sandboxMode;
    };
    
    // Plugin event
    struct PluginEvent {
        std::string eventType;
        std::string pluginName;
        std::string data;
        uint64_t timestamp;
    };
    
    class CPluginManager {
    private:
        // Plugin storage
        std::map<std::string, std::shared_ptr<IPlugin>> m_loadedPlugins;
        std::map<std::string, PluginInfo> m_availablePlugins;
        std::vector<PluginEvent> m_eventHistory;
        
        // Configuration
        PluginConfig m_config;
        std::string m_pluginsDirectory;
        std::string m_configDirectory;
        
        // State
        bool m_isInitialized;
        std::string m_lastError;
        
        // Event handlers
        std::map<std::string, std::vector<std::function<void(const PluginEvent&)>>> m_eventHandlers;
        
        // Private methods
        bool LoadPluginFromFile(const std::string& filePath);
        bool UnloadPluginInternal(const std::string& pluginName);
        bool ValidatePlugin(const std::string& filePath);
        void ScanPluginsDirectory();
        void LoadPluginConfiguration();
        void SavePluginConfiguration();
        void RegisterEventHandlers();
        void TriggerEvent(const PluginEvent& event);
        bool CheckDependencies(const std::string& dependencies);
        void LogPluginEvent(const std::string& pluginName, const std::string& event, const std::string& data = "");
        
    public:
        CPluginManager();
        ~CPluginManager();
        
        // Initialization
        bool Initialize(const PluginConfig& config);
        void Shutdown();
        bool IsInitialized() const;
        
        // Plugin management
        bool LoadPlugin(const std::string& pluginName);
        bool LoadPluginFromFile(const std::string& pluginName, const std::string& filePath);
        bool UnloadPlugin(const std::string& pluginName);
        bool ReloadPlugin(const std::string& pluginName);
        bool EnablePlugin(const std::string& pluginName);
        bool DisablePlugin(const std::string& pluginName);
        std::vector<std::string> GetLoadedPlugins() const;
        std::vector<std::string> GetAvailablePlugins() const;
        std::vector<std::string> GetEnabledPlugins() const;
        
        // Plugin information
        PluginInfo GetPluginInfo(const std::string& pluginName) const;
        PluginStatus GetPluginStatus(const std::string& pluginName) const;
        std::string GetLastError() const;
        
        // Plugin search and discovery
        bool InstallPlugin(const std::string& pluginFile);
        bool UninstallPlugin(const std::string& pluginName);
        bool UpdatePlugin(const std::string& pluginName);
        bool SearchForPlugins(const std::string& searchTerm);
        std::vector<PluginInfo> GetPluginUpdates() const;
        
        // Configuration
        PluginConfig GetConfig() const;
        void SetConfig(const PluginConfig& config);
        bool LoadPluginConfig(const std::string& pluginName);
        bool SavePluginConfig(const std::string& pluginName);
        
        // Event system
        void RegisterEventHandler(const std::string& eventType, std::function<void(const PluginEvent&)> handler);
        void UnregisterEventHandler(const std::string& eventType, std::function<void(const PluginEvent&)> handler);
        void TriggerPluginEvent(const std::string& eventType, const std::string& data = "");
        std::vector<PluginEvent> GetEventHistory() const;
        void ClearEventHistory();
        
        // Plugin communication
        bool SendPluginMessage(const std::string& fromPlugin, const std::string& toPlugin, const std::string& message);
        std::vector<std::string> GetPluginMessages(const std::string& pluginName) const;
        void ClearPluginMessages(const std::string& pluginName);
        
        // Security and validation
        bool VerifyPluginSignature(const std::string& pluginFile);
        bool IsPluginTrusted(const std::string& pluginName);
        bool IsPluginBlacklisted(const std::string& pluginName);
        void AddTrustedPlugin(const std::string& pluginName);
        void RemoveTrustedPlugin(const std::string& pluginName);
        void AddBlacklistedPlugin(const std::string& pluginName);
        void RemoveBlacklistedPlugin(const std::string& pluginName);
        
        // Performance monitoring
        void UpdateAllPlugins();
        void RenderAllPlugins();
        std::map<std::string, float> GetPluginPerformanceMetrics() const;
        std::map<std::string, uint64_t> GetPluginUsageStatistics() const;
        
        // Advanced features
        bool EnableSandboxMode(bool enable);
        bool IsSandboxModeEnabled() const;
        bool CreatePluginSandbox(const std::string& pluginName);
        void DestroyPluginSandbox(const std::string& pluginName);
        std::vector<std::string> GetSandboxedPlugins() const;
        
        // Debugging
        void EnablePluginDebugMode(const std::string& pluginName, bool enable);
        bool IsPluginDebugModeEnabled(const std::string& pluginName) const;
        std::vector<std::string> GetPluginLogs(const std::string& pluginName) const;
        void ClearPluginLogs(const std::string& pluginName);
        
        // Hot reloading
        bool EnableHotReload(bool enable);
        bool IsHotReloadEnabled() const;
        void MonitorPluginFiles();
        
        // Plugin marketplace (future feature)
        bool BrowseMarketplace();
        bool DownloadFromMarketplace(const std::string& pluginId);
        std::vector<PluginInfo> GetMarketplacePlugins() const;
    };
}

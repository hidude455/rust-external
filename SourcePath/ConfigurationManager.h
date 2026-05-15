/*
 * Configuration Manager for Rust Anti-Cheat Evasion System
 * Centralized settings management with profile support
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <memory>

namespace Config {
    
    // Configuration value types
    enum class ConfigType {
        Boolean,
        Integer,
        Float,
        String,
        Color,
        Array
    };
    
    // Configuration entry structure
    struct ConfigEntry {
        std::string key;
        std::string category;
        ConfigType type;
        std::string value;
        std::string defaultValue;
        std::string description;
        bool isAdvanced;
        bool requiresRestart;
    };
    
    // Configuration profile structure
    struct ConfigProfile {
        std::string name;
        std::string description;
        std::map<std::string, std::string> settings;
        bool isActive;
        uint64_t createdTime;
        uint64_t lastUsed;
    };
    
    // Configuration validation result
    struct ValidationResult {
        bool isValid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        std::string summary;
    };
    
    class CConfigurationManager {
    private:
        // Configuration storage
        std::map<std::string, std::string> m_configValues;
        std::vector<ConfigEntry> m_configEntries;
        std::vector<ConfigProfile> m_profiles;
        std::string m_currentProfile;
        
        // File paths
        std::string m_configFile;
        std::string m_profilesFile;
        std::string m_backupFile;
        
        // Validation
        bool m_isValidated;
        std::string m_lastValidationResult;
        
        // Private methods
        void LoadDefaultConfiguration();
        void LoadConfigurationFromFile();
        void SaveConfigurationToFile();
        void LoadProfilesFromFile();
        void SaveProfilesToFile();
        bool ValidateConfiguration();
        void CreateBackup();
        void RestoreFromBackup();
        std::string GetConfigValue(const std::string& key);
        void SetConfigValue(const std::string& key, const std::string& value);
        
    public:
        CConfigurationManager();
        ~CConfigurationManager();
        
        // Initialization and cleanup
        bool Initialize();
        void Shutdown();
        bool IsInitialized() const;
        
        // Configuration management
        bool LoadConfiguration(const std::string& configFile = "rust_config.ini");
        bool SaveConfiguration(const std::string& configFile = "rust_config.ini");
        bool ResetToDefaults();
        
        // Profile management
        bool CreateProfile(const std::string& name, const std::string& description);
        bool DeleteProfile(const std::string& name);
        bool LoadProfile(const std::string& name);
        std::vector<std::string> GetAvailableProfiles() const;
        std::string GetCurrentProfile() const;
        bool SetCurrentProfile(const std::string& name);
        
        // Configuration access
        bool GetBool(const std::string& key, bool defaultValue = false);
        int GetInt(const std::string& key, int defaultValue = 0);
        float GetFloat(const std::string& key, float defaultValue = 0.0f);
        std::string GetString(const std::string& key, const std::string& defaultValue = "");
        ImVec4 GetColor(const std::string& key, const ImVec4& defaultValue = ImVec4(1,1,1,1));
        
        // Configuration modification
        bool SetBool(const std::string& key, bool value);
        bool SetInt(const std::string& key, int value);
        bool SetFloat(const std::string& key, float value);
        bool SetString(const std::string& key, const std::string& value);
        bool SetColor(const std::string& key, const ImVec4& value);
        
        // Validation
        ValidationResult ValidateCurrentConfiguration();
        ValidationResult ValidateProfile(const std::string& profileName);
        
        // Advanced features
        void EnableAdvancedMode(bool enable);
        bool IsAdvancedModeEnabled() const;
        void ExportConfiguration(const std::string& filePath);
        bool ImportConfiguration(const std::string& filePath);
        
        // Auto-save
        void EnableAutoSave(bool enable);
        bool IsAutoSaveEnabled() const;
        void ForceAutoSave();
        
        // Hotkey management
        bool RegisterHotkey(const std::string& action, int virtualKey);
        bool UnregisterHotkey(const std::string& action);
        std::map<std::string, int> GetRegisteredHotkeys() const;
        
        // Monitoring
        void LogConfigurationChange(const std::string& key, const std::string& oldValue, const std::string& newValue);
        std::vector<std::string> GetRecentChanges() const;
        void ClearRecentChanges();
        
        // Integration with other systems
        void ApplyConfigurationToESP();
        void ApplyConfigurationToAimbot();
        void ApplyConfigurationToVPN();
        void ApplyConfigurationToHardwareSpoofer();
        void ApplyConfigurationToNetworkBypass();
        void ApplyConfigurationToProtection();
        void ApplyConfigurationToAll();
    };
}

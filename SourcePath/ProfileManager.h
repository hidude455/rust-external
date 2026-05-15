/*
 * Profile Manager for Rust Anti-Cheat Evasion System
 * Complete user profile management with settings and preferences
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

namespace Profile {
    
    // Profile types
    enum class ProfileType {
        Default,
        Custom,
        Preset,
        Imported,
        Temporary
    };
    
    // Profile settings categories
    enum class ProfileCategory {
        General,
        ESP,
        Aimbot,
        VPN,
        HardwareSpoofer,
        NetworkBypass,
        Protection,
        Advanced,
        Custom
    };
    
    // Profile entry
    struct ProfileEntry {
        std::string key;
        std::string category;
        std::string value;
        std::string defaultValue;
        std::string description;
        bool isAdvanced;
        bool requiresRestart;
        ProfileCategory categoryEnum;
    };
    
    // User profile
    struct UserProfile {
        std::string name;
        std::string description;
        ProfileType type;
        uint64_t createdTime;
        uint64_t lastUsed;
        uint64_t usageTime;
        std::map<std::string, std::string> settings;
        std::vector<ProfileEntry> profileEntries;
        bool isActive;
        bool isReadOnly;
        std::string version;
        std::string author;
        std::vector<std::string> tags;
        std::map<std::string, std::string> metadata;
    };
    
    // Profile configuration
    struct ProfileConfig {
        std::string profilesDirectory;
        std::string activeProfile;
        bool autoSave;
        bool autoLoadLastProfile;
        int maxProfiles;
        bool enableProfileValidation;
        bool enableProfileBackup;
        bool enableProfileSharing;
        bool enableCloudProfiles;
        std::string cloudEndpoint;
        std::string cloudUsername;
        std::string cloudPassword;
        bool enableProfileEncryption;
        std::string encryptionKey;
        std::vector<std::string> defaultProfiles;
        bool enableProfileTemplates;
        std::vector<std::string> profileTemplates;
    };
    
    // Profile template
    struct ProfileTemplate {
        std::string name;
        std::string description;
        std::string category;
        std::vector<ProfileEntry> templateEntries;
        std::string author;
        std::string version;
        std::vector<std::string> tags;
        bool isBuiltIn;
    };
    
    // Profile import/export result
    struct ProfileResult {
        bool success;
        std::string message;
        std::string errorDetails;
        std::string profileName;
        uint64_t operationTime;
        std::vector<std::string> warnings;
    };
    
    class CProfileManager {
    private:
        // Configuration
        ProfileConfig m_config;
        
        // State
        bool m_isInitialized;
        std::string m_currentProfile;
        std::vector<UserProfile> m_profiles;
        std::map<std::string, UserProfile*> m_profileMap;
        
        // Templates
        std::vector<ProfileTemplate> m_templates;
        std::map<std::string, ProfileTemplate*> m_templateMap;
        
        // Callbacks
        std::function<void(const std::string&)> m_profileSwitchCallback;
        std::function<void(const UserProfile&)> m_profileCreatedCallback;
        std::function<void(const std::string&)> m_profileDeletedCallback;
        std::function<void(const ProfileResult&)> m_importExportCallback;
        
        // Private methods
        bool LoadProfilesFromDirectory();
        bool SaveProfilesToDirectory();
        bool LoadProfileFromFile(const std::string& filePath, UserProfile& profile);
        bool SaveProfileToFile(const UserProfile& profile, const std::string& filePath);
        void InitializeDefaultProfiles();
        void InitializeProfileTemplates();
        bool ValidateProfile(const UserProfile& profile);
        void UpdateProfileUsage(const std::string& profileName);
        std::string GenerateProfileId() const;
        void CreateProfileDirectory();
        std::string GetProfileFilePath(const std::string& profileName) const;
        void LoadProfileConfiguration();
        void SaveProfileConfiguration();
        
    public:
        CProfileManager();
        ~CProfileManager();
        
        // Initialization
        bool Initialize(const ProfileConfig& config);
        void Shutdown();
        bool IsInitialized() const;
        
        // Configuration
        bool LoadConfiguration(const std::string& configFile = "rust_profiles.ini");
        bool SaveConfiguration(const std::string& configFile = "rust_profiles.ini");
        ProfileConfig GetConfig() const;
        void SetConfig(const ProfileConfig& config);
        
        // Profile management
        ProfileResult CreateProfile(const std::string& name, const std::string& description = "");
        ProfileResult CreateProfileFromTemplate(const std::string& templateName, const std::string& profileName = "");
        ProfileResult DeleteProfile(const std::string& profileName);
        ProfileResult DuplicateProfile(const std::string& sourceProfile, const std::string& newName);
        ProfileResult RenameProfile(const std::string& oldName, const std::string& newName);
        ProfileResult SetProfileDescription(const std::string& profileName, const std::string& description);
        ProfileResult SetProfileTags(const std::string& profileName, const std::vector<std::string>& tags);
        
        // Profile switching
        ProfileResult SwitchToProfile(const std::string& profileName);
        ProfileResult SwitchToNextProfile();
        ProfileResult SwitchToPreviousProfile();
        ProfileResult LoadLastUsedProfile();
        std::string GetCurrentProfile() const;
        UserProfile* GetCurrentProfileObject() const;
        
        // Profile access
        std::vector<UserProfile> GetProfiles() const;
        std::vector<std::string> GetProfileNames() const;
        UserProfile* GetProfile(const std::string& profileName) const;
        bool ProfileExists(const std::string& profileName) const;
        int GetProfileCount() const;
        UserProfile* GetActiveProfile() const;
        
        // Profile settings
        ProfileResult SetProfileSetting(const std::string& profileName, const std::string& key, const std::string& value);
        std::string GetProfileSetting(const std::string& profileName, const std::string& key, const std::string& defaultValue = "") const;
        ProfileResult ResetProfileSettings(const std::string& profileName);
        ProfileResult ResetProfileToDefaults(const std::string& profileName);
        std::map<std::string, std::string> GetAllProfileSettings(const std::string& profileName) const;
        
        // Profile categories
        std::vector<ProfileEntry> GetProfileCategory(const std::string& profileName, ProfileCategory category) const;
        ProfileResult SetProfileCategory(const std::string& profileName, ProfileCategory category, const std::vector<ProfileEntry>& entries);
        
        // Import/Export
        ProfileResult ImportProfile(const std::string& filePath);
        ProfileResult ExportProfile(const std::string& profileName, const std::string& filePath);
        ProfileResult ExportAllProfiles(const std::string& directoryPath);
        ProfileResult ImportProfilesFromDirectory(const std::string& directoryPath);
        std::vector<std::string> GetSupportedImportFormats() const;
        std::vector<std::string> GetSupportedExportFormats() const;
        
        // Templates
        std::vector<ProfileTemplate> GetTemplates() const;
        std::vector<ProfileTemplate> GetTemplatesByCategory(const std::string& category) const;
        ProfileTemplate* GetTemplate(const std::string& templateName) const;
        ProfileResult CreateTemplate(const std::string& name, const std::string& description, const std::vector<ProfileEntry>& entries);
        ProfileResult DeleteTemplate(const std::string& templateName);
        ProfileResult UpdateTemplate(const std::string& templateName, const ProfileTemplate& template);
        
        // Cloud profiles
        ProfileResult UploadProfileToCloud(const std::string& profileName);
        ProfileResult DownloadProfileFromCloud(const std::string& profileName);
        std::vector<std::string> GetCloudProfileList();
        ProfileResult SyncCloudProfiles();
        bool IsCloudSyncEnabled() const;
        
        // Profile validation
        ProfileResult ValidateAllProfiles();
        std::vector<std::string> GetProfileValidationErrors(const std::string& profileName) const;
        ProfileResult RepairProfile(const std::string& profileName);
        ProfileResult OptimizeProfile(const std::string& profileName);
        
        // Profile backup
        ProfileResult BackupProfile(const std::string& profileName);
        ProfileResult RestoreProfile(const std::string& profileName);
        std::vector<std::string> GetProfileBackups() const;
        ProfileResult DeleteProfileBackup(const std::string& backupName);
        
        // Profile search and filtering
        std::vector<UserProfile> SearchProfiles(const std::string& searchTerm, 
                                             const std::vector<std::string>& tags = {},
                                             ProfileType type = ProfileType::Custom) const;
        std::vector<UserProfile> GetProfilesByType(ProfileType type) const;
        std::vector<UserProfile> GetProfilesByTag(const std::string& tag) const;
        std::vector<UserProfile> GetRecentlyUsedProfiles(int count = 5) const;
        
        // Profile statistics
        uint64_t GetProfileUsageTime(const std::string& profileName) const;
        std::map<std::string, uint64_t> GetAllProfileUsageTimes() const;
        std::vector<std::string> GetMostUsedProfiles(int count = 3) const;
        std::vector<std::string> GetLeastUsedProfiles(int count = 3) const;
        
        // Advanced features
        ProfileResult MergeProfiles(const std::vector<std::string>& profileNames, const std::string& newName);
        ProfileResult SplitProfile(const std::string& profileName, const std::vector<std::string>& splitNames);
        ProfileResult CloneProfile(const std::string& sourceProfile, const std::string& newName);
        ProfileResult SetProfileReadOnly(const std::string& profileName, bool readOnly);
        bool IsProfileReadOnly(const std::string& profileName) const;
        
        // Callbacks
        void SetProfileSwitchCallback(std::function<void(const std::string&)> callback);
        void SetProfileCreatedCallback(std::function<void(const UserProfile&)> callback);
        void SetProfileDeletedCallback(std::function<void(const std::string&)> callback);
        void SetImportExportCallback(std::function<void(const ProfileResult&)> callback);
        
        // Real-time updates
        bool EnableAutoSave(bool enable);
        bool IsAutoSaveEnabled() const;
        void ForceAutoSave();
        bool EnableAutoLoadLastProfile(bool enable);
        bool IsAutoLoadLastProfileEnabled() const;
        
        // Performance optimization
        void OptimizeProfileLoading();
        void SetMaxProfiles(int maxProfiles);
        int GetMaxProfiles() const;
        void EnableProfileValidation(bool enable);
        bool IsProfileValidationEnabled() const;
        
        // Debugging
        void EnableDebugMode(bool enable);
        bool IsDebugModeEnabled() const;
        void DumpProfile(const std::string& profileName) const;
        void DumpAllProfiles() const;
        ProfileResult TestProfileIntegrity(const std::string& profileName);
        
        // Integration with other systems
        void ApplyProfileToESP(const std::string& profileName);
        void ApplyProfileToAimbot(const std::string& profileName);
        void ApplyProfileToVPN(const std::string& profileName);
        void ApplyProfileToHardwareSpoofer(const std::string& profileName);
        void ApplyProfileToNetworkBypass(const std::string& profileName);
        void ApplyProfileToProtection(const std::string& profileName);
        void ApplyProfileToAll(const std::string& profileName);
        
        // Security
        bool EnableProfileEncryption(bool enable);
        bool IsProfileEncryptionEnabled() const;
        void SetEncryptionKey(const std::string& key);
        std::string GetEncryptionKey() const;
        bool EncryptProfile(const UserProfile& profile, std::string& encryptedData);
        bool DecryptProfile(const std::string& encryptedData, UserProfile& profile);
    };
}

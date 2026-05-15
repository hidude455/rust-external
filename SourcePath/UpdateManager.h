/*
 * Update Manager for Rust Anti-Cheat Evasion System
 * Automatic updates with version checking and rollback
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <functional>

namespace Update {
    
    // Update status
    enum class UpdateStatus {
        NotChecked,
        Checking,
        UpdateAvailable,
        UpToDate,
        Downloading,
        Installing,
        Installed,
        Failed,
        RolledBack
    };
    
    // Update information
    struct UpdateInfo {
        std::string version;
        std::string releaseDate;
        std::string description;
        std::string downloadUrl;
        uint64_t fileSize;
        std::string checksum;
        bool isCritical;
        bool isBeta;
        std::vector<std::string> changes;
    };
    
    // Update result
    struct UpdateResult {
        bool success;
        UpdateStatus status;
        std::string message;
        std::string errorDetails;
        std::string installedVersion;
        uint64_t downloadTime;
        uint64_t installTime;
    };
    
    // Update configuration
    struct UpdateConfig {
        bool autoCheckEnabled;
        int checkIntervalHours;
        std::string updateServer;
        std::string currentVersion;
        bool allowBetaUpdates;
        bool autoInstall;
        bool createBackup;
        bool requireConfirmation;
        std::string backupPath;
    };
    
    class CUpdateManager {
    private:
        // Update state
        UpdateStatus m_currentStatus;
        UpdateInfo m_availableUpdate;
        UpdateConfig m_config;
        std::string m_currentVersion;
        std::string m_updatePath;
        std::string m_backupPath;
        std::string m_tempPath;
        
        // Update progress
        float m_downloadProgress;
        float m_installProgress;
        bool m_isUpdating;
        uint64_t m_lastCheckTime;
        
        // Callbacks
        std::function<void(float)> m_progressCallback;
        std::function<void(const std::string&)> m_statusCallback;
        std::function<void(const UpdateResult&)> m_completeCallback;
        
        // Private methods
        bool CheckForUpdatesInternal();
        bool DownloadUpdate(const std::string& url, const std::string& filePath);
        bool VerifyChecksum(const std::string& filePath, const std::string& expectedChecksum);
        bool InstallUpdate(const std::string& updateFile);
        bool CreateSystemBackup();
        bool RollbackUpdate();
        std::string GetCurrentVersion();
        std::string GetLatestVersion();
        bool ParseUpdateInfo(const std::string& jsonData, UpdateInfo& info);
        void SetStatus(UpdateStatus status, const std::string& message = "");
        void UpdateProgress(float downloadProgress, float installProgress = -1.0f);
        void CleanupTempFiles();
        
    public:
        CUpdateManager();
        ~CUpdateManager();
        
        // Initialization
        bool Initialize(const UpdateConfig& config);
        void Shutdown();
        bool IsInitialized() const;
        
        // Update checking
        bool CheckForUpdates();
        bool CheckForUpdatesAsync();
        UpdateStatus GetStatus() const;
        UpdateInfo GetAvailableUpdate() const;
        std::string GetCurrentVersion() const;
        
        // Update management
        UpdateResult DownloadAndInstall();
        UpdateResult DownloadUpdateOnly();
        UpdateResult InstallFromFile(const std::string& filePath);
        bool RollbackToVersion(const std::string& version);
        bool CreateManualBackup();
        
        // Configuration
        bool LoadConfiguration(const std::string& configFile = "rust_update.ini");
        bool SaveConfiguration(const std::string& configFile = "rust_update.ini");
        UpdateConfig GetConfig() const;
        void SetConfig(const UpdateConfig& config);
        
        // Callbacks
        void SetProgressCallback(std::function<void(float)> callback);
        void SetStatusCallback(std::function<void(const std::string&)> callback);
        void SetCompleteCallback(std::function<void(const UpdateResult&)> callback);
        
        // Progress tracking
        float GetDownloadProgress() const;
        float GetInstallProgress() const;
        bool IsUpdating() const;
        uint64_t GetLastCheckTime() const;
        
        // Advanced features
        bool ScheduleUpdateCheck(int hoursFromNow);
        bool CancelUpdate();
        std::vector<std::string> GetUpdateHistory() const;
        void ClearUpdateHistory();
        bool ValidateUpdateFile(const std::string& filePath);
        
        // Beta features
        bool EnableBetaUpdates(bool enable);
        bool IsBetaUpdatesEnabled() const;
        bool IsBetaVersion(const std::string& version) const;
        
        // Security
        bool VerifyUpdateSignature(const std::string& filePath);
        std::string GetUpdateSignature(const std::string& filePath);
        bool IsUpdateTrusted(const UpdateInfo& info);
    };
}

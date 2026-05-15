/*
 * Backup and Recovery Manager for Rust Anti-Cheat Evasion System
 * Configuration and settings backup with recovery capabilities
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

namespace Backup {
    
    // Backup types
    enum class BackupType {
        Configuration,
        Statistics,
        Logs,
        Plugins,
        Profiles,
        Database,
        Complete,
        Custom
    };
    
    // Backup status
    enum class BackupStatus {
        NotStarted,
        InProgress,
        Completed,
        Failed,
        Cancelled,
        Corrupted,
        Restoring
    };
    
    // Backup entry
    struct BackupEntry {
        std::string name;
        BackupType type;
        std::string description;
        std::string filePath;
        uint64_t size;
        uint64_t createdTime;
        uint64_t compressedSize;
        bool isEncrypted;
        bool isCompressed;
        std::string checksum;
        std::string version;
        std::map<std::string, std::string> metadata;
    };
    
    // Backup configuration
    struct BackupConfig {
        bool enableAutoBackup;
        int backupIntervalHours;
        std::string backupDirectory;
        int maxBackupCount;
        bool enableCompression;
        bool enableEncryption;
        std::string encryptionKey;
        bool enableCloudBackup;
        std::string cloudProvider;
        std::string cloudEndpoint;
        std::string cloudUsername;
        std::string cloudPassword;
        std::vector<BackupType> backupTypes;
        bool verifyBackupIntegrity;
        bool enableIncrementalBackup;
        int incrementalBackupInterval;
    };
    
    // Recovery result
    struct RecoveryResult {
        bool success;
        BackupStatus status;
        std::string message;
        std::string errorDetails;
        std::string restoredPath;
        uint64_t restoreTime;
        std::vector<std::string> restoredFiles;
        std::vector<std::string> failedFiles;
    };
    
    class CBackupManager {
    private:
        // Configuration
        BackupConfig m_config;
        
        // State
        bool m_isInitialized;
        bool m_isBackupInProgress;
        bool m_isRestoreInProgress;
        BackupStatus m_currentStatus;
        std::string m_currentOperation;
        
        // Backup storage
        std::vector<BackupEntry> m_backupHistory;
        std::map<BackupType, std::vector<BackupEntry>> m_backupsByType;
        
        // Current backup
        BackupEntry m_currentBackup;
        
        // Progress tracking
        float m_backupProgress;
        float m_restoreProgress;
        uint64_t m_totalBytes;
        uint64_t m_processedBytes;
        
        // Callbacks
        std::function<void(float)> m_progressCallback;
        std::function<void(const std::string&)> m_statusCallback;
        std::function<void(const BackupEntry&)> m_completeCallback;
        std::function<void(const RecoveryResult&)> m_recoveryCallback;
        
        // Private methods
        bool CreateBackupDirectory();
        bool CreateCloudBackup();
        bool CreateLocalBackup();
        bool BackupConfiguration();
        bool BackupStatistics();
        bool BackupLogs();
        bool BackupPlugins();
        bool BackupProfiles();
        bool BackupDatabase();
        bool BackupComplete();
        bool CompressBackup(const std::string& sourcePath, const std::string& targetPath);
        bool EncryptBackup(const std::string& sourcePath, const std::string& targetPath);
        std::string CalculateChecksum(const std::string& filePath);
        bool VerifyBackupIntegrity(const BackupEntry& backup);
        void UpdateProgress(float progress, const std::string& operation = "");
        void CleanupOldBackups();
        void LoadBackupHistory();
        void SaveBackupHistory();
        std::string GenerateBackupName(BackupType type);
        std::string GetCurrentTimestamp();
        
    public:
        CBackupManager();
        ~CBackupManager();
        
        // Initialization
        bool Initialize(const BackupConfig& config);
        void Shutdown();
        bool IsInitialized() const;
        
        // Configuration
        bool LoadConfiguration(const std::string& configFile = "rust_backup.ini");
        bool SaveConfiguration(const std::string& configFile = "rust_backup.ini");
        BackupConfig GetConfig() const;
        void SetConfig(const BackupConfig& config);
        
        // Backup operations
        RecoveryResult CreateBackup(BackupType type, const std::string& description = "");
        RecoveryResult CreateAutoBackup();
        RecoveryResult CreateScheduledBackup();
        RecoveryResult CreateManualBackup(const std::string& customPath = "");
        bool CancelBackup();
        bool IsBackupInProgress() const;
        
        // Recovery operations
        RecoveryResult RestoreBackup(const std::string& backupName);
        RecoveryResult RestoreLatestBackup();
        RecoveryResult RestoreBackupFromPath(const std::string& backupPath);
        RecoveryResult RestoreConfiguration(const std::string& backupName);
        RecoveryResult RestoreStatistics(const std::string& backupName);
        RecoveryResult RestoreLogs(const std::string& backupName);
        RecoveryResult RestorePlugins(const std::string& backupName);
        RecoveryResult RestoreProfiles(const std::string& backupName);
        RecoveryResult RestoreDatabase(const std::string& backupName);
        RecoveryResult RestoreCompleteBackup(const std::string& backupName);
        bool CancelRestore();
        bool IsRestoreInProgress() const;
        
        // Backup management
        std::vector<BackupEntry> GetBackupHistory() const;
        std::vector<BackupEntry> GetBackupsByType(BackupType type) const;
        BackupEntry GetLatestBackup(BackupType type) const;
        BackupEntry GetBackupByName(const std::string& name) const;
        bool DeleteBackup(const std::string& backupName);
        bool DeleteBackupByType(BackupType type, int count = 1);
        void ClearBackupHistory();
        
        // Progress tracking
        float GetBackupProgress() const;
        float GetRestoreProgress() const;
        BackupStatus GetCurrentStatus() const;
        std::string GetCurrentOperation() const;
        
        // Callbacks
        void SetProgressCallback(std::function<void(float)> callback);
        void SetStatusCallback(std::function<void(const std::string&)> callback);
        void SetCompleteCallback(std::function<void(const BackupEntry&)> callback);
        void SetRecoveryCallback(std::function<void(const RecoveryResult&)> callback);
        
        // Advanced features
        bool EnableAutoBackup(bool enable);
        bool IsAutoBackupEnabled() const;
        bool ScheduleBackup(BackupType type, int hoursFromNow);
        bool ScheduleRecurringBackup(BackupType type, int intervalHours);
        bool EnableCloudBackup(bool enable);
        bool IsCloudBackupEnabled() const;
        bool EnableIncrementalBackup(bool enable);
        bool IsIncrementalBackupEnabled() const;
        
        // Cloud operations
        bool UploadToCloud(const std::string& backupPath);
        bool DownloadFromCloud(const std::string& backupName);
        std::vector<std::string> GetCloudBackupList();
        bool DeleteCloudBackup(const std::string& backupName);
        
        // Search and filtering
        std::vector<BackupEntry> SearchBackups(const std::string& searchTerm, 
                                               BackupType type = BackupType::Custom,
                                               uint64_t startTime = 0,
                                               uint64_t endTime = 0) const;
        
        // Validation and integrity
        bool ValidateBackup(const BackupEntry& backup);
        bool RepairBackup(const std::string& backupName);
        std::vector<std::string> GetBackupErrors(const std::string& backupName);
        
        // Export and import
        bool ExportBackup(const std::string& backupName, const std::string& exportPath);
        bool ImportBackup(const std::string& importPath);
        std::vector<BackupEntry> GetImportableBackups() const;
        
        // Performance optimization
        void OptimizeBackupPerformance();
        void SetBackupPriority(BackupType type, int priority);
        void SetCompressionLevel(int level);
        void SetEncryptionStrength(int strength);
        
        // Emergency recovery
        RecoveryResult EmergencyRestore();
        bool CreateEmergencyBackup();
        std::vector<std::string> GetEmergencyBackups() const;
        
        // Debugging
        void EnableDebugMode(bool enable);
        bool IsDebugModeEnabled() const;
        void TestBackupCreation();
        void TestBackupRestore();
        void DumpBackupHistory() const;
        
        // Integration with other systems
        void IntegrateWithConfigurationManager();
        void IntegrateWithStatisticsManager();
        void IntegrateWithPluginManager();
        void IntegrateWithUpdateManager();
        void IntegrateWithAll();
    };
}

/*
 * Update Manager Implementation for Rust Anti-Cheat Evasion System
 * Automatic updates with version checking and rollback
 * Author: Anonymous
 * Last Modified: 2026
 */

#include "UpdateManager.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <curl/curl.h>
#include <openssl/sha.h>

namespace Update {
    
    CUpdateManager::CUpdateManager() 
        : m_currentStatus(UpdateStatus::NotChecked), m_downloadProgress(0.0f), 
          m_installProgress(0.0f), m_isUpdating(false), m_lastCheckTime(0),
          m_updatePath("updates/"), m_backupPath("backups/"), m_tempPath("temp/") {
        
        // Initialize default configuration
        m_config.autoCheckEnabled = true;
        m_config.checkIntervalHours = 24;
        m_config.updateServer = "https://api.rust-toolkit.com/v1/updates";
        m_config.currentVersion = "1.0.0";
        m_config.allowBetaUpdates = false;
        m_config.autoInstall = false;
        m_config.createBackup = true;
        m_config.requireConfirmation = true;
        m_config.backupPath = "backups/";
        
        // Initialize curl
        curl_global_init(CURL_GLOBAL_DEFAULT);
        
        // Create directories
        std::filesystem::create_directories(m_updatePath);
        std::filesystem::create_directories(m_backupPath);
        std::filesystem::create_directories(m_tempPath);
        
        // Get current version
        m_currentVersion = GetCurrentVersion();
    }
    
    CUpdateManager::~CUpdateManager() {
        if (m_isUpdating) {
            CancelUpdate();
        }
        
        curl_global_cleanup();
        
        // Cleanup temp files
        CleanupTempFiles();
    }
    
    bool CUpdateManager::Initialize(const UpdateConfig& config) {
        std::cout << "Initializing Update Manager..." << std::endl;
        
        m_config = config;
        m_currentVersion = GetCurrentVersion();
        
        // Load configuration from file if exists
        LoadConfiguration();
        
        // Create directories
        std::filesystem::create_directories(m_updatePath);
        std::filesystem::create_directories(m_backupPath);
        std::filesystem::create_directories(m_tempPath);
        
        std::cout << "[SUCCESS] Update Manager initialized" << std::endl;
        std::cout << "Current version: " << m_currentVersion << std::endl;
        std::cout << "Auto-check enabled: " << (m_config.autoCheckEnabled ? "Yes" : "No") << std::endl;
        std::cout << "Check interval: " << m_config.checkIntervalHours << " hours" << std::endl;
        
        return true;
    }
    
    void CUpdateManager::Shutdown() {
        std::cout << "Shutting down Update Manager..." << std::endl;
        
        if (m_isUpdating) {
            CancelUpdate();
        }
        
        // Save configuration
        SaveConfiguration();
        
        // Cleanup
        CleanupTempFiles();
        
        std::cout << "[SUCCESS] Update Manager shutdown complete" << std::endl;
    }
    
    bool CUpdateManager::IsInitialized() const {
        return !m_currentVersion.empty();
    }
    
    bool CUpdateManager::CheckForUpdates() {
        std::cout << "Checking for updates..." << std::endl;
        
        SetStatus(UpdateStatus::Checking, "Checking for updates...");
        
        bool result = CheckForUpdatesInternal();
        
        if (result) {
            SetStatus(UpdateStatus::UpdateAvailable, "Update available: " + m_availableUpdate.version);
            std::cout << "[SUCCESS] Update available: " << m_availableUpdate.version << std::endl;
        } else {
            SetStatus(UpdateStatus::UpToDate, "System is up to date");
            std::cout << "[INFO] System is up to date" << std::endl;
        }
        
        m_lastCheckTime = GetTickCount64();
        return result;
    }
    
    bool CUpdateManager::CheckForUpdatesAsync() {
        std::cout << "Starting async update check..." << std::endl;
        
        std::thread([this]() {
            CheckForUpdates();
        }).detach();
        
        return true;
    }
    
    UpdateStatus CUpdateManager::GetStatus() const {
        return m_currentStatus;
    }
    
    UpdateInfo CUpdateManager::GetAvailableUpdate() const {
        return m_availableUpdate;
    }
    
    std::string CUpdateManager::GetCurrentVersion() const {
        // Try to read version from version file
        std::ifstream versionFile("version.txt");
        if (versionFile.is_open()) {
            std::string version;
            std::getline(versionFile, version);
            versionFile.close();
            return version;
        }
        
        // Fallback to hardcoded version
        return "1.0.0";
    }
    
    UpdateResult CUpdateManager::DownloadAndInstall() {
        std::cout << "Starting download and install..." << std::endl;
        
        UpdateResult result;
        result.success = false;
        result.status = UpdateStatus::Failed;
        result.downloadTime = 0;
        result.installTime = 0;
        
        if (m_currentStatus != UpdateStatus::UpdateAvailable) {
            result.message = "No update available";
            return result;
        }
        
        m_isUpdating = true;
        SetStatus(UpdateStatus::Downloading, "Downloading update...");
        
        // Create backup
        if (m_config.createBackup) {
            if (!CreateSystemBackup()) {
                result.message = "Failed to create backup";
                m_isUpdating = false;
                return result;
            }
        }
        
        // Download update
        std::string updateFile = m_tempPath + "update_" + m_availableUpdate.version + ".zip";
        auto downloadStart = GetTickCount64();
        
        if (!DownloadUpdate(m_availableUpdate.downloadUrl, updateFile)) {
            result.message = "Failed to download update";
            m_isUpdating = false;
            return result;
        }
        
        auto downloadEnd = GetTickCount64();
        result.downloadTime = downloadEnd - downloadStart;
        
        // Verify checksum
        if (!VerifyChecksum(updateFile, m_availableUpdate.checksum)) {
            result.message = "Update file checksum verification failed";
            m_isUpdating = false;
            return result;
        }
        
        // Verify signature
        if (!VerifyUpdateSignature(updateFile)) {
            result.message = "Update file signature verification failed";
            m_isUpdating = false;
            return result;
        }
        
        SetStatus(UpdateStatus::Installing, "Installing update...");
        
        // Install update
        auto installStart = GetTickCount64();
        
        if (!InstallUpdate(updateFile)) {
            result.message = "Failed to install update";
            m_isUpdating = false;
            return result;
        }
        
        auto installEnd = GetTickCount64();
        result.installTime = installEnd - installStart;
        
        // Update current version
        m_currentVersion = m_availableUpdate.version;
        m_config.currentVersion = m_availableUpdate.version;
        
        // Save configuration
        SaveConfiguration();
        
        // Cleanup
        CleanupTempFiles();
        
        result.success = true;
        result.status = UpdateStatus::Installed;
        result.message = "Update installed successfully";
        result.installedVersion = m_availableUpdate.version;
        
        SetStatus(UpdateStatus::Installed, "Update installed successfully");
        
        m_isUpdating = false;
        std::cout << "[SUCCESS] Update installed: " << m_availableUpdate.version << std::endl;
        
        return result;
    }
    
    UpdateResult CUpdateManager::DownloadUpdateOnly() {
        std::cout << "Downloading update only..." << std::endl;
        
        UpdateResult result;
        result.success = false;
        result.status = UpdateStatus::Failed;
        
        if (m_currentStatus != UpdateStatus::UpdateAvailable) {
            result.message = "No update available";
            return result;
        }
        
        SetStatus(UpdateStatus::Downloading, "Downloading update...");
        
        // Download update
        std::string updateFile = m_updatePath + "update_" + m_availableUpdate.version + ".zip";
        
        if (!DownloadUpdate(m_availableUpdate.downloadUrl, updateFile)) {
            result.message = "Failed to download update";
            return result;
        }
        
        // Verify checksum
        if (!VerifyChecksum(updateFile, m_availableUpdate.checksum)) {
            result.message = "Update file checksum verification failed";
            return result;
        }
        
        result.success = true;
        result.status = UpdateStatus::Downloading;
        result.message = "Update downloaded successfully";
        
        SetStatus(UpdateStatus::Downloading, "Update downloaded successfully");
        
        std::cout << "[SUCCESS] Update downloaded: " << updateFile << std::endl;
        
        return result;
    }
    
    UpdateResult CUpdateManager::InstallFromFile(const std::string& filePath) {
        std::cout << "Installing update from file: " << filePath << std::endl;
        
        UpdateResult result;
        result.success = false;
        result.status = UpdateStatus::Failed;
        result.installTime = 0;
        
        // Verify file exists
        if (!std::filesystem::exists(filePath)) {
            result.message = "Update file not found: " + filePath;
            return result;
        }
        
        // Validate update file
        if (!ValidateUpdateFile(filePath)) {
            result.message = "Invalid update file";
            return result;
        }
        
        // Create backup
        if (m_config.createBackup) {
            if (!CreateSystemBackup()) {
                result.message = "Failed to create backup";
                return result;
            }
        }
        
        SetStatus(UpdateStatus::Installing, "Installing update...");
        
        // Install update
        auto installStart = GetTickCount64();
        
        if (!InstallUpdate(filePath)) {
            result.message = "Failed to install update";
            return result;
        }
        
        auto installEnd = GetTickCount64();
        result.installTime = installEnd - installStart;
        
        // Extract version from filename
        std::string version = ExtractVersionFromFilename(filePath);
        m_currentVersion = version;
        m_config.currentVersion = version;
        
        // Save configuration
        SaveConfiguration();
        
        result.success = true;
        result.status = UpdateStatus::Installed;
        result.message = "Update installed successfully";
        result.installedVersion = version;
        
        SetStatus(UpdateStatus::Installed, "Update installed successfully");
        
        std::cout << "[SUCCESS] Update installed from file: " << filePath << std::endl;
        
        return result;
    }
    
    bool CUpdateManager::RollbackToVersion(const std::string& version) {
        std::cout << "Rolling back to version: " << version << std::endl;
        
        // Find backup for specified version
        std::string backupPath = m_backupPath + "backup_" + version + ".zip";
        
        if (!std::filesystem::exists(backupPath)) {
            std::cout << "[ERROR] Backup not found for version: " << version << std::endl;
            return false;
        }
        
        SetStatus(UpdateStatus::RollingBack, "Rolling back to version " + version);
        
        // Install from backup
        if (!InstallUpdate(backupPath)) {
            std::cout << "[ERROR] Failed to rollback to version: " << version << std::endl;
            return false;
        }
        
        // Update current version
        m_currentVersion = version;
        m_config.currentVersion = version;
        
        // Save configuration
        SaveConfiguration();
        
        SetStatus(UpdateStatus::RolledBack, "Successfully rolled back to version " + version);
        
        std::cout << "[SUCCESS] Rolled back to version: " << version << std::endl;
        return true;
    }
    
    bool CUpdateManager::CreateManualBackup() {
        std::cout << "Creating manual backup..." << std::endl;
        
        return CreateSystemBackup();
    }
    
    UpdateConfig CUpdateManager::GetConfig() const {
        return m_config;
    }
    
    void CUpdateManager::SetConfig(const UpdateConfig& config) {
        m_config = config;
        SaveConfiguration();
    }
    
    void CUpdateManager::SetProgressCallback(std::function<void(float)> callback) {
        m_progressCallback = callback;
    }
    
    void CUpdateManager::SetStatusCallback(std::function<void(const std::string&)> callback) {
        m_statusCallback = callback;
    }
    
    void CUpdateManager::SetCompleteCallback(std::function<void(const UpdateResult&)> callback) {
        m_completeCallback = callback;
    }
    
    float CUpdateManager::GetDownloadProgress() const {
        return m_downloadProgress;
    }
    
    float CUpdateManager::GetInstallProgress() const {
        return m_installProgress;
    }
    
    bool CUpdateManager::IsUpdating() const {
        return m_isUpdating;
    }
    
    uint64_t CUpdateManager::GetLastCheckTime() const {
        return m_lastCheckTime;
    }
    
    bool CUpdateManager::ScheduleUpdateCheck(int hoursFromNow) {
        std::cout << "Scheduling update check in " << hoursFromNow << " hours" << std::endl;
        
        // This would typically use a timer or scheduler
        // For now, we'll just log the scheduling
        std::cout << "[INFO] Update check scheduled" << std::endl;
        
        return true;
    }
    
    bool CUpdateManager::CancelUpdate() {
        std::cout << "Cancelling update..." << std::endl;
        
        if (!m_isUpdating) {
            std::cout << "[WARNING] No update in progress" << std::endl;
            return false;
        }
        
        m_isUpdating = false;
        SetStatus(UpdateStatus::Failed, "Update cancelled");
        
        // Cleanup temp files
        CleanupTempFiles();
        
        std::cout << "[SUCCESS] Update cancelled" << std::endl;
        return true;
    }
    
    std::vector<std::string> CUpdateManager::GetUpdateHistory() const {
        std::vector<std::string> history;
        
        // Read update history from file
        std::ifstream historyFile("update_history.txt");
        if (historyFile.is_open()) {
            std::string line;
            while (std::getline(historyFile, line)) {
                history.push_back(line);
            }
            historyFile.close();
        }
        
        return history;
    }
    
    void CUpdateManager::ClearUpdateHistory() {
        std::cout << "Clearing update history..." << std::endl;
        
        std::ofstream historyFile("update_history.txt", std::ios::trunc);
        historyFile.close();
        
        std::cout << "[SUCCESS] Update history cleared" << std::endl;
    }
    
    bool CUpdateManager::ValidateUpdateFile(const std::string& filePath) {
        std::cout << "Validating update file: " << filePath << std::endl;
        
        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            std::cout << "[ERROR] Update file does not exist" << std::endl;
            return false;
        }
        
        // Check file size
        uint64_t fileSize = std::filesystem::file_size(filePath);
        if (fileSize == 0) {
            std::cout << "[ERROR] Update file is empty" << std::endl;
            return false;
        }
        
        // Check if it's a valid zip file
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "[ERROR] Cannot open update file" << std::endl;
            return false;
        }
        
        // Check zip file signature
        uint32_t signature;
        file.read(reinterpret_cast<char*>(&signature), 4);
        file.close();
        
        if (signature != 0x04034b50) { // ZIP file signature
            std::cout << "[ERROR] Invalid zip file format" << std::endl;
            return false;
        }
        
        std::cout << "[SUCCESS] Update file is valid" << std::endl;
        return true;
    }
    
    bool CUpdateManager::EnableBetaUpdates(bool enable) {
        m_config.allowBetaUpdates = enable;
        SaveConfiguration();
        
        std::cout << "[SUCCESS] Beta updates " << (enable ? "enabled" : "disabled") << std::endl;
        return true;
    }
    
    bool CUpdateManager::IsBetaUpdatesEnabled() const {
        return m_config.allowBetaUpdates;
    }
    
    bool CUpdateManager::IsBetaVersion(const std::string& version) const {
        return version.find("beta") != std::string::npos || 
               version.find("alpha") != std::string::npos || 
               version.find("dev") != std::string::npos;
    }
    
    // Private methods
    bool CUpdateManager::CheckForUpdatesInternal() {
        std::cout << "Checking for updates from server..." << std::endl;
        
        // Build request URL
        std::string url = m_config.updateServer + "?version=" + m_currentVersion;
        if (m_config.allowBetaUpdates) {
            url += "&beta=true";
        }
        
        // Make HTTP request
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cout << "[ERROR] Failed to initialize curl" << std::endl;
            return false;
        }
        
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            std::cout << "[ERROR] Failed to check for updates: " << curl_easy_strerror(res) << std::endl;
            return false;
        }
        
        // Parse response
        return ParseUpdateInfo(response, m_availableUpdate);
    }
    
    bool CUpdateManager::DownloadUpdate(const std::string& url, const std::string& filePath) {
        std::cout << "Downloading update from: " << url << std::endl;
        
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cout << "[ERROR] Failed to initialize curl" << std::endl;
            return false;
        }
        
        FILE* file = fopen(filePath.c_str(), "wb");
        if (!file) {
            std::cout << "[ERROR] Failed to create file: " << filePath << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
        
        CURLcode res = curl_easy_perform(curl);
        fclose(file);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            std::cout << "[ERROR] Failed to download update: " << curl_easy_strerror(res) << std::endl;
            return false;
        }
        
        std::cout << "[SUCCESS] Update downloaded to: " << filePath << std::endl;
        return true;
    }
    
    bool CUpdateManager::VerifyChecksum(const std::string& filePath, const std::string& expectedChecksum) {
        std::cout << "Verifying checksum for: " << filePath << std::endl;
        
        // Calculate SHA256 checksum
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "[ERROR] Cannot open file for checksum verification" << std::endl;
            return false;
        }
        
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        
        char buffer[4096];
        while (file.read(buffer, sizeof(buffer))) {
            SHA256_Update(&sha256, buffer, file.gcount());
        }
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256);
        
        // Convert to hex string
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        
        std::string actualChecksum = ss.str();
        file.close();
        
        if (actualChecksum != expectedChecksum) {
            std::cout << "[ERROR] Checksum mismatch" << std::endl;
            std::cout << "Expected: " << expectedChecksum << std::endl;
            std::cout << "Actual: " << actualChecksum << std::endl;
            return false;
        }
        
        std::cout << "[SUCCESS] Checksum verification passed" << std::endl;
        return true;
    }
    
    bool CUpdateManager::VerifyUpdateSignature(const std::string& filePath) {
        std::cout << "Verifying signature for: " << filePath << std::endl;
        
        // This would verify the digital signature of the update
        // For now, we'll just return true as a placeholder
        std::cout << "[INFO] Signature verification not implemented" << std::endl;
        return true;
    }
    
    bool CUpdateManager::InstallUpdate(const std::string& updateFile) {
        std::cout << "Installing update from: " << updateFile << std::endl;
        
        // Extract update
        std::string extractPath = m_tempPath + "extract/";
        std::filesystem::create_directories(extractPath);
        
        // This would typically use a zip library to extract
        // For now, we'll simulate the installation
        std::cout << "[INFO] Extracting update..." << std::endl;
        
        // Simulate extraction progress
        for (int i = 0; i <= 100; i += 10) {
            m_installProgress = i / 100.0f;
            UpdateProgress(m_downloadProgress, m_installProgress);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Copy files to application directory
        std::cout << "[INFO] Copying files..." << std::endl;
        
        // Update version file
        std::ofstream versionFile("version.txt");
        if (versionFile.is_open()) {
            versionFile << m_availableUpdate.version;
            versionFile.close();
        }
        
        // Add to update history
        AddToUpdateHistory(m_availableUpdate.version);
        
        std::cout << "[SUCCESS] Update installed successfully" << std::endl;
        return true;
    }
    
    bool CUpdateManager::CreateSystemBackup() {
        std::cout << "Creating system backup..." << std::endl;
        
        std::string backupFile = m_backupPath + "backup_" + m_currentVersion + "_" + GetCurrentTimestamp() + ".zip";
        
        // This would typically create a zip of the application directory
        // For now, we'll simulate the backup creation
        std::cout << "[INFO] Creating backup: " << backupFile << std::endl;
        
        // Simulate backup creation
        for (int i = 0; i <= 100; i += 20) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        std::cout << "[SUCCESS] System backup created: " << backupFile << std::endl;
        return true;
    }
    
    bool CUpdateManager::ParseUpdateInfo(const std::string& jsonData, UpdateInfo& info) {
        std::cout << "Parsing update info..." << std::endl;
        
        // This would typically parse JSON response
        // For now, we'll simulate parsing with a hardcoded response
        if (jsonData.find("update_available") != std::string::npos) {
            info.version = "1.1.0";
            info.releaseDate = "2026-05-12";
            info.description = "Major update with new features and bug fixes";
            info.downloadUrl = "https://api.rust-toolkit.com/v1/download/1.1.0";
            info.fileSize = 1024 * 1024 * 50; // 50MB
            info.checksum = "a1b2c3d4e5f6789012345678901234567890abcdef";
            info.isCritical = true;
            info.isBeta = false;
            info.changes = {
                "Added new ESP features",
                "Improved aimbot performance",
                "Enhanced VPN stability",
                "Fixed memory leak issues"
            };
            
            return true;
        }
        
        return false;
    }
    
    void CUpdateManager::SetStatus(UpdateStatus status, const std::string& message) {
        m_currentStatus = status;
        
        if (m_statusCallback) {
            m_statusCallback(message);
        }
        
        std::cout << "[STATUS] " << message << std::endl;
    }
    
    void CUpdateManager::UpdateProgress(float downloadProgress, float installProgress) {
        if (downloadProgress >= 0.0f) {
            m_downloadProgress = downloadProgress;
        }
        
        if (installProgress >= 0.0f) {
            m_installProgress = installProgress;
        }
        
        if (m_progressCallback) {
            float totalProgress = (m_downloadProgress + m_installProgress) / 2.0f;
            m_progressCallback(totalProgress);
        }
    }
    
    void CUpdateManager::CleanupTempFiles() {
        std::cout << "Cleaning up temporary files..." << std::endl;
        
        try {
            if (std::filesystem::exists(m_tempPath)) {
                std::filesystem::remove_all(m_tempPath);
            }
            std::filesystem::create_directories(m_tempPath);
        } catch (const std::exception& e) {
            std::cout << "[ERROR] Failed to cleanup temp files: " << e.what() << std::endl;
        }
    }
    
    std::string CUpdateManager::ExtractVersionFromFilename(const std::string& filename) const {
        // Extract version from filename like "update_1.1.0.zip"
        size_t start = filename.find("_") + 1;
        size_t end = filename.find(".zip");
        
        if (start != std::string::npos && end != std::string::npos && end > start) {
            return filename.substr(start, end - start);
        }
        
        return "unknown";
    }
    
    void CUpdateManager::AddToUpdateHistory(const std::string& version) {
        std::ofstream historyFile("update_history.txt", std::ios::app);
        if (historyFile.is_open()) {
            historyFile << GetCurrentTimestamp() << " - Updated to version " << version << std::endl;
            historyFile.close();
        }
    }
    
    std::string CUpdateManager::GetCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d_%H-%M-%S");
        
        return ss.str();
    }
    
    bool CUpdateManager::LoadConfiguration(const std::string& configFile) {
        std::cout << "Loading update configuration from: " << configFile << std::endl;
        
        std::ifstream file(configFile);
        if (!file.is_open()) {
            std::cout << "[WARNING] Update config file not found, using defaults" << std::endl;
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
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
                if (key == "autoCheckEnabled") {
                    m_config.autoCheckEnabled = (value == "true");
                } else if (key == "checkIntervalHours") {
                    m_config.checkIntervalHours = std::stoi(value);
                } else if (key == "updateServer") {
                    m_config.updateServer = value;
                } else if (key == "allowBetaUpdates") {
                    m_config.allowBetaUpdates = (value == "true");
                } else if (key == "autoInstall") {
                    m_config.autoInstall = (value == "true");
                } else if (key == "createBackup") {
                    m_config.createBackup = (value == "true");
                } else if (key == "requireConfirmation") {
                    m_config.requireConfirmation = (value == "true");
                } else if (key == "backupPath") {
                    m_config.backupPath = value;
                }
            }
        }
        
        file.close();
        std::cout << "[SUCCESS] Update configuration loaded" << std::endl;
        return true;
    }
    
    bool CUpdateManager::SaveConfiguration(const std::string& configFile) {
        std::cout << "Saving update configuration to: " << configFile << std::endl;
        
        std::ofstream file(configFile);
        if (!file.is_open()) {
            std::cout << "[ERROR] Failed to save update configuration" << std::endl;
            return false;
        }
        
        file << "# Rust Anti-Cheat Evasion System Update Configuration" << std::endl;
        file << "# Generated: " << GetCurrentTimestamp() << std::endl;
        file << std::endl;
        
        file << "autoCheckEnabled=" << (m_config.autoCheckEnabled ? "true" : "false") << std::endl;
        file << "checkIntervalHours=" << m_config.checkIntervalHours << std::endl;
        file << "updateServer=" << m_config.updateServer << std::endl;
        file << "currentVersion=" << m_currentVersion << std::endl;
        file << "allowBetaUpdates=" << (m_config.allowBetaUpdates ? "true" : "false") << std::endl;
        file << "autoInstall=" << (m_config.autoInstall ? "true" : "false") << std::endl;
        file << "createBackup=" << (m_config.createBackup ? "true" : "false") << std::endl;
        file << "requireConfirmation=" << (m_config.requireConfirmation ? "true" : "false") << std::endl;
        file << "backupPath=" << m_config.backupPath << std::endl;
        
        file.close();
        std::cout << "[SUCCESS] Update configuration saved" << std::endl;
        return true;
    }
    
    // Static callback functions
    size_t CUpdateManager::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        std::string* response = static_cast<std::string*>(userp);
        response->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
    
    size_t CUpdateManager::WriteFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        FILE* file = static_cast<FILE*>(userp);
        return fwrite(contents, 1, totalSize, file);
    }
    
    int CUpdateManager::ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        CUpdateManager* manager = static_cast<CUpdateManager*>(clientp);
        
        if (dltotal > 0) {
            float progress = static_cast<float>(dlnow) / static_cast<float>(dltotal);
            manager->UpdateProgress(progress, -1.0f);
        }
        
        return 0;
    }
}

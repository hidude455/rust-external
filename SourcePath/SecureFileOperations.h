/*
 * Secure File Operations for Rust Anti-Cheat Evasion System
 * Provides path validation and secure file operations
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <algorithm>
#include <Windows.h>

namespace Secure {
    
    // Path validation and security
    class PathValidator {
    private:
        std::vector<std::string> m_allowedDirectories;
        std::vector<std::string> m_blockedPaths;
        std::vector<std::string> m_allowedExtensions;
        std::vector<std::string> m_blockedExtensions;
        std::regex m_safePathRegex;
        bool m_initialized;
        
    public:
        PathValidator() : m_initialized(false) {
            Initialize();
        }
        
        void Initialize() {
            if (m_initialized) return;
            
            // Default allowed directories
            m_allowedDirectories = {
                "./",
                ".\\",
                "config/",
                "config\\",
                "logs/",
                "logs\\",
                "backups/",
                "backups\\",
                "profiles/",
                "profiles\\",
                "plugins/",
                "plugins\\",
                "analytics/",
                "analytics\\"
            };
            
            // Default blocked paths
            m_blockedPaths = {
                "..",
                "../",
                "..\\",
                "C:/",
                "C:\\",
                "D:/",
                "D:\\",
                "E:/",
                "E:\\",
                "/etc/",
                "/var/",
                "/usr/",
                "/bin/",
                "/sbin/",
                "Windows/",
                "Windows\\",
                "System32/",
                "System32\\",
                "Program Files/",
                "Program Files\\",
                "Program Files (x86)/",
                "Program Files (x86)\\"
            };
            
            // Default allowed extensions
            m_allowedExtensions = {
                ".ini",
                ".cfg",
                ".conf",
                ".json",
                ".xml",
                ".txt",
                ".log",
                ".dat",
                ".bin",
                ".enc",
                ".key",
                ".bak",
                ".tmp",
                ".profile",
                ".plugin",
                ".dll",
                ".so"
            };
            
            // Default blocked extensions
            m_blockedExtensions = {
                ".exe",
                ".bat",
                ".cmd",
                ".com",
                ".pif",
                ".scr",
                ".vbs",
                ".js",
                ".jar",
                ".app",
                ".deb",
                ".rpm",
                ".dmg",
                ".pkg",
                ".msi",
                ".msp",
                ".msu"
            };
            
            // Safe path regex (alphanumeric, underscores, hyphens, forward/backward slashes)
            m_safePathRegex = std::regex(R"(^[a-zA-Z0-9_\-/\\\.]+$)");
            
            m_initialized = true;
        }
        
        bool IsValidPath(const std::string& path) const {
            if (!m_initialized) return false;
            
            // Check for empty path
            if (path.empty()) return false;
            
            // Check for null bytes
            if (path.find('\0') != std::string::npos) return false;
            
            // Check for blocked paths
            for (const auto& blocked : m_blockedPaths) {
                if (path.find(blocked) != std::string::npos) {
                    return false;
                }
            }
            
            // Check path length (prevent buffer overflow)
            if (path.length() > MAX_PATH) return false;
            
            // Check for path traversal attempts
            if (ContainsPathTraversal(path)) return false;
            
            // Check for invalid characters
            if (!IsValidPathCharacters(path)) return false;
            
            // Check if path is within allowed directories
            if (!IsInAllowedDirectory(path)) return false;
            
            return true;
        }
        
        bool IsValidFile(const std::string& filePath) const {
            if (!IsValidPath(filePath)) return false;
            
            // Check extension
            std::string extension = GetFileExtension(filePath);
            if (!extension.empty()) {
                // Check if extension is blocked
                for (const auto& blocked : m_blockedExtensions) {
                    if (extension == blocked) {
                        return false;
                    }
                }
                
                // Check if extension is allowed (if we have allowed extensions)
                if (!m_allowedExtensions.empty()) {
                    bool allowed = false;
                    for (const auto& allowed : m_allowedExtensions) {
                        if (extension == allowed) {
                            allowed = true;
                            break;
                        }
                    }
                    if (!allowed) return false;
                }
            }
            
            return true;
        }
        
        bool IsValidDirectory(const std::string& dirPath) const {
            if (!IsValidPath(dirPath)) return false;
            
            // Check if it's a directory (ends with slash or backslash)
            char lastChar = dirPath.back();
            return (lastChar == '/' || lastChar == '\\');
        }
        
        std::string SanitizePath(const std::string& path) const {
            std::string sanitized = path;
            
            // Replace backslashes with forward slashes for consistency
            std::replace(sanitized.begin(), sanitized.end(), '\\', '/');
            
            // Remove multiple consecutive slashes
            sanitized = std::regex_replace(sanitized, std::regex("/+"), "/");
            
            // Remove trailing slash (unless it's root)
            if (sanitized.length() > 1 && sanitized.back() == '/') {
                sanitized.pop_back();
            }
            
            return sanitized;
        }
        
        std::string GetSafePath(const std::string& basePath, const std::string& relativePath) const {
            if (!IsValidPath(relativePath)) return "";
            
            std::string sanitizedBase = SanitizePath(basePath);
            std::string sanitizedRelative = SanitizePath(relativePath);
            
            // Combine paths
            std::string combined = sanitizedBase;
            if (!combined.empty() && combined.back() != '/') {
                combined += "/";
            }
            combined += sanitizedRelative;
            
            // Validate the combined path
            if (!IsValidPath(combined)) return "";
            
            return combined;
        }
        
        bool IsInAllowedDirectory(const std::string& path) const {
            std::string normalizedPath = SanitizePath(path);
            
            for (const auto& allowed : m_allowedDirectories) {
                std::string normalizedAllowed = SanitizePath(allowed);
                
                // Remove trailing slash for comparison
                if (!normalizedAllowed.empty() && normalizedAllowed.back() == '/') {
                    normalizedAllowed.pop_back();
                }
                
                // Check if path starts with allowed directory
                if (normalizedPath.find(normalizedAllowed) == 0) {
                    return true;
                }
            }
            
            return false;
        }
        
        void AddAllowedDirectory(const std::string& directory) {
            std::string normalized = SanitizePath(directory);
            if (!normalized.empty() && std::find(m_allowedDirectories.begin(), m_allowedDirectories.end(), normalized) == m_allowedDirectories.end()) {
                m_allowedDirectories.push_back(normalized);
            }
        }
        
        void AddBlockedPath(const std::string& path) {
            if (std::find(m_blockedPaths.begin(), m_blockedPaths.end(), path) == m_blockedPaths.end()) {
                m_blockedPaths.push_back(path);
            }
        }
        
        void AddAllowedExtension(const std::string& extension) {
            std::string ext = extension;
            if (ext[0] != '.') ext = "." + ext;
            
            if (std::find(m_allowedExtensions.begin(), m_allowedExtensions.end(), ext) == m_allowedExtensions.end()) {
                m_allowedExtensions.push_back(ext);
            }
        }
        
        void AddBlockedExtension(const std::string& extension) {
            std::string ext = extension;
            if (ext[0] != '.') ext = "." + ext;
            
            if (std::find(m_blockedExtensions.begin(), m_blockedExtensions.end(), ext) == m_blockedExtensions.end()) {
                m_blockedExtensions.push_back(ext);
            }
        }
        
        std::vector<std::string> GetAllowedDirectories() const {
            return m_allowedDirectories;
        }
        
        std::vector<std::string> GetBlockedPaths() const {
            return m_blockedPaths;
        }
        
        std::vector<std::string> GetAllowedExtensions() const {
            return m_allowedExtensions;
        }
        
        std::vector<std::string> GetBlockedExtensions() const {
            return m_blockedExtensions;
        }
        
    private:
        bool ContainsPathTraversal(const std::string& path) const {
            // Check for common path traversal patterns
            std::vector<std::string> traversalPatterns = {
                "../",
                "..\\",
                "%2e%2e%2f",  // URL encoded ../
                "%2e%2e%5c",  // URL encoded ..\
                "..%2f",
                "..%5c",
                "%2e%2e/",
                "%2e%2e\\"
            };
            
            std::string lowerPath = path;
            std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
            
            for (const auto& pattern : traversalPatterns) {
                if (lowerPath.find(pattern) != std::string::npos) {
                    return true;
                }
            }
            
            return false;
        }
        
        bool IsValidPathCharacters(const std::string& path) const {
            // Check for invalid Windows characters
            std::vector<char> invalidChars = {
                '<', '>', ':', '"', '|', '?', '*'
            };
            
            for (char c : path) {
                if (c < 32 || c > 126) return false; // Non-printable characters
                
                for (char invalid : invalidChars) {
                    if (c == invalid) return false;
                }
            }
            
            return true;
        }
        
        std::string GetFileExtension(const std::string& filePath) const {
            size_t dotPos = filePath.find_last_of('.');
            if (dotPos == std::string::npos) return "";
            
            return filePath.substr(dotPos);
        }
    };
    
    // Secure file operations wrapper
    class SecureFileOperations {
    private:
        PathValidator m_validator;
        std::string m_baseDirectory;
        bool m_initialized;
        
    public:
        SecureFileOperations(const std::string& baseDirectory = "./") 
            : m_baseDirectory(baseDirectory), m_initialized(false) {
            Initialize();
        }
        
        void Initialize() {
            if (m_initialized) return;
            
            // Normalize base directory
            m_baseDirectory = m_validator.SanitizePath(m_baseDirectory);
            
            // Add base directory to allowed directories
            m_validator.AddAllowedDirectory(m_baseDirectory);
            
            m_initialized = true;
        }
        
        bool WriteFile(const std::string& filePath, const std::string& content) {
            if (!m_initialized) return false;
            
            // Validate path
            std::string fullPath = m_validator.GetSafePath(m_baseDirectory, filePath);
            if (fullPath.empty()) {
                LOG_ERROR("Invalid file path: " + filePath);
                return false;
            }
            
            // Validate file
            if (!m_validator.IsValidFile(fullPath)) {
                LOG_ERROR("File validation failed: " + fullPath);
                return false;
            }
            
            try {
                // Create directory if it doesn't exist
                std::filesystem::path path(fullPath);
                std::filesystem::create_directories(path.parent_path());
                
                // Write file
                std::ofstream file(fullPath, std::ios::binary);
                if (!file.is_open()) {
                    LOG_ERROR("Failed to open file for writing: " + fullPath);
                    return false;
                }
                
                file.write(content.c_str(), content.size());
                file.close();
                
                return true;
            }
            catch (const std::exception& e) {
                LOG_ERROR("File write error: " + std::string(e.what()));
                return false;
            }
        }
        
        bool ReadFile(const std::string& filePath, std::string& content) {
            if (!m_initialized) return false;
            
            // Validate path
            std::string fullPath = m_validator.GetSafePath(m_baseDirectory, filePath);
            if (fullPath.empty()) {
                LOG_ERROR("Invalid file path: " + filePath);
                return false;
            }
            
            // Validate file
            if (!m_validator.IsValidFile(fullPath)) {
                LOG_ERROR("File validation failed: " + fullPath);
                return false;
            }
            
            try {
                // Check if file exists
                if (!std::filesystem::exists(fullPath)) {
                    LOG_ERROR("File does not exist: " + fullPath);
                    return false;
                }
                
                // Read file
                std::ifstream file(fullPath, std::ios::binary);
                if (!file.is_open()) {
                    LOG_ERROR("Failed to open file for reading: " + fullPath);
                    return false;
                }
                
                content = std::string((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
                file.close();
                
                return true;
            }
            catch (const std::exception& e) {
                LOG_ERROR("File read error: " + std::string(e.what()));
                return false;
            }
        }
        
        bool DeleteFile(const std::string& filePath) {
            if (!m_initialized) return false;
            
            // Validate path
            std::string fullPath = m_validator.GetSafePath(m_baseDirectory, filePath);
            if (fullPath.empty()) {
                LOG_ERROR("Invalid file path: " + filePath);
                return false;
            }
            
            // Validate file
            if (!m_validator.IsValidFile(fullPath)) {
                LOG_ERROR("File validation failed: " + fullPath);
                return false;
            }
            
            try {
                // Check if file exists
                if (!std::filesystem::exists(fullPath)) {
                    return true; // File doesn't exist, consider it deleted
                }
                
                // Delete file
                if (!std::filesystem::remove(fullPath)) {
                    LOG_ERROR("Failed to delete file: " + fullPath);
                    return false;
                }
                
                return true;
            }
            catch (const std::exception& e) {
                LOG_ERROR("File delete error: " + std::string(e.what()));
                return false;
            }
        }
        
        bool FileExists(const std::string& filePath) {
            if (!m_initialized) return false;
            
            // Validate path
            std::string fullPath = m_validator.GetSafePath(m_baseDirectory, filePath);
            if (fullPath.empty()) {
                return false;
            }
            
            // Validate file
            if (!m_validator.IsValidFile(fullPath)) {
                return false;
            }
            
            try {
                return std::filesystem::exists(fullPath);
            }
            catch (const std::exception& e) {
                LOG_ERROR("File exists check error: " + std::string(e.what()));
                return false;
            }
        }
        
        bool CreateDirectory(const std::string& dirPath) {
            if (!m_initialized) return false;
            
            // Validate path
            std::string fullPath = m_validator.GetSafePath(m_baseDirectory, dirPath);
            if (fullPath.empty()) {
                LOG_ERROR("Invalid directory path: " + dirPath);
                return false;
            }
            
            // Validate directory
            if (!m_validator.IsValidDirectory(fullPath)) {
                LOG_ERROR("Directory validation failed: " + fullPath);
                return false;
            }
            
            try {
                // Create directory
                if (!std::filesystem::create_directories(fullPath)) {
                    LOG_ERROR("Failed to create directory: " + fullPath);
                    return false;
                }
                
                return true;
            }
            catch (const std::exception& e) {
                LOG_ERROR("Directory creation error: " + std::string(e.what()));
                return false;
            }
        }
        
        bool DeleteDirectory(const std::string& dirPath, bool recursive = false) {
            if (!m_initialized) return false;
            
            // Validate path
            std::string fullPath = m_validator.GetSafePath(m_baseDirectory, dirPath);
            if (fullPath.empty()) {
                LOG_ERROR("Invalid directory path: " + dirPath);
                return false;
            }
            
            // Validate directory
            if (!m_validator.IsValidDirectory(fullPath)) {
                LOG_ERROR("Directory validation failed: " + fullPath);
                return false;
            }
            
            try {
                // Check if directory exists
                if (!std::filesystem::exists(fullPath)) {
                    return true; // Directory doesn't exist, consider it deleted
                }
                
                // Delete directory
                if (recursive) {
                    if (!std::filesystem::remove_all(fullPath)) {
                        LOG_ERROR("Failed to delete directory recursively: " + fullPath);
                        return false;
                    }
                } else {
                    if (!std::filesystem::remove(fullPath)) {
                        LOG_ERROR("Failed to delete directory: " + fullPath);
                        return false;
                    }
                }
                
                return true;
            }
            catch (const std::exception& e) {
                LOG_ERROR("Directory delete error: " + std::string(e.what()));
                return false;
            }
        }
        
        std::vector<std::string> ListFiles(const std::string& dirPath, const std::string& extension = "") {
            std::vector<std::string> files;
            
            if (!m_initialized) return files;
            
            // Validate path
            std::string fullPath = m_validator.GetSafePath(m_baseDirectory, dirPath);
            if (fullPath.empty()) {
                LOG_ERROR("Invalid directory path: " + dirPath);
                return files;
            }
            
            // Validate directory
            if (!m_validator.IsValidDirectory(fullPath)) {
                LOG_ERROR("Directory validation failed: " + fullPath);
                return files;
            }
            
            try {
                // Check if directory exists
                if (!std::filesystem::exists(fullPath)) {
                    return files;
                }
                
                // List files
                for (const auto& entry : std::filesystem::directory_iterator(fullPath)) {
                    if (entry.is_regular_file()) {
                        std::string filePath = entry.path().string();
                        
                        // Check extension filter
                        if (!extension.empty()) {
                            std::string fileExt = entry.path().extension().string();
                            if (fileExt != extension) continue;
                        }
                        
                        // Validate file
                        if (m_validator.IsValidFile(filePath)) {
                            // Convert to relative path
                            std::string relativePath = filePath.substr(m_baseDirectory.length());
                            if (relativePath[0] == '/' || relativePath[0] == '\\') {
                                relativePath = relativePath.substr(1);
                            }
                            files.push_back(relativePath);
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("Directory listing error: " + std::string(e.what()));
            }
            
            return files;
        }
        
        bool CopyFile(const std::string& sourcePath, const std::string& destPath) {
            if (!m_initialized) return false;
            
            // Validate source path
            std::string fullSourcePath = m_validator.GetSafePath(m_baseDirectory, sourcePath);
            if (fullSourcePath.empty()) {
                LOG_ERROR("Invalid source path: " + sourcePath);
                return false;
            }
            
            // Validate destination path
            std::string fullDestPath = m_validator.GetSafePath(m_baseDirectory, destPath);
            if (fullDestPath.empty()) {
                LOG_ERROR("Invalid destination path: " + destPath);
                return false;
            }
            
            // Validate files
            if (!m_validator.IsValidFile(fullSourcePath) || !m_validator.IsValidFile(fullDestPath)) {
                LOG_ERROR("File validation failed for copy operation");
                return false;
            }
            
            try {
                // Create destination directory if needed
                std::filesystem::path destPathObj(fullDestPath);
                std::filesystem::create_directories(destPathObj.parent_path());
                
                // Copy file
                std::filesystem::copy_file(fullSourcePath, fullDestPath);
                
                return true;
            }
            catch (const std::exception& e) {
                LOG_ERROR("File copy error: " + std::string(e.what()));
                return false;
            }
        }
        
        bool MoveFile(const std::string& sourcePath, const std::string& destPath) {
            if (!m_initialized) return false;
            
            // Validate paths
            std::string fullSourcePath = m_validator.GetSafePath(m_baseDirectory, sourcePath);
            std::string fullDestPath = m_validator.GetSafePath(m_baseDirectory, destPath);
            
            if (fullSourcePath.empty() || fullDestPath.empty()) {
                LOG_ERROR("Invalid path for move operation");
                return false;
            }
            
            // Validate files
            if (!m_validator.IsValidFile(fullSourcePath) || !m_validator.IsValidFile(fullDestPath)) {
                LOG_ERROR("File validation failed for move operation");
                return false;
            }
            
            try {
                // Create destination directory if needed
                std::filesystem::path destPathObj(fullDestPath);
                std::filesystem::create_directories(destPathObj.parent_path());
                
                // Move file
                std::filesystem::rename(fullSourcePath, fullDestPath);
                
                return true;
            }
            catch (const std::exception& e) {
                LOG_ERROR("File move error: " + std::string(e.what()));
                return false;
            }
        }
        
        PathValidator& GetValidator() {
            return m_validator;
        }
        
        void SetBaseDirectory(const std::string& baseDirectory) {
            m_baseDirectory = m_validator.SanitizePath(baseDirectory);
            m_validator.AddAllowedDirectory(m_baseDirectory);
        }
        
        std::string GetBaseDirectory() const {
            return m_baseDirectory;
        }
    };
    
} // namespace Secure

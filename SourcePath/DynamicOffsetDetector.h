/*
 * Dynamic Offset Detector for Rust Anti-Cheat Evasion System
 * Automatically detects and updates memory offsets after game updates
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <Windows.h>
#include <Psapi.h>
#include <algorithm>

namespace Memory {
    
    // Offset information structure
    struct OffsetInfo {
        std::string name;
        std::string description;
        uint64_t address;
        uint64_t offset;
        std::vector<uint8_t> pattern;
        std::vector<uint8_t> mask;
        bool isRelative;
        uint64_t baseAddress;
        std::vector<uint64_t> offsets;
        bool isValid;
        std::time_t lastUpdated;
        int confidence;
    };
    
    // Pattern matching result
    struct PatternMatch {
        uint64_t address;
        int confidence;
        std::vector<uint8_t> matchedBytes;
        std::string method;
    };
    
    // Game version information
    struct GameVersion {
        std::string version;
        std::string buildNumber;
        std::string executableName;
        uint32_t executableSize;
        std::time_t lastModified;
        std::vector<uint8_t> executableHash;
    };
    
    // Dynamic offset detector
    class DynamicOffsetDetector {
    private:
        std::map<std::string, OffsetInfo> m_offsets;
        std::map<std::string, std::vector<uint8_t>> m_patterns;
        std::map<std::string, std::vector<uint8_t>> m_masks;
        GameVersion m_currentVersion;
        uintptr_t m_moduleBase;
        size_t m_moduleSize;
        std::vector<uint8_t> m_moduleData;
        std::mutex m_mutex;
        bool m_initialized;
        HANDLE m_processHandle;
        
        // Predefined patterns for common game structures
        std::map<std::string, std::vector<std::string>> m_predefinedPatterns;
        
    public:
        DynamicOffsetDetector() : m_initialized(false), m_processHandle(nullptr), m_moduleBase(0), m_moduleSize(0) {
            InitializePredefinedPatterns();
        }
        
        ~DynamicOffsetDetector() {
            Cleanup();
        }
        
        bool Initialize(HANDLE processHandle, const std::string& moduleName = "Rust.exe") {
            if (m_initialized) return true;
            
            m_processHandle = processHandle;
            if (!m_processHandle) return false;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            // Get module information
            MODULEINFO moduleInfo = {};
            HMODULE hModule = GetModuleHandleA(moduleName.c_str());
            
            if (!hModule) {
                LOG_ERROR("Failed to get module handle for: " + moduleName);
                return false;
            }
            
            if (!GetModuleInformation(m_processHandle, hModule, &moduleInfo, sizeof(moduleInfo))) {
                LOG_ERROR("Failed to get module information");
                return false;
            }
            
            m_moduleBase = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
            m_moduleSize = moduleInfo.SizeOfImage;
            
            // Read module data for pattern scanning
            m_moduleData.resize(m_moduleSize);
            SIZE_T bytesRead = 0;
            
            if (!ReadProcessMemory(m_processHandle, reinterpret_cast<LPCVOID>(m_moduleBase), 
                                    m_moduleData.data(), m_moduleSize, &bytesRead) || bytesRead != m_moduleSize) {
                LOG_ERROR("Failed to read module data");
                return false;
            }
            
            // Get game version information
            UpdateGameVersion();
            
            // Initialize default offsets
            InitializeDefaultOffsets();
            
            m_initialized = true;
            LOG_INFO("Dynamic offset detector initialized successfully");
            
            return true;
        }
        
        void Cleanup() {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            m_moduleData.clear();
            m_offsets.clear();
            m_patterns.clear();
            m_masks.clear();
            m_moduleBase = 0;
            m_moduleSize = 0;
            m_initialized = false;
        }
        
        bool DetectAllOffsets() {
            if (!m_initialized) return false;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            bool success = true;
            
            // Detect entity list offset
            if (!DetectOffset("EntityList")) success = false;
            
            // Detect entity count offset
            if (!DetectOffset("EntityCount")) success = false;
            
            // Detect local player offset
            if (!DetectOffset("LocalPlayer")) success = false;
            
            // Detect health offset
            if (!DetectOffset("Health")) success = false;
            
            // Detect position offset
            if (!DetectOffset("Position")) success = false;
            
            // Detect team offset
            if (!DetectOffset("Team")) success = false;
            
            // Detect weapon offset
            if (!DetectOffset("Weapon")) success = false;
            
            // Detect view matrix offset
            if (!DetectOffset("ViewMatrix")) success = false;
            
            // Detect name offset
            if (!DetectOffset("Name")) success = false;
            
            // Detect bone matrix offset
            if (!DetectOffset("BoneMatrix")) success = false;
            
            // Detect ammo offset
            if (!DetectOffset("Ammo")) success = false;
            
            LOG_INFO("Offset detection completed with " + std::string(success ? "success" : "partial success"));
            return success;
        }
        
        bool DetectOffset(const std::string& offsetName) {
            if (!m_initialized) return false;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_predefinedPatterns.find(offsetName);
            if (it == m_predefinedPatterns.end()) {
                LOG_ERROR("No patterns defined for offset: " + offsetName);
                return false;
            }
            
            OffsetInfo offsetInfo;
            offsetInfo.name = offsetName;
            offsetInfo.lastUpdated = std::time(nullptr);
            offsetInfo.confidence = 0;
            
            // Try each pattern
            for (const auto& patternStr : it->second) {
                PatternMatch match = FindPattern(patternStr);
                
                if (match.address != 0 && match.confidence > offsetInfo.confidence) {
                    offsetInfo.address = match.address;
                    offsetInfo.confidence = match.confidence;
                    offsetInfo.method = match.method;
                    
                    // Extract pattern and mask
                    ExtractPatternAndMask(patternStr, offsetInfo.pattern, offsetInfo.mask);
                }
            }
            
            if (offsetInfo.confidence > 0) {
                // Calculate relative offset
                offsetInfo.offset = offsetInfo.address - m_moduleBase;
                offsetInfo.baseAddress = m_moduleBase;
                offsetInfo.isValid = true;
                
                // Add multi-level offset calculation
                CalculateMultiLevelOffsets(offsetInfo);
                
                m_offsets[offsetName] = offsetInfo;
                
                LOG_INFO("Detected offset " + offsetName + " at 0x" + 
                        std::to_string(offsetInfo.offset) + " with confidence " + 
                        std::to_string(offsetInfo.confidence));
                
                return true;
            } else {
                LOG_ERROR("Failed to detect offset: " + offsetName);
                return false;
            }
        }
        
        uint64_t GetOffset(const std::string& offsetName) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_offsets.find(offsetName);
            if (it != m_offsets.end() && it->second.isValid) {
                return it->second.offset;
            }
            
            return 0;
        }
        
        std::vector<uint64_t> GetMultiLevelOffset(const std::string& offsetName) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_offsets.find(offsetName);
            if (it != m_offsets.end() && it->second.isValid) {
                return it->second.offsets;
            }
            
            return {};
        }
        
        bool IsOffsetValid(const std::string& offsetName) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_offsets.find(offsetName);
            return (it != m_offsets.end() && it->second.isValid);
        }
        
        std::map<std::string, OffsetInfo> GetAllOffsets() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_offsets;
        }
        
        bool UpdateOffsets() {
            if (!m_initialized) return false;
            
            // Check if game version changed
            GameVersion currentVersion = GetGameVersion();
            if (currentVersion.version != m_currentVersion.version ||
                currentVersion.buildNumber != m_currentVersion.buildNumber) {
                
                LOG_INFO("Game version changed, updating offsets");
                m_currentVersion = currentVersion;
                
                // Re-read module data
                SIZE_T bytesRead = 0;
                if (!ReadProcessMemory(m_processHandle, reinterpret_cast<LPCVOID>(m_moduleBase), 
                                        m_moduleData.data(), m_moduleSize, &bytesRead) || bytesRead != m_moduleSize) {
                    LOG_ERROR("Failed to re-read module data");
                    return false;
                }
                
                // Redetect all offsets
                return DetectAllOffsets();
            }
            
            return true;
        }
        
        bool ValidateOffsets() {
            if (!m_initialized) return false;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            bool allValid = true;
            
            for (auto& pair : m_offsets) {
                OffsetInfo& offset = pair.second;
                
                if (!offset.isValid) continue;
                
                // Validate address is within module bounds
                if (offset.address < m_moduleBase || offset.address >= m_moduleBase + m_moduleSize) {
                    LOG_WARNING("Offset " + offset.name + " is out of bounds, marking as invalid");
                    offset.isValid = false;
                    allValid = false;
                    continue;
                }
                
                // Validate pattern still matches
                if (!ValidatePattern(offset)) {
                    LOG_WARNING("Offset " + offset.name + " pattern no longer matches, marking as invalid");
                    offset.isValid = false;
                    allValid = false;
                    continue;
                }
                
                // Update confidence based on validation
                offset.confidence = std::min(100, offset.confidence + 10);
            }
            
            return allValid;
        }
        
        void AddCustomPattern(const std::string& offsetName, const std::string& pattern) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            m_predefinedPatterns[offsetName].push_back(pattern);
        }
        
        void AddCustomPattern(const std::string& offsetName, const std::vector<std::string>& patterns) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            for (const auto& pattern : patterns) {
                m_predefinedPatterns[offsetName].push_back(pattern);
            }
        }
        
        GameVersion GetGameVersion() const {
            return m_currentVersion;
        }
        
        bool IsInitialized() const {
            return m_initialized;
        }
        
    private:
        void InitializePredefinedPatterns() {
            // Entity list patterns
            m_predefinedPatterns["EntityList"] = {
                "48 8B 0D ? ? ? ? 48 8B 14 ? ? ? ? 4C 8B 5C ? ? ? 48 8B 74 ? ? ? 4C 8B 7C ? ? ? 49 8B 7C ? ? ? 48 85 C9",
                "48 8B 05 ? ? ? ? 48 8B 0C ? ? ? ? 48 8B 5C ? ? ? 48 8B 74 ? ? ? 48 85 C9",
                "48 8B 1D ? ? ? ? 48 8B 5B ? ? ? ? 48 8B 73 ? ? ? 48 8B 7B ? ? ? ? 48 85 C9"
            };
            
            // Entity count patterns
            m_predefinedPatterns["EntityCount"] = {
                "8B 05 ? ? ? ? 83 F8 7F 7F 0E",
                "48 8B 05 ? ? ? ? 48 83 F8 7F 48 0F 4F C8",
                "8B 0D ? ? ? ? 83 F9 7F 7F 0E"
            };
            
            // Local player patterns
            m_predefinedPatterns["LocalPlayer"] = {
                "48 8B 05 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9",
                "48 8B 0D ? ? ? ? 48 8B 89 ? ? ? ? 48 85 C9",
                "48 8B 1D ? ? ? ? 48 8B 9B ? ? ? ? 48 85 C9"
            };
            
            // Health patterns
            m_predefinedPatterns["Health"] = {
                "F3 0F 11 8B ? ? ? ? E8 ? ? ? ? 48 85 C0",
                "F3 44 0F 11 83 ? ? ? ? E8 ? ? ? ? 48 85 C0",
                "89 83 ? ? ? ? E8 ? ? ? ? 48 85 C0"
            };
            
            // Position patterns
            m_predefinedPatterns["Position"] = {
                "F3 0F 11 8B ? ? ? ? F3 0F 11 8B ? ? ? ? F3 0F 11 8B ? ? ? ?",
                "48 8B 83 ? ? ? ? 48 8B 8B ? ? ? ? 48 8B 93 ? ? ? ?",
                "C7 83 ? ? ? ? ? ? ? ? C7 83 ? ? ? ? ? ? ? ?"
            };
            
            // Team patterns
            m_predefinedPatterns["Team"] = {
                "89 83 ? ? ? ? 48 8B 83 ? ? ? ? 89 83 ? ? ? ?",
                "48 8B 83 ? ? ? ? 48 8B 8B ? ? ? ? 48 8B 93 ? ? ? ?",
                "C7 83 ? ? ? ? ? ? ? ? 89 83 ? ? ? ?"
            };
            
            // Weapon patterns
            m_predefinedPatterns["Weapon"] = {
                "48 8B 83 ? ? ? ? 48 8B 40 ? 48 89 83 ? ? ? ?",
                "48 8B 8B ? ? ? ? 48 8B 40 ? 48 89 8B ? ? ? ?",
                "48 8B 93 ? ? ? ? 48 8B 40 ? 48 89 93 ? ? ? ?"
            };
            
            // View matrix patterns
            m_predefinedPatterns["ViewMatrix"] = {
                "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 40 ?",
                "48 8B 15 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 05 ? ? ? ?",
                "48 8D 15 ? ? ? ? E8 ? ? ? ? 48 8B 1D ? ? ? ? 48 8B 43 ?"
            };
            
            // Name patterns
            m_predefinedPatterns["Name"] = {
                "48 8B 83 ? ? ? ? 48 8B 40 ? 48 85 C0 75 ? E8 ? ? ? ?",
                "48 8B 8B ? ? ? ? 48 8B 40 ? 48 85 C0 75 ? E8 ? ? ? ?",
                "48 8B 93 ? ? ? ? 48 8B 40 ? 48 85 C0 75 ? E8 ? ? ? ?"
            };
            
            // Bone matrix patterns
            m_predefinedPatterns["BoneMatrix"] = {
                "48 8B 83 ? ? ? ? 48 8B 80 ? ? ? ? 48 8B 00 48 85 C9",
                "48 8B 8B ? ? ? ? 48 8B 80 ? ? ? ? 48 8B 00 48 85 C9",
                "48 8B 93 ? ? ? ? 48 8B 80 ? ? ? ? 48 8B 00 48 85 C9"
            };
            
            // Ammo patterns
            m_predefinedPatterns["Ammo"] = {
                "89 83 ? ? ? ? 48 8B 83 ? ? ? ? 89 83 ? ? ? ?",
                "48 8B 83 ? ? ? ? 48 8B 40 ? 48 89 83 ? ? ? ?",
                "C7 83 ? ? ? ? ? ? ? ? 89 83 ? ? ? ?"
            };
        }
        
        void InitializeDefaultOffsets() {
            // Initialize with default values (will be updated by detection)
            OffsetInfo info;
            
            // Entity list
            info.name = "EntityList";
            info.offset = 0x12345678; // Default placeholder
            info.isValid = false;
            m_offsets["EntityList"] = info;
            
            // Entity count
            info.name = "EntityCount";
            info.offset = 0x12345679;
            info.isValid = false;
            m_offsets["EntityCount"] = info;
            
            // Local player
            info.name = "LocalPlayer";
            info.offset = 0x1234567A;
            info.isValid = false;
            m_offsets["LocalPlayer"] = info;
        }
        
        PatternMatch FindPattern(const std::string& patternStr) {
            PatternMatch match = {};
            
            std::vector<uint8_t> pattern;
            std::vector<uint8_t> mask;
            
            if (!ParsePattern(patternStr, pattern, mask)) {
                return match;
            }
            
            // Search in module data
            for (size_t i = 0; i <= m_moduleData.size() - pattern.size(); i++) {
                bool found = true;
                
                for (size_t j = 0; j < pattern.size(); j++) {
                    if (mask[j] == 0xFF && m_moduleData[i + j] != pattern[j]) {
                        found = false;
                        break;
                    }
                }
                
                if (found) {
                    match.address = m_moduleBase + i;
                    match.confidence = 100;
                    match.method = "Pattern Scan";
                    
                    // Copy matched bytes for verification
                    match.matchedBytes.resize(pattern.size());
                    for (size_t j = 0; j < pattern.size(); j++) {
                        match.matchedBytes[j] = m_moduleData[i + j];
                    }
                    
                    return match;
                }
            }
            
            return match;
        }
        
        bool ParsePattern(const std::string& patternStr, std::vector<uint8_t>& pattern, std::vector<uint8_t>& mask) {
            std::vector<std::string> bytes = SplitString(patternStr, ' ');
            
            for (const auto& byteStr : bytes) {
                if (byteStr == "?") {
                    pattern.push_back(0x00);
                    mask.push_back(0x00);
                } else {
                    uint8_t byte = static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16));
                    pattern.push_back(byte);
                    mask.push_back(0xFF);
                }
            }
            
            return !pattern.empty();
        }
        
        void ExtractPatternAndMask(const std::string& patternStr, std::vector<uint8_t>& pattern, std::vector<uint8_t>& mask) {
            ParsePattern(patternStr, pattern, mask);
        }
        
        void CalculateMultiLevelOffsets(OffsetInfo& offsetInfo) {
            // For now, just add the base offset
            offsetInfo.offsets.clear();
            offsetInfo.offsets.push_back(offsetInfo.offset);
            
            // In a real implementation, this would calculate multi-level pointer chains
            // based on the pattern and additional analysis
        }
        
        bool ValidatePattern(const OffsetInfo& offsetInfo) {
            if (offsetInfo.pattern.empty() || offsetInfo.mask.empty()) {
                return true; // No pattern to validate
            }
            
            // Read bytes at the address
            std::vector<uint8_t> bytes;
            bytes.resize(offsetInfo.pattern.size());
            
            SIZE_T bytesRead = 0;
            if (!ReadProcessMemory(m_processHandle, reinterpret_cast<LPCVOID>(offsetInfo.address), 
                                    bytes.data(), bytes.size(), &bytesRead) || bytesRead != bytes.size()) {
                return false;
            }
            
            // Validate pattern matches
            for (size_t i = 0; i < offsetInfo.pattern.size(); i++) {
                if (offsetInfo.mask[i] == 0xFF && bytes[i] != offsetInfo.pattern[i]) {
                    return false;
                }
            }
            
            return true;
        }
        
        void UpdateGameVersion() {
            m_currentVersion.executableName = "Rust.exe";
            
            // Get executable size
            WIN32_FILE_ATTRIBUTE_DATA fileData;
            if (GetFileAttributesExA("Rust.exe", GetFileExInfoStandard, &fileData)) {
                m_currentVersion.executableSize = fileData.nFileSizeLow;
                m_currentVersion.lastModified = fileData.ftLastWriteTime.dwLowDateTime;
            }
            
            // Calculate hash
            m_currentVersion.executableHash = CalculateHash(m_moduleData);
            
            // Try to extract version from resources
            m_currentVersion.version = ExtractVersionFromResources();
            m_currentVersion.buildNumber = ExtractBuildNumber();
        }
        
        GameVersion GetGameVersion() const {
            return m_currentVersion;
        }
        
        std::vector<uint8_t> CalculateHash(const std::vector<uint8_t>& data) {
            std::vector<uint8_t> hash(32); // SHA256
            
            // Simplified hash calculation (in real implementation, use proper SHA256)
            for (size_t i = 0; i < data.size(); i++) {
                hash[i % 32] ^= data[i];
            }
            
            return hash;
        }
        
        std::string ExtractVersionFromResources() {
            // In a real implementation, this would extract version from PE resources
            return "1.0.0"; // Placeholder
        }
        
        std::string ExtractBuildNumber() {
            // In a real implementation, this would extract build number from PE resources
            return "12345"; // Placeholder
        }
        
        std::vector<std::string> SplitString(const std::string& str, char delimiter) {
            std::vector<std::string> result;
            std::stringstream ss(str);
            std::string item;
            
            while (std::getline(ss, item, delimiter)) {
                result.push_back(item);
            }
            
            return result;
        }
    };
    
} // namespace Memory

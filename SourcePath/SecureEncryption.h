/*
 * AES-256 Encryption System for Rust Anti-Cheat Evasion System
 * Provides secure encryption for configuration files and sensitive data
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <wincrypt.h>

namespace Secure {
    
    // AES-256 encryption class
    class AES256Encryptor {
    private:
        std::vector<uint8_t> m_key;
        std::vector<uint8_t> m_iv;
        HCRYPTPROV m_cryptoProvider;
        HCRYPTKEY m_cryptoKey;
        bool m_initialized;
        
    public:
        AES256Encryptor() : m_cryptoProvider(0), m_cryptoKey(0), m_initialized(false) {
            Initialize();
        }
        
        ~AES256Encryptor() {
            Cleanup();
        }
        
        bool Initialize() {
            if (m_initialized) return true;
            
            // Acquire cryptographic context
            if (!CryptAcquireContext(&m_cryptoProvider, NULL, NULL, PROV_RSA_AES, 0)) {
                if (GetLastError() != NTE_BAD_KEYSET) {
                    return false;
                }
                
                // Try to create a new key container
                if (!CryptAcquireContext(&m_cryptoProvider, NULL, NULL, PROV_RSA_AES, CRYPT_NEWKEYSET)) {
                    return false;
                }
            }
            
            // Generate random key and IV
            GenerateRandomKey();
            GenerateRandomIV();
            
            // Create the key
            if (!CreateKey()) {
                Cleanup();
                return false;
            }
            
            m_initialized = true;
            return true;
        }
        
        bool SetKey(const std::string& key) {
            if (key.length() != 32) return false; // AES-256 requires 32-byte key
            
            m_key.clear();
            m_key.reserve(32);
            for (size_t i = 0; i < 32; i++) {
                m_key.push_back(static_cast<uint8_t>(key[i]));
            }
            
            return true;
        }
        
        bool SetKey(const std::vector<uint8_t>& key) {
            if (key.size() != 32) return false;
            
            m_key = key;
            return true;
        }
        
        bool SetIV(const std::vector<uint8_t>& iv) {
            if (iv.size() != 16) return false; // AES block size is 16 bytes
            
            m_iv = iv;
            return true;
        }
        
        std::vector<uint8_t> GetKey() const {
            return m_key;
        }
        
        std::vector<uint8_t> GetIV() const {
            return m_iv;
        }
        
        std::string Encrypt(const std::string& plaintext) {
            if (!m_initialized) return "";
            
            std::vector<uint8_t> input(plaintext.begin(), plaintext.end());
            std::vector<uint8_t> encrypted = Encrypt(input);
            
            return Base64Encode(encrypted);
        }
        
        std::string Decrypt(const std::string& ciphertext) {
            if (!m_initialized) return "";
            
            std::vector<uint8_t> encrypted = Base64Decode(ciphertext);
            std::vector<uint8_t> decrypted = Decrypt(encrypted);
            
            return std::string(decrypted.begin(), decrypted.end());
        }
        
        std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& data) {
            if (!m_initialized) return {};
            
            // Pad data to block size
            std::vector<uint8_t> paddedData = PadData(data);
            
            // Prepare encryption buffer
            std::vector<uint8_t> encrypted(paddedData.size());
            
            // Set IV
            if (!SetIVParameter()) {
                return {};
            }
            
            // Encrypt data
            DWORD encryptedLen = static_cast<DWORD>(paddedData.size());
            if (!CryptEncrypt(m_cryptoKey, 0, FALSE, 0, encrypted.data(), &encryptedLen, encryptedLen)) {
                return {};
            }
            
            encrypted.resize(encryptedLen);
            return encrypted;
        }
        
        std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& data) {
            if (!m_initialized || data.empty()) return {};
            
            // Set IV
            if (!SetIVParameter()) {
                return {};
            }
            
            // Prepare decryption buffer
            std::vector<uint8_t> decrypted(data.size());
            
            // Decrypt data
            DWORD decryptedLen = static_cast<DWORD>(data.size());
            if (!CryptDecrypt(m_cryptoKey, 0, FALSE, 0, decrypted.data(), &decryptedLen)) {
                return {};
            }
            
            decrypted.resize(decryptedLen);
            
            // Remove padding
            return RemovePadding(decrypted);
        }
        
        // Static methods for key generation
        static std::vector<uint8_t> GenerateRandomKey() {
            std::vector<uint8_t> key(32);
            
            HCRYPTPROV prov;
            if (CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, 0)) {
                CryptGenRandom(prov, 32, key.data());
                CryptReleaseContext(prov, 0);
            }
            
            return key;
        }
        
        static std::vector<uint8_t> GenerateRandomIV() {
            std::vector<uint8_t> iv(16);
            
            HCRYPTPROV prov;
            if (CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, 0)) {
                CryptGenRandom(prov, 16, iv.data());
                CryptReleaseContext(prov, 0);
            }
            
            return iv;
        }
        
        // Key derivation from password
        static std::vector<uint8_t> DeriveKeyFromPassword(const std::string& password, 
                                                         const std::vector<uint8_t>& salt, 
                                                         int iterations = 10000) {
            // Simple PBKDF2-like key derivation (simplified)
            std::vector<uint8_t> key(32);
            std::vector<uint8_t> hash(32);
            
            // Initial hash
            std::string input = password + std::string(salt.begin(), salt.end());
            
            for (int i = 0; i < iterations; i++) {
                // In a real implementation, use proper PBKDF2
                for (size_t j = 0; j < input.length(); j++) {
                    hash[j % 32] ^= static_cast<uint8_t>(input[j]);
                }
                
                input = std::string(hash.begin(), hash.end());
            }
            
            // Copy to key
            for (int i = 0; i < 32; i++) {
                key[i] = hash[i];
            }
            
            return key;
        }
        
    private:
        void GenerateRandomKey() {
            m_key = GenerateRandomKey();
        }
        
        void GenerateRandomIV() {
            m_iv = GenerateRandomIV();
        }
        
        bool CreateKey() {
            if (m_key.size() != 32) return false;
            
            // Create blob structure for the key
            struct {
                BLOBHEADER header;
                DWORD keySize;
                BYTE keyData[32];
            } keyBlob;
            
            keyBlob.header.bType = PLAINTEXTKEYBLOB;
            keyBlob.header.bVersion = CUR_BLOB_VERSION;
            keyBlob.header.reserved = 0;
            keyBlob.header.aiKeyAlg = CALG_AES_256;
            keyBlob.keySize = 32;
            
            memcpy(keyBlob.keyData, m_key.data(), 32);
            
            // Import the key
            if (!CryptImportKey(m_cryptoProvider, reinterpret_cast<BYTE*>(&keyBlob), 
                              sizeof(keyBlob), 0, 0, &m_cryptoKey)) {
                return false;
            }
            
            return true;
        }
        
        bool SetIVParameter() {
            if (m_iv.size() != 16) return false;
            
            // Set IV parameter
            DWORD mode = CRYPT_MODE_CBC;
            if (!CryptSetKeyParam(m_cryptoKey, KP_MODE, reinterpret_cast<BYTE*>(&mode), 0)) {
                return false;
            }
            
            if (!CryptSetKeyParam(m_cryptoKey, KP_IV, m_iv.data(), 0)) {
                return false;
            }
            
            return true;
        }
        
        std::vector<uint8_t> PadData(const std::vector<uint8_t>& data) {
            std::vector<uint8_t> padded = data;
            
            // PKCS#7 padding
            uint8_t paddingValue = 16 - (data.size() % 16);
            for (int i = 0; i < paddingValue; i++) {
                padded.push_back(paddingValue);
            }
            
            return padded;
        }
        
        std::vector<uint8_t> RemovePadding(const std::vector<uint8_t>& data) {
            if (data.empty()) return {};
            
            uint8_t paddingValue = data.back();
            if (paddingValue == 0 || paddingValue > 16) {
                return data; // Invalid padding
            }
            
            // Verify padding
            for (int i = 1; i <= paddingValue; i++) {
                if (data[data.size() - i] != paddingValue) {
                    return data; // Invalid padding
                }
            }
            
            // Remove padding
            return std::vector<uint8_t>(data.begin(), data.end() - paddingValue);
        }
        
        std::string Base64Encode(const std::vector<uint8_t>& data) {
            const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string result;
            
            int val = 0, valb = -6;
            for (uint8_t c : data) {
                val = (val << 8) + c;
                valb += 8;
                while (valb >= 0) {
                    result.push_back(chars[(val >> valb) & 0x3F]);
                    valb -= 6;
                }
            }
            
            if (valb > -6) {
                result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
            }
            
            while (result.size() % 4) {
                result.push_back('=');
            }
            
            return result;
        }
        
        std::vector<uint8_t> Base64Decode(const std::string& str) {
            const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::vector<uint8_t> result;
            
            int val = 0, valb = -8;
            for (char c : str) {
                if (c == '=') break;
                
                const char* pos = strchr(chars, c);
                if (!pos) continue;
                
                val = (val << 6) + static_cast<int>(pos - chars);
                valb += 6;
                
                if (valb >= 0) {
                    result.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
                    valb -= 8;
                }
            }
            
            return result;
        }
        
        void Cleanup() {
            if (m_cryptoKey) {
                CryptDestroyKey(m_cryptoKey);
                m_cryptoKey = 0;
            }
            
            if (m_cryptoProvider) {
                CryptReleaseContext(m_cryptoProvider, 0);
                m_cryptoProvider = 0;
            }
            
            m_initialized = false;
        }
    };
    
    // Secure configuration manager with encryption
    class SecureConfigManager {
    private:
        AES256Encryptor m_encryptor;
        std::string m_configFile;
        std::map<std::string, std::string> m_configCache;
        std::vector<uint8_t> m_encryptionKey;
        std::vector<uint8_t> m_salt;
        bool m_isEncrypted;
        
    public:
        SecureConfigManager(const std::string& configFile, bool encrypt = true) 
            : m_configFile(configFile), m_isEncrypted(encrypt) {
            
            // Generate or load encryption key
            if (m_isEncrypted) {
                LoadOrGenerateKey();
            }
        }
        
        bool SetString(const std::string& key, const std::string& value) {
            m_configCache[key] = value;
            return SaveConfiguration();
        }
        
        std::string GetString(const std::string& key, const std::string& defaultValue = "") {
            if (m_configCache.empty()) {
                LoadConfiguration();
            }
            
            auto it = m_configCache.find(key);
            return (it != m_configCache.end()) ? it->second : defaultValue;
        }
        
        bool SetBool(const std::string& key, bool value) {
            return SetString(key, value ? "true" : "false");
        }
        
        bool GetBool(const std::string& key, bool defaultValue = false) {
            std::string value = GetString(key, defaultValue ? "true" : "false");
            return (value == "true" || value == "1" || value == "yes");
        }
        
        bool SetInt(const std::string& key, int value) {
            return SetString(key, std::to_string(value));
        }
        
        int GetInt(const std::string& key, int defaultValue = 0) {
            std::string value = GetString(key, std::to_string(defaultValue));
            try {
                return std::stoi(value);
            } catch (...) {
                return defaultValue;
            }
        }
        
        bool SetFloat(const std::string& key, float value) {
            return SetString(key, std::to_string(value));
        }
        
        float GetFloat(const std::string& key, float defaultValue = 0.0f) {
            std::string value = GetString(key, std::to_string(defaultValue));
            try {
                return std::stof(value);
            } catch (...) {
                return defaultValue;
            }
        }
        
        bool SaveConfiguration() {
            try {
                std::stringstream config;
                
                // Write configuration data
                for (const auto& pair : m_configCache) {
                    config << pair.first << "=" << pair.second << "\n";
                }
                
                std::string configData = config.str();
                
                if (m_isEncrypted) {
                    // Encrypt configuration
                    std::string encrypted = m_encryptor.Encrypt(configData);
                    
                    // Write encrypted data
                    std::ofstream file(m_configFile, std::ios::binary);
                    if (!file.is_open()) return false;
                    
                    file << "ENCRYPTED\n";
                    file << "KEY=" << Base64Encode(m_encryptionKey) << "\n";
                    file << "SALT=" << Base64Encode(m_salt) << "\n";
                    file << "DATA=" << encrypted << "\n";
                    
                    file.close();
                } else {
                    // Write plain text
                    std::ofstream file(m_configFile);
                    if (!file.is_open()) return false;
                    
                    file << configData;
                    file.close();
                }
                
                return true;
            }
            catch (...) {
                return false;
            }
        }
        
        bool LoadConfiguration() {
            std::ifstream file(m_configFile);
            if (!file.is_open()) return false;
            
            try {
                if (m_isEncrypted) {
                    // Read encrypted file
                    std::string line;
                    std::string keyData, saltData, encryptedData;
                    
                    while (std::getline(file, line)) {
                        if (line.find("KEY=") == 0) {
                            keyData = line.substr(4);
                        } else if (line.find("SALT=") == 0) {
                            saltData = line.substr(5);
                        } else if (line.find("DATA=") == 0) {
                            encryptedData = line.substr(5);
                        }
                    }
                    
                    file.close();
                    
                    if (keyData.empty() || saltData.empty() || encryptedData.empty()) {
                        return false;
                    }
                    
                    // Decode key and salt
                    m_encryptionKey = Base64Decode(keyData);
                    m_salt = Base64Decode(saltData);
                    
                    // Set encryption key
                    if (!m_encryptor.SetKey(m_encryptionKey)) {
                        return false;
                    }
                    
                    // Decrypt configuration
                    std::string decrypted = m_encryptor.Decrypt(encryptedData);
                    
                    // Parse configuration
                    ParseConfiguration(decrypted);
                } else {
                    // Read plain text
                    std::string content((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
                    file.close();
                    
                    ParseConfiguration(content);
                }
                
                return true;
            }
            catch (...) {
                return false;
            }
        }
        
        void SetEncryptionKey(const std::vector<uint8_t>& key) {
            m_encryptionKey = key;
            m_encryptor.SetKey(key);
        }
        
        void SetEncryptionKey(const std::string& password) {
            m_encryptionKey = AES256Encryptor::DeriveKeyFromPassword(password, m_salt);
            m_encryptor.SetKey(m_encryptionKey);
        }
        
        std::vector<uint8_t> GetEncryptionKey() const {
            return m_encryptionKey;
        }
        
        bool IsEncrypted() const {
            return m_isEncrypted;
        }
        
        void EnableEncryption(bool enable) {
            m_isEncrypted = enable;
            if (enable && m_encryptionKey.empty()) {
                LoadOrGenerateKey();
            }
        }
        
        std::map<std::string, std::string> GetAllConfig() const {
            return m_configCache;
        }
        
        void ClearCache() {
            m_configCache.clear();
        }
        
    private:
        void LoadOrGenerateKey() {
            // Try to load existing key
            std::ifstream keyFile("rust_config.key");
            if (keyFile.is_open()) {
                std::string keyData, saltData;
                std::string line;
                
                while (std::getline(keyFile, line)) {
                    if (line.find("KEY=") == 0) {
                        keyData = line.substr(4);
                    } else if (line.find("SALT=") == 0) {
                        saltData = line.substr(5);
                    }
                }
                
                keyFile.close();
                
                if (!keyData.empty() && !saltData.empty()) {
                    m_encryptionKey = Base64Decode(keyData);
                    m_salt = Base64Decode(saltData);
                }
            }
            
            // Generate new key if needed
            if (m_encryptionKey.empty()) {
                m_encryptionKey = AES256Encryptor::GenerateRandomKey();
                m_salt = AES256Encryptor::GenerateRandomIV();
                
                // Save key
                std::ofstream keyFile("rust_config.key");
                if (keyFile.is_open()) {
                    keyFile << "KEY=" << Base64Encode(m_encryptionKey) << "\n";
                    keyFile << "SALT=" << Base64Encode(m_salt) << "\n";
                    keyFile.close();
                }
            }
            
            // Set key for encryptor
            m_encryptor.SetKey(m_encryptionKey);
        }
        
        void ParseConfiguration(const std::string& content) {
            std::stringstream ss(content);
            std::string line;
            
            while (std::getline(ss, line)) {
                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    m_configCache[key] = value;
                }
            }
        }
        
        std::string Base64Encode(const std::vector<uint8_t>& data) {
            return m_encryptor.Encrypt(std::string(data.begin(), data.end()));
        }
        
        std::vector<uint8_t> Base64Decode(const std::string& data) {
            // Simple base64 decode (simplified)
            std::vector<uint8_t> result;
            // In a real implementation, use proper base64 decoding
            return result;
        }
    };
    
} // namespace Secure

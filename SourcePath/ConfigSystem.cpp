#include "ConfigSystem.h"
#include <unordered_map>
#include <algorithm>

namespace Config {

    CConfigSystem::CConfigSystem() : m_encryptionKey("RustExternal2026!@#") {}

    bool CConfigSystem::Initialize(const std::string& configName) {
        m_configPath = GetConfigDirectory() + "\\" + configName + ".cfg";
        return true;
    }

    std::string CConfigSystem::GetConfigDirectory() {
        char appData[MAX_PATH] = {};
        SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, appData);
        std::string path = std::string(appData) + "\\RustExternal\\Configs";
        CreateDirectoryA((std::string(appData) + "\\RustExternal").c_str(), nullptr);
        CreateDirectoryA(path.c_str(), nullptr);
        return path;
    }

    std::string CConfigSystem::XORCipher(const std::string& data) {
        std::string result = data;
        for (size_t i = 0; i < result.size(); i++) {
            result[i] ^= m_encryptionKey[i % m_encryptionKey.size()];
        }
        return result;
    }

    bool CConfigSystem::LoadString(const std::string& key, std::string& value, const std::string& defaultVal) {
        value = defaultVal;
        std::ifstream file(m_configPath);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            line = XORCipher(line);
            size_t eq = line.find('=');
            if (eq != std::string::npos && line.substr(0, eq) == key) {
                value = line.substr(eq + 1);
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }

    bool CConfigSystem::LoadInt(const std::string& key, int& value, int defaultVal) {
        std::string strVal;
        if (LoadString(key, strVal)) {
            value = std::stoi(strVal);
            return true;
        }
        value = defaultVal;
        return false;
    }

    bool CConfigSystem::LoadFloat(const std::string& key, float& value, float defaultVal) {
        std::string strVal;
        if (LoadString(key, strVal)) {
            value = std::stof(strVal);
            return true;
        }
        value = defaultVal;
        return false;
    }

    bool CConfigSystem::LoadBool(const std::string& key, bool& value, bool defaultVal) {
        std::string strVal;
        if (LoadString(key, strVal)) {
            value = (strVal == "1" || strVal == "true");
            return true;
        }
        value = defaultVal;
        return false;
    }

    bool CConfigSystem::SaveString(const std::string& key, const std::string& value) {
        std::unordered_map<std::string, std::string> data;

        std::ifstream inFile(m_configPath);
        if (inFile.is_open()) {
            std::string line;
            while (std::getline(inFile, line)) {
                line = XORCipher(line);
                size_t eq = line.find('=');
                if (eq != std::string::npos) {
                    data[line.substr(0, eq)] = line.substr(eq + 1);
                }
            }
            inFile.close();
        }

        data[key] = value;

        std::ofstream outFile(m_configPath);
        if (!outFile.is_open()) return false;

        for (const auto& [k, v] : data) {
            outFile << XORCipher(k + "=" + v) << "\n";
        }
        outFile.close();

        DWORD attrs = GetFileAttributesA(m_configPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES) {
            SetFileAttributesA(m_configPath.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
        }

        return true;
    }

    bool CConfigSystem::SaveInt(const std::string& key, int value) {
        return SaveString(key, std::to_string(value));
    }

    bool CConfigSystem::SaveFloat(const std::string& key, float value) {
        return SaveString(key, std::to_string(value));
    }

    bool CConfigSystem::SaveBool(const std::string& key, bool value) {
        return SaveString(key, value ? "1" : "0");
    }

    bool CConfigSystem::Save() { return true; }
    bool CConfigSystem::Load() { return true; }

}

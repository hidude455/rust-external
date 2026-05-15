#pragma once
#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <shlobj.h>

namespace Config {

    class CConfigSystem {
    private:
        std::string m_configPath;
        std::string m_encryptionKey;

        std::string GetConfigDirectory();
        std::string XORCipher(const std::string& data);

    public:
        CConfigSystem();

        bool Initialize(const std::string& configName);

        bool LoadString(const std::string& key, std::string& value, const std::string& defaultVal = "");
        bool LoadInt(const std::string& key, int& value, int defaultVal = 0);
        bool LoadFloat(const std::string& key, float& value, float defaultVal = 0.0f);
        bool LoadBool(const std::string& key, bool& value, bool defaultVal = false);

        bool SaveString(const std::string& key, const std::string& value);
        bool SaveInt(const std::string& key, int value);
        bool SaveFloat(const std::string& key, float value);
        bool SaveBool(const std::string& key, bool value);

        bool Save();
        bool Load();

        void SetEncryptionKey(const std::string& key) { m_encryptionKey = key; }
    };

}

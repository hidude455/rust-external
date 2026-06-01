#pragma once
#include <string>
#include <vector>

class CRustInjector;

class ConfigurationManager {
public:
    ConfigurationManager(CRustInjector* injector);
    ~ConfigurationManager();

    bool LoadConfiguration(const std::string& profileName);
    bool SaveConfiguration(const std::string& profileName);
    std::vector<std::string> GetAvailableProfiles();

private:
    CRustInjector* m_injector;
    std::string m_configDirectory;

    void CreateConfigDirectoryIfNotExists();
    std::string GetProfilePath(const std::string& profileName);

    void WriteBool(const char* section, const char* key, bool value, const char* file);
    void WriteFloat(const char* section, const char* key, float value, const char* file);
    void WriteInt(const char* section, const char* key, int value, const char* file);

    bool ReadBool(const char* section, const char* key, bool defaultValue, const char* file);
    float ReadFloat(const char* section, const char* key, float defaultValue, const char* file);
    int ReadInt(const char* section, const char* key, int defaultValue, const char* file);
};

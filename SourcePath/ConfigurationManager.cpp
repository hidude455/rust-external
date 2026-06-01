#include "ConfigurationManager.h"
#include "RustInjector.h"
#include <windows.h>
#include <shlwapi.h>
#include <direct.h>
#include <algorithm>

#pragma comment(lib, "shlwapi.lib")

ConfigurationManager::ConfigurationManager(CRustInjector* injector) : m_injector(injector) {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    PathRemoveFileSpecA(path);
    m_configDirectory = std::string(path) + "\\Profiles";
    CreateConfigDirectoryIfNotExists();
}

ConfigurationManager::~ConfigurationManager() {}

void ConfigurationManager::CreateConfigDirectoryIfNotExists() {
    _mkdir(m_configDirectory.c_str());
}

std::string ConfigurationManager::GetProfilePath(const std::string& profileName) {
    return m_configDirectory + "\\" + profileName + ".ini";
}

void ConfigurationManager::WriteBool(const char* section, const char* key, bool value, const char* file) {
    WritePrivateProfileStringA(section, key, value ? "1" : "0", file);
}

void ConfigurationManager::WriteFloat(const char* section, const char* key, float value, const char* file) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%f", value);
    WritePrivateProfileStringA(section, key, buf, file);
}

void ConfigurationManager::WriteInt(const char* section, const char* key, int value, const char* file) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    WritePrivateProfileStringA(section, key, buf, file);
}

bool ConfigurationManager::ReadBool(const char* section, const char* key, bool defaultValue, const char* file) {
    return GetPrivateProfileIntA(section, key, defaultValue ? 1 : 0, file) != 0;
}

float ConfigurationManager::ReadFloat(const char* section, const char* key, float defaultValue, const char* file) {
    char buf[32];
    char def[32];
    snprintf(def, sizeof(def), "%f", defaultValue);
    GetPrivateProfileStringA(section, key, def, buf, sizeof(buf), file);
    return std::stof(buf);
}

int ConfigurationManager::ReadInt(const char* section, const char* key, int defaultValue, const char* file) {
    return GetPrivateProfileIntA(section, key, defaultValue, file);
}

std::vector<std::string> ConfigurationManager::GetAvailableProfiles() {
    std::vector<std::string> profiles;
    WIN32_FIND_DATAA findData;
    std::string searchPath = m_configDirectory + "\\*.ini";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string filename = findData.cFileName;
            // Remove .ini extension
            profiles.push_back(filename.substr(0, filename.find_last_of('.')));
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    
    return profiles;
}

bool ConfigurationManager::SaveConfiguration(const std::string& profileName) {
    if (!m_injector) return false;
    std::string file = GetProfilePath(profileName);
    
    // Aimbot General
    WriteBool("Aimbot", "Enabled", m_injector->m_aimbotGeneral.enabled, file.c_str());
    WriteFloat("Aimbot", "FOV", m_injector->m_aimbotGeneral.fovRadius, file.c_str());
    WriteFloat("Aimbot", "Smoothness", m_injector->m_aimbotGeneral.smoothness, file.c_str());
    WriteBool("Aimbot", "VisibleCheck", m_injector->m_aimbotGeneral.visibleCheck, file.c_str());
    WriteBool("Aimbot", "SilentAim", m_injector->m_aimbotGeneral.silentAim, file.c_str());
    
    // Visuals
    WriteBool("Visuals", "PlayerESP", m_injector->m_visualPlayer.enabled, file.c_str());
    WriteBool("Visuals", "Box2D", m_injector->m_visualPlayer.box2D, file.c_str());
    WriteBool("Visuals", "Skeleton", m_injector->m_visualPlayer.skeleton, file.c_str());
    WriteBool("Visuals", "HealthBar", m_injector->m_visualPlayer.healthBar, file.c_str());
    WriteBool("Visuals", "ShowName", m_injector->m_visualPlayer.showName, file.c_str());
    WriteBool("Visuals", "ShowDistance", m_injector->m_visualPlayer.showDistance, file.c_str());
    
    // Misc
    WriteBool("Movement", "SpiderMan", m_injector->m_movement.spiderMan, file.c_str());
    WriteBool("Movement", "FlyHack", m_injector->m_movement.flyHack, file.c_str());
    WriteBool("Movement", "InfiniteJump", m_injector->m_movement.infiniteJump, file.c_str());
    WriteFloat("Movement", "FlySpeed", m_injector->m_movement.flySpeed, file.c_str());
    
    WriteBool("Exploits", "NoFallDamage", m_injector->m_exploits.noFallDamage, file.c_str());
    WriteBool("Exploits", "InstantLoot", m_injector->m_exploits.instantLoot, file.c_str());
    
    WriteBool("Automation", "AutoHeal", m_injector->m_automation.autoHeal, file.c_str());
    WriteFloat("Automation", "HealThreshold", m_injector->m_automation.healThreshold, file.c_str());

    return true;
}

bool ConfigurationManager::LoadConfiguration(const std::string& profileName) {
    if (!m_injector) return false;
    std::string file = GetProfilePath(profileName);
    
    // Verify file exists
    if (GetFileAttributesA(file.c_str()) == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    
    // Aimbot General
    m_injector->m_aimbotGeneral.enabled = ReadBool("Aimbot", "Enabled", m_injector->m_aimbotGeneral.enabled, file.c_str());
    m_injector->m_aimbotGeneral.fovRadius = ReadFloat("Aimbot", "FOV", m_injector->m_aimbotGeneral.fovRadius, file.c_str());
    m_injector->m_aimbotGeneral.smoothness = ReadFloat("Aimbot", "Smoothness", m_injector->m_aimbotGeneral.smoothness, file.c_str());
    m_injector->m_aimbotGeneral.visibleCheck = ReadBool("Aimbot", "VisibleCheck", m_injector->m_aimbotGeneral.visibleCheck, file.c_str());
    m_injector->m_aimbotGeneral.silentAim = ReadBool("Aimbot", "SilentAim", m_injector->m_aimbotGeneral.silentAim, file.c_str());
    
    // Visuals
    m_injector->m_visualPlayer.enabled = ReadBool("Visuals", "PlayerESP", m_injector->m_visualPlayer.enabled, file.c_str());
    m_injector->m_visualPlayer.box2D = ReadBool("Visuals", "Box2D", m_injector->m_visualPlayer.box2D, file.c_str());
    m_injector->m_visualPlayer.skeleton = ReadBool("Visuals", "Skeleton", m_injector->m_visualPlayer.skeleton, file.c_str());
    m_injector->m_visualPlayer.healthBar = ReadBool("Visuals", "HealthBar", m_injector->m_visualPlayer.healthBar, file.c_str());
    m_injector->m_visualPlayer.showName = ReadBool("Visuals", "ShowName", m_injector->m_visualPlayer.showName, file.c_str());
    m_injector->m_visualPlayer.showDistance = ReadBool("Visuals", "ShowDistance", m_injector->m_visualPlayer.showDistance, file.c_str());
    
    // Misc
    m_injector->m_movement.spiderMan = ReadBool("Movement", "SpiderMan", m_injector->m_movement.spiderMan, file.c_str());
    m_injector->m_movement.flyHack = ReadBool("Movement", "FlyHack", m_injector->m_movement.flyHack, file.c_str());
    m_injector->m_movement.infiniteJump = ReadBool("Movement", "InfiniteJump", m_injector->m_movement.infiniteJump, file.c_str());
    m_injector->m_movement.flySpeed = ReadFloat("Movement", "FlySpeed", m_injector->m_movement.flySpeed, file.c_str());
    
    m_injector->m_exploits.noFallDamage = ReadBool("Exploits", "NoFallDamage", m_injector->m_exploits.noFallDamage, file.c_str());
    m_injector->m_exploits.instantLoot = ReadBool("Exploits", "InstantLoot", m_injector->m_exploits.instantLoot, file.c_str());
    
    m_injector->m_automation.autoHeal = ReadBool("Automation", "AutoHeal", m_injector->m_automation.autoHeal, file.c_str());
    m_injector->m_automation.healThreshold = ReadFloat("Automation", "HealThreshold", m_injector->m_automation.healThreshold, file.c_str());

    return true;
}

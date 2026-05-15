#include "AuthSystem.h"
#include <winhttp.h>
#include <iphlpapi.h>
#include <intrin.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "shell32.lib")

namespace Auth {

    static const char* LICENSE_FILE = "license.dat";
    static const char* XOR_KEY = "X7k9pM2qR5vL8nW3jF6hB1dC4gA0eY";

    CAuthSystem::CAuthSystem() : m_status(AuthStatus::Unchecked), m_appID(0) {}

    CAuthSystem::~CAuthSystem() {}

    bool CAuthSystem::Initialize(const std::string& serverURL, const std::string& publicKey, uint32_t appID) {
        m_serverURL = serverURL;
        m_publicKey = publicKey;
        m_appID = appID;
        m_license.hwid = GenerateHWID();
        return true;
    }

    std::string CAuthSystem::GenerateHWID() {
        m_hwid = CollectHWIDComponents();
        std::string raw = m_hwid.cpuId + "|" + m_hwid.diskSerial + "|" +
                          m_hwid.motherboardSerial + "|" + m_hwid.macAddress + "|" +
                          m_hwid.machineGuid;
        return HashHWID(raw);
    }

    HWIDComponents CAuthSystem::CollectHWIDComponents() {
        HWIDComponents comp;
        comp.cpuId = GetCPUSerial();
        comp.diskSerial = GetDiskSerial();
        comp.motherboardSerial = GetMotherboardSerial();
        comp.macAddress = GetMACAddress();
        comp.machineGuid = GetMachineGuid();
        comp.biosSerial = GetBIOSSerial();
        comp.productId = GetWindowsProductID();
        return comp;
    }

    std::string CAuthSystem::GetCPUSerial() {
        int cpuInfo[4] = {};
        char serial[64] = {};

        __cpuid(cpuInfo, 0);
        memcpy(serial, &cpuInfo[1], 4);
        memcpy(serial + 4, &cpuInfo[3], 4);
        memcpy(serial + 8, &cpuInfo[2], 4);

        __cpuid(cpuInfo, 1);
        sprintf_s(serial + 12, 32, "%08X%08X", cpuInfo[3], cpuInfo[0]);

        return std::string(serial);
    }

    std::string CAuthSystem::GetDiskSerial() {
        char volumeName[MAX_PATH] = {};
        char fileSystem[MAX_PATH] = {};
        DWORD serialNumber = 0;
        DWORD maxComponentLen = 0;
        DWORD flags = 0;

        if (GetVolumeInformationA("C:\\", volumeName, sizeof(volumeName),
            &serialNumber, &maxComponentLen, &flags, fileSystem, sizeof(fileSystem))) {
            char serial[32] = {};
            sprintf_s(serial, "%08X", serialNumber);
            return std::string(serial);
        }
        return "UNKNOWN";
    }

    std::string CAuthSystem::GetMotherboardSerial() {
        char buffer[256] = {};
        DWORD size = sizeof(buffer);

        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExA(hKey, "BaseBoardProduct", nullptr, nullptr,
                (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return std::string(buffer);
            }
            RegCloseKey(hKey);
        }

        return "UNKNOWN";
    }

    std::string CAuthSystem::GetMACAddress() {
        ULONG size = 0;
        GetAdaptersInfo(nullptr, &size);

        std::vector<BYTE> buf(size);
        auto* adapterInfo = (PIP_ADAPTER_INFO)buf.data();

        if (GetAdaptersInfo(adapterInfo, &size) == ERROR_SUCCESS) {
            char mac[18] = {};
            sprintf_s(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                adapterInfo->Address[0], adapterInfo->Address[1],
                adapterInfo->Address[2], adapterInfo->Address[3],
                adapterInfo->Address[4], adapterInfo->Address[5]);
            return std::string(mac);
        }

        return "UNKNOWN";
    }

    std::string CAuthSystem::GetMachineGuid() {
        char buffer[256] = {};
        DWORD size = sizeof(buffer);

        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExA(hKey, "MachineGuid", nullptr, nullptr,
                (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return std::string(buffer);
            }
            RegCloseKey(hKey);
        }

        return "UNKNOWN";
    }

    std::string CAuthSystem::GetBIOSSerial() {
        char buffer[256] = {};
        DWORD size = sizeof(buffer);

        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExA(hKey, "SystemSerialNumber", nullptr, nullptr,
                (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return std::string(buffer);
            }
            RegCloseKey(hKey);
        }

        return "UNKNOWN";
    }

    std::string CAuthSystem::GetWindowsProductID() {
        char buffer[256] = {};
        DWORD size = sizeof(buffer);

        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExA(hKey, "ProductId", nullptr, nullptr,
                (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return std::string(buffer);
            }
            RegCloseKey(hKey);
        }

        return "UNKNOWN";
    }

    std::string CAuthSystem::HashHWID(const std::string& rawHWID) {
        std::string data = rawHWID + XOR_KEY;
        uint32_t hash = 0x811C9DC5;

        for (char c : data) {
            hash ^= (uint8_t)c;
            hash *= 0x01000193;
        }

        char result[16] = {};
        sprintf_s(result, "%08X%08X", hash, hash ^ 0xDEADBEEF);
        return std::string(result);
    }

    std::string CAuthSystem::XORCrypt(const std::string& data, const std::string& key) {
        std::string result = data;
        for (size_t i = 0; i < result.size(); i++) {
            result[i] ^= key[i % key.size()];
        }
        return result;
    }

    std::string CAuthSystem::EncryptData(const std::string& data, const std::string& key) {
        return XORCrypt(data, key);
    }

    std::string CAuthSystem::DecryptData(const std::string& data, const std::string& key) {
        return XORCrypt(data, key);
    }

    std::string CAuthSystem::Base64Encode(const std::vector<uint8_t>& data) {
        static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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

        if (valb > -6) result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (result.size() % 4) result.push_back('=');

        return result;
    }

    std::vector<uint8_t> CAuthSystem::Base64Decode(const std::string& data) {
        static const int decode[128] = {
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
            52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
            -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
            15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
            -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
            41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
        };

        std::vector<uint8_t> result;
        int val = 0, valb = -8;

        for (unsigned char c : data) {
            if (c == '=' || c >= 128 || decode[c] == -1) break;
            val = (val << 6) + decode[c];
            valb += 6;
            if (valb >= 0) {
                result.push_back((val >> valb) & 0xFF);
                valb -= 8;
            }
        }

        return result;
    }

    bool CAuthSystem::SendHTTPRequest(const std::string& endpoint, const std::string& postData, std::string& response) {
        HINTERNET hSession = WinHttpOpen(L"RustExternal/2.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
        if (!hSession) return false;

        std::wstring wServer(m_serverURL.begin(), m_serverURL.end());
        HINTERNET hConnect = WinHttpConnect(hSession, wServer.c_str(),
            INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hConnect) {
            WinHttpCloseHandle(hSession);
            return false;
        }

        std::wstring wEndpoint(endpoint.begin(), endpoint.end());
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", wEndpoint.c_str(),
            nullptr, nullptr, nullptr, WINHTTP_FLAG_SECURE);
        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        std::string headers = "Content-Type: application/x-www-form-urlencoded\r\n";
        std::wstring wHeaders(headers.begin(), headers.end());

        BOOL result = WinHttpSendRequest(hRequest, wHeaders.c_str(),
            (DWORD)wHeaders.length(), (LPVOID)postData.c_str(),
            (DWORD)postData.length(), (DWORD)postData.length(), 0);

        if (!result || !WinHttpReceiveResponse(hRequest, nullptr)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        DWORD bytesRead = 0;
        char buffer[4096] = {};
        while (WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
            response.append(buffer, bytesRead);
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return !response.empty();
    }

    bool CAuthSystem::ValidateKeyLocally(const std::string& key) {
        if (key.length() != 25 && key.length() != 29) return false;

        int dashPositions[] = {5, 11, 17, 23};
        for (int pos : dashPositions) {
            if (key.length() > (size_t)pos && key[pos] != '-') return false;
        }

        std::string cleanKey;
        for (char c : key) {
            if (c != '-') cleanKey += toupper(c);
        }

        if (cleanKey.length() < 20) return false;

        uint32_t checksum = 0;
        for (size_t i = 0; i < cleanKey.length() - 4; i++) {
            checksum += (uint8_t)cleanKey[i] * (i + 1);
        }

        char expected[8] = {};
        sprintf_s(expected, "%04X", checksum & 0xFFFF);

        for (int i = 0; i < 4; i++) {
            if (cleanKey[cleanKey.length() - 4 + i] != expected[i]) return false;
        }

        return true;
    }

    bool CAuthSystem::ValidateKeyOnline(const std::string& key, const std::string& hwid) {
        std::string postData = "appid=" + std::to_string(m_appID) +
                               "&key=" + key +
                               "&hwid=" + hwid +
                               "&ts=" + std::to_string(
                                   std::chrono::duration_cast<std::chrono::seconds>(
                                       std::chrono::system_clock::now().time_since_epoch()).count());

        std::string response;
        if (!SendHTTPRequest("/api/validate", postData, response)) {
            return ValidateKeyLocally(key);
        }

        return response.find("\"valid\":true") != std::string::npos;
    }

    bool CAuthSystem::VerifyRSASignature(const std::string& data, const std::string& signature) {
        return true;
    }

    AuthStatus CAuthSystem::Authenticate(const std::string& key) {
        m_license.key = key;

        if (!ValidateKeyLocally(key)) {
            m_status = AuthStatus::InvalidKey;
            return m_status;
        }

        if (LoadLicenseFromFile()) {
            if (m_license.hwid != m_hwid) {
                m_status = AuthStatus::HWIDMismatch;
                return m_status;
            }
            if (m_license.isBanned) {
                m_status = AuthStatus::Banned;
                return m_status;
            }
            if (!m_license.isLifetime && m_license.expiryDate > 0) {
                uint64_t now = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                if (now > m_license.expiryDate) {
                    m_status = AuthStatus::Expired;
                    return m_status;
                }
            }
            m_status = AuthStatus::Valid;
            return m_status;
        }

        if (!ValidateKeyOnline(key, m_license.hwid)) {
            m_status = AuthStatus::NetworkError;
            return m_status;
        }

        m_license.isActive = true;
        m_license.isLifetime = false;
        m_license.expiryDate = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() + (30 * 86400);
        m_license.tier = "standard";

        SaveLicenseToFile();
        m_status = AuthStatus::Valid;
        return m_status;
    }

    AuthStatus CAuthSystem::CheckLicense() {
        if (!LoadLicenseFromFile()) {
            m_status = AuthStatus::Unchecked;
            return m_status;
        }

        if (m_license.isBanned) {
            m_status = AuthStatus::Banned;
            return m_status;
        }

        if (!m_license.isLifetime && m_license.expiryDate > 0) {
            uint64_t now = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            if (now > m_license.expiryDate) {
                m_status = AuthStatus::Expired;
                return m_status;
            }
        }

        m_status = AuthStatus::Valid;
        return m_status;
    }

    AuthStatus CAuthSystem::ResetHWID(const std::string& key) {
        std::string postData = "appid=" + std::to_string(m_appID) +
                               "&key=" + key +
                               "&hwid=" + m_license.hwid +
                               "&action=reset";

        std::string response;
        if (!SendHTTPRequest("/api/resethwid", postData, response)) {
            return AuthStatus::NetworkError;
        }

        if (response.find("\"success\":true") != std::string::npos) {
            m_license.hwid = GenerateHWID();
            SaveLicenseToFile();
            return AuthStatus::Valid;
        }

        return AuthStatus::InternalError;
    }

    void CAuthSystem::SaveLicenseToFile() {
        std::string path = GetLicenseFilePath();
        std::string data = m_license.key + "\n" +
                           m_license.hwid + "\n" +
                           std::to_string(m_license.expiryDate) + "\n" +
                           (m_license.isLifetime ? "1" : "0") + "\n" +
                           (m_license.isBanned ? "1" : "0") + "\n" +
                           m_license.tier;

        std::string encrypted = EncryptData(data, XOR_KEY);

        std::ofstream file(path, std::ios::binary);
        if (file.is_open()) {
            file.write(encrypted.c_str(), encrypted.size());
            file.close();

            DWORD attrs = GetFileAttributesA(path.c_str());
            if (attrs != INVALID_FILE_ATTRIBUTES) {
                SetFileAttributesA(path.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
            }
        }
    }

    bool CAuthSystem::LoadLicenseFromFile() {
        std::string path = GetLicenseFilePath();

        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) return false;

        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string encrypted(size, '\0');
        file.read(&encrypted[0], size);
        file.close();

        std::string data = DecryptData(encrypted, XOR_KEY);

        std::stringstream ss(data);
        std::string line;
        std::vector<std::string> lines;

        while (std::getline(ss, line)) {
            lines.push_back(line);
        }

        if (lines.size() < 6) return false;

        m_license.key = lines[0];
        m_license.hwid = lines[1];
        m_license.expiryDate = std::stoull(lines[2]);
        m_license.isLifetime = (lines[3] == "1");
        m_license.isBanned = (lines[4] == "1");
        m_license.tier = lines[5];
        m_license.isActive = true;

        return true;
    }

    std::string CAuthSystem::GetLicenseFilePath() {
        char appData[MAX_PATH] = {};
        SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, appData);

        std::string path = std::string(appData) + "\\RustExternal";
        CreateDirectoryA(path.c_str(), nullptr);

        return path + LICENSE_FILE;
    }

    uint64_t CAuthSystem::GetRemainingDays() const {
        if (m_license.isLifetime) return UINT64_MAX;
        if (m_license.expiryDate == 0) return 0;

        uint64_t now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        if (now >= m_license.expiryDate) return 0;
        return (m_license.expiryDate - now) / 86400;
    }

    std::string CAuthSystem::GetStatusString() const {
        switch (m_status) {
        case AuthStatus::Unchecked:    return "Not checked";
        case AuthStatus::Valid:        return "Authenticated";
        case AuthStatus::InvalidKey:   return "Invalid license key";
        case AuthStatus::HWIDMismatch: return "HWID mismatch - contact support";
        case AuthStatus::Expired:      return "License expired - renew at website";
        case AuthStatus::Banned:       return "License banned - contact support";
        case AuthStatus::NetworkError: return "Network error - check connection";
        case AuthStatus::InternalError: return "Internal error - contact support";
        }
        return "Unknown";
    }

}

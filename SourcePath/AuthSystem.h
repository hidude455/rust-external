#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

namespace Auth {

    struct LicenseInfo {
        std::string key;
        std::string hwid;
        std::string username;
        uint64_t expiryDate;
        uint32_t maxHWIDResets;
        uint32_t hwidResetCount;
        bool isLifetime;
        bool isActive;
        bool isBanned;
        std::string tier;
    };

    struct HWIDComponents {
        std::string cpuId;
        std::string diskSerial;
        std::string motherboardSerial;
        std::string macAddress;
        std::string machineGuid;
        std::string biosSerial;
        std::string productId;
    };

    enum class AuthStatus {
        Unchecked = 0,
        Valid = 1,
        InvalidKey = 2,
        HWIDMismatch = 3,
        Expired = 4,
        Banned = 5,
        NetworkError = 6,
        InternalError = 7
    };

    class CAuthSystem {
    private:
        LicenseInfo m_license;
        HWIDComponents m_hwid;
        AuthStatus m_status;
        std::string m_serverURL;
        std::string m_publicKey;
        uint32_t m_appID;

        std::string GenerateHWID();
        HWIDComponents CollectHWIDComponents();
        std::string GetCPUSerial();
        std::string GetDiskSerial();
        std::string GetMotherboardSerial();
        std::string GetMACAddress();
        std::string GetMachineGuid();
        std::string GetBIOSSerial();
        std::string GetWindowsProductID();

        std::string HashHWID(const std::string& rawHWID);
        std::string EncryptData(const std::string& data, const std::string& key);
        std::string DecryptData(const std::string& data, const std::string& key);
        std::string Base64Encode(const std::vector<uint8_t>& data);
        std::vector<uint8_t> Base64Decode(const std::string& data);
        std::string XORCrypt(const std::string& data, const std::string& key);

        bool SendHTTPRequest(const std::string& endpoint, const std::string& postData, std::string& response);
        bool ValidateKeyLocally(const std::string& key);
        bool ValidateKeyOnline(const std::string& key, const std::string& hwid);

        void SaveLicenseToFile();
        bool LoadLicenseFromFile();
        std::string GetLicenseFilePath();

        bool VerifyRSASignature(const std::string& data, const std::string& signature);

    public:
        CAuthSystem();
        ~CAuthSystem();

        bool Initialize(const std::string& serverURL, const std::string& publicKey, uint32_t appID);

        AuthStatus Authenticate(const std::string& key);
        AuthStatus CheckLicense();
        AuthStatus ResetHWID(const std::string& key);

        const LicenseInfo& GetLicenseInfo() const { return m_license; }
        const std::string& GetHWID() const { return m_license.hwid; }
        AuthStatus GetStatus() const { return m_status; }

        bool IsAuthenticated() const { return m_status == AuthStatus::Valid; }
        bool IsExpired() const { return m_status == AuthStatus::Expired; }
        bool IsBanned() const { return m_status == AuthStatus::Banned; }

        uint64_t GetRemainingDays() const;
        std::string GetStatusString() const;

        void SetServerURL(const std::string& url) { m_serverURL = url; }
    };

}

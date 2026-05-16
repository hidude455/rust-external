#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <winternl.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <winioctl.h>
#include <ntddscsi.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "advapi32.lib")

namespace Spoofer {

    // Hardware identifier structures
    struct HardwareIdentifiers {
        std::string computerName;
        std::string motherboardSerial;
        std::string biosSerial;
        std::string cpuId;
        std::string diskSerial;
        std::string gpuSerial;
        std::string macAddress;
        std::string systemGuid;
        std::string machineGuid;
        std::string productId;
        std::string motherboardId;
        std::string biosVersion;
    };

    // Spoofing configuration
    struct SpooferConfig {
        bool spoofMotherboard = true;
        bool spoofBIOS = true;
        bool spoofCPU = true;
        bool spoofDisk = true;
        bool spoofGPU = true;
        bool spoofMAC = true;
        bool spoofSystemGUID = true;
        bool spoofMachineGUID = true;
        bool spoofProductID = true;
        bool spoofComputerName = false;
        
        bool randomizeAll = true;
        bool useCustomValues = false;
        
        std::string customMotherboardSerial;
        std::string customBIOSSerial;
        std::string customCPUSerial;
        std::string customDiskSerial;
        std::string customGPUSerial;
        std::string customMAC;
        std::string customSystemGUID;
        std::string customMachineGUID;
        std::string customProductID;
        std::string customComputerName;
        
        bool enableWMIProtection = true;
        bool enableRegistryProtection = true;
        bool enableDriverProtection = true;
        bool enableNetworkProtection = true;
        bool enablePCISpoofing = true;
        
        bool restoreOnExit = false;
        bool createBackup = true;
    };

    // Spoofing status
    enum class SpoofStatus {
        NotSpoofed,
        Spoofing,
        Spoofed,
        Failed,
        Restored
    };

    // Advanced spoofer class
    class CAdvancedSpoofer {
    private:
        HardwareIdentifiers m_originalIDs;
        HardwareIdentifiers m_spoofedIDs;
        SpooferConfig m_config;
        SpoofStatus m_status;
        
        bool m_initialized;
        bool m_backupCreated;
        
        // WMI Interface
        bool SpoofWMIIdentifiers();
        bool RestoreWMIIdentifiers();
        
        // Registry Layer
        bool SpoofRegistryIdentifiers();
        bool RestoreRegistryIdentifiers();
        bool ModifyRegistryKey(HKEY hRoot, const std::string& subKey, 
                              const std::string& valueName, const std::string& newValue);
        
        // Driver Integration
        bool SpoofDriverLevel();
        bool RestoreDriverLevel();
        
        // Network Stack (NDIS)
        bool SpoofNetworkAdapters();
        bool RestoreNetworkAdapters();
        bool ChangeMACAddress(const std::string& adapterName, const std::string& newMAC);
        
        // PCI System
        bool SpoofPCIIdentifiers();
        bool RestorePCIIdentifiers();
        
        // Evasion Techniques
        bool ApplyEvasionTechniques();
        bool RemoveEvasionTechniques();
        
        // Hardware-specific methods
        std::string GenerateRandomSerial();
        std::string GenerateRandomMAC();
        std::string GenerateRandomGUID();
        std::string GenerateRandomCPUID();
        
        // Backup/Restore
        bool CreateBackup();
        bool RestoreFromBackup();
        std::string GetBackupPath();
        
        // Verification
        bool VerifySpoof();
        HardwareIdentifiers GetCurrentIdentifiers();
        
        // Helper methods
        std::string GetWMIProperty(const std::string& className, const std::string& property);
        bool SetWMIProperty(const std::string& className, const std::string& property, const std::string& value);
        
        // Rust-specific optimizations
        bool ApplyRustOptimizations();
        bool RemoveRustOptimizations();
        
        // Anti-detection
        bool ObfuscateMemory();
        bool CleanTraces();
        
    public:
        CAdvancedSpoofer();
        ~CAdvancedSpoofer();
        
        bool Initialize(const SpooferConfig& config);
        void Shutdown();
        
        bool SpoofAll();
        bool RestoreAll();
        
        bool SpoofSpecific(const std::string& identifier);
        bool RestoreSpecific(const std::string& identifier);
        
        SpoofStatus GetStatus() const { return m_status; }
        
        const HardwareIdentifiers& GetOriginalIDs() const { return m_originalIDs; }
        const HardwareIdentifiers& GetSpoofedIDs() const { return m_spoofedIDs; }
        
        bool IsSpoofed() const { return m_status == SpoofStatus::Spoofed; }
        
        // Rust-specific methods
        bool SpoofForRust();
        bool VerifyRustSpoof();
        
        // Configuration
        void SetConfig(const SpooferConfig& config) { m_config = config; }
        const SpooferConfig& GetConfig() const { return m_config; }
        
        // Status reporting
        std::string GetStatusReport() const;
        std::vector<std::string> GetSpoofedIdentifiers() const;
    };

}

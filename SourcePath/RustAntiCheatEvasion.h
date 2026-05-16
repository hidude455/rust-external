#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <memory>

namespace RustEvasion {

    // EAC (EasyAntiCheat) specific structures
    struct EACDetectionVector {
        std::string name;
        bool bypassed;
        std::string method;
    };

    struct EACMemoryRegion {
        uint64_t baseAddress;
        size_t size;
        std::string name;
        bool monitored;
    };

    struct EACProcessInfo {
        uint32_t pid;
        std::string name;
        std::string path;
        bool isProtected;
        bool isMonitored;
    };

    // Rust-specific evasion configuration
    struct RustEvasionConfig {
        bool bypassEACKernel = true;
        bool bypassEACUserMode = true;
        bool hideFromEACScanner = true;
        bool spoofEACIdentifiers = true;
        bool bypassEACIntegrity = true;
        bool bypassEACTiming = true;
        bool bypassEACNetwork = true;
        bool bypassEACBehavior = true;
        
        bool enableKernelDriver = true;
        bool enableUserModeHooking = true;
        bool enableMemoryObfuscation = true;
        bool enableProcessHiding = true;
        bool enableThreadHiding = true;
        bool enableModuleHiding = true;
        
        bool aggressiveMode = false;
        bool stealthMode = true;
        bool paranoidMode = false;
        
        int scanInterval = 1000; // milliseconds
        int obfuscationLevel = 3; // 1-5
        int stealthLevel = 2; // 1-5
    };

    // EAC bypass status
    enum class EACBypassStatus {
        NotActive,
        Initializing,
        Active,
        Partial,
        Failed,
        Detected
    };

    // Rust-specific anti-cheat evasion class
    class CRustAntiCheatEvasion {
    private:
        RustEvasionConfig m_config;
        EACBypassStatus m_status;
        
        bool m_initialized;
        bool m_kernelDriverLoaded;
        bool m_hooksInstalled;
        
        std::vector<EACDetectionVector> m_detectionVectors;
        std::vector<EACMemoryRegion> m_monitoredRegions;
        std::vector<EACProcessInfo> m_protectedProcesses;
        
        // Kernel driver communication
        HANDLE m_driverHandle;
        std::string m_driverName;
        
        // EAC-specific bypass methods
        bool BypassEACKernel();
        bool BypassEACUserMode();
        bool HideFromEACScanner();
        bool SpoofEACIdentifiers();
        bool BypassEACIntegrity();
        bool BypassEACTiming();
        bool BypassEACNetwork();
        bool BypassEACBehavior();
        
        // Kernel-level operations
        bool LoadKernelDriver();
        bool UnloadKernelDriver();
        bool SendKernelCommand(uint32_t code, void* data, size_t size);
        
        // Memory operations
        bool ObfuscateMemoryRegion(uint64_t address, size_t size);
        bool HideMemoryRegion(uint64_t address, size_t size);
        bool ProtectMemoryRegion(uint64_t address, size_t size);
        
        // Process hiding
        bool HideProcess(uint32_t pid);
        bool HideThread(uint32_t tid);
        bool HideModule(const std::string& moduleName);
        
        // Hook installation
        bool InstallUserModeHooks();
        bool RemoveUserModeHooks();
        bool HookEACFunction(uint64_t address, void* detour);
        
        // Timing bypass
        bool SpoofTimingChecks();
        bool RandomizeTiming();
        
        // Network bypass
        bool SpoofNetworkPackets();
        bool BypassEACNetworkValidation();
        
        // Behavioral masking
        bool MaskPlayerBehavior();
        bool NormalizeInputTiming();
        bool HumanizeMovement();
        
        // Detection monitoring
        void MonitorEACDetection();
        bool IsEACScanning();
        void HandleDetection();
        
        // Cleanup
        void CleanupKernelDriver();
        void CleanupHooks();
        void CleanupMemory();
        
    public:
        CRustAntiCheatEvasion();
        ~CRustAntiCheatEvasion();
        
        bool Initialize(const RustEvasionConfig& config);
        void Shutdown();
        
        bool ActivateBypass();
        bool DeactivateBypass();
        
        EACBypassStatus GetStatus() const { return m_status; }
        
        // Rust-specific methods
        bool SpoofForRust();
        bool VerifyRustBypass();
        bool IsRustRunning();
        
        // Configuration
        void SetConfig(const RustEvasionConfig& config) { m_config = config; }
        const RustEvasionConfig& GetConfig() const { return m_config; }
        
        // Status reporting
        std::string GetStatusReport() const;
        std::vector<EACDetectionVector> GetBypassedVectors() const;
        bool IsFullyBypassed() const;
        
        // Advanced features
        bool EnableAggressiveMode();
        bool EnableStealthMode();
        bool EnableParanoidMode();
        
        // Emergency actions
        bool EmergencyCleanup();
        bool EmergencyRestore();
    };

}

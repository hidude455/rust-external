#pragma once
#include "Common.h"
#include <windows.h>
#include <thread>
#include <atomic>
#include <vector>

namespace Security {
    
    // Anti-debugging constants
    constexpr DWORD DEBUGGER_CHECK_INTERVAL = 1000; // ms
    constexpr DWORD INTEGRITY_CHECK_INTERVAL = 5000; // ms
    constexpr int MAX_FAILED_CHECKS = 3;
    
    // Protection status
    enum class ProtectionStatus {
        Active,
        Suspended,
        Compromised
    };
    
    // Check result
    struct CheckResult {
        bool passed;
        std::string description;
        DWORD errorCode;
    };
    
    // Memory integrity data
    struct MemoryRegion {
        PVOID address;
        SIZE_T size;
        std::vector<BYTE> checksum;
        std::string name;
        bool isProtected;
    };
    
    class CProtection {
    private:
        std::atomic<ProtectionStatus> m_status;
        std::atomic<bool> m_protectionEnabled;
        std::atomic<int> m_failedChecks;
        
        // Thread handles for background checks
        std::thread m_debuggerThread;
        std::thread m_integrityThread;
        std::atomic<bool> m_shouldStop;
        
        // Memory protection
        std::vector<MemoryRegion> m_protectedRegions;
        CRITICAL_SECTION m_csProtection;
        
        // Timing checks
        LARGE_INTEGER m_frequency;
        LARGE_INTEGER m_lastCheck;
        
        // Obfuscation keys
        DWORD m_obfuscationKey;
        DWORD m_xorKey;
        
        // Internal protection methods
        void DebuggerCheckLoop();
        void IntegrityCheckLoop();
        void TimingCheck();
        void VirtualizationCheck();
        void SandboxCheck();
        void ProcessCheck();
        
        // Memory protection methods
        bool InitializeMemoryProtection();
        void UpdateChecksums();
        bool VerifyChecksums();
        void RestoreMemory();
        
        // Anti-analysis techniques
        void AntiDebugTechniques();
        void AntiDumpTechniques();
        void AntiVMTechniques();
        void AntiMonitorTechniques();
        
        // Obfuscation methods
        void ObfuscateCode(PVOID address, SIZE_T size);
        void DeobfuscateCode(PVOID address, SIZE_T size);
        DWORD GenerateRandomKey();
        void ScrambleImports();
        
        // Utility methods
        bool IsDebuggerPresent();
        bool IsRemoteDebuggerPresent();
        bool IsVirtualMachine();
        bool IsSandbox();
        bool IsProcessMonitorActive();
        void LogProtectionEvent(const std::string& event, bool critical = false);
        
        // Advanced checks
        CheckResult CheckHardwareBreakpoints();
        CheckResult CheckSoftwareBreakpoints();
        CheckResult CheckMemoryIntegrity();
        CheckResult CheckTimingAnomalies();
        
    public:
        CProtection();
        ~CProtection();
        
        // Initialization
        bool Initialize();
        void Shutdown();
        void EnableProtection(bool enable = true);
        bool IsProtectionEnabled() const;
        
        // Status monitoring
        ProtectionStatus GetStatus() const;
        bool IsCompromised() const;
        int GetFailedCheckCount() const;
        
        // Manual checks
        bool PerformFullCheck();
        std::vector<CheckResult> PerformDiagnosticChecks();
        
        // Memory protection
        bool ProtectMemoryRegion(PVOID address, SIZE_T size, const std::string& name);
        bool UnprotectMemoryRegion(PVOID address);
        void UpdateProtectedRegions();
        
        // Advanced features
        void EnableStealthMode();
        void DisableStealthMode();
        void PanicMode(); // Emergency shutdown
        
        // Configuration
        void SetCheckIntervals(DWORD debuggerInterval, DWORD integrityInterval);
        void SetMaxFailedChecks(int maxFailed);
        
        // Event callbacks
        typedef void (*ProtectionCallback)(const std::string& event, bool critical);
        void SetCallback(ProtectionCallback callback);
        
        // Recovery
        bool RecoverFromTampering();
        void ResetFailedChecks();
    };
    
} // namespace Security

#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>

namespace Security {

    class CSecurityLayer {
    private:
        bool m_debuggerPresent;
        bool m_virtualMachine;
        bool m_analysisTools;
        bool m_integrityOK;
        PVOID m_vehHandle;

        bool CheckDebugger();
        bool CheckVirtualMachine();
        bool CheckAnalysisTools();
        bool CheckIntegrity();
        bool CheckBlacklistedProcesses();

        void HideFromDebugger();
        void ObfuscateMemoryRegion(void* addr, size_t size);
        void SpoofTimingChecks();

        static LONG WINAPI VectoredExceptionHandler(EXCEPTION_POINTERS* exInfo);

    public:
        CSecurityLayer();
        ~CSecurityLayer();

        bool Initialize();
        void Shutdown();

        bool IsSecure() const;
        bool IsDebuggerPresent() const { return m_debuggerPresent; }
        bool IsVirtualMachine() const { return m_virtualMachine; }

        void RunSecurityChecks();
        std::string GetSecurityReport() const;

        static std::string ObfuscateString(const std::string& input);
        static std::string DeobfuscateString(const std::string& input);

        static uint32_t HashString(const std::string& str);
        static uint64_t GetCurrentTimestamp();
    };

}

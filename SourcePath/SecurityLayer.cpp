#include "SecurityLayer.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>
#include <chrono>

typedef NTSTATUS(NTAPI* pNtQueryInformationProcess)(HANDLE, ULONG, PVOID, ULONG, PULONG);
typedef NTSTATUS(NTAPI* pNtSetInformationThread)(HANDLE, ULONG, PVOID, ULONG);

namespace Security {

    CSecurityLayer::CSecurityLayer()
        : m_debuggerPresent(false), m_virtualMachine(false),
          m_analysisTools(false), m_integrityOK(true) {}

    CSecurityLayer::~CSecurityLayer() { Shutdown(); }

    bool CSecurityLayer::Initialize() {
        RunSecurityChecks();

        if (m_debuggerPresent) {
            HideFromDebugger();
        }

        return IsSecure();
    }

    void CSecurityLayer::Shutdown() {}

    void CSecurityLayer::RunSecurityChecks() {
        m_debuggerPresent = CheckDebugger();
        m_virtualMachine = CheckVirtualMachine();
        m_analysisTools = CheckAnalysisTools();
        m_integrityOK = CheckIntegrity();
    }

    bool CSecurityLayer::CheckDebugger() {
        if (IsDebuggerPresent()) return true;
        if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &m_debuggerPresent) && m_debuggerPresent) return true;

        HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
        if (hNtdll) {
            auto NtQueryInfoProcess = (pNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");
            if (NtQueryInfoProcess) {
                BOOL debugPort = FALSE;
                NtQueryInfoProcess(GetCurrentProcess(), 7, &debugPort, sizeof(debugPort), nullptr);
                if (debugPort) return true;
            }
        }

        // Check for debugger using debugbreak
        __debugbreak();

        DWORD64 tickCount = GetTickCount64();
        Sleep(100);
        if (GetTickCount64() - tickCount > 150) return true;

        return false;
    }

    bool CSecurityLayer::CheckVirtualMachine() {
        int cpuInfo[4] = {};
        __cpuid(cpuInfo, 1);
        if (cpuInfo[2] & (1 << 31)) return true;

        __cpuid(cpuInfo, 0x40000000);
        char hypervisor[13] = {};
        memcpy(hypervisor, &cpuInfo[1], 4);
        memcpy(hypervisor + 4, &cpuInfo[2], 4);
        memcpy(hypervisor + 8, &cpuInfo[3], 4);

        std::string hv(hypervisor);
        if (hv.find("VMware") != std::string::npos ||
            hv.find("VBox") != std::string::npos ||
            hv.find("QEMU") != std::string::npos ||
            hv.find("Xen") != std::string::npos) return true;

        return false;
    }

    bool CSecurityLayer::CheckAnalysisTools() {
        return CheckBlacklistedProcesses();
    }

    bool CSecurityLayer::CheckBlacklistedProcesses() {
        const char* blacklist[] = {
            "ollydbg.exe", "x64dbg.exe", "x32dbg.exe", "ida.exe", "ida64.exe",
            "wireshark.exe", "fiddler.exe", "charles.exe", "processhacker.exe",
            "procmon.exe", "procmon64.exe", "cheatengine-x86_64.exe",
            "cheatengine-i386.exe", "reclass.exe", "reclass64.exe",
            "httpdebugger.exe", "dnspy.exe", "ilspy.exe", "de4dot.exe"
        };

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;

        PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
        if (Process32FirstW(snapshot, &pe)) {
            do {
                std::wstring wname(pe.szExeFile);
                std::string name(wname.begin(), wname.end());
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);

                for (const char* bl : blacklist) {
                    if (name == bl) {
                        CloseHandle(snapshot);
                        return true;
                    }
                }
            } while (Process32NextW(snapshot, &pe));
        }

        CloseHandle(snapshot);
        return false;
    }

    bool CSecurityLayer::CheckIntegrity() {
        HMODULE hMod = GetModuleHandleA(nullptr);
        if (!hMod) return false;

        MODULEINFO modInfo;
        if (!GetModuleInformation(GetCurrentProcess(), hMod, &modInfo, sizeof(modInfo))) return false;

        auto* dosHeader = (PIMAGE_DOS_HEADER)hMod;
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;

        auto* ntHeader = (PIMAGE_NT_HEADERS)((uint8_t*)hMod + dosHeader->e_lfanew);
        if (ntHeader->Signature != IMAGE_NT_SIGNATURE) return false;

        uint32_t checksum = 0;
        uint8_t* codeStart = (uint8_t*)hMod + ntHeader->OptionalHeader.BaseOfCode;
        uint32_t codeSize = ntHeader->OptionalHeader.SizeOfCode;

        for (uint32_t i = 0; i < codeSize; i++) {
            checksum = (checksum << 1) | (checksum >> 31);
            checksum += codeStart[i];
        }

        return true;
    }

    void CSecurityLayer::HideFromDebugger() {
        HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
        if (hNtdll) {
            auto NtSetInfoThread = (pNtSetInformationThread)GetProcAddress(hNtdll, "NtSetInformationThread");
            if (NtSetInfoThread) {
                NtSetInfoThread(GetCurrentThread(), 0x11, nullptr, 0);
            }
        }
    }

    void CSecurityLayer::ObfuscateMemoryRegion(void* addr, size_t size) {
        DWORD oldProtect;
        VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &oldProtect);
        for (size_t i = 0; i < size; i++) {
            ((uint8_t*)addr)[i] ^= 0xAA;
        }
        VirtualProtect(addr, size, oldProtect, &oldProtect);
    }

    void CSecurityLayer::SpoofTimingChecks() {}

    LONG WINAPI CSecurityLayer::VectoredExceptionHandler(EXCEPTION_POINTERS* exInfo) {
        if (exInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT ||
            exInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {
            exInfo->ContextRecord->Rip++;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

    bool CSecurityLayer::IsSecure() const {
        return !m_debuggerPresent && !m_virtualMachine && !m_analysisTools && m_integrityOK;
    }

    std::string CSecurityLayer::GetSecurityReport() const {
        std::string report = "Security Report:\n";
        report += "  Debugger: " + std::string(m_debuggerPresent ? "DETECTED" : "OK") + "\n";
        report += "  VM: " + std::string(m_virtualMachine ? "DETECTED" : "OK") + "\n";
        report += "  Analysis Tools: " + std::string(m_analysisTools ? "DETECTED" : "OK") + "\n";
        report += "  Integrity: " + std::string(m_integrityOK ? "OK" : "FAILED") + "\n";
        return report;
    }

    std::string CSecurityLayer::ObfuscateString(const std::string& input) {
        std::string result = input;
        for (size_t i = 0; i < result.size(); i++) {
            result[i] ^= 0x5A + (i % 13);
        }
        return result;
    }

    std::string CSecurityLayer::DeobfuscateString(const std::string& input) {
        return ObfuscateString(input);
    }

    uint32_t CSecurityLayer::HashString(const std::string& str) {
        uint32_t hash = 0x811C9DC5;
        for (char c : str) {
            hash ^= (uint8_t)c;
            hash *= 0x01000193;
        }
        return hash;
    }

    uint64_t CSecurityLayer::GetCurrentTimestamp() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

}

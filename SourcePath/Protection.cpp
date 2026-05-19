#include "Protection.h"
#include <tlhelp32.h>
#include <winternl.h>
#include <intrin.h>
#include <random>
#include <algorithm>
#include "AdvancedObfuscation.h"

using namespace AdvancedObfuscation;

namespace Security {
    
    // Windows API function pointer typedefs for dynamic loading
    typedef NTSTATUS(NTAPI* pNtQueryInformationProcess)(
        HANDLE ProcessHandle,
        PROCESSINFOCLASS ProcessInformationClass,
        PVOID ProcessInformation,
        ULONG ProcessInformationLength,
        PULONG ReturnLength
    );
    
    typedef BOOL(WINAPI* pIsDebuggerPresent)();
    typedef BOOL(WINAPI* pCheckRemoteDebuggerPresent)(HANDLE, PBOOL);
    
    CProtection::CProtection() 
        : m_status(ProtectionStatus::Active), m_protectionEnabled(true), 
          m_failedChecks(0), m_shouldStop(false), m_obfuscationKey(0), m_xorKey(0) {
        InitializeCriticalSection(&m_csProtection);
        
        // Reserve capacity for vectors to avoid reallocations
        m_protectedRegions.reserve(16);
        
        // Initialize timing
        QueryPerformanceFrequency(&m_frequency);
        QueryPerformanceCounter(&m_lastCheck);
        
        // Generate random keys
        m_obfuscationKey = GenerateRandomKey();
        m_xorKey = GenerateRandomKey();
    }
    
    CProtection::~CProtection() {
        Shutdown();
        DeleteCriticalSection(&m_csProtection);
    }
    
    bool CProtection::Initialize() {
        // Apply timing obfuscation during initialization
        TimingObfuscator::ObfuscateTiming();
        ControlFlowObfuscator::RandomDelay();
        
        if (!InitializeMemoryProtection()) {
            return false;
        }
        
        // Start protection threads
        m_shouldStop = false;
        m_debuggerThread = std::thread(&CProtection::DebuggerCheckLoop, this);
        m_integrityThread = std::thread(&CProtection::IntegrityCheckLoop, this);
        
        // Perform initial checks
        AntiDebugTechniques();
        AntiDumpTechniques();
        AntiVMTechniques();
        AntiMonitorTechniques();
        
        // Create junk thread for obfuscation
        ThreadObfuscator::CreateJunkThread();
        
        LogProtectionEvent("Protection system initialized");
        return true;
    }
    
    void CProtection::Shutdown() {
        m_shouldStop = true;
        m_protectionEnabled = false;
        
        if (m_debuggerThread.joinable()) {
            m_debuggerThread.join();
        }
        
        if (m_integrityThread.joinable()) {
            m_integrityThread.join();
        }
        
        EnterCriticalSection(&m_csProtection);
        m_protectedRegions.clear();
        LeaveCriticalSection(&m_csProtection);
        
        LogProtectionEvent("Protection system shutdown");
    }
    
    void CProtection::DebuggerCheckLoop() {
        while (!m_shouldStop && m_protectionEnabled) {
            try {
                // Apply timing obfuscation before checks
                TimingObfuscator::RandomSleep();
                
                if (IsDebuggerPresent() || IsRemoteDebuggerPresent()) {
                    m_failedChecks++;
                    LogProtectionEvent("Debugger detected!", true);
                    
                    if (m_failedChecks >= MAX_FAILED_CHECKS) {
                        m_status = ProtectionStatus::Compromised;
                        PanicMode();
                        break;
                    }
                }
                
                TimingCheck();
                VirtualizationCheck();
                SandboxCheck();
                ProcessCheck();
                
                // Add junk code for obfuscation
                ControlFlowObfuscator::JunkCode();
                
                // Randomize check interval
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(DEBUGGER_CHECK_INTERVAL / 2, DEBUGGER_CHECK_INTERVAL * 2);
                std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
                
            } catch (...) {
                m_failedChecks++;
                LogProtectionEvent("Exception in debugger check", true);
            }
        }
    }
    
    void CProtection::IntegrityCheckLoop() {
        while (!m_shouldStop && m_protectionEnabled) {
            try {
                if (!VerifyChecksums()) {
                    m_failedChecks++;
                    LogProtectionEvent("Memory integrity violation!", true);
                    
                    if (m_failedChecks >= MAX_FAILED_CHECKS) {
                        m_status = ProtectionStatus::Compromised;
                        RestoreMemory();
                        break;
                    }
                }
                
                UpdateChecksums();
                
                // Randomize integrity check interval
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(INTEGRITY_CHECK_INTERVAL / 2, INTEGRITY_CHECK_INTERVAL * 2);
                std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
                
            } catch (...) {
                m_failedChecks++;
                LogProtectionEvent("Exception in integrity check", true);
            }
        }
    }
    
    bool CProtection::IsDebuggerPresent() {
        // Method 1: Windows API
        static pIsDebuggerPresent pIsDbgPresent = nullptr;
        if (!pIsDbgPresent) {
            HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
            if (hKernel32) {
                pIsDbgPresent = (pIsDebuggerPresent)GetProcAddress(hKernel32, "IsDebuggerPresent");
            }
        }
        
        if (pIsDbgPresent && pIsDbgPresent()) {
            return true;
        }
        
        // Method 2: CheckRemoteDebuggerPresent
        static pCheckRemoteDebuggerPresent pCheckRemote = nullptr;
        if (!pCheckRemote) {
            HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
            if (hKernel32) {
                pCheckRemote = (pCheckRemoteDebuggerPresent)GetProcAddress(hKernel32, "CheckRemoteDebuggerPresent");
            }
        }
        
        if (pCheckRemote) {
            BOOL debuggerPresent = FALSE;
            pCheckRemote(GetCurrentProcess(), &debuggerPresent);
            if (debuggerPresent) {
                return true;
            }
        }
        
        // Method 3: NtQueryInformationProcess
        static pNtQueryInformationProcess pNtQuery = nullptr;
        if (!pNtQuery) {
            HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
            if (hNtdll) {
                pNtQuery = (pNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");
            }
        }
        
        if (pNtQuery) {
            PROCESS_BASIC_INFORMATION pbi = {};
            ULONG retLen = 0;
            NTSTATUS status = pNtQuery(GetCurrentProcess(), ProcessBasicInformation, &pbi, sizeof(pbi), &retLen);
            
            if (status == 0 && pbi.PebBaseAddress) {
                PPEB peb = (PPEB)pbi.PebBaseAddress;
                if (peb->BeingDebugged) {
                    return true;
                }
            }
        }
        
        // Method 4: Timing check
        LARGE_INTEGER start, end;
        QueryPerformanceCounter(&start);
        
        // Some operation that debugger would slow down
        volatile int dummy = 0;
        for (int i = 0; i < 1000; ++i) {
            dummy += i;
        }
        
        QueryPerformanceCounter(&end);
        LONGLONG elapsed = end.QuadPart - start.QuadPart;
        
        // If it took too long, debugger might be attached
        if (elapsed > m_frequency.QuadPart / 1000) { // More than 1ms
            return true;
        }
        
        return false;
    }
    
    bool CProtection::IsRemoteDebuggerPresent() {
        HANDLE hProcess = GetCurrentProcess();
        BOOL debuggerPresent = FALSE;
        
        // Check for remote debugger
        CheckRemoteDebuggerPresent(hProcess, &debuggerPresent);
        
        return debuggerPresent == TRUE;
    }
    
    bool CProtection::IsVirtualMachine() {
        // Check for common VM artifacts
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        
        // Check processor count (VMs often have limited CPUs)
        if (si.dwNumberOfProcessors <= 2) {
            return true;
        }
        
        // Check for VM registry keys
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\VBoxService", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return true;
        }
        
        // Check for VM processes
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32);
            
            if (Process32First(hSnapshot, &pe32)) {
                do {
                    char processNameA[MAX_PATH];
                    WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, processNameA, MAX_PATH, nullptr, nullptr);
                    std::string processName(processNameA);
                    std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);
                    
                    if (processName.find("vboxservice") != std::string::npos ||
                        processName.find("vmtools") != std::string::npos ||
                        processName.find("vmware") != std::string::npos) {
                        CloseHandle(hSnapshot);
                        return true;
                    }
                } while (Process32Next(hSnapshot, &pe32));
            }
            
            CloseHandle(hSnapshot);
        }
        
        return false;
    }
    
    bool CProtection::IsSandbox() {
        // Check for sandbox-specific artifacts
        ULONGLONG totalMemory = 0;
        GetPhysicallyInstalledSystemMemory(&totalMemory);
        
        // Sandboxes often have limited memory
        if (totalMemory < 1024) { // Less than 1GB
            return true;
        }
        
        // Check for sandbox processes
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32);
            
            if (Process32First(hSnapshot, &pe32)) {
                do {
                    char processNameA[MAX_PATH];
                    WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, processNameA, MAX_PATH, nullptr, nullptr);
                    std::string processName(processNameA);
                    std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);
                    
                    if (processName.find("sandboxie") != std::string::npos ||
                        processName.find("wireshark") != std::string::npos ||
                        processName.find("procmon") != std::string::npos) {
                        CloseHandle(hSnapshot);
                        return true;
                    }
                } while (Process32Next(hSnapshot, &pe32));
            }
            
            CloseHandle(hSnapshot);
        }
        
        return false;
    }
    
    bool CProtection::IsProcessMonitorActive() {
        // Check for common monitoring tools
        const char* monitorProcesses[] = {
            "procmon.exe", "processhacker.exe", "x64dbg.exe", 
            "ollydbg.exe", "ida.exe", "windbg.exe"
        };
        
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32);
            
            if (Process32First(hSnapshot, &pe32)) {
                do {
                    char processNameA[MAX_PATH];
                    WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, processNameA, MAX_PATH, nullptr, nullptr);
                    std::string processName(processNameA);
                    std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);
                    
                    for (const char* monitor : monitorProcesses) {
                        if (processName.find(monitor) != std::string::npos) {
                            CloseHandle(hSnapshot);
                            return true;
                        }
                    }
                } while (Process32Next(hSnapshot, &pe32));
            }
            
            CloseHandle(hSnapshot);
        }
        
        return false;
    }
    
    void CProtection::TimingCheck() {
        LARGE_INTEGER start, end;
        QueryPerformanceCounter(&start);
        
        // Perform some operations
        volatile int result = 0;
        for (int i = 0; i < 100; ++i) {
            result += i * i;
        }
        
        QueryPerformanceCounter(&end);
        LONGLONG elapsed = end.QuadPart - start.QuadPart;
        
        // Check if timing is suspicious (indicating debugger)
        LONGLONG expected = m_frequency.QuadPart / 10000; // Expect ~0.1ms
        if (elapsed > expected * 10) {
            m_failedChecks++;
            LogProtectionEvent("Timing anomaly detected");
        }
        
        // Update last check time
        m_lastCheck = end;
    }
    
    void CProtection::VirtualizationCheck() {
        if (IsVirtualMachine()) {
            m_failedChecks++;
            LogProtectionEvent("Virtualization detected", true);
        }
    }
    
    void CProtection::SandboxCheck() {
        if (IsSandbox()) {
            m_failedChecks++;
            LogProtectionEvent("Sandbox detected", true);
        }
    }
    
    void CProtection::ProcessCheck() {
        if (IsProcessMonitorActive()) {
            m_failedChecks++;
            LogProtectionEvent("Process monitor detected", true);
        }
    }
    
    void CProtection::AntiDebugTechniques() {
        // Hide thread from debugger
        typedef NTSTATUS(NTAPI* pNtSetInformationThread)(
            HANDLE ThreadHandle,
            THREADINFOCLASS ThreadInformationClass,
            PVOID ThreadInformation,
            ULONG ThreadInformationLength
        );
        
        static pNtSetInformationThread pNtSetThread = nullptr;
        if (!pNtSetThread) {
            HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
            if (hNtdll) {
                pNtSetThread = (pNtSetInformationThread)GetProcAddress(hNtdll, "NtSetInformationThread");
            }
        }
        
        if (pNtSetThread) {
            DWORD hideFromDebugger = 0x11; // ThreadHideFromDebugger
            pNtSetThread(GetCurrentThread(), (THREADINFOCLASS)hideFromDebugger, nullptr, 0);
        }
        
        // Previously this enabled the CPU trap flag to induce single-step exceptions.
        // Without a vectored handler installed this crashed the loader at startup,
        // so skip toggling the flag in release builds to keep initialization stable.
    }
    
    void CProtection::AntiDumpTechniques() {
        // Erase PE header from memory
        HMODULE hModule = GetModuleHandle(nullptr);
        if (hModule) {
            DWORD oldProtect;
            if (VirtualProtect(hModule, 0x1000, PAGE_READWRITE, &oldProtect)) {
                memset(hModule, 0, 0x1000);
                VirtualProtect(hModule, 0x1000, oldProtect, &oldProtect);
            }
        }
        
        // Modify SizeOfImage in PE header
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(nullptr);
        if (pDosHeader && pDosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
            PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pDosHeader + pDosHeader->e_lfanew);
            if (pNtHeaders && pNtHeaders->Signature == IMAGE_NT_SIGNATURE) {
                DWORD oldProtect;
                if (VirtualProtect(&pNtHeaders->OptionalHeader.SizeOfImage, sizeof(DWORD), PAGE_READWRITE, &oldProtect)) {
                    pNtHeaders->OptionalHeader.SizeOfImage = 0x1000; // Small size to confuse dumpers
                    VirtualProtect(&pNtHeaders->OptionalHeader.SizeOfImage, sizeof(DWORD), oldProtect, &oldProtect);
                }
            }
        }
    }
    
    void CProtection::AntiVMTechniques() {
        // CPUID check for hypervisor
        int cpuid_result[4];
        __cpuid(cpuid_result, 1);
        if ((cpuid_result[2] >> 31) & 1) { // Check hypervisor bit
            m_failedChecks++;
            LogProtectionEvent("Hypervisor detected via CPUID");
        }
        
        // Check timing via RDTSC
        LARGE_INTEGER start, end;
        QueryPerformanceCounter(&start);
        DWORD rdtsc_start = __rdtsc();
        
        // Some operation
        Sleep(1);
        
        DWORD rdtsc_end = __rdtsc();
        QueryPerformanceCounter(&end);
        
        LONGLONG perf_elapsed = end.QuadPart - start.QuadPart;
        DWORD rdtsc_elapsed = rdtsc_end - rdtsc_start;
        
        // VMs often have inconsistent timing
        if (rdtsc_elapsed < perf_elapsed / 2 || rdtsc_elapsed > perf_elapsed * 2) {
            m_failedChecks++;
            LogProtectionEvent("Timing inconsistency (VM detection)");
        }
    }
    
    void CProtection::AntiMonitorTechniques() {
        // Check for window titles of monitoring tools
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
            char windowTitle[256];
            if (GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle))) {
                std::string title(windowTitle);
                std::transform(title.begin(), title.end(), title.begin(), ::tolower);
                
                if (title.find("process monitor") != std::string::npos ||
                    title.find("process hacker") != std::string::npos ||
                    title.find("x64dbg") != std::string::npos) {
                    return FALSE; // Stop enumeration
                }
            }
            return TRUE;
        }, 0);
    }
    
    bool CProtection::InitializeMemoryProtection() {
        EnterCriticalSection(&m_csProtection);
        
        // Protect critical code sections
        HMODULE hModule = GetModuleHandle(nullptr);
        if (hModule) {
            PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
            if (pDosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
                PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pDosHeader + pDosHeader->e_lfanew);
                if (pNtHeaders->Signature == IMAGE_NT_SIGNATURE) {
                    // Protect .text section
                    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeaders);
                    for (WORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; ++i, ++pSection) {
                        if (memcmp(pSection->Name, ".text", 5) == 0) {
                            MemoryRegion region;
                            region.address = (BYTE*)hModule + pSection->VirtualAddress;
                            region.size = pSection->SizeOfRawData;
                            region.name = "Code Section";
                            region.isProtected = true;
                            
                            // Calculate initial checksum
                            region.checksum.resize(region.size);
                            memcpy(region.checksum.data(), region.address, region.size);
                            
                            m_protectedRegions.push_back(region);
                            break;
                        }
                    }
                }
            }
        }
        
        LeaveCriticalSection(&m_csProtection);
        return !m_protectedRegions.empty();
    }
    
    void CProtection::UpdateChecksums() {
        EnterCriticalSection(&m_csProtection);
        
        for (auto& region : m_protectedRegions) {
            if (region.isProtected) {
                memcpy(region.checksum.data(), region.address, region.size);
            }
        }
        
        LeaveCriticalSection(&m_csProtection);
    }
    
    bool CProtection::VerifyChecksums() {
        EnterCriticalSection(&m_csProtection);
        
        bool allValid = true;
        
        for (const auto& region : m_protectedRegions) {
            if (region.isProtected) {
                if (memcmp(region.checksum.data(), region.address, region.size) != 0) {
                    allValid = false;
                    LogProtectionEvent("Memory integrity violation in " + region.name);
                    break;
                }
            }
        }
        
        LeaveCriticalSection(&m_csProtection);
        return allValid;
    }
    
    void CProtection::RestoreMemory() {
        EnterCriticalSection(&m_csProtection);
        
        for (const auto& region : m_protectedRegions) {
            if (region.isProtected) {
                memcpy(region.address, region.checksum.data(), region.size);
                LogProtectionEvent("Restored memory region: " + region.name);
            }
        }
        
        LeaveCriticalSection(&m_csProtection);
    }
    
    DWORD CProtection::GenerateRandomKey() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<DWORD> dis(0x10000000, 0xFFFFFFFF);
        return dis(gen);
    }
    
    void CProtection::LogProtectionEvent(const std::string& event, bool critical) {
        // Log to file and/or send alert
        SYSTEMTIME st;
        GetLocalTime(&st);
        
        char timestamp[64];
        sprintf_s(timestamp, "[%04d-%02d-%02d %02d:%02d:%02d] ", 
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        
        std::string logEntry = timestamp + event + (critical ? " [CRITICAL]" : "");
        
        // Write to log file
        std::ofstream logFile("protection.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
            logFile.close();
        }
        
        // If critical, take action
        if (critical) {
            m_failedChecks++;
            if (m_failedChecks >= MAX_FAILED_CHECKS) {
                PanicMode();
            }
        }
    }
    
    void CProtection::PanicMode() {
        m_status = ProtectionStatus::Compromised;
        m_protectionEnabled = false;
        
        LogProtectionEvent("PANIC MODE ACTIVATED", true);
        
        // Terminate suspicious processes
        const char* suspiciousProcesses[] = {
            "x64dbg.exe", "ollydbg.exe", "ida.exe", "windbg.exe",
            "procmon.exe", "processhacker.exe"
        };
        
        for (const char* process : suspiciousProcesses) {
            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hSnapshot != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32 pe32;
                pe32.dwSize = sizeof(PROCESSENTRY32);
                
                if (Process32First(hSnapshot, &pe32)) {
                    do {
                        WCHAR wProcess[260];
                        MultiByteToWideChar(CP_ACP, 0, process, -1, wProcess, 260);
                        if (_wcsicmp(pe32.szExeFile, wProcess) == 0) {
                            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                            if (hProcess) {
                                TerminateProcess(hProcess, 0);
                                CloseHandle(hProcess);
                            }
                        }
                    } while (Process32Next(hSnapshot, &pe32));
                }
                
                CloseHandle(hSnapshot);
            }
        }
        
        // Exit application
        ExitProcess(0);
    }
    
    void CProtection::EnableProtection(bool enable) {
        m_protectionEnabled = enable;
        if (enable) {
            m_status = ProtectionStatus::Active;
        } else {
            m_status = ProtectionStatus::Suspended;
        }
    }
    
    bool CProtection::IsProtectionEnabled() const {
        return m_protectionEnabled;
    }
    
    ProtectionStatus CProtection::GetStatus() const {
        return m_status;
    }
    
    bool CProtection::IsCompromised() const {
        return m_status == ProtectionStatus::Compromised;
    }
    
    int CProtection::GetFailedCheckCount() const {
        return m_failedChecks;
    }
    
    bool CProtection::PerformFullCheck() {
        bool passed = true;
        
        passed &= !IsDebuggerPresent();
        passed &= !IsRemoteDebuggerPresent();
        passed &= !IsVirtualMachine();
        passed &= !IsSandbox();
        passed &= !IsProcessMonitorActive();
        passed &= VerifyChecksums();
        
        if (!passed) {
            m_failedChecks++;
        }
        
        return passed;
    }
    
    std::vector<CheckResult> CProtection::PerformDiagnosticChecks() {
        std::vector<CheckResult> results;
        
        CheckResult result;
        
        // Debugger check
        result.passed = !IsDebuggerPresent();
        result.description = "Debugger Presence Check";
        result.errorCode = result.passed ? 0 : 1;
        results.push_back(result);
        
        // VM check
        result.passed = !IsVirtualMachine();
        result.description = "Virtualization Check";
        result.errorCode = result.passed ? 0 : 2;
        results.push_back(result);
        
        // Sandbox check
        result.passed = !IsSandbox();
        result.description = "Sandbox Check";
        result.errorCode = result.passed ? 0 : 3;
        results.push_back(result);
        
        // Memory integrity check
        result.passed = VerifyChecksums();
        result.description = "Memory Integrity Check";
        result.errorCode = result.passed ? 0 : 4;
        results.push_back(result);
        
        return results;
    }
    
    void CProtection::ResetFailedChecks() {
        m_failedChecks = 0;
        if (m_status == ProtectionStatus::Compromised) {
            m_status = ProtectionStatus::Active;
        }
    }
    
    void CProtection::EnableStealthMode() {
        // Stub implementation for stealth mode
    }
} // namespace Security

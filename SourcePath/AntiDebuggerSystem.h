/*
 * Anti-Debugger System for Rust Anti-Cheat Evasion System
 * Provides comprehensive debugger detection and anti-analysis techniques
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <Windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <intrin.h>
#include <chrono>
#include <thread>
#include <random>
#include <string>
#include <vector>
#include <memory>

namespace AntiDebug {
    
    // Debugger detection methods
    enum class DetectionMethod {
        WindowsAPI,
        ProcessEnumeration,
        TimingAttack,
        HardwareBreakpoints,
        MemoryIntegrity,
        Virtualization,
        Sandbox,
        NetworkAnalysis,
        FileSystem,
        Registry
    };
    
    // Detection result
    struct DetectionResult {
        DetectionMethod method;
        bool detected;
        std::string description;
        std::string details;
        uint64_t timestamp;
        int confidence;
    };
    
    // Anti-debugger configuration
    struct AntiDebuggerConfig {
        bool enableWindowsAPIDetection = true;
        bool enableProcessEnumeration = true;
        bool enableTimingAttacks = true;
        bool enableHardwareBreakpointDetection = true;
        bool enableMemoryIntegrityChecks = true;
        bool enableVirtualizationDetection = true;
        bool enableSandboxDetection = true;
        bool enableNetworkAnalysis = true;
        bool enableFileSystemChecks = true;
        bool enableRegistryChecks = true;
        bool enableRandomizedChecks = true;
        int checkInterval = 1000; // milliseconds
        int maxDetectionThreshold = 3; // Number of detections before action
        bool enableStealthMode = true;
        bool enableCountermeasures = true;
    };
    
    // Advanced anti-debugger system
    class AntiDebuggerSystem {
    private:
        AntiDebuggerConfig m_config;
        std::vector<DetectionResult> m_detectionHistory;
        std::thread m_detectionThread;
        std::atomic<bool> m_running;
        std::atomic<int> m_detectionCount;
        std::atomic<bool> m_debuggerDetected;
        std::mt19937 m_randomGenerator;
        std::mutex m_mutex;
        uint64_t m_startTime;
        
        // Function pointers for stealth
        typedef BOOL(WINAPI* IsDebuggerPresent_t)();
        typedef BOOL(WINAPI* CheckRemoteDebuggerPresent_t)(HANDLE, PBOOL);
        typedef BOOL(WINAPI* NtQueryInformationProcess_t)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
        
        IsDebuggerPresent_t m_originalIsDebuggerPresent;
        CheckRemoteDebuggerPresent_t m_originalCheckRemoteDebuggerPresent;
        NtQueryInformationProcess_t m_originalNtQueryInformationProcess;
        
    public:
        AntiDebuggerSystem(const AntiDebuggerConfig& config = AntiDebuggerConfig()) 
            : m_config(config), m_running(false), m_detectionCount(0), m_debuggerDetected(false),
              m_randomGenerator(std::random_device{}()), m_startTime(GetTickCount64()) {
            
            // Initialize function pointers
            m_originalIsDebuggerPresent = reinterpret_cast<IsDebuggerPresent_t>(
                GetProcAddress(GetModuleHandleA("kernel32.dll"), "IsDebuggerPresent"));
            m_originalCheckRemoteDebuggerPresent = reinterpret_cast<CheckRemoteDebuggerPresent_t>(
                GetProcAddress(GetModuleHandleA("kernel32.dll"), "CheckRemoteDebuggerPresent"));
            m_originalNtQueryInformationProcess = reinterpret_cast<NtQueryInformationProcess_t>(
                GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess"));
        }
        
        ~AntiDebuggerSystem() {
            Stop();
        }
        
        void Start() {
            if (m_running.exchange(true)) return;
            
            LOG_INFO("Starting anti-debugger system");
            
            // Apply stealth measures
            if (m_config.enableStealthMode) {
                ApplyStealthMeasures();
            }
            
            // Start detection thread
            m_detectionThread = std::thread([this]() {
                DetectionLoop();
            });
        }
        
        void Stop() {
            if (!m_running.exchange(false)) return;
            
            LOG_INFO("Stopping anti-debugger system");
            
            // Stop detection thread
            if (m_detectionThread.joinable()) {
                m_detectionThread.join();
            }
            
            // Restore original functions
            RestoreStealthMeasures();
        }
        
        bool IsDebuggerDetected() const {
            return m_debuggerDetected.load();
        }
        
        std::vector<DetectionResult> GetDetectionHistory() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_detectionHistory;
        }
        
        int GetDetectionCount() const {
            return m_detectionCount.load();
        }
        
        void SetConfig(const AntiDebuggerConfig& config) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_config = config;
        }
        
        AntiDebuggerConfig GetConfig() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_config;
        }
        
        void ResetDetectionCount() {
            m_detectionCount.store(0);
            m_debuggerDetected.store(false);
            
            std::lock_guard<std::mutex> lock(m_mutex);
            m_detectionHistory.clear();
        }
        
        // Immediate detection check
        bool PerformImmediateCheck() {
            bool detected = false;
            
            if (m_config.enableWindowsAPIDetection) {
                detected |= CheckWindowsAPI();
            }
            
            if (m_config.enableProcessEnumeration) {
                detected |= CheckProcessEnumeration();
            }
            
            if (m_config.enableHardwareBreakpointDetection) {
                detected |= CheckHardwareBreakpoints();
            }
            
            if (m_config.enableMemoryIntegrityChecks) {
                detected |= CheckMemoryIntegrity();
            }
            
            if (m_config.enableVirtualizationDetection) {
                detected |= CheckVirtualization();
            }
            
            if (m_config.enableSandboxDetection) {
                detected |= CheckSandbox();
            }
            
            if (detected) {
                m_detectionCount++;
                if (m_detectionCount >= m_config.maxDetectionThreshold) {
                    m_debuggerDetected.store(true);
                    
                    if (m_config.enableCountermeasures) {
                        ActivateCountermeasures();
                    }
                }
            }
            
            return detected;
        }
        
    private:
        void DetectionLoop() {
            while (m_running) {
                try {
                    bool detected = false;
                    
                    // Randomize check order and timing
                    if (m_config.enableRandomizedChecks) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(
                            m_config.checkInterval + (m_randomGenerator() % 500)));
                    } else {
                        std::this_thread::sleep_for(std::chrono::milliseconds(m_config.checkInterval));
                    }
                    
                    // Perform checks
                    if (m_config.enableWindowsAPIDetection) {
                        detected |= CheckWindowsAPI();
                    }
                    
                    if (m_config.enableProcessEnumeration) {
                        detected |= CheckProcessEnumeration();
                    }
                    
                    if (m_config.enableTimingAttacks) {
                        detected |= CheckTimingAttacks();
                    }
                    
                    if (m_config.enableHardwareBreakpointDetection) {
                        detected |= CheckHardwareBreakpoints();
                    }
                    
                    if (m_config.enableMemoryIntegrityChecks) {
                        detected |= CheckMemoryIntegrity();
                    }
                    
                    if (m_config.enableVirtualizationDetection) {
                        detected |= CheckVirtualization();
                    }
                    
                    if (m_config.enableSandboxDetection) {
                        detected |= CheckSandbox();
                    }
                    
                    if (m_config.enableNetworkAnalysis) {
                        detected |= CheckNetworkAnalysis();
                    }
                    
                    if (m_config.enableFileSystemChecks) {
                        detected |= CheckFileSystem();
                    }
                    
                    if (m_config.enableRegistryChecks) {
                        detected |= CheckRegistry();
                    }
                    
                    if (detected) {
                        m_detectionCount++;
                        if (m_detectionCount >= m_config.maxDetectionThreshold) {
                            m_debuggerDetected.store(true);
                            
                            if (m_config.enableCountermeasures) {
                                ActivateCountermeasures();
                            }
                        }
                    }
                    
                } catch (const std::exception& e) {
                    LOG_ERROR("Anti-debugger detection error: " + std::string(e.what()));
                }
            }
        }
        
        bool CheckWindowsAPI() {
            bool detected = false;
            
            // Method 1: IsDebuggerPresent
            if (m_originalIsDebuggerPresent && m_originalIsDebuggerPresent()) {
                AddDetectionResult(DetectionMethod::WindowsAPI, true, "IsDebuggerPresent detected debugger");
                detected = true;
            }
            
            // Method 2: CheckRemoteDebuggerPresent
            if (m_originalCheckRemoteDebuggerPresent) {
                BOOL isDebugged = FALSE;
                if (m_originalCheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebugged) && isDebugged) {
                    AddDetectionResult(DetectionMethod::WindowsAPI, true, "CheckRemoteDebuggerPresent detected debugger");
                    detected = true;
                }
            }
            
            // Method 3: NtQueryInformationProcess
            if (m_originalNtQueryInformationProcess) {
                PROCESS_BASIC_INFORMATION pbi = {};
                ULONG returnLength = 0;
                
                if (m_originalNtQueryInformationProcess(GetCurrentProcess(), 
                                                      ProcessBasicInformation,
                                                      &pbi, sizeof(pbi), &returnLength) == 0) {
                    if (pbi.PebBaseAddress && pbi.PebBaseAddress->BeingDebugged) {
                        AddDetectionResult(DetectionMethod::WindowsAPI, true, "PEB BeingDebugged flag set");
                        detected = true;
                    }
                }
            }
            
            // Method 4: GetTickCount64 timing
            uint64_t start = GetTickCount64();
            DebugBreak(); // This will be caught by debugger
            uint64_t end = GetTickCount64();
            
            if (end - start > 100) { // Too slow = debugger present
                AddDetectionResult(DetectionMethod::TimingAttack, true, "DebugBreak timing attack detected debugger");
                detected = true;
            }
            
            return detected;
        }
        
        bool CheckProcessEnumeration() {
            bool detected = false;
            
            // Check for common debugger processes
            std::vector<std::wstring> debuggerProcesses = {
                L"ollydbg.exe",
                L"x64dbg.exe",
                L"windbg.exe",
                L"ida.exe",
                L"ida64.exe",
                L"cheatengine.exe",
                L"processhacker.exe",
                L"procexp.exe",
                L"reclass.exe",
                L"scylla.exe",
                L"x64netdumper.exe"
            };
            
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32W pe32;
                pe32.dwSize = sizeof(pe32);
                
                if (Process32FirstW(snapshot, &pe32)) {
                    do {
                        for (const auto& debugger : debuggerProcesses) {
                            if (_wcsicmp(pe32.szExeFile, debugger.c_str()) == 0) {
                                AddDetectionResult(DetectionMethod::ProcessEnumeration, true, 
                                               "Debugger process found: " + std::string(pe32.szExeFile, pe32.szExeFile + wcslen(pe32.szExeFile)));
                                detected = true;
                                break;
                            }
                        }
                    } while (Process32NextW(snapshot, &pe32));
                }
                
                CloseHandle(snapshot);
            }
            
            return detected;
        }
        
        bool CheckTimingAttacks() {
            bool detected = false;
            
            // Method 1: RDTSC timing
            uint64_t start = __rdtsc();
            
            // Perform some operations
            volatile int x = 0;
            for (int i = 0; i < 1000; i++) {
                x += i;
            }
            
            uint64_t end = __rdtsc();
            uint64_t cycles = end - start;
            
            // If it takes too many cycles, debugger might be present
            if (cycles > 50000) {
                AddDetectionResult(DetectionMethod::TimingAttack, true, "RDTSC timing attack detected debugger");
                detected = true;
            }
            
            // Method 2: QueryPerformanceCounter timing
            LARGE_INTEGER freq, start2, end2;
            QueryPerformanceFrequency(&freq);
            QueryPerformanceCounter(&start2);
            
            // Perform operations
            volatile int y = 0;
            for (int i = 0; i < 1000; i++) {
                y += i;
            }
            
            QueryPerformanceCounter(&end2);
            double time = static_cast<double>(end2.QuadPart - start2.QuadPart) / freq.QuadPart;
            
            // If it takes too much time, debugger might be present
            if (time > 0.001) { // 1ms threshold
                AddDetectionResult(DetectionMethod::TimingAttack, true, "QueryPerformanceCounter timing attack detected debugger");
                detected = true;
            }
            
            return detected;
        }
        
        bool CheckHardwareBreakpoints() {
            bool detected = false;
            
            CONTEXT ctx = {};
            ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
            
            if (GetThreadContext(GetCurrentThread(), &ctx)) {
                // Check for hardware breakpoints
                if (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0) {
                    AddDetectionResult(DetectionMethod::HardwareBreakpoints, true, "Hardware breakpoints detected");
                    detected = true;
                }
                
                // Check for debug registers being used
                if (ctx.Dr6 != 0 || ctx.Dr7 != 0) {
                    AddDetectionResult(DetectionMethod::HardwareBreakpoints, true, "Debug registers active");
                    detected = true;
                }
            }
            
            return detected;
        }
        
        bool CheckMemoryIntegrity() {
            bool detected = false;
            
            // Check for INT3 breakpoints (0xCC)
            const uint8_t* code = reinterpret_cast<const uint8_t*>(CheckMemoryIntegrity);
            for (int i = 0; i < 16; i++) {
                if (code[i] == 0xCC) {
                    AddDetectionResult(DetectionMethod::MemoryIntegrity, true, "INT3 breakpoint detected");
                    detected = true;
                    break;
                }
            }
            
            // Check for modified API functions
            HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
            if (kernel32) {
                uint8_t* isDebuggerPresent = reinterpret_cast<uint8_t*>(
                    GetProcAddress(kernel32, "IsDebuggerPresent"));
                
                if (isDebuggerPresent) {
                    // Check for hooking
                    if (isDebuggerPresent[0] == 0xE9 || isDebuggerPresent[0] == 0xE8 || isDebuggerPresent[0] == 0xCC) {
                        AddDetectionResult(DetectionMethod::MemoryIntegrity, true, "API function hooked");
                        detected = true;
                    }
                }
            }
            
            return detected;
        }
        
        bool CheckVirtualization() {
            bool detected = false;
            
            // Check for virtualization artifacts
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            
            // Check for common VM CPU features
            int cpuInfo[4];
            __cpuid(cpuInfo, 1);
            
            // Check for hypervisor bit
            if (cpuInfo[2] & (1 << 31)) {
                AddDetectionResult(DetectionMethod::Virtualization, true, "Hypervisor bit set in CPUID");
                detected = true;
            }
            
            // Check for VM-specific registry keys
            HKEY hKey;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Oracle\\VirtualBox", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                AddDetectionResult(DetectionMethod::Virtualization, true, "VirtualBox registry key found");
                detected = true;
                RegCloseKey(hKey);
            }
            
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\VMware, Inc.\\VMware Tools", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                AddDetectionResult(DetectionMethod::Virtualization, true, "VMware registry key found");
                detected = true;
                RegCloseKey(hKey);
            }
            
            return detected;
        }
        
        bool CheckSandbox() {
            bool detected = false;
            
            // Check for sandbox-specific processes
            std::vector<std::wstring> sandboxProcesses = {
                L"sandboxie.exe",
                L"sbiesvc.exe",
                L"wireshark.exe",
                L"fiddler.exe",
                L"procmon.exe",
                L"autoruns.exe",
                L"autorunsc.exe"
            };
            
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32W pe32;
                pe32.dwSize = sizeof(pe32);
                
                if (Process32FirstW(snapshot, &pe32)) {
                    do {
                        for (const auto& sandbox : sandboxProcesses) {
                            if (_wcsicmp(pe32.szExeFile, sandbox.c_str()) == 0) {
                                AddDetectionResult(DetectionMethod::Sandbox, true, 
                                               "Sandbox process found: " + std::string(pe32.szExeFile, pe32.szExeFile + wcslen(pe32.szExeFile)));
                                detected = true;
                                break;
                            }
                        }
                    } while (Process32NextW(snapshot, &pe32));
                }
                
                CloseHandle(snapshot);
            }
            
            return detected;
        }
        
        bool CheckNetworkAnalysis() {
            bool detected = false;
            
            // Check for network monitoring tools
            std::vector<std::wstring> networkTools = {
                L"wireshark.exe",
                L"tcpdump.exe",
                L"networkminer.exe",
                L"nmap.exe",
                L"netcat.exe"
            };
            
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32W pe32;
                pe32.dwSize = sizeof(pe32);
                
                if (Process32FirstW(snapshot, &pe32)) {
                    do {
                        for (const auto& tool : networkTools) {
                            if (_wcsicmp(pe32.szExeFile, tool.c_str()) == 0) {
                                AddDetectionResult(DetectionMethod::NetworkAnalysis, true, 
                                               "Network analysis tool found: " + std::string(pe32.szExeFile, pe32.szExeFile + wcslen(pe32.szExeFile)));
                                detected = true;
                                break;
                            }
                        }
                    } while (Process32NextW(snapshot, &pe32));
                }
                
                CloseHandle(snapshot);
            }
            
            return detected;
        }
        
        bool CheckFileSystem() {
            bool detected = false;
            
            // Check for analysis tools in common directories
            std::vector<std::string> analysisPaths = {
                "C:\\Program Files\\Wireshark",
                "C:\\Program Files\\x64dbg",
                "C:\\Program Files\\OllyDbg",
                "C:\\Program Files\\IDA",
                "C:\\Program Files\\Cheat Engine",
                "C:\\Program Files\\Process Hacker"
            };
            
            for (const auto& path : analysisPaths) {
                if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                    AddDetectionResult(DetectionMethod::FileSystem, true, "Analysis tool directory found: " + path);
                    detected = true;
                }
            }
            
            return detected;
        }
        
        bool CheckRegistry() {
            bool detected = false;
            
            // Check for debugger registry entries
            std::vector<std::string> registryKeys = {
                "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug",
                "SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug"
            };
            
            for (const auto& keyPath : registryKeys) {
                HKEY hKey;
                if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                    char debugger[MAX_PATH];
                    DWORD size = sizeof(debugger);
                    
                    if (RegQueryValueExA(hKey, "Debugger", nullptr, nullptr, 
                                       reinterpret_cast<LPBYTE>(debugger), &size) == ERROR_SUCCESS) {
                        if (strlen(debugger) > 0) {
                            AddDetectionResult(DetectionMethod::Registry, true, "Debugger registry entry found");
                            detected = true;
                        }
                    }
                    
                    RegCloseKey(hKey);
                }
            }
            
            return detected;
        }
        
        void ApplyStealthMeasures() {
            // Hide process from task manager (simplified)
            // In a real implementation, this would use more advanced techniques
            
            // Clear PE header timestamps
            HMODULE hModule = GetModuleHandle(nullptr);
            if (hModule) {
                IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(hModule);
                IMAGE_NT_HEADERS* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(
                    reinterpret_cast<char*>(dosHeader) + dosHeader->e_lfanew);
                
                // Clear timestamp
                ntHeaders->FileHeader.TimeDateStamp = 0;
            }
        }
        
        void RestoreStealthMeasures() {
            // Restore any modified functions or data
        }
        
        void AddDetectionResult(DetectionMethod method, bool detected, const std::string& description) {
            DetectionResult result;
            result.method = method;
            result.detected = detected;
            result.description = description;
            result.timestamp = GetTickCount64();
            result.confidence = 100; // High confidence for direct detections
            
            std::lock_guard<std::mutex> lock(m_mutex);
            m_detectionHistory.push_back(result);
            
            // Keep only last 100 results
            if (m_detectionHistory.size() > 100) {
                m_detectionHistory.erase(m_detectionHistory.begin());
            }
        }
        
        void ActivateCountermeasures() {
            LOG_WARNING("Debugger detected! Activating countermeasures...");
            
            // In a real implementation, this might:
            // 1. Terminate the process
            // 2. Corrupt critical data
            // 3. Enter infinite loop
            // 4. Crash the system
            // 5. Send alert to server
            
            // For now, we'll just log and continue
            LOG_ERROR("Debugger detected - system compromised");
            
            // Optional: Terminate process
            // ExitProcess(0);
        }
    };
    
} // namespace AntiDebug

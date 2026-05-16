#include "Loader.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>
#include "AdvancedObfuscation.h"

#pragma comment(lib, "psapi.lib")

using namespace AdvancedObfuscation;

namespace Loader {

    CLoader::CLoader()
        : m_status(LoaderStatus::Idle), m_running(false) {
        std::random_device rd;
        m_rng = std::mt19937(rd());
        
        // Reserve capacity for vectors to avoid reallocations
        m_foundProcesses.reserve(32);
        m_injectionHistory.reserve(100);
    }

    CLoader::~CLoader() { Shutdown(); }

    bool CLoader::Initialize() {
        // Apply timing obfuscation during initialization
        TimingObfuscator::ObfuscateTiming();
        ControlFlowObfuscator::RandomDelay();
        
        SetStatus(LoaderStatus::Idle, "Loader initialized");
        return true;
    }

    void CLoader::Shutdown() {
        m_running = false;
        SetStatus(LoaderStatus::Idle, "Loader shutdown");
    }

    void CLoader::SetStatus(LoaderStatus status, const std::string& message) {
        m_status = status;
        if (m_statusCallback) {
            m_statusCallback(status, message);
        }
    }

    void CLoader::RandomDelay(uint32_t minMs, uint32_t maxMs) {
        if (!m_config.randomizeInjection) return;
        std::uniform_int_distribution<uint32_t> dist(minMs, maxMs);
        Sleep(dist(m_rng));
    }

    ProcessInfo CLoader::FindProcess(const std::string& name, const std::string& windowTitle) {
        ProcessInfo result = {};
        SetStatus(LoaderStatus::Searching, "Searching for " + name);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return result;

        PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
        if (Process32FirstW(snapshot, &pe)) {
            do {
                std::wstring wname(pe.szExeFile);
                std::string procName(wname.begin(), wname.end());

                if (_stricmp(procName.c_str(), name.c_str()) == 0) {
                    result.pid = pe.th32ProcessID;
                    result.name = procName;
                    result.isRunning = true;

                    HANDLE hProcess = OpenProcess(
                        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, result.pid);
                    if (hProcess) {
                        BOOL isWow64 = FALSE;
                        IsWow64Process(hProcess, &isWow64);
#ifdef _WIN64
                        result.is64Bit = !isWow64;
#else
                        result.is64Bit = false;
#endif
                        HMODULE hMods[1024];
                        DWORD cbNeeded;
                        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
                            MODULEINFO modInfo;
                            if (GetModuleInformation(hProcess, hMods[0], &modInfo, sizeof(modInfo))) {
                                result.baseAddress = (uint64_t)modInfo.lpBaseOfDll;
                                result.size = modInfo.SizeOfImage;
                            }
                        }
                        CloseHandle(hProcess);
                    }

                    if (!windowTitle.empty()) {
                        HWND hwnd = nullptr;
                        do {
                            hwnd = FindWindowExA(nullptr, hwnd, nullptr, nullptr);
                            if (hwnd) {
                                DWORD wndPid = 0;
                                GetWindowThreadProcessId(hwnd, &wndPid);
                                if (wndPid == result.pid) {
                                    char title[256] = {};
                                    GetWindowTextA(hwnd, title, sizeof(title));
                                    std::string wndTitle(title);
                                    if (wndTitle.find(windowTitle) != std::string::npos) {
                                        result.windowTitle = wndTitle;
                                        break;
                                    }
                                }
                            }
                        } while (hwnd);
                    }

                    CloseHandle(snapshot);
                    SetStatus(LoaderStatus::Found, "Found " + name + " (PID: " + std::to_string(result.pid) + ")");
                    return result;
                }
            } while (Process32NextW(snapshot, &pe));
        }

        CloseHandle(snapshot);
        SetStatus(LoaderStatus::Waiting, "Process not found, waiting...");
        return result;
    }

    std::vector<ProcessInfo> CLoader::FindAllProcesses(const std::string& name) {
        std::vector<ProcessInfo> results;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return results;

        PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
        if (Process32FirstW(snapshot, &pe)) {
            do {
                std::wstring wname(pe.szExeFile);
                std::string procName(wname.begin(), wname.end());
                if (_stricmp(procName.c_str(), name.c_str()) == 0) {
                    ProcessInfo info;
                    info.pid = pe.th32ProcessID;
                    info.name = procName;
                    info.isRunning = true;
                    results.push_back(info);
                }
            } while (Process32NextW(snapshot, &pe));
        }

        CloseHandle(snapshot);
        return results;
    }

    bool CLoader::IsProcessValid(const ProcessInfo& info) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, info.pid);
        if (!hProcess) return false;
        DWORD exitCode = 0;
        bool valid = GetExitCodeProcess(hProcess, &exitCode) && exitCode == STILL_ACTIVE;
        CloseHandle(hProcess);
        return valid;
    }

    uint64_t CLoader::GetProcessBaseAddress(uint32_t pid) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!hProcess) return 0;

        HMODULE hMods[1024];
        DWORD cbNeeded;
        uint64_t base = 0;

        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
            MODULEINFO modInfo;
            if (GetModuleInformation(hProcess, hMods[0], &modInfo, sizeof(modInfo))) {
                base = (uint64_t)modInfo.lpBaseOfDll;
            }
        }

        CloseHandle(hProcess);
        return base;
    }

    HANDLE CLoader::OpenTargetProcess(uint32_t pid, bool forInjection) {
        DWORD access = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
        if (forInjection) {
            access |= PROCESS_CREATE_THREAD | PROCESS_VM_WRITE | PROCESS_VM_OPERATION;
        }
        return OpenProcess(access, FALSE, pid);
    }

    bool CLoader::AllocateAndWriteDLLPath(HANDLE hProcess, const std::string& dllPath, void*& remotePath) {
        size_t pathSize = dllPath.length() + 1;
        remotePath = VirtualAllocEx(hProcess, nullptr, pathSize,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!remotePath) return false;

        return WriteProcessMemory(hProcess, remotePath, dllPath.c_str(), pathSize, nullptr) != FALSE;
    }

    void CLoader::CleanupInjection(HANDLE hProcess, HANDLE hThread, void* remoteMem) {
        if (hThread) {
            WaitForSingleObject(hThread, 5000);
            CloseHandle(hThread);
        }
        if (remoteMem) {
            VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        }
    }

    void CLoader::ErasePEHeaders(HANDLE hProcess, uint64_t moduleBase) {
        if (!m_config.erasePEHeaders) return;

        uint8_t zeros[0x1000] = {};
        DWORD oldProtect = 0;
        VirtualProtectEx(hProcess, (LPVOID)moduleBase, 0x1000, PAGE_READWRITE, &oldProtect);
        WriteProcessMemory(hProcess, (LPVOID)moduleBase, zeros, 0x1000, nullptr);
        VirtualProtectEx(hProcess, (LPVOID)moduleBase, 0x1000, oldProtect, &oldProtect);
    }

    void CLoader::HideDLLModule(HANDLE hProcess, const std::string& dllName) {
        if (!m_config.hideModule) return;

        HMODULE hMods[1024];
        DWORD cbNeeded;
        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
            for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                char modName[MAX_PATH] = {};
                if (GetModuleFileNameExA(hProcess, hMods[i], modName, sizeof(modName))) {
                    std::string name(modName);
                    if (name.find(dllName) != std::string::npos) {
                        MODULEINFO modInfo;
                        if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo))) {
                            ErasePEHeaders(hProcess, (uint64_t)modInfo.lpBaseOfDll);
                        }
                    }
                }
            }
        }
    }

    bool CLoader::InjectLoadLibrary(const ProcessInfo& target) {
        // Apply control flow obfuscation before injection
        ControlFlowObfuscator::ObfuscatedLoop(5);
        ControlFlowObfuscator::JunkCode();
        
        HANDLE hProcess = OpenTargetProcess(target.pid, true);
        if (!hProcess) return false;

        void* remotePath = nullptr;
        if (!AllocateAndWriteDLLPath(hProcess, m_config.dllPath, remotePath)) {
            CloseHandle(hProcess);
            return false;
        }

        RandomDelay(10, 100);

        LPTHREAD_START_ROUTINE loadLibraryAddr = (LPTHREAD_START_ROUTINE)
            GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
        if (!loadLibraryAddr) {
            VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false;
        }

        HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
            loadLibraryAddr, remotePath, 0, nullptr);
        if (!hThread) {
            VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false;
        }

        CleanupInjection(hProcess, hThread, remotePath);

        if (m_config.hideModule || m_config.erasePEHeaders) {
            Sleep(500);
            char dllName[MAX_PATH] = {};
            const char* lastSlash = strrchr(m_config.dllPath.c_str(), '\\');
            strcpy_s(dllName, lastSlash ? lastSlash + 1 : m_config.dllPath.c_str());
            HideDLLModule(hProcess, dllName);
        }

        CloseHandle(hProcess);
        return true;
    }

    bool CLoader::InjectManualMap(const ProcessInfo& target) {
        HANDLE hProcess = OpenTargetProcess(target.pid, true);
        if (!hProcess) return false;

        HANDLE hFile = CreateFileA(m_config.dllPath.c_str(), GENERIC_READ,
            FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            CloseHandle(hProcess);
            return false;
        }

        DWORD fileSize = GetFileSize(hFile, nullptr);
        std::vector<uint8_t> fileData(fileSize);
        DWORD bytesRead = 0;
        ReadFile(hFile, fileData.data(), fileSize, &bytesRead, nullptr);
        CloseHandle(hFile);

        auto* dosHeader = (PIMAGE_DOS_HEADER)fileData.data();
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            CloseHandle(hProcess);
            return false;
        }

        auto* ntHeader = (PIMAGE_NT_HEADERS)(fileData.data() + dosHeader->e_lfanew);
        if (ntHeader->Signature != IMAGE_NT_SIGNATURE) {
            CloseHandle(hProcess);
            return false;
        }

        uint32_t imageSize = ntHeader->OptionalHeader.SizeOfImage;
        void* remoteImage = VirtualAllocEx(hProcess, nullptr, imageSize,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!remoteImage) {
            CloseHandle(hProcess);
            return false;
        }

        uint32_t headerSize = ntHeader->OptionalHeader.SizeOfHeaders;
        WriteProcessMemory(hProcess, remoteImage, fileData.data(), headerSize, nullptr);

        auto* sectionHeader = IMAGE_FIRST_SECTION(ntHeader);
        for (WORD i = 0; i < ntHeader->FileHeader.NumberOfSections; i++) {
            if (sectionHeader[i].SizeOfRawData > 0) {
                void* sectionDest = (uint8_t*)remoteImage + sectionHeader[i].VirtualAddress;
                WriteProcessMemory(hProcess, sectionDest,
                    fileData.data() + sectionHeader[i].PointerToRawData,
                    sectionHeader[i].SizeOfRawData, nullptr);
            }
        }

        uint64_t delta = (uint64_t)remoteImage - ntHeader->OptionalHeader.ImageBase;
        if (delta != 0) {
            auto* baseReloc = (PIMAGE_BASE_RELOCATION)((uint64_t)remoteImage +
                ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
            DWORD relocSize = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

            DWORD offset = 0;
            while (offset < relocSize) {
                auto* block = (PIMAGE_BASE_RELOCATION)((uint64_t)baseReloc + offset);
                if (block->SizeOfBlock == 0) break;

                DWORD entryCount = (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                WORD* entries = (WORD*)((uint64_t)block + sizeof(IMAGE_BASE_RELOCATION));

                for (DWORD j = 0; j < entryCount; j++) {
                    if (entries[j] >> 12 == IMAGE_REL_BASED_DIR64) {
                        uint64_t* patchAddr = (uint64_t*)((uint64_t)remoteImage +
                            block->VirtualAddress + (entries[j] & 0xFFF));
                        uint64_t patchValue = 0;
                        ReadProcessMemory(hProcess, patchAddr, &patchValue, 8, nullptr);
                        patchValue += delta;
                        WriteProcessMemory(hProcess, patchAddr, &patchValue, 8, nullptr);
                    }
                }
                offset += block->SizeOfBlock;
            }
        }

        if (ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size > 0) {
            auto* importDesc = (PIMAGE_IMPORT_DESCRIPTOR)((uint64_t)remoteImage +
                ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

            while (importDesc->Name) {
                char* moduleName = (char*)((uint64_t)remoteImage + importDesc->Name);
                HMODULE hMod = LoadLibraryA(moduleName);
                if (!hMod) break;

                auto* thunk = (PIMAGE_THUNK_DATA)((uint64_t)remoteImage + importDesc->FirstThunk);
                auto* origThunk = importDesc->OriginalFirstThunk ?
                    (PIMAGE_THUNK_DATA)((uint64_t)remoteImage + importDesc->OriginalFirstThunk) : thunk;

                while (origThunk->u1.AddressOfData) {
                    FARPROC funcAddr = nullptr;
                    if (origThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
                        funcAddr = GetProcAddress(hMod, (LPCSTR)(origThunk->u1.Ordinal & 0xFFFF));
                    } else {
                        auto* byName = (PIMAGE_IMPORT_BY_NAME)((uint64_t)remoteImage +
                            origThunk->u1.AddressOfData);
                        funcAddr = GetProcAddress(hMod, byName->Name);
                    }
                    WriteProcessMemory(hProcess, &thunk->u1.Function, &funcAddr, 8, nullptr);
                    origThunk++;
                    thunk++;
                }
                importDesc++;
            }
        }

        using DllMain_t = BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID);
        auto dllMain = (DllMain_t)((uint64_t)remoteImage + ntHeader->OptionalHeader.AddressOfEntryPoint);

        HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
            (LPTHREAD_START_ROUTINE)dllMain, remoteImage, DLL_PROCESS_ATTACH, nullptr);

        if (hThread) {
            WaitForSingleObject(hThread, 10000);
            CloseHandle(hThread);
        }

        CloseHandle(hProcess);
        return true;
    }

    bool CLoader::InjectThreadHijack(const ProcessInfo& target) {
        HANDLE hProcess = OpenTargetProcess(target.pid, true);
        if (!hProcess) return false;

        THREADENTRY32 te = { sizeof(THREADENTRY32) };
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            CloseHandle(hProcess);
            return false;
        }

        DWORD targetThreadId = 0;
        if (Thread32First(hSnapshot, &te)) {
            do {
                if (te.th32OwnerProcessID == target.pid) {
                    targetThreadId = te.th32ThreadID;
                    break;
                }
            } while (Thread32Next(hSnapshot, &te));
        }
        CloseHandle(hSnapshot);

        if (!targetThreadId) {
            CloseHandle(hProcess);
            return false;
        }

        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT |
            THREAD_SET_CONTEXT, FALSE, targetThreadId);
        if (!hThread) {
            CloseHandle(hProcess);
            return false;
        }

        SuspendThread(hThread);

        void* remotePath = nullptr;
        if (!AllocateAndWriteDLLPath(hProcess, m_config.dllPath, remotePath)) {
            ResumeThread(hThread);
            CloseHandle(hThread);
            CloseHandle(hProcess);
            return false;
        }

        void* remoteCode = VirtualAllocEx(hProcess, nullptr, 0x1000,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!remoteCode) {
            VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
            ResumeThread(hThread);
            CloseHandle(hThread);
            CloseHandle(hProcess);
            return false;
        }

        uint8_t shellcode[] = {
            0x48, 0x83, 0xEC, 0x28,
            0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xD0,
            0x48, 0x83, 0xC4, 0x28,
            0xC3
        };

        memcpy(shellcode + 4, &remotePath, 8);
        void* loadLibAddr = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
        memcpy(shellcode + 14, &loadLibAddr, 8);

        WriteProcessMemory(hProcess, remoteCode, shellcode, sizeof(shellcode), nullptr);

        CONTEXT ctx = {};
        ctx.ContextFlags = CONTEXT_CONTROL;
        GetThreadContext(hThread, &ctx);

        uint64_t originalRip = ctx.Rip;
        ctx.Rip = (DWORD64)remoteCode;
        SetThreadContext(hThread, &ctx);

        ResumeThread(hThread);

        Sleep(500);

        SuspendThread(hThread);
        ctx.Rip = originalRip;
        SetThreadContext(hThread, &ctx);
        ResumeThread(hThread);

        VirtualFreeEx(hProcess, remoteCode, 0, MEM_RELEASE);
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hThread);
        CloseHandle(hProcess);
        return true;
    }

    bool CLoader::InjectAPC(const ProcessInfo& target) {
        HANDLE hProcess = OpenTargetProcess(target.pid, true);
        if (!hProcess) return false;

        void* remotePath = nullptr;
        if (!AllocateAndWriteDLLPath(hProcess, m_config.dllPath, remotePath)) {
            CloseHandle(hProcess);
            return false;
        }

        void* loadLibAddr = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

        THREADENTRY32 te = { sizeof(THREADENTRY32) };
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false;
        }

        bool injected = false;
        if (Thread32First(hSnapshot, &te)) {
            do {
                if (te.th32OwnerProcessID == target.pid) {
                    HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, te.th32ThreadID);
                    if (hThread) {
                        QueueUserAPC((PAPCFUNC)loadLibAddr, hThread, (ULONG_PTR)remotePath);
                        CloseHandle(hThread);
                        injected = true;
                    }
                }
            } while (Thread32Next(hSnapshot, &te));
        }

        CloseHandle(hSnapshot);

        if (!injected) {
            VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false;
        }

        Sleep(1000);
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return true;
    }

    bool CLoader::InjectProcessHollow(const ProcessInfo& target) {
        return false;
    }

    bool CLoader::InjectKernelDriver(const ProcessInfo& target) {
        return false;
    }

    ProcessInfo CLoader::ScanForProcess() {
        m_foundProcesses = FindAllProcesses(m_config.processName);
        if (m_foundProcesses.empty()) {
            return FindProcess(m_config.processName, m_config.windowTitle);
        }

        ProcessInfo best = m_foundProcesses[0];
        best.baseAddress = GetProcessBaseAddress(best.pid);
        SetStatus(LoaderStatus::Found, "Found " + best.name + " (PID: " + std::to_string(best.pid) + ")");
        return best;
    }

    bool CLoader::IsProcessFound() const {
        return !m_foundProcesses.empty();
    }

    InjectionResult CLoader::Inject() {
        InjectionResult result = {};
        result.timestamp = std::chrono::system_clock::now();

        ProcessInfo target = ScanForProcess();
        if (target.pid == 0) {
            result.success = false;
            result.errorMessage = "Process not found";
            SetStatus(LoaderStatus::Failed, result.errorMessage);
            return result;
        }

        result.targetPID = target.pid;
        result.methodUsed = m_config.method;

        SetStatus(LoaderStatus::Injecting, "Injecting using " +
            std::to_string((int)m_config.method) + "...");

        RandomDelay(m_config.injectDelayMs / 2, m_config.injectDelayMs);

        bool success = false;
        switch (m_config.method) {
        case InjectionMethod::LoadLibrary:
            success = InjectLoadLibrary(target);
            break;
        case InjectionMethod::ManualMap:
            success = InjectManualMap(target);
            break;
        case InjectionMethod::ThreadHijack:
            success = InjectThreadHijack(target);
            break;
        case InjectionMethod::APCInjection:
            success = InjectAPC(target);
            break;
        case InjectionMethod::ProcessHollow:
            success = InjectProcessHollow(target);
            break;
        case InjectionMethod::KernelDriver:
            success = InjectKernelDriver(target);
            break;
        }

        result.success = success;
        if (success) {
            result.errorMessage = "Injection successful";
            SetStatus(LoaderStatus::Injected, result.errorMessage);
        } else {
            result.errorMessage = "Injection failed";
            SetStatus(LoaderStatus::Failed, result.errorMessage);
        }

        m_injectionHistory.push_back(result);
        return result;
    }

    bool CLoader::Eject(uint32_t pid, const std::string& dllPath) {
        HANDLE hProcess = OpenTargetProcess(pid, true);
        if (!hProcess) return false;

        HMODULE hMods[1024];
        DWORD cbNeeded;
        bool found = false;

        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
            for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                char modName[MAX_PATH] = {};
                if (GetModuleFileNameExA(hProcess, hMods[i], modName, sizeof(modName))) {
                    if (_stricmp(modName, dllPath.c_str()) == 0) {
                        HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
                            (LPTHREAD_START_ROUTINE)GetProcAddress(
                                GetModuleHandleA("kernel32.dll"), "FreeLibrary"),
                            hMods[i], 0, nullptr);
                        if (hThread) {
                            WaitForSingleObject(hThread, 5000);
                            CloseHandle(hThread);
                            found = true;
                        }
                        break;
                    }
                }
            }
        }

        CloseHandle(hProcess);
        return found;
    }

    void CLoader::RunAutoInject() {
        m_running = true;
        SetStatus(LoaderStatus::Waiting, "Auto-inject enabled, waiting for " + m_config.processName);

        while (m_running) {
            ProcessInfo target = ScanForProcess();
            if (target.pid != 0) {
                RandomDelay(500, m_config.injectDelayMs);
                InjectionResult result = Inject();
                if (result.success) {
                    SetStatus(LoaderStatus::Injected, "Auto-injected successfully");
                    break;
                }
            }
            Sleep(2000);
        }
    }

    void CLoader::StopAutoInject() {
        m_running = false;
    }

}

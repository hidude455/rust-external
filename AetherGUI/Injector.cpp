#include "Injector.h"
#include <psapi.h>

CInjector::CInjector()
    : m_processId(0)
    , m_processHandle(nullptr)
    , m_processFound(false)
{
}

CInjector::~CInjector() {
    if (m_processHandle && m_processHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_processHandle);
    }
}

bool CInjector::EnableDebugPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }

    LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    bool result = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, nullptr, nullptr) != FALSE;
    CloseHandle(hToken);
    return result;
}

bool CInjector::FindProcess(const std::wstring& processName) {
    EnableDebugPrivilege();

    if (m_processHandle && m_processHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return false;
    }

    m_processFound = false;
    m_processId = 0;

    do {
        if (_wcsicmp(pe32.szExeFile, processName.c_str()) == 0) {
            m_processId = pe32.th32ProcessID;
            m_processName = pe32.szExeFile;
            m_processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_processId);

            if (!m_processHandle || m_processHandle == INVALID_HANDLE_VALUE) {
                m_processHandle = nullptr;
                m_processFound = false;
                m_processId = 0;
                m_processName.clear();
            } else {
                m_processFound = true;
            }
            break;
        }
    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return m_processFound;
}

HMODULE CInjector::LoadRemoteModule(HANDLE hProcess, const std::wstring& dllPath) {
    size_t pathLen = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, nullptr, pathLen, MEM_COMMIT, PAGE_READWRITE);
    
    if (!pRemoteMem) {
        return nullptr;
    }

    if (!WriteProcessMemory(hProcess, pRemoteMem, dllPath.c_str(), pathLen, nullptr)) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        return nullptr;
    }

    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    LPVOID pLoadLibraryW = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryW");

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, 
        (LPTHREAD_START_ROUTINE)pLoadLibraryW, pRemoteMem, 0, nullptr);

    if (!hThread) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        return nullptr;
    }

    WaitForSingleObject(hThread, INFINITE);
    
    DWORD exitCode;
    GetExitCodeThread(hThread, &exitCode);
    
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);

    return (HMODULE)exitCode;
}

bool CInjector::InjectDLL(const std::wstring& dllPath) {
    if (!m_processFound || !m_processHandle) {
        return false;
    }

    HMODULE hModule = LoadRemoteModule(m_processHandle, dllPath);
    return hModule != nullptr;
}

bool CInjector::ManualMap(HANDLE hProcess, const std::wstring& dllPath) {
    // Simplified manual mapping implementation
    // In a full implementation, this would:
    // 1. Read the DLL from disk
    // 2. Parse PE headers
    // 3. Allocate memory in target process
    // 4. Copy sections
    // 5. Relocate image
    // 6. Resolve imports
    // 7. Execute TLS callbacks
    // 8. Call DllMain
    
    // For now, fall back to LoadLibrary injection
    return InjectDLL(dllPath);
}

bool CInjector::InjectManualMap(const std::wstring& dllPath) {
    if (!m_processFound || !m_processHandle) {
        return false;
    }
    return ManualMap(m_processHandle, dllPath);
}

bool CInjector::IsProcessFound() const {
    return m_processFound;
}

DWORD CInjector::GetProcessId() const {
    return m_processId;
}

std::wstring CInjector::GetProcessName() const {
    return m_processName;
}

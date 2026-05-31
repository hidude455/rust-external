#pragma once
#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include <memory>

class CInjector {
public:
    CInjector();
    ~CInjector();

    bool FindProcess(const std::wstring& processName);
    bool InjectDLL(const std::wstring& dllPath);
    bool InjectManualMap(const std::wstring& dllPath);
    
    bool IsProcessFound() const;
    DWORD GetProcessId() const;
    std::wstring GetProcessName() const;

private:
    bool EnableDebugPrivilege();
    HMODULE LoadRemoteModule(HANDLE hProcess, const std::wstring& dllPath);
    bool ManualMap(HANDLE hProcess, const std::wstring& dllPath);

private:
    DWORD m_processId;
    HANDLE m_processHandle;
    std::wstring m_processName;
    bool m_processFound;
};

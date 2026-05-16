#include "MemoryManager.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>

namespace MIT {
    MemoryManager::MemoryManager() : processHandle(nullptr), processId(0), isAttached(false), gen(rd()), dis(0, 1000) {
    }

    MemoryManager::~MemoryManager() {
        Detach();
    }

    void MemoryManager::GenerateJunkCode() {
        // Simple junk code generation for obfuscation
        volatile int junk = dis(gen);
        for (int i = 0; i < junk % 5; ++i) {
            junk = (junk * 1103515245 + 12345) & 0x7fffffff;
        }
        
        // Add some meaningless operations
        if (junk % 2 == 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(junk % 100));
        }
    }

    bool MemoryManager::AttachToProcess(const std::string& processName) {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;

        bool found = false;
        if (Process32First(snapshot, &entry)) {
            do {
                std::wstring wProcessName(entry.szExeFile);
                std::string currentProcessName(wProcessName.begin(), wProcessName.end());
                
                if (currentProcessName == processName) {
                    processId = entry.th32ProcessID;
                    found = true;
                    break;
                }
            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);

        if (!found) return false;

        processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
        if (processHandle == nullptr) return false;

        isAttached = true;
        return true;
    }

    void MemoryManager::Detach() {
        if (processHandle && processHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(processHandle);
            processHandle = nullptr;
        }
        isAttached = false;
        processId = 0;
    }

    uintptr_t MemoryManager::GetModuleBaseAddress(const std::string& moduleName) {
        if (!isAttached) return 0;

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;

        MODULEENTRY32 entry;
        entry.dwSize = sizeof(MODULEENTRY32);
        uintptr_t baseAddress = 0;

        if (Module32First(snapshot, &entry)) {
            do {
                std::wstring wModuleName(entry.szModule);
                std::string currentModuleName(wModuleName.begin(), wModuleName.end());
                
                if (currentModuleName == moduleName) {
                    baseAddress = (uintptr_t)entry.modBaseAddr;
                    break;
                }
            } while (Module32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return baseAddress;
    }

    uintptr_t MemoryManager::CalculateAddress(uintptr_t base, const std::vector<uintptr_t>& offsets) {
        uintptr_t address = base;
        for (size_t i = 0; i < offsets.size(); ++i) {
            address = ReadMemory<uintptr_t>(address);
            if (address == 0) return 0;
            address += offsets[i];
        }
        return address;
    }

    uintptr_t MemoryManager::FollowPointerChain(uintptr_t base, const std::vector<uintptr_t>& offsets) {
        return CalculateAddress(base, offsets);
    }

    std::vector<uintptr_t> MemoryManager::ScanPattern(const std::string& pattern, const std::string& mask) {
        std::vector<uintptr_t> results;
        
        if (!isAttached) return results;

        MODULEINFO moduleInfo;
        if (!GetModuleInformation(processHandle, GetModuleHandle(nullptr), &moduleInfo, sizeof(moduleInfo))) {
            return results;
        }

        uintptr_t base = (uintptr_t)moduleInfo.lpBaseOfDll;
        uintptr_t size = moduleInfo.SizeOfImage;

        std::vector<uint8_t> buffer(size);
        SIZE_T bytesRead;
        
        if (!ReadProcessMemory(processHandle, (LPCVOID)base, buffer.data(), size, &bytesRead)) {
            return results;
        }

        for (size_t i = 0; i <= bytesRead - pattern.length(); ++i) {
            bool found = true;
            for (size_t j = 0; j < pattern.length(); ++j) {
                if (mask[j] != '?' && buffer[i + j] != (uint8_t)pattern[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                results.push_back(base + i);
            }
        }

        return results;
    }

    std::vector<Entity> MemoryManager::GetEntities() {
        std::vector<Entity> entities;
        
        // This would be implemented based on the specific game's memory structure
        // For now, returning empty vector as placeholder
        
        return entities;
    }

    WeaponData MemoryManager::GetCurrentWeapon() {
        WeaponData weapon{};
        
        // This would be implemented based on the specific game's memory structure
        // For now, returning empty weapon data as placeholder
        
        return weapon;
    }

    Vector3 MemoryManager::GetLocalPlayerPosition() {
        Vector3 position{};
        
        // This would be implemented based on the specific game's memory structure
        // For now, returning zero position as placeholder
        
        return position;
    }

    bool MemoryManager::IsProcessValid() {
        if (!isAttached || !processHandle) return false;
        
        DWORD exitCode;
        return GetExitCodeProcess(processHandle, &exitCode) && exitCode == STILL_ACTIVE;
    }
}

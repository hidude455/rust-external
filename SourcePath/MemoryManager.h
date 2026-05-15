#pragma once
#include "Common.h"

namespace MIT {
    class MemoryManager {
    private:
        HANDLE processHandle;
        DWORD processId;
        std::mutex memoryMutex;
        bool isAttached;
        
        // Junk code obfuscation
        std::random_device rd;
        std::mt19937 gen;
        std::uniform_int_distribution<> dis;

        void GenerateJunkCode();
        uintptr_t CalculateAddress(uintptr_t base, const std::vector<uintptr_t>& offsets);

    public:
        MemoryManager();
        ~MemoryManager();

        bool AttachToProcess(const std::string& processName);
        void Detach();
        
        template<typename T>
        T ReadMemory(uintptr_t address) {
            if (!isAttached) return T{};
            
            std::lock_guard<std::mutex> lock(memoryMutex);
            T value{};
            SIZE_T bytesRead = 0;
            
            GenerateJunkCode(); // Obfuscate memory access
            
            if (ReadProcessMemory(processHandle, (LPCVOID)address, &value, sizeof(T), &bytesRead)) {
                return value;
            }
            
            return T{};
        }

        template<typename T>
        bool WriteMemory(uintptr_t address, const T& value) {
            if (!isAttached) return false;
            
            std::lock_guard<std::mutex> lock(memoryMutex);
            SIZE_T bytesWritten = 0;
            
            GenerateJunkCode(); // Obfuscate memory access
            
            return WriteProcessMemory(processHandle, (LPVOID)address, &value, sizeof(T), &bytesWritten) && 
                   bytesWritten == sizeof(T);
        }

        uintptr_t GetModuleBaseAddress(const std::string& moduleName);
        std::vector<Entity> GetEntities();
        WeaponData GetCurrentWeapon();
        Vector3 GetLocalPlayerPosition();
        bool IsProcessValid();
        
        // Advanced memory functions
        std::vector<uintptr_t> ScanPattern(const std::string& pattern, const std::string& mask);
        uintptr_t FollowPointerChain(uintptr_t base, const std::vector<uintptr_t>& offsets);
    };
}

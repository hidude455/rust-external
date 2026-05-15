/*
 * Memory Access Module
 * Handles all game memory operations with DMA-style access patterns
 * Author: Anonymous
 * Last Modified: 2026
 * 
 * This module provides safe memory reading/writing with anti-detection measures.
 * Uses hardware abstraction layer for DMA operations when available.
 */

#pragma once
#include "Core.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <vector>

namespace GameEnhance {
    
    // Memory region information for safe access
    struct MemRegion {
        uint64_t baseAddr;      // Starting address
        size_t regionSize;       // Size of region
        uint32_t protection;     // Memory protection flags
        bool canRead;           // Read permission
        bool canWrite;          // Write permission
        std::string regionName;  // Region identifier
    };
    
    // DMA device structure for hardware-level access
    struct DMADevice {
        HANDLE deviceHandle;     // Device handle
        void* mappedMemory;      // Mapped memory region
        size_t memSize;          // Size of mapped memory
        uint32_t deviceId;       // Device identifier
        bool isActive;           // Device status
        uint64_t processBase;    // Target process base address
    };
    
    class CMemoryAccess {
    private:
        // DMA devices for memory access
        std::vector<DMADevice> m_dmaDevices;
        DMADevice* m_activeDevice;
        
        // Process information
        uint32_t m_targetPid;
        uint64_t m_processBase;
        std::string m_processName;
        
        // Thread safety
        CRITICAL_SECTION m_csLock;
        
        // Memory regions cache
        std::vector<MemRegion> m_cachedRegions;
        bool m_regionsCached;
        
        // Anti-detection measures
        uint32_t m_accessCounter;
        uint64_t m_lastAccessTime;
        std::vector<uint64_t> m_accessHistory;
        
        // Private helper methods
        bool FindDMADevices();
        bool InitializeDMADevice(DMADevice& device);
        bool MapProcessMemory(DMADevice& device);
        void ScanMemoryRegions();
        bool IsValidAddress(uint64_t address);
        void SimulateNormalAccess(uint64_t address);
        void RandomizeAccessPattern();
        uint32_t GetRandomDelay();
        
        // Advanced memory operations
        std::vector<uint64_t> FindPattern(const std::string& pattern, const std::string& mask);
        uint64_t FollowPointerChain(uint64_t base, const std::vector<uint64_t>& offsets);
        
    public:
        CMemoryAccess();
        ~CMemoryAccess();
        
        // Initialization and cleanup
        bool Initialize(const std::string& processName);
        void Shutdown();
        bool IsInitialized() const;
        
        // Process management
        bool AttachToProcess(uint32_t pid);
        void DetachFromProcess();
        bool IsProcessValid() const;
        uint32_t GetTargetPid() const { return m_targetPid; }
        
        // Basic memory operations
        template<typename T>
        T ReadMemory(uint64_t address) {
            if (!IsValidAddress(address)) {
                return T{};
            }
            
            EnterCriticalSection(&m_csLock);
            
            // Simulate legitimate access patterns
            SimulateNormalAccess(address);
            RandomizeAccessPattern();
            
            T value = {};
            bool success = false;
            
            if (m_activeDevice && m_activeDevice->mappedMemory) {
                // DMA-based memory access
                uint64_t offset = address - m_activeDevice->processBase;
                if (offset < m_activeDevice->memSize) {
                    __try {
                        value = *reinterpret_cast<T*>((uint64_t)m_activeDevice->mappedMemory + offset);
                        success = true;
                    } __except(EXCEPTION_EXECUTE_HANDLER) {
                        // Handle access violations
                        success = false;
                    }
                }
            } else {
                // Fallback to Windows API
                HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, m_targetPid);
                if (hProcess) {
                    SIZE_T bytesRead = 0;
                    success = ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), &bytesRead) && 
                             bytesRead == sizeof(T);
                    CloseHandle(hProcess);
                }
            }
            
            // Update access tracking
            m_accessCounter++;
            m_lastAccessTime = GetTickCount64();
            m_accessHistory.push_back(address);
            if (m_accessHistory.size() > 100) {
                m_accessHistory.erase(m_accessHistory.begin());
            }
            
            LeaveCriticalSection(&m_csLock);
            
            return success ? value : T{};
        }
        
        template<typename T>
        bool WriteMemory(uint64_t address, const T& value) {
            if (!IsValidAddress(address)) {
                return false;
            }
            
            EnterCriticalSection(&m_csLock);
            
            SimulateNormalAccess(address);
            RandomizeAccessPattern();
            
            bool success = false;
            
            if (m_activeDevice && m_activeDevice->mappedMemory) {
                // DMA-based memory write
                uint64_t offset = address - m_activeDevice->processBase;
                if (offset < m_activeDevice->memSize) {
                    __try {
                        *reinterpret_cast<T*>((uint64_t)m_activeDevice->mappedMemory + offset) = value;
                        success = true;
                    } __except(EXCEPTION_EXECUTE_HANDLER) {
                        success = false;
                    }
                }
            } else {
                // Fallback to Windows API
                HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, m_targetPid);
                if (hProcess) {
                    SIZE_T bytesWritten = 0;
                    success = WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), &bytesWritten) && 
                             bytesWritten == sizeof(T);
                    CloseHandle(hProcess);
                }
            }
            
            LeaveCriticalSection(&m_csLock);
            return success;
        }
        
        // Advanced memory operations
        uint64_t GetModuleBase(const std::string& moduleName);
        std::vector<uint64_t> ScanForPattern(const std::string& pattern, const std::string& mask);
        uint64_t FollowPointers(uint64_t base, const std::vector<uint64_t>& offsets);
        
        // Game-specific functions
        std::vector<GameEntity> GetGameEntities();
        WeaponInfo GetCurrentWeapon();
        Vec3 GetPlayerPosition();
        Vec3 GetPlayerViewAngles();
        
        // Memory region management
        bool GetMemoryRegion(uint64_t address, MemRegion& region);
        void UpdateMemoryRegions();
        void ClearMemoryCache();
        
        // DMA device management
        int GetDeviceCount() const;
        bool SetActiveDevice(uint32_t deviceId);
        DMADevice* GetActiveDevice() const { return m_activeDevice; }
        
        // Statistics and monitoring
        uint32_t GetAccessCount() const { return m_accessCounter; }
        uint64_t GetLastAccessTime() const { return m_lastAccessTime; }
        const std::vector<uint64_t>& GetAccessHistory() const { return m_accessHistory; }
        
        // Security features
        void EnableStealthMode();
        void DisableStealthMode();
        bool IsStealthModeEnabled() const;
        void FlushAccessHistory();
    };
    
} // namespace GameEnhance

#pragma once
#include "Common.h"
#include <windows.h>
#include <vector>
#include <memory>

namespace Hardware {
    
    // DMA device structure for hardware abstraction
    struct DMADevice {
        HANDLE deviceHandle;
        PVOID mappedMemory;
        SIZE_T memorySize;
        DWORD deviceId;
        bool isActive;
    };

    // Memory region descriptor for DMA operations
    struct MemoryRegion {
        UINT64 baseAddress;
        SIZE_T size;
        DWORD protection;
        bool isReadable;
        bool isWritable;
        std::string regionName;
    };

    class CDMAController {
    private:
        std::vector<DMADevice> m_devices;
        DMADevice* m_activeDevice;
        CRITICAL_SECTION m_csLock;
        
        // Hardware-specific offsets and configurations
        UINT64 m_processBase;
        UINT64 m_moduleBase;
        DWORD m_targetPID;
        
        // Cache for frequently accessed memory regions
        std::vector<MemoryRegion> m_cachedRegions;
        bool m_regionsCached;
        
        // Internal helper functions
        bool InitializeDevice(DMADevice& device);
        bool MapPhysicalMemory(DMADevice& device);
        void ScanMemoryRegions();
        bool ValidateAddress(UINT64 address);
        void FlushCache();
        
        // Anti-detection measures
        void RandomizeAccessPattern();
        void SimulateLegitimateAccess(UINT64 address);
        DWORD GenerateRandomDelay();

    public:
        CDMAController();
        ~CDMAController();
        
        // Device management
        bool Initialize();
        void Shutdown();
        bool SetActiveDevice(DWORD deviceId);
        DWORD GetDeviceCount() const;
        
        // Memory operations with DMA
        template<typename T>
        T ReadMemory(UINT64 address) {
            if (!m_activeDevice || !ValidateAddress(address)) {
                return T{};
            }
            
            EnterCriticalSection(&m_csLock);
            
            // Simulate legitimate access patterns
            SimulateLegitimateAccess(address);
            RandomizeAccessPattern();
            
            T value = {};
            if (m_activeDevice->mappedMemory) {
                // Direct access without SEH for x64 compatibility
                UINT64 offset = address - m_processBase;
                if (offset < m_activeDevice->memorySize) {
                    value = *reinterpret_cast<T*>((UINT64)m_activeDevice->mappedMemory + offset);
                }
            }
            
            LeaveCriticalSection(&m_csLock);
            return value;
        }
        
        template<typename T>
        bool WriteMemory(UINT64 address, const T& value) {
            if (!m_activeDevice || !ValidateAddress(address)) {
                return false;
            }
            
            EnterCriticalSection(&m_csLock);
            
            SimulateLegitimateAccess(address);
            RandomizeAccessPattern();
            
            bool success = false;
            if (m_activeDevice->mappedMemory) {
                // Direct access without SEH for x64 compatibility
                UINT64 offset = address - m_processBase;
                if (offset < m_activeDevice->memorySize) {
                    *reinterpret_cast<T*>((UINT64)m_activeDevice->mappedMemory + offset) = value;
                    success = true;
                }
            }
            
            LeaveCriticalSection(&m_csLock);
            return success;
        }
        
        // Advanced memory operations
        std::vector<UINT64> ScanPattern(const std::string& pattern, const std::string& mask);
        UINT64 FollowPointerChain(UINT64 base, const std::vector<UINT64>& offsets);
        bool GetMemoryRegion(UINT64 address, MemoryRegion& region);
        
        // Process management
        bool AttachToProcess(DWORD pid);
        void DetachFromProcess();
        UINT64 GetModuleBase(const std::string& moduleName);
        
        // Performance optimization
        void OptimizeAccessPattern();
        void WarmupCache();
        
        // Security features
        void EnableStealthMode();
        void DisableStealthMode();
        bool IsStealthModeEnabled() const;
    };

} // namespace Hardware

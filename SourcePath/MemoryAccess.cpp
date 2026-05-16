/*
 * Memory Access Implementation
 * Low-level memory operations with DMA support and anti-detection
 * Author: Anonymous
 * Last Modified: 2026
 */

#include "MemoryAccess.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>

// Define custom memory protection constants
#define PAGE_READABLE (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY)
#define PAGE_WRITEABLE (PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_READWRITE | PAGE_WRITECOPY)

namespace GameEnhance {
    
    CMemoryAccess::CMemoryAccess() 
        : m_activeDevice(nullptr), m_targetPid(0), m_processBase(0), 
          m_regionsCached(false), m_accessCounter(0), m_lastAccessTime(0) {
        InitializeCriticalSection(&m_csLock);
        
        // Initialize DMA device list
        m_dmaDevices.reserve(4);
        for (uint32_t i = 0; i < 4; ++i) {
            DMADevice device = {};
            device.deviceId = i;
            device.isActive = false;
            device.deviceHandle = INVALID_HANDLE_VALUE;
            m_dmaDevices.push_back(device);
        }
    }
    
    CMemoryAccess::~CMemoryAccess() {
        Shutdown();
        DeleteCriticalSection(&m_csLock);
    }
    
    bool CMemoryAccess::Initialize(const std::string& processName) {
        m_processName = processName;
        
        // Find DMA devices
        if (!FindDMADevices()) {
            // Fall back to software mode if no DMA devices found
            // This is normal for development/testing
        }
        
        // Set first active device as default
        if (!m_dmaDevices.empty() && m_dmaDevices[0].isActive) {
            m_activeDevice = &m_dmaDevices[0];
        }
        
        // Find target process
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        bool found = false;
        if (Process32First(hSnapshot, &pe32)) {
            do {
                std::wstring wProcessName(pe32.szExeFile);
                std::string currentName(wProcessName.begin(), wProcessName.end());
                
                if (currentName == processName) {
                    m_targetPid = pe32.th32ProcessID;
                    found = true;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        
        CloseHandle(hSnapshot);
        
        if (!found) {
            return false;
        }
        
        // Attach to process
        if (!AttachToProcess(m_targetPid)) {
            return false;
        }
        
        // Scan memory regions
        ScanMemoryRegions();
        
        return true;
    }
    
    void CMemoryAccess::Shutdown() {
        DetachFromProcess();
        
        EnterCriticalSection(&m_csLock);
        
        // Cleanup DMA devices
        for (auto& device : m_dmaDevices) {
            if (device.deviceHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(device.deviceHandle);
                device.deviceHandle = INVALID_HANDLE_VALUE;
            }
            
            if (device.mappedMemory) {
                VirtualFree(device.mappedMemory, 0, MEM_RELEASE);
                device.mappedMemory = nullptr;
            }
            
            device.isActive = false;
        }
        
        m_activeDevice = nullptr;
        m_cachedRegions.clear();
        m_regionsCached = false;
        
        LeaveCriticalSection(&m_csLock);
    }
    
    bool CMemoryAccess::IsInitialized() const {
        return m_targetPid != 0 && (m_activeDevice != nullptr || m_targetPid != 0);
    }
    
    bool CMemoryAccess::AttachToProcess(uint32_t pid) {
        m_targetPid = pid;
        
        // Get process base address
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            HMODULE hMods[1024];
            DWORD cbNeeded;
            
            if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
                MODULEINFO modInfo;
                if (GetModuleInformation(hProcess, hMods[0], &modInfo, sizeof(modInfo))) {
                    m_processBase = (uint64_t)modInfo.lpBaseOfDll;
                }
            }
            
            CloseHandle(hProcess);
        }
        
        // Initialize DMA device for this process
        if (m_activeDevice) {
            m_activeDevice->processBase = m_processBase;
            if (!MapProcessMemory(*m_activeDevice)) {
                return false;
            }
        }
        
        return m_processBase != 0;
    }
    
    void CMemoryAccess::DetachFromProcess() {
        m_targetPid = 0;
        m_processBase = 0;
        m_cachedRegions.clear();
        m_regionsCached = false;
    }
    
    bool CMemoryAccess::IsProcessValid() const {
        if (m_targetPid == 0) return false;
        
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, m_targetPid);
        if (hProcess) {
            DWORD exitCode;
            bool valid = GetExitCodeProcess(hProcess, &exitCode) && exitCode == STILL_ACTIVE;
            CloseHandle(hProcess);
            return valid;
        }
        
        return false;
    }
    
    bool CMemoryAccess::FindDMADevices() {
        // Simulate DMA device discovery
        // In real implementation, this would scan for actual DMA hardware
        
        for (auto& device : m_dmaDevices) {
            if (InitializeDMADevice(device)) {
                device.isActive = true;
            }
        }
        
        return std::any_of(m_dmaDevices.begin(), m_dmaDevices.end(), 
                          [](const DMADevice& d) { return d.isActive; });
    }
    
    bool CMemoryAccess::InitializeDMADevice(DMADevice& device) {
        // Try to open physical memory handle
        device.deviceHandle = CreateFileA(
            "\\\\.\\PhysicalMemory",
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );
        
        if (device.deviceHandle == INVALID_HANDLE_VALUE) {
            // Simulate DMA device for development
            device.memSize = 0x20000000; // 512MB simulated
            device.mappedMemory = VirtualAlloc(nullptr, device.memSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            return device.mappedMemory != nullptr;
        }
        
        return MapProcessMemory(device);
    }
    
    bool CMemoryAccess::MapProcessMemory(DMADevice& device) {
        // Map physical memory through DMA
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(device.deviceHandle, nullptr, &mbi, sizeof(mbi))) {
            device.memSize = mbi.RegionSize;
            device.mappedMemory = VirtualAlloc(nullptr, device.memSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            return device.mappedMemory != nullptr;
        }
        
        return false;
    }
    
    void CMemoryAccess::ScanMemoryRegions() {
        if (m_regionsCached) return;
        
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_targetPid);
        if (!hProcess) return;
        
        uint64_t currentAddr = 0;
        MEMORY_BASIC_INFORMATION mbi;
        
        while (VirtualQueryEx(hProcess, (LPCVOID)currentAddr, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_READABLE)) {
                MemRegion region = {};
                region.baseAddr = (uint64_t)mbi.BaseAddress;
                region.regionSize = mbi.RegionSize;
                region.protection = mbi.Protect;
                region.canRead = (mbi.Protect & PAGE_READABLE) != 0;
                region.canWrite = (mbi.Protect & PAGE_WRITEABLE) != 0;
                region.regionName = "Region_" + std::to_string(m_cachedRegions.size());
                
                m_cachedRegions.push_back(region);
            }
            
            currentAddr += mbi.RegionSize;
            if (currentAddr >= 0x7FFFFFFF) break; // Avoid infinite loop
        }
        
        CloseHandle(hProcess);
        m_regionsCached = true;
    }
    
    bool CMemoryAccess::IsValidAddress(uint64_t address) {
        if (!m_regionsCached) {
            ScanMemoryRegions();
        }
        
        for (const auto& region : m_cachedRegions) {
            if (address >= region.baseAddr && address < region.baseAddr + region.regionSize) {
                return region.canRead;
            }
        }
        
        return false;
    }
    
    void CMemoryAccess::SimulateNormalAccess(uint64_t address) {
        // Simulate legitimate memory access patterns
        static uint64_t lastAddr = 0;
        static int accessCount = 0;
        
        if (address != lastAddr) {
            accessCount++;
            
            // Occasionally read nearby addresses to confuse heuristics
            if (accessCount % 7 == 0) {
                ReadMemory<uint32_t>(address - 4);
                ReadMemory<uint32_t>(address + 4);
            }
            
            // Reset counter occasionally
            if (accessCount > 50) {
                accessCount = 0;
            }
            
            lastAddr = address;
        }
    }
    
    void CMemoryAccess::RandomizeAccessPattern() {
        // Add random delays to simulate human behavior
        uint32_t delay = GetRandomDelay();
        if (delay > 0) {
            Sleep(delay);
        }
    }
    
    uint32_t CMemoryAccess::GetRandomDelay() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<uint32_t> dis(0, 50);
        
        return dis(gen);
    }
    
    uint64_t CMemoryAccess::GetModuleBase(const std::string& moduleName) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_targetPid);
        if (!hProcess) return 0;
        
        HMODULE hMods[1024];
        DWORD cbNeeded;
        
        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
            for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
                char szModName[MAX_PATH];
                if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName))) {
                    std::string modName(szModName);
                    if (modName.find(moduleName) != std::string::npos) {
                        MODULEINFO modInfo;
                        if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo))) {
                            CloseHandle(hProcess);
                            return (uint64_t)modInfo.lpBaseOfDll;
                        }
                    }
                }
            }
        }
        
        CloseHandle(hProcess);
        return 0;
    }
    
    std::vector<uint64_t> CMemoryAccess::ScanForPattern(const std::string& pattern, const std::string& mask) {
        std::vector<uint64_t> results;
        
        if (!m_regionsCached) {
            ScanMemoryRegions();
        }
        
        for (const auto& region : m_cachedRegions) {
            if (!region.canRead) continue;
            
            uint64_t startAddr = region.baseAddr;
            uint64_t endAddr = region.baseAddr + region.regionSize - pattern.length();
            
            for (uint64_t addr = startAddr; addr < endAddr; ++addr) {
                bool found = true;
                
                for (size_t i = 0; i < pattern.length(); ++i) {
                    if (mask[i] != '?' && ReadMemory<uint8_t>(addr + i) != (uint8_t)pattern[i]) {
                        found = false;
                        break;
                    }
                }
                
                if (found) {
                    results.push_back(addr);
                }
            }
        }
        
        return results;
    }
    
    uint64_t CMemoryAccess::FollowPointers(uint64_t base, const std::vector<uint64_t>& offsets) {
        uint64_t current = base;
        
        for (uint64_t offset : offsets) {
            current = ReadMemory<uint64_t>(current);
            if (current == 0) {
                return 0;
            }
            current += offset;
        }
        
        return current;
    }
    
    std::vector<GameEntity> CMemoryAccess::GetGameEntities() {
        std::vector<GameEntity> entities;
        
        // This would be implemented based on the specific game's memory structure
        // For now, return empty vector as placeholder
        
        // Example implementation (would need game-specific addresses):
        // uint64_t entityList = ReadMemory<uint64_t>(m_processBase + ENTITY_LIST_OFFSET);
        // int entityCount = ReadMemory<int>(m_processBase + ENTITY_COUNT_OFFSET);
        // 
        // for (int i = 0; i < entityCount; ++i) {
        //     uint64_t entityPtr = ReadMemory<uint64_t>(entityList + i * 8);
        //     if (entityPtr) {
        //         GameEntity entity;
        //         entity.position = ReadMemory<Vec3>(entityPtr + POSITION_OFFSET);
        //         entity.type = (EntityType)ReadMemory<int>(entityPtr + TYPE_OFFSET);
        //         entity.health = ReadMemory<int>(entityPtr + HEALTH_OFFSET);
        //         entity.distance = entity.position.GetDistance(GetPlayerPosition());
        //         entities.push_back(entity);
        //     }
        // }
        
        return entities;
    }
    
    WeaponInfo CMemoryAccess::GetCurrentWeapon() {
        WeaponInfo weapon;
        
        // This would be implemented based on the specific game's memory structure
        // For now, return empty weapon info as placeholder
        
        return weapon;
    }
    
    Vec3 CMemoryAccess::GetPlayerPosition() {
        Vec3 position;
        
        // This would be implemented based on the specific game's memory structure
        // For now, return zero position as placeholder
        
        return position;
    }
    
    Vec3 CMemoryAccess::GetPlayerViewAngles() {
        Vec3 angles;
        
        // This would be implemented based on the specific game's memory structure
        // For now, return zero angles as placeholder
        
        return angles;
    }
    
    bool CMemoryAccess::GetMemoryRegion(uint64_t address, MemRegion& region) {
        if (!m_regionsCached) {
            ScanMemoryRegions();
        }
        
        for (const auto& cachedRegion : m_cachedRegions) {
            if (address >= cachedRegion.baseAddr && address < cachedRegion.baseAddr + cachedRegion.regionSize) {
                region = cachedRegion;
                return true;
            }
        }
        
        return false;
    }
    
    void CMemoryAccess::UpdateMemoryRegions() {
        m_cachedRegions.clear();
        m_regionsCached = false;
        ScanMemoryRegions();
    }
    
    void CMemoryAccess::ClearMemoryCache() {
        m_cachedRegions.clear();
        m_regionsCached = false;
    }
    
    int CMemoryAccess::GetDeviceCount() const {
        int count = 0;
        for (const auto& device : m_dmaDevices) {
            if (device.isActive) {
                count++;
            }
        }
        return count;
    }
    
    bool CMemoryAccess::SetActiveDevice(uint32_t deviceId) {
        if (deviceId >= m_dmaDevices.size()) {
            return false;
        }
        
        EnterCriticalSection(&m_csLock);
        
        if (m_dmaDevices[deviceId].isActive) {
            m_activeDevice = &m_dmaDevices[deviceId];
            LeaveCriticalSection(&m_csLock);
            return true;
        }
        
        LeaveCriticalSection(&m_csLock);
        return false;
    }
    
    void CMemoryAccess::EnableStealthMode() {
        // Enable additional stealth measures
        // ClearAccessHistory(); // Not available on this platform
    }
    
    void CMemoryAccess::DisableStealthMode() {
        // Disable stealth mode for performance
    }
    
    bool CMemoryAccess::IsStealthModeEnabled() const {
        return true; // Always enabled for security
    }
    
    void CMemoryAccess::FlushAccessHistory() {
        EnterCriticalSection(&m_csLock);
        m_accessHistory.clear();
        m_accessCounter = 0;
        LeaveCriticalSection(&m_csLock);
    }
    
} // namespace GameEnhance

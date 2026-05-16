
#pragma once
#include <windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>

namespace DMA {

    enum class DMADeviceType {
        PCILeech = 0,
        Screamer = 1,
        LeetDMA = 2,
        CustomFPGA = 3,
        Emulated = 99
    };

    enum class DMASpeed {
        USB2 = 0,
        USB3 = 1,
        PCIe_Gen2 = 2,
        PCIe_Gen3 = 3
    };

    struct DMAConfig {
        DMADeviceType deviceType = DMADeviceType::PCILeech;
        DMASpeed speed = DMASpeed::USB3;
        uint32_t maxTransferSize = 0x1000;
        uint32_t scatterCount = 16;
        bool useScatterReads = true;
        bool validateReads = true;
        uint32_t retryCount = 3;
        uint32_t retryDelayMs = 1;
    };

    struct ScatterEntry {
        uint64_t virtualAddress;
        uint64_t physicalAddress;
        uint32_t size;
        void* buffer;
        bool success;
    };

    class DMAEngine {
    private:
        DMAConfig m_config;
        HANDLE m_deviceHandle;
        bool m_initialized;
        uint64_t m_targetCR3;
        uint64_t m_targetProcessBase;
        uint32_t m_targetPID;
        std::mutex m_mutex;

        bool OpenDevice();
        void CloseDevice();
        bool SendCommand(uint32_t cmd, void* inData, uint32_t inSize, void* outData, uint32_t outSize);
        bool ReadPhysicalBlock(uint64_t physAddr, void* buffer, uint32_t size);
        bool WritePhysicalBlock(uint64_t physAddr, const void* buffer, uint32_t size);
        bool TranslateVirtToPhys(uint64_t virtAddr, uint64_t cr3, uint64_t& physAddr);

    public:
        DMAEngine();
        ~DMAEngine();

        bool Initialize(const DMAConfig& config);
        void Shutdown();
        bool IsInitialized() const { return m_initialized; }

        bool SetTargetProcess(uint32_t pid, uint64_t cr3, uint64_t processBase);
        uint64_t GetTargetCR3() const { return m_targetCR3; }
        uint64_t GetTargetProcessBase() const { return m_targetProcessBase; }
        uint32_t GetTargetPID() const { return m_targetPID; }

        bool ReadPhysical(uint64_t physAddr, void* buffer, uint32_t size);
        bool WritePhysical(uint64_t physAddr, const void* buffer, uint32_t size);

        bool ScatterRead(const std::vector<ScatterEntry>& entries);
        bool ScatterReadVirtual(const std::vector<ScatterEntry>& entries, uint64_t cr3);

        bool ReadVirtual(uint64_t virtAddr, void* buffer, uint32_t size, uint64_t cr3);
        bool WriteVirtual(uint64_t virtAddr, const void* buffer, uint32_t size, uint64_t cr3);

        template<typename T>
        T ReadVirtual(uint64_t virtAddr, uint64_t cr3) {
            T value{};
            ReadVirtual(virtAddr, &value, sizeof(T), cr3);
            return value;
        }

        template<typename T>
        bool WriteVirtual(uint64_t virtAddr, const T& value, uint64_t cr3) {
            return WriteVirtual(virtAddr, &value, sizeof(T), cr3);
        }

        const DMAConfig& GetConfig() const { return m_config; }
    };

}

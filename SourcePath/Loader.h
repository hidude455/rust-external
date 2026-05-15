#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <random>

namespace Loader {

    enum class InjectionMethod {
        LoadLibrary = 0,
        ManualMap = 1,
        ThreadHijack = 2,
        APCInjection = 3,
        ProcessHollow = 4,
        KernelDriver = 5
    };

    enum class LoaderStatus {
        Idle = 0,
        Searching = 1,
        Found = 2,
        Injecting = 3,
        Injected = 4,
        Failed = 5,
        Waiting = 6
    };

    struct ProcessInfo {
        uint32_t pid;
        std::string name;
        std::string windowTitle;
        uint64_t baseAddress;
        uint64_t size;
        bool is64Bit;
        bool isRunning;
    };

    struct InjectionConfig {
        InjectionMethod method = InjectionMethod::LoadLibrary;
        std::string dllPath;
        std::string processName = "RustClient.exe";
        std::string windowTitle = "Rust";
        bool autoInject = true;
        bool stealthMode = true;
        bool hideModule = true;
        bool erasePEHeaders = true;
        bool randomizeInjection = true;
        uint32_t injectDelayMs = 2000;
        uint32_t retryCount = 3;
        uint32_t retryDelayMs = 1000;
    };

    struct InjectionResult {
        bool success;
        InjectionMethod methodUsed;
        uint32_t targetPID;
        uint64_t injectedBase;
        std::string errorMessage;
        std::chrono::system_clock::time_point timestamp;
    };

    using StatusCallback = std::function<void(LoaderStatus, const std::string&)>;

    class CLoader {
    private:
        InjectionConfig m_config;
        LoaderStatus m_status;
        StatusCallback m_statusCallback;
        std::vector<ProcessInfo> m_foundProcesses;
        std::vector<InjectionResult> m_injectionHistory;

        std::mt19937 m_rng;
        bool m_running;

        ProcessInfo FindProcess(const std::string& name, const std::string& windowTitle);
        std::vector<ProcessInfo> FindAllProcesses(const std::string& name);
        bool IsProcessValid(const ProcessInfo& info);
        uint64_t GetProcessBaseAddress(uint32_t pid);

        bool InjectLoadLibrary(const ProcessInfo& target);
        bool InjectManualMap(const ProcessInfo& target);
        bool InjectThreadHijack(const ProcessInfo& target);
        bool InjectAPC(const ProcessInfo& target);
        bool InjectProcessHollow(const ProcessInfo& target);
        bool InjectKernelDriver(const ProcessInfo& target);

        bool AllocateAndWriteDLLPath(HANDLE hProcess, const std::string& dllPath, void*& remotePath);
        HANDLE OpenTargetProcess(uint32_t pid, bool forInjection);
        void CleanupInjection(HANDLE hProcess, HANDLE hThread, void* remoteMem);

        void RandomDelay(uint32_t minMs, uint32_t maxMs);
        void SetStatus(LoaderStatus status, const std::string& message = "");

        void ErasePEHeaders(HANDLE hProcess, uint64_t moduleBase);
        void HideDLLModule(HANDLE hProcess, const std::string& dllName);

    public:
        CLoader();
        ~CLoader();

        void SetConfig(const InjectionConfig& config) { m_config = config; }
        const InjectionConfig& GetConfig() const { return m_config; }

        void SetStatusCallback(StatusCallback callback) { m_statusCallback = callback; }

        bool Initialize();
        void Shutdown();

        ProcessInfo ScanForProcess();
        bool IsProcessFound() const;

        InjectionResult Inject();
        bool Eject(uint32_t pid, const std::string& dllPath);

        LoaderStatus GetStatus() const { return m_status; }
        const std::vector<ProcessInfo>& GetFoundProcesses() const { return m_foundProcesses; }
        const std::vector<InjectionResult>& GetInjectionHistory() const { return m_injectionHistory; }

        void RunAutoInject();
        void StopAutoInject();
    };

}

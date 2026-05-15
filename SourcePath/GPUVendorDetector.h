/*
 * GPU Vendor Detector for Rust Anti-Cheat Evasion System
 * Provides GPU vendor detection and hardware-specific optimizations
 * Author: Anonymous
 * Last Modified: 2026
 */

#pragma once
#include "Core.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3d12.h>
#include <winternl.h>

namespace Hardware {
    
    // GPU vendor enumeration
    enum class GPUVendor {
        Unknown,
        NVIDIA,
        AMD,
        Intel,
        Qualcomm,
        ARM,
        Microsoft,
        VMware,
        VirtualBox
    };
    
    // GPU architecture
    enum class GPUArchitecture {
        Unknown,
        Fermi,
        Kepler,
        Maxwell,
        Pascal,
        Volta,
        Turing,
        Ampere,
        Ada,
        RDNA1,
        RDNA2,
        RDNA3,
        Vega,
        GCN,
        Gen7,
        Gen8,
        Gen9,
        Gen11,
        Gen12,
        Gen12LP,
        Xe
    };
    
    // GPU information structure
    struct GPUInfo {
        GPUVendor vendor;
        GPUArchitecture architecture;
        std::string name;
        std::string driverVersion;
        std::string deviceID;
        std::string vendorID;
        uint64_t dedicatedVideoMemory;
        uint64_t sharedSystemMemory;
        uint32_t featureLevel;
        bool supportsDirectX11;
        bool supportsDirectX12;
        bool supportsRayTracing;
        bool supportsVariableRateShading;
        bool supportsMeshShaders;
        bool supportsSamplerFeedback;
        std::vector<std::string> extensions;
        std::map<std::string, bool> features;
        bool isIntegrated;
        bool isDiscrete;
        bool isVirtual;
    };
    
    // GPU vendor detector
    class GPUVendorDetector {
    private:
        std::vector<GPUInfo> m_gpus;
        std::map<std::string, GPUVendor> m_vendorMap;
        std::map<std::string, GPUArchitecture> m_architectureMap;
        std::mutex m_mutex;
        bool m_initialized;
        
        // DXGI interfaces
        IDXGIFactory* m_dxgiFactory;
        std::vector<IDXGIAdapter*> m_dxgiAdapters;
        
    public:
        GPUVendorDetector() : m_dxgiFactory(nullptr), m_initialized(false) {
            InitializeVendorMaps();
            InitializeArchitectureMaps();
        }
        
        ~GPUVendorDetector() {
            Cleanup();
        }
        
        bool Initialize() {
            if (m_initialized) return true;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            
            // Initialize DXGI
            if (!InitializeDXGI()) {
                LOG_ERROR("Failed to initialize DXGI");
                return false;
            }
            
            // Detect GPUs
            if (!DetectGPUs()) {
                LOG_ERROR("Failed to detect GPUs");
                return false;
            }
            
            m_initialized = true;
            LOG_INFO("GPU vendor detector initialized successfully");
            
            return true;
        }
        
        void Cleanup() {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            // Release DXGI adapters
            for (auto* adapter : m_dxgiAdapters) {
                if (adapter) {
                    adapter->Release();
                }
            }
            m_dxgiAdapters.clear();
            
            // Release DXGI factory
            if (m_dxgiFactory) {
                m_dxgiFactory->Release();
                m_dxgiFactory = nullptr;
            }
            
            m_gpus.clear();
            m_initialized = false;
        }
        
        std::vector<GPUInfo> GetGPUs() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_gpus;
        }
        
        GPUInfo GetPrimaryGPU() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (m_gpus.empty()) {
                return GPUInfo();
            }
            
            // Return the first discrete GPU, or the first GPU if none are discrete
            for (const auto& gpu : m_gpus) {
                if (gpu.isDiscrete) {
                    return gpu;
                }
            }
            
            return m_gpus[0];
        }
        
        GPUVendor GetGPUVendor(const std::string& gpuName) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            for (const auto& pair : m_vendorMap) {
                if (gpuName.find(pair.first) != std::string::npos) {
                    return pair.second;
                }
            }
            
            return GPUVendor::Unknown;
        }
        
        GPUArchitecture GetGPUArchitecture(const std::string& gpuName) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            for (const auto& pair : m_architectureMap) {
                if (gpuName.find(pair.first) != std::string::npos) {
                    return pair.second;
                }
            }
            
            return GPUArchitecture::Unknown;
        }
        
        bool IsNVidiaGPU(const GPUInfo& gpu) const {
            return gpu.vendor == GPUVendor::NVIDIA;
        }
        
        bool IsAMDGPU(const GPUInfo& gpu) const {
            return gpu.vendor == GPUVendor::AMD;
        }
        
        bool IsIntelGPU(const GPUInfo& gpu) const {
            return gpu.vendor == GPUVendor::Intel;
        }
        
        bool SupportsDirectX11(const GPUInfo& gpu) const {
            return gpu.supportsDirectX11;
        }
        
        bool SupportsDirectX12(const GPUInfo& gpu) const {
            return gpu.supportsDirectX12;
        }
        
        bool SupportsRayTracing(const GPUInfo& gpu) const {
            return gpu.supportsRayTracing;
        }
        
        bool IsInitialized() const {
            return m_initialized;
        }
        
        std::string GetVendorString(GPUVendor vendor) const {
            switch (vendor) {
                case GPUVendor::NVIDIA: return "NVIDIA";
                case GPUVendor::AMD: return "AMD";
                case GPUVendor::Intel: return "Intel";
                case GPUVendor::Qualcomm: return "Qualcomm";
                case GPUVendor::ARM: return "ARM";
                case GPUVendor::Microsoft: return "Microsoft";
                case GPUVendor::VMware: return "VMware";
                case GPUVendor::VirtualBox: return "VirtualBox";
                default: return "Unknown";
            }
        }
        
        std::string GetArchitectureString(GPUArchitecture architecture) const {
            switch (architecture) {
                case GPUArchitecture::Fermi: return "Fermi";
                case GPUArchitecture::Kepler: return "Kepler";
                case GPUArchitecture::Maxwell: return "Maxwell";
                case GPUArchitecture::Pascal: return "Pascal";
                case GPUArchitecture::Volta: return "Volta";
                case GPUArchitecture::Turing: return "Turing";
                case GPUArchitecture::Ampere: return "Ampere";
                case GPUArchitecture::Ada: return "Ada";
                case GPUArchitecture::RDNA1: return "RDNA 1";
                case GPUArchitecture::RDNA2: return "RDNA 2";
                case GPUArchitecture::RDNA3: return "RDNA 3";
                case GPUArchitecture::Vega: return "Vega";
                case GPUArchitecture::GCN: return "GCN";
                case GPUArchitecture::Gen7: return "Gen 7";
                case GPUArchitecture::Gen8: return "Gen 8";
                case GPUArchitecture::Gen9: return "Gen 9";
                case GPUArchitecture::Gen11: return "Gen 11";
                case GPUArchitecture::Gen12: return "Gen 12";
                case GPUArchitecture::Gen12LP: return "Gen 12LP";
                case GPUArchitecture::Xe: return "Xe";
                default: return "Unknown";
            }
        }
        
    private:
        void InitializeVendorMaps() {
            // NVIDIA keywords
            m_vendorMap["GeForce"] = GPUVendor::NVIDIA;
            m_vendorMap["Quadro"] = GPUVendor::NVIDIA;
            m_vendorMap["Tesla"] = GPUVendor::NVIDIA;
            m_vendorMap["RTX"] = GPUVendor::NVIDIA;
            m_vendorMap["GTX"] = GPUVendor::NVIDIA;
            m_vendorMap["GT"] = GPUVendor::NVIDIA;
            m_vendorMap["NVIDIA"] = GPUVendor::NVIDIA;
            
            // AMD keywords
            m_vendorMap["Radeon"] = GPUVendor::AMD;
            m_vendorMap["Ryzen"] = GPUVendor::AMD;
            m_vendorMap["RX"] = GPUVendor::AMD;
            m_vendorMap["HD"] = GPUVendor::AMD;
            m_vendorMap["R9"] = GPUVendor::AMD;
            m_vendorMap["R7"] = GPUVendor::AMD;
            m_vendorMap["AMD"] = GPUVendor::AMD;
            
            // Intel keywords
            m_vendorMap["Intel"] = GPUVendor::Intel;
            m_vendorMap["HD Graphics"] = GPUVendor::Intel;
            m_vendorMap["Iris"] = GPUVendor::Intel;
            m_vendorMap["UHD"] = GPUVendor::Intel;
            m_vendorMap["Xe"] = GPUVendor::Intel;
            
            // Virtual GPU keywords
            m_vendorMap["VMware"] = GPUVendor::VMware;
            m_vendorMap["VirtualBox"] = GPUVendor::VirtualBox;
            m_vendorMap["Microsoft"] = GPUVendor::Microsoft;
        }
        
        void InitializeArchitectureMaps() {
            // NVIDIA architectures
            m_architectureMap["Fermi"] = GPUArchitecture::Fermi;
            m_architectureMap["Kepler"] = GPUArchitecture::Kepler;
            m_architectureMap["Maxwell"] = GPUArchitecture::Maxwell;
            m_architectureMap["Pascal"] = GPUArchitecture::Pascal;
            m_architectureMap["Volta"] = GPUArchitecture::Volta;
            m_architectureMap["Turing"] = GPUArchitecture::Turing;
            m_architectureMap["Ampere"] = GPUArchitecture::Ampere;
            m_architectureMap["Ada"] = GPUArchitecture::Ada;
            m_architectureMap["RTX 20"] = GPUArchitecture::Turing;
            m_architectureMap["RTX 30"] = GPUArchitecture::Ampere;
            m_architectureMap["RTX 40"] = GPUArchitecture::Ada;
            m_architectureMap["GTX 10"] = GPUArchitecture::Pascal;
            m_architectureMap["GTX 9"] = GPUArchitecture::Maxwell;
            m_architectureMap["GTX 7"] = GPUArchitecture::Kepler;
            m_architectureMap["GTX 6"] = GPUArchitecture::Kepler;
            
            // AMD architectures
            m_architectureMap["RDNA"] = GPUArchitecture::RDNA1;
            m_architectureMap["RDNA 2"] = GPUArchitecture::RDNA2;
            m_architectureMap["RDNA 3"] = GPUArchitecture::RDNA3;
            m_architectureMap["RX 5000"] = GPUArchitecture::RDNA1;
            m_architectureMap["RX 6000"] = GPUArchitecture::RDNA2;
            m_architectureMap["RX 7000"] = GPUArchitecture::RDNA3;
            m_architectureMap["Vega"] = GPUArchitecture::Vega;
            m_architectureMap["RX Vega"] = GPUArchitecture::Vega;
            m_architectureMap["RX 500"] = GPUArchitecture::GCN;
            m_architectureMap["RX 400"] = GPUArchitecture::GCN;
            m_architectureMap["R9"] = GPUArchitecture::GCN;
            m_architectureMap["R7"] = GPUArchitecture::GCN;
            
            // Intel architectures
            m_architectureMap["Gen 7"] = GPUArchitecture::Gen7;
            m_architectureMap["Gen 8"] = GPUArchitecture::Gen8;
            m_architectureMap["Gen 9"] = GPUArchitecture::Gen9;
            m_architectureMap["Gen 11"] = GPUArchitecture::Gen11;
            m_architectureMap["Gen 12"] = GPUArchitecture::Gen12;
            m_architectureMap["Gen12LP"] = GPUArchitecture::Gen12LP;
            m_architectureMap["Xe"] = GPUArchitecture::Xe;
        }
        
        bool InitializeDXGI() {
            HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&m_dxgiFactory);
            if (FAILED(hr)) {
                LOG_ERROR("Failed to create DXGI factory");
                return false;
            }
            
            return true;
        }
        
        bool DetectGPUs() {
            if (!m_dxgiFactory) {
                return false;
            }
            
            // Enumerate adapters
            UINT adapterIndex = 0;
            IDXGIAdapter* adapter = nullptr;
            
            while (m_dxgiFactory->EnumAdapters(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND) {
                if (adapter) {
                    DXGI_ADAPTER_DESC desc;
                    if (SUCCEEDED(adapter->GetDesc(&desc))) {
                        GPUInfo gpuInfo = CreateGPUInfo(adapter, desc);
                        m_gpus.push_back(gpuInfo);
                        m_dxgiAdapters.push_back(adapter);
                    } else {
                        adapter->Release();
                    }
                    adapterIndex++;
                }
            }
            
            return !m_gpus.empty();
        }
        
        GPUInfo CreateGPUInfo(IDXGIAdapter* adapter, const DXGI_ADAPTER_DESC& desc) {
            GPUInfo gpuInfo;
            
            // Basic information
            gpuInfo.name = std::wstring(desc.Description, desc.Description + wcslen(desc.Description));
            gpuInfo.vendorID = std::to_string(desc.VendorId);
            gpuInfo.deviceID = std::to_string(desc.DeviceId);
            gpuInfo.dedicatedVideoMemory = desc.DedicatedVideoMemory;
            gpuInfo.sharedSystemMemory = desc.SharedSystemMemory;
            
            // Detect vendor
            gpuInfo.vendor = GetGPUVendor(gpuInfo.name);
            
            // Detect architecture
            gpuInfo.architecture = GetGPUArchitecture(gpuInfo.name);
            
            // Determine if integrated or discrete
            gpuInfo.isIntegrated = (desc.DedicatedVideoMemory < 512 * 1024 * 1024); // Less than 512MB
            gpuInfo.isDiscrete = !gpuInfo.isIntegrated;
            
            // Check if virtual
            gpuInfo.isVirtual = (gpuInfo.vendor == GPUVendor::VMware || 
                              gpuInfo.vendor == GPUVendor::VirtualBox ||
                              gpuInfo.vendor == GPUVendor::Microsoft);
            
            // Test DirectX support
            TestDirectXSupport(adapter, gpuInfo);
            
            // Get driver version
            GetDriverVersion(adapter, gpuInfo);
            
            // Detect features
            DetectGPUFeatures(adapter, gpuInfo);
            
            return gpuInfo;
        }
        
        void TestDirectXSupport(IDXGIAdapter* adapter, GPUInfo& gpuInfo) {
            // Test DirectX 11
            D3D_FEATURE_LEVEL featureLevels[] = {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0
            };
            
            UINT createDeviceFlags = 0;
#ifdef _DEBUG
            createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
            
            D3D_FEATURE_LEVEL featureLevel;
            ID3D11Device* device = nullptr;
            
            HRESULT hr = D3D11CreateDevice(
                adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags,
                featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
                &device, &featureLevel, nullptr);
            
            if (SUCCEEDED(hr) && device) {
                gpuInfo.supportsDirectX11 = true;
                gpuInfo.featureLevel = featureLevel;
                device->Release();
                
                // Test DirectX 12
                TestDirectX12Support(adapter, gpuInfo);
            } else {
                gpuInfo.supportsDirectX11 = false;
                gpuInfo.supportsDirectX12 = false;
            }
        }
        
        void TestDirectX12Support(IDXGIAdapter* adapter, GPUInfo& gpuInfo) {
            // Simplified DirectX 12 test
            // In a real implementation, this would test D3D12CreateDevice
            gpuInfo.supportsDirectX12 = false;
            
            // Check for DirectX 12 Ultimate features
            if (gpuInfo.supportsDirectX12) {
                // Test ray tracing support
                gpuInfo.supportsRayTracing = TestRayTracingSupport(adapter);
                
                // Test variable rate shading
                gpuInfo.supportsVariableRateShading = TestVariableRateShadingSupport(adapter);
                
                // Test mesh shaders
                gpuInfo.supportsMeshShaders = TestMeshShadersSupport(adapter);
                
                // Test sampler feedback
                gpuInfo.supportsSamplerFeedback = TestSamplerFeedbackSupport(adapter);
            }
        }
        
        bool TestRayTracingSupport(IDXGIAdapter* adapter) {
            // Simplified ray tracing test
            // In a real implementation, this would check for DXR support
            return (gpuInfo.vendor == GPUVendor::NVIDIA && gpuInfo.architecture >= GPUArchitecture::Turing) ||
                   (gpuInfo.vendor == GPUVendor::AMD && gpuInfo.architecture >= GPUArchitecture::RDNA2);
        }
        
        bool TestVariableRateShadingSupport(IDXGIAdapter* adapter) {
            // Simplified VRS test
            return (gpuInfo.vendor == GPUVendor::NVIDIA && gpuInfo.architecture >= GPUArchitecture::Turing) ||
                   (gpuInfo.vendor == GPUVendor::AMD && gpuInfo.architecture >= GPUArchitecture::RDNA2);
        }
        
        bool TestMeshShadersSupport(IDXGIAdapter* adapter) {
            // Simplified mesh shaders test
            return (gpuInfo.vendor == GPUVendor::NVIDIA && gpuInfo.architecture >= GPUArchitecture::Turing) ||
                   (gpuInfo.vendor == GPUVendor::AMD && gpuInfo.architecture >= GPUArchitecture::RDNA2);
        }
        
        bool TestSamplerFeedbackSupport(IDXGIAdapter* adapter) {
            // Simplified sampler feedback test
            return (gpuInfo.vendor == GPUVendor::NVIDIA && gpuInfo.architecture >= GPUArchitecture::Turing) ||
                   (gpuInfo.vendor == GPUVendor::Intel && gpuInfo.architecture >= GPUArchitecture::Gen12);
        }
        
        void GetDriverVersion(IDXGIAdapter* adapter, GPUInfo& gpuInfo) {
            // Get driver version from adapter
            LARGE_INTEGER driverVersion;
            if (SUCCEEDED(adapter->CheckInterfaceSupport(__uuidof(IDXGIAdapter), &driverVersion))) {
                // Convert driver version to string
                uint32_t major = (driverVersion.HighPart >> 16) & 0xFFFF;
                uint32_t minor = driverVersion.HighPart & 0xFFFF;
                uint32_t build = driverVersion.LowPart;
                
                gpuInfo.driverVersion = std::to_string(major) + "." + 
                                       std::to_string(minor) + "." + 
                                       std::to_string(build);
            } else {
                gpuInfo.driverVersion = "Unknown";
            }
        }
        
        void DetectGPUFeatures(IDXGIAdapter* adapter, GPUInfo& gpuInfo) {
            // Common features
            gpuInfo.features["DirectX11"] = gpuInfo.supportsDirectX11;
            gpuInfo.features["DirectX12"] = gpuInfo.supportsDirectX12;
            gpuInfo.features["RayTracing"] = gpuInfo.supportsRayTracing;
            gpuInfo.features["VariableRateShading"] = gpuInfo.supportsVariableRateShading;
            gpuInfo.features["MeshShaders"] = gpuInfo.supportsMeshShaders;
            gpuInfo.features["SamplerFeedback"] = gpuInfo.supportsSamplerFeedback;
            
            // Vendor-specific features
            if (gpuInfo.vendor == GPUVendor::NVIDIA) {
                gpuInfo.features["NVAPI"] = true;
                gpuInfo.features["CUDA"] = true;
                gpuInfo.features["PhysX"] = true;
                gpuInfo.features["G-Sync"] = true;
                
                if (gpuInfo.architecture >= GPUArchitecture::Pascal) {
                    gpuInfo.features["Ansel"] = true;
                }
                
                if (gpuInfo.architecture >= GPUArchitecture::Turing) {
                    gpuInfo.features["DLSS"] = true;
                    gpuInfo.features["NVIDIA Reflex"] = true;
                }
                
                if (gpuInfo.architecture >= GPUArchitecture::Ampere) {
                    gpuInfo.features["RTX IO"] = true;
                }
            } else if (gpuInfo.vendor == GPUVendor::AMD) {
                gpuInfo.features["AMDGPU"] = true;
                gpuInfo.features["OpenCL"] = true;
                gpuInfo.features["FreeSync"] = true;
                
                if (gpuInfo.architecture >= GPUArchitecture::RDNA2) {
                    gpuInfo.features["FidelityFX Super Resolution"] = true;
                    gpuInfo.features["AMD Smart Access Memory"] = true;
                }
                
                if (gpuInfo.architecture >= GPUArchitecture::RDNA3) {
                    gpuInfo.features["FSR 3.0"] = true;
                }
            } else if (gpuInfo.vendor == GPUVendor::Intel) {
                gpuInfo.features["OpenCL"] = true;
                gpuInfo.features["Intel Graphics"] = true;
                
                if (gpuInfo.architecture >= GPUArchitecture::Gen12) {
                    gpuInfo.features["XeSS"] = true;
                    gpuInfo.features["Deep Link"] = true;
                }
            }
            
            // Memory-based features
            if (gpuInfo.dedicatedVideoMemory >= 4 * 1024 * 1024 * 1024) { // 4GB+
                gpuInfo.features["HighMemory"] = true;
            }
            
            if (gpuInfo.dedicatedVideoMemory >= 8 * 1024 * 1024 * 1024) { // 8GB+
                gpuInfo.features["UltraHighMemory"] = true;
            }
        }
    };
    
} // namespace Hardware

#pragma once
#include <windows.h>
#include <d3d11.h>
#include <string>
#include <memory>
#include <vector>
#include "Injector.h"
#include "KernelInterface.h"
#include "MemoryManager.h"
#include "ESP.h"
#include "Renderer.h"
#include "RendererDX.h"
#include "../MenuPath/imgui/imgui.h"

class CRustInjector {
public:
    CRustInjector();
    ~CRustInjector();

    bool Initialize(HINSTANCE hInstance);
    int Run();
    void Shutdown();

private:
    bool CreateMainWindow();
    bool InitializeDirectX();
    bool InitializeImGui();
    void CleanupDirectX();
    void CleanupImGui();
    
    void RenderFrame();
    void RenderUI();
    void RenderStatusTab();
    void RenderDLLTab();
    void RenderOptionsTab();
    void RenderESPTab();
    void RenderLogTab();
    void ApplyDarkTheme();
    void AppendLog(const std::string& message);
    
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    IDXGISwapChain* m_swapChain;
    ID3D11RenderTargetView* m_renderTarget;
    
    std::unique_ptr<CInjector> m_injector;
    std::unique_ptr<KernelInterface::CKernelInterface> m_kernelInterface;
    std::unique_ptr<MemoryManager> m_memoryManager;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<ESP> m_esp;
    bool m_running;
    bool m_initialized;
    bool m_driverLoaded;
    
    // UI State
    char m_dllPath[MAX_PATH];
    bool m_autoInject;
    bool m_manualMap;
    std::string m_statusMessage;
    std::vector<std::string> m_logMessages;
    
    // Navigation State
    int m_selectedTab;
    float m_animationTimer;
    
    // ESP Settings
    bool m_espEnabled;
    bool m_espShowCircle;
    bool m_espShowInventory;
    bool m_espShowChams;
    bool m_espGalaxyMode;
    float m_espMaxDistance;
    float m_espCircleRadius;
};

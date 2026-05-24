#pragma once
#include <windows.h>
#include <d3d11.h>
#include <string>
#include <memory>
#include "Injector.h"
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
    void ApplyPurpleTheme();
    
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    IDXGISwapChain* m_swapChain;
    ID3D11RenderTargetView* m_renderTarget;
    
    std::unique_ptr<CInjector> m_injector;
    bool m_running;
    bool m_initialized;
    
    // UI State
    char m_dllPath[MAX_PATH];
    bool m_autoInject;
    bool m_manualMap;
    std::string m_statusMessage;
    ImVector<char> m_logMessages;
};

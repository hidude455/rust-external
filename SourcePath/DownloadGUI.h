#pragma once
#include <windows.h>
#include <d3d11.h>
#include <string>
#include <memory>
#include "Downloader.h"
#include "../MenuPath/imgui/imgui.h"
#include "../MenuPath/imgui/backends/imgui_impl_win32.h"
#include "../MenuPath/imgui/backends/imgui_impl_dx11.h"

class CDownloadGUI {
public:
    CDownloadGUI();
    ~CDownloadGUI();

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
    void SetupDownloadQueue();
    bool CheckVisualStudioInstalled();
    void ShowVisualStudioWarning();
    bool CheckDirectXInstalled();
    void ExecuteInstaller(const std::string& installerPath);
    
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    IDXGISwapChain* m_swapChain;
    ID3D11RenderTargetView* m_renderTarget;
    
    std::unique_ptr<CDownloader> m_downloader;
    bool m_running;
    bool m_initialized;
    
    // UI State
    float m_totalProgress;
    std::string m_statusMessage;
    bool m_autoStart;
    bool m_downloadComplete;
    
    // Animation State
    float m_animationTime;
    float m_pulseIntensity;
    float m_rotationAngle;
    std::vector<std::string> m_logMessages;
    int m_maxLogMessages;
    
    // Installation State
    bool m_installingDependencies;
    std::vector<std::string> m_installQueue;
    bool m_isInstaller;
};

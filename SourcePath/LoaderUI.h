#pragma once
#include <windows.h>
#include <d3d11.h>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include "ExternalCheat.h"

void LoaderLog(const std::string& message);

struct LoaderLaunchResult {
    bool shouldLaunch = false;
    std::unique_ptr<App::CExternalCheat> cheat;
};

class CLoaderUI {
public:
    CLoaderUI();
    ~CLoaderUI();

    // Returns true on successful initialization; false if setup fails
    bool Initialize();

    // Runs the loader UI; returns true if the user successfully authenticated and chose to launch
    LoaderLaunchResult Run();

private:
    // Window / device setup
    bool CreateRenderWindow();
    bool InitializeDevice();
    void CleanupDevice();
    void CleanupImGui();

    void RenderFrame();
    void RenderUI();

    void LaunchCheat();

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    WNDCLASSEX m_windowClass{};
    HWND m_hwnd = nullptr;

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_renderTarget = nullptr;

    bool m_running = false;
    bool m_initialized = false;
    std::atomic<bool> m_busy{ false };
    std::atomic<bool> m_shouldLaunch{ false };

    std::atomic<float> m_progress{ 0.0f };
    std::atomic<bool> m_showProgress{ false };

    std::string m_statusMessage;
    std::string m_errorMessage;
    std::string m_progressLabel;
    std::mutex m_stateMutex;
    char m_licenseKeyBuffer[128]{};

    std::unique_ptr<App::CExternalCheat> m_cheat;
};

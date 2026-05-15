#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <string>
#include <functional>
#include <chrono>
#include <dwmapi.h>
#include "MenuManager.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwmapi.lib")

namespace Render {

    struct OverlayConfig {
        int width = 1920;
        int height = 1080;
        bool fullscreen = true;
        bool vsync = false;
        bool transparent = true;
        bool clickThrough = true;
        float targetFPS = 144.0f;
        std::string windowTitle = "Overlay";
    };

    using RenderCallback = std::function<void(ID3D11DeviceContext*, int, int)>;

    class COverlayRenderer {
    private:
        HWND m_hwnd;
        WNDCLASSEXA m_wc;

        ID3D11Device* m_device;
        ID3D11DeviceContext* m_context;
        IDXGISwapChain* m_swapChain;
        ID3D11RenderTargetView* m_renderTarget;
        ID3D11BlendState* m_blendState;

        OverlayConfig m_config;
        RenderCallback m_renderCallback;
        CMenuManager* m_menuManager;
        bool m_running;
        bool m_initialized;

        std::chrono::high_resolution_clock::time_point m_lastFrame;
        float m_fps;

        bool CreateOverlayWindow();
        bool InitializeD3D11();
        void CleanupD3D11();
        void RenderFrame();
        void UpdateFPS();

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static COverlayRenderer* s_instance;

    public:
        COverlayRenderer();
        ~COverlayRenderer();

        bool Initialize(const OverlayConfig& config);
        void Shutdown();
        void Run();
        void Stop();

        void SetRenderCallback(RenderCallback callback) { m_renderCallback = callback; }
        void SetMenuManager(CMenuManager* menu) { m_menuManager = menu; }

        HWND GetHWND() const { return m_hwnd; }
        bool IsRunning() const { return m_running; }
        float GetFPS() const { return m_fps; }
        int GetWidth() const { return m_config.width; }
        int GetHeight() const { return m_config.height; }

        ID3D11Device* GetDevice() { return m_device; }
        ID3D11DeviceContext* GetContext() { return m_context; }
    };

}

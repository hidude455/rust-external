#include "OverlayRenderer.h"

namespace Render {

    COverlayRenderer* COverlayRenderer::s_instance = nullptr;

    COverlayRenderer::COverlayRenderer()
        : m_hwnd(nullptr), m_device(nullptr), m_context(nullptr),
          m_swapChain(nullptr), m_renderTarget(nullptr), m_blendState(nullptr),
          m_menuManager(nullptr), m_running(false), m_initialized(false), m_fps(0.0f) {
        s_instance = this;
    }

    COverlayRenderer::~COverlayRenderer() {
        Shutdown();
        s_instance = nullptr;
    }

    LRESULT CALLBACK COverlayRenderer::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_KEYDOWN && wParam == VK_RSHIFT) {
            if (s_instance && s_instance->m_menuManager) {
                s_instance->m_menuManager->Toggle();
            }
            return 0;
        }
        if (msg == WM_KEYDOWN) {
            ImGui_ImplWin32_ProcessHook(hwnd, msg, wParam, lParam);
        }
        switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            if (s_instance && s_instance->m_swapChain && wParam != SIZE_MINIMIZED) {
                s_instance->CleanupD3D11();
                s_instance->InitializeD3D11();
            }
            return 0;
        case WM_ERASEBKGND:
            return 1;
        }
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }

    bool COverlayRenderer::CreateOverlayWindow() {
        m_wc = {};
        m_wc.cbSize = sizeof(WNDCLASSEXA);
        m_wc.style = CS_HREDRAW | CS_VREDRAW;
        m_wc.lpfnWndProc = WndProc;
        m_wc.hInstance = GetModuleHandleA(nullptr);
        m_wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
        m_wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
        m_wc.lpszClassName = "RustExternalOverlay";

        if (!RegisterClassExA(&m_wc)) return false;

        DWORD exStyle = WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
        DWORD style = WS_POPUP;

        if (!m_config.clickThrough) exStyle &= ~WS_EX_TRANSPARENT;

        m_hwnd = CreateWindowExA(exStyle, m_wc.lpszClassName, m_config.windowTitle.c_str(),
            style, 0, 0, m_config.width, m_config.height,
            nullptr, nullptr, m_wc.hInstance, nullptr);

        if (!m_hwnd) return false;

        SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(m_hwnd, &margins);

        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);

        return true;
    }

    bool COverlayRenderer::InitializeD3D11() {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Width = m_config.width;
        sd.BufferDesc.Height = m_config.height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = m_config.vsync ? 60 : 0;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = m_hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
        D3D_FEATURE_LEVEL selectedLevel;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &sd,
            &m_swapChain, &m_device, &selectedLevel, &m_context);

        if (FAILED(hr)) return false;

        ID3D11Texture2D* backBuffer = nullptr;
        m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        if (!backBuffer) return false;

        m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTarget);
        backBuffer->Release();

        D3D11_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        m_device->CreateBlendState(&blendDesc, &m_blendState);

        m_context->OMSetRenderTargets(1, &m_renderTarget, nullptr);
        m_context->OMSetBlendState(m_blendState, nullptr, 0xFFFFFFFF);

        D3D11_VIEWPORT vp = {};
        vp.Width = (float)m_config.width;
        vp.Height = (float)m_config.height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->RSSetViewports(1, &vp);

        return true;
    }

    void COverlayRenderer::CleanupD3D11() {
        if (m_blendState) { m_blendState->Release(); m_blendState = nullptr; }
        if (m_renderTarget) { m_renderTarget->Release(); m_renderTarget = nullptr; }
        if (m_swapChain) { m_swapChain->Release(); m_swapChain = nullptr; }
        if (m_context) { m_context->Release(); m_context = nullptr; }
        if (m_device) { m_device->Release(); m_device = nullptr; }
    }

    bool COverlayRenderer::Initialize(const OverlayConfig& config) {
        m_config = config;
        if (!CreateOverlayWindow()) return false;
        if (!InitializeD3D11()) return false;
        m_initialized = true;
        m_running = true;
        m_lastFrame = std::chrono::high_resolution_clock::now();
        return true;
    }

    void COverlayRenderer::Shutdown() {
        m_running = false;
        CleanupD3D11();
        if (m_hwnd) { DestroyWindow(m_hwnd); m_hwnd = nullptr; }
        UnregisterClassA(m_wc.lpszClassName, m_wc.hInstance);
        m_initialized = false;
    }

    void COverlayRenderer::UpdateFPS() {
        auto now = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float>(now - m_lastFrame).count();
        m_lastFrame = now;
        m_fps = elapsed > 0.0f ? 1.0f / elapsed : 0.0f;
    }

    void COverlayRenderer::RenderFrame() {
        if (!m_initialized || !m_device || !m_context) return;

        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_context->ClearRenderTargetView(m_renderTarget, clearColor);

        if (m_renderCallback) {
            m_renderCallback(m_context, m_config.width, m_config.height);
        }

        if (m_menuManager) {
            m_menuManager->Render();
        }

        m_swapChain->Present(m_config.vsync ? 1 : 0, 0);
    }

    void COverlayRenderer::Run() {
        if (!m_initialized) return;

        MSG msg = {};
        while (m_running) {
            while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
                if (msg.message == WM_QUIT) { m_running = false; break; }
            }
            if (!m_running) break;

            UpdateFPS();
            RenderFrame();

            if (m_config.targetFPS > 0) {
                float targetMs = 1000.0f / m_config.targetFPS;
                auto frameEnd = std::chrono::high_resolution_clock::now();
                float frameMs = std::chrono::duration<float, std::milli>(frameEnd - m_lastFrame).count();
                if (frameMs < targetMs) Sleep((DWORD)(targetMs - frameMs));
            }
        }
    }

    void COverlayRenderer::Stop() { m_running = false; }

}

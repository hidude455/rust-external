#include "Renderer.h"
#include "../MenuPath/imgui/imgui.h"
#include "../MenuPath/imgui/backends/imgui_impl_win32.h"
#include "../MenuPath/imgui/backends/imgui_impl_dx11.h"
#include <algorithm>

namespace MIT {
    Renderer::Renderer() : windowHandle(nullptr), device(nullptr), context(nullptr), 
                          swapChain(nullptr), renderTargetView(nullptr), imguiContext(nullptr),
                          vertexShader(nullptr), pixelShader(nullptr), inputLayout(nullptr),
                          vertexBuffer(nullptr), indexBuffer(nullptr), blendState(nullptr),
                          rasterizerState(nullptr), depthStencilState(nullptr), 
                          fontTextureView(nullptr), fontSampler(nullptr),
                          fps(0.0f), frameTime(0.0f), fpsTimer(std::chrono::high_resolution_clock::now()),
                          framesSinceLastUpdate(0) {
    }

    Renderer::~Renderer() {
        Shutdown();
    }

    bool Renderer::Initialize(HWND targetWindow) {
        windowHandle = targetWindow;
        
        if (!InitializeD3D11()) {
            LOG_ERROR("Failed to initialize D3D11");
            return false;
        }
        
        if (!InitializeImGui()) {
            LOG_ERROR("Failed to initialize ImGui");
            return false;
        }
        
        lastFrameTime = std::chrono::high_resolution_clock::now();
        fpsHistory.reserve(60);
        
        LOG_INFO("Renderer initialized successfully");
        return true;
    }

    bool Renderer::InitializeD3D11() {
        DXGI_SWAP_CHAIN_DESC swapDesc = {};
        swapDesc.BufferCount = 2;
        swapDesc.BufferDesc.Width = 0; // Use window width
        swapDesc.BufferDesc.Height = 0; // Use window height
        swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapDesc.OutputWindow = windowHandle;
        swapDesc.SampleDesc.Count = 1;
        swapDesc.SampleDesc.Quality = 0;
        swapDesc.Windowed = TRUE;
        swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        HRESULT result = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            &featureLevel, 1, D3D11_SDK_VERSION,
            &swapDesc, &swapChain, &device, nullptr, &context
        );

        if (FAILED(result)) {
            LOG_ERROR("D3D11CreateDeviceAndSwapChain failed: " + std::to_string(result));
            return false;
        }

        CreateRenderTarget();
        return true;
    }

    bool Renderer::InitializeImGui() {
        IMGUI_CHECKVERSION();
        imguiContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(static_cast<ImGuiContext*>(imguiContext));
        
        if (!ImGui_ImplWin32_Init(windowHandle)) {
            LOG_ERROR("Failed to initialize ImGui Win32 backend");
            return false;
        }
        
        if (!ImGui_ImplDX11_Init(device, context)) {
            LOG_ERROR("Failed to initialize ImGui DX11 backend");
            return false;
        }
        
        SetupImGuiStyle();
        
        return true;
    }

    void Renderer::CreateRenderTarget() {
        ID3D11Texture2D* backBuffer;
        swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
        context->OMSetRenderTargets(1, &renderTargetView, nullptr);
        backBuffer->Release();
    }

    void Renderer::CleanupRenderTarget() {
        if (renderTargetView) {
            renderTargetView->Release();
            renderTargetView = nullptr;
        }
    }

    void Renderer::SetupImGuiStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        
        // Krypton-style dark theme
        style.WindowPadding = ImVec2(8, 8);
        style.FramePadding = ImVec2(4, 4);
        style.ItemSpacing = ImVec2(8, 4);
        style.ItemInnerSpacing = ImVec2(4, 4);
        style.TouchExtraPadding = ImVec2(0, 0);
        style.IndentSpacing = 21.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 10.0f;
        
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 1.0f;
        
        style.WindowRounding = 4.0f;
        style.ChildRounding = 4.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.LogSliderDeadzone = 4.0f;
        style.TabRounding = 4.0f;
        
        // Colors
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.94f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.94f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.28f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.47f, 1.00f, 0.67f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.47f, 1.00f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.47f, 1.00f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.47f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.47f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.47f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.00f, 0.47f, 1.00f, 0.65f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.47f, 1.00f, 0.79f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.47f, 1.00f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.00f, 0.47f, 1.00f, 0.31f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.47f, 1.00f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.47f, 1.00f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.47f, 1.00f, 0.52f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.47f, 1.00f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.47f, 1.00f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.47f, 1.00f, 0.52f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.00f, 0.47f, 1.00f, 1.00f);
    }

    void Renderer::BeginFrame() {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void Renderer::EndFrame() {
        ImGui::EndFrame();
    }

    void Renderer::RenderFrame() {
        LimitFrameRate();
        UpdatePerformanceMetrics();
        
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context->ClearRenderTargetView(renderTargetView, clearColor);
        
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        swapChain->Present(1, 0); // V-sync enabled
    }

    void Renderer::LimitFrameRate() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime).count();
        
        float elapsedMs = elapsed / 1000.0f;
        if (elapsedMs < targetFrameTime) {
            std::chrono::microseconds sleepTime(static_cast<long long>(targetFrameTime - elapsedMs));
            std::this_thread::sleep_for(sleepTime);
        }
        
        lastFrameTime = std::chrono::high_resolution_clock::now();
    }

    void Renderer::UpdatePerformanceMetrics() {
        ++framesSinceLastUpdate;

        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedMicros = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - fpsTimer).count();

        if (elapsedMicros >= 1'000'000) { // Update every second
            const float elapsedSeconds = static_cast<float>(elapsedMicros) / 1'000'000.0f;

            if (framesSinceLastUpdate > 0 && elapsedSeconds > 0.0f) {
                fps = static_cast<float>(framesSinceLastUpdate) / elapsedSeconds;
                frameTime = (elapsedSeconds * 1000.0f) / static_cast<float>(framesSinceLastUpdate);

                if (fpsHistory.size() >= 60) {
                    fpsHistory.erase(fpsHistory.begin());
                }
                fpsHistory.push_back(fps);
            }

            framesSinceLastUpdate = 0;
            fpsTimer = currentTime;
        }
    }

    void Renderer::Shutdown() {
        CleanupImGui();
        CleanupD3D11();
    }

    void Renderer::CleanupImGui() {
        if (imguiContext) {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext(static_cast<ImGuiContext*>(imguiContext));
            imguiContext = nullptr;
        }
    }

    void Renderer::CleanupD3D11() {
        CleanupRenderTarget();
        
        if (swapChain) {
            swapChain->Release();
            swapChain = nullptr;
        }
        if (context) {
            context->Release();
            context = nullptr;
        }
        if (device) {
            device->Release();
            device = nullptr;
        }
    }

    bool Renderer::IsInitialized() const {
        return device && context && swapChain && renderTargetView;
    }

    void Renderer::Resize(UINT width, UINT height) {
        if (!IsInitialized()) return;
        
        context->OMSetRenderTargets(0, nullptr, nullptr);
        CleanupRenderTarget();
        
        swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        CreateRenderTarget();
    }
}

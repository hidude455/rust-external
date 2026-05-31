#pragma once
#include "Common.h"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

// Forward declarations for ImGui
struct ImDrawData;
struct ImVec4;

namespace MIT {
    class Renderer {
    private:
        HWND windowHandle;
        ID3D11Device* device;
        ID3D11DeviceContext* context;
        IDXGISwapChain* swapChain;
        ID3D11RenderTargetView* renderTargetView;
        
        // Frame limiting
        std::chrono::high_resolution_clock::time_point lastFrameTime;
        const float targetFrameTime = 1000.0f / 60.0f; // 60 FPS target
        
        // Performance monitoring
        float fps;
        float frameTime;
        std::vector<float> fpsHistory;
        std::chrono::high_resolution_clock::time_point fpsTimer;
        uint32_t framesSinceLastUpdate = 0;
        
        // ImGui specific
        void* imguiContext;
        ID3D11VertexShader* vertexShader;
        ID3D11PixelShader* pixelShader;
        ID3D11InputLayout* inputLayout;
        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;
        ID3D11BlendState* blendState;
        ID3D11RasterizerState* rasterizerState;
        ID3D11DepthStencilState* depthStencilState;
        ID3D11ShaderResourceView* fontTextureView;
        ID3D11SamplerState* fontSampler;

        bool InitializeD3D11();
        bool InitializeImGui();
        void CleanupD3D11();
        void CleanupImGui();
        void CreateRenderTarget();
        void CleanupRenderTarget();
        
        // Shader compilation
        HRESULT CreateShaderFromFile(const char* fileName, const char* entryPoint, const char* shaderModel, ID3D10Blob** ppBlobOut);

    public:
        Renderer();
        ~Renderer();
        
        bool Initialize(HWND targetWindow);
        void Shutdown();
        
        void BeginFrame();
        void EndFrame();
        void RenderFrame();
        
        // Frame limiting
        void LimitFrameRate();
        float GetFPS() const { return fps; }
        float GetFrameTime() const { return frameTime; }
        
        // ImGui rendering
        void ImGuiRenderDrawData(ImDrawData* draw_data);
        void SetupImGuiStyle();
        
        // Utility functions
        bool IsInitialized() const;
        void Resize(UINT width, UINT height);
        
        // Performance monitoring
        void UpdatePerformanceMetrics();
        const std::vector<float>& GetFPSHistory() const { return fpsHistory; }
    };
}

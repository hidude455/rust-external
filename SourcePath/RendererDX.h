/*
 * DirectX 11 Renderer Module
 * Handles all rendering operations with optimized pipeline
 * Author: Anonymous
 * Last Modified: 2026
 * 
 * This module provides high-performance DirectX 11 rendering with
 * frame limiting, resource management, and ImGui integration.
 */

#pragma once
#include "Core.h"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

// Forward declarations for ImGui
struct ImDrawData;
struct ImVec4;

namespace GameEnhance {
    
    // Frame statistics for performance monitoring
    struct FrameStats {
        float currentFPS;           // Current frames per second
        float frameTime;            // Time taken for last frame
        float avgFrameTime;         // Average frame time over history
        float minFrameTime;         // Minimum frame time
        float maxFrameTime;         // Maximum frame time
        int frameCount;            // Total frames rendered
        std::vector<float> fpsHistory; // FPS history for graph
    };
    
    // Rendering configuration
    struct RenderConfig {
        bool vsyncEnabled = true;        // Vertical sync
        float targetFPS = 60.0f;        // Target frame rate
        bool showPerformance = true;      // Show performance overlay
        bool enableAntialiasing = true;   // MSAA support
        int sampleCount = 4;             // MSAA sample count
        float gammaCorrection = 2.2f;    // Gamma correction value
    };
    
    class CDXRenderer {
    private:
        // DirectX 11 core objects
        ID3D11Device* m_device;                    // D3D11 device
        ID3D11DeviceContext* m_context;            // Device context
        IDXGISwapChain* m_swapChain;                // Swap chain
        ID3D11RenderTargetView* m_renderTargetView; // Render target
        ID3D11DepthStencilView* m_depthStencilView; // Depth buffer
        ID3D11Texture2D* m_depthStencilBuffer;     // Depth texture
        
        // ImGui specific objects
        void* m_imguiContext;                       // ImGui context
        ID3D11VertexShader* m_vertexShader;          // Vertex shader
        ID3D11PixelShader* m_pixelShader;           // Pixel shader
        ID3D11InputLayout* m_inputLayout;           // Input layout
        ID3D11Buffer* m_vertexBuffer;               // Vertex buffer
        ID3D11Buffer* m_indexBuffer;                // Index buffer
        ID3D11Buffer* m_constantBuffer;             // Constant buffer
        ID3D11BlendState* m_blendState;             // Blend state
        ID3D11RasterizerState* m_rasterizerState;   // Rasterizer state
        ID3D11DepthStencilState* m_depthStencilState; // Depth state
        ID3D11ShaderResourceView* m_fontTextureView;  // Font texture
        ID3D11SamplerState* m_fontSampler;           // Font sampler
        
        // Window information
        HWND m_targetWindow;                          // Target window handle
        int m_windowWidth;                           // Window width
        int m_windowHeight;                          // Window height
        
        // Frame timing and limiting
        LARGE_INTEGER m_frequency;                     // Performance counter frequency
        LARGE_INTEGER m_lastFrameTime;                // Last frame timestamp
        LARGE_INTEGER m_frameStartTime;                // Current frame start
        float m_targetFrameTime;                      // Target time per frame
        float m_accumulatedTime;                       // Accumulated time for averaging
        
        // Performance monitoring
        FrameStats m_frameStats;
        RenderConfig m_config;
        
        // Internal helper methods
        bool InitializeD3D11();
        bool InitializeImGui();
        bool CreateSwapChain();
        bool CreateRenderTarget();
        bool CreateDepthBuffer();
        bool CreateShaders();
        bool CreateRenderStates();
        bool SetupImGui();
        
        // Shader compilation helpers
        HRESULT CompileShaderFromFile(const char* fileName, const char* entryPoint, 
                                  const char* shaderModel, ID3D10Blob** ppBlobOut);
        HRESULT CreateShaderFromMemory(const void* shaderData, SIZE_T dataSize, 
                                   const char* entryPoint, const char* shaderModel);
        
        // Rendering pipeline helpers
        void SetupRenderTargets();
        void ClearRenderTargets();
        void PresentFrame();
        void UpdateFrameStats();
        void LimitFrameRate();
        
        // Resource management
        void CleanupD3DResources();
        void CleanupImGuiResources();
        void ResizeBuffers(UINT width, UINT height);
        
    public:
        CDXRenderer();
        ~CDXRenderer();
        
        // Initialization and cleanup
        bool Initialize(HWND targetWindow);
        void Shutdown();
        bool IsInitialized() const;
        
        // Frame rendering
        void BeginFrame();
        void EndFrame();
        void RenderFrame();
        
        // ImGui rendering
        void RenderImGuiData(ImDrawData* drawData);
        void SetupImGuiStyle();
        
        // Configuration
        void SetConfig(const RenderConfig& config);
        const RenderConfig& GetConfig() const { return m_config; }
        
        // Window management
        void Resize(UINT width, UINT height);
        void SetWindow(HWND window);
        HWND GetWindow() const { return m_targetWindow; }
        
        // Performance monitoring
        const FrameStats& GetFrameStats() const { return m_frameStats; }
        float GetCurrentFPS() const { return m_frameStats.currentFPS; }
        float GetFrameTime() const { return m_frameStats.frameTime; }
        
        // Resource access
        ID3D11Device* GetDevice() const { return m_device; }
        ID3D11DeviceContext* GetContext() const { return m_context; }
        ID3D11RenderTargetView* GetRenderTarget() const { return m_renderTargetView; }
        
        // Advanced features
        void TakeScreenshot(const std::string& filename);
        void SetGamma(float gamma);
        void EnableVSync(bool enable);
        void SetTargetFPS(float fps);
        
        // Debug utilities
        void BeginEvent(const std::string& name);
        void EndEvent();
        void SetMarker(const std::string& name);
    };
    
} // namespace GameEnhance

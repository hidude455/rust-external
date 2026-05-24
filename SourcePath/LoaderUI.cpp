#include "LoaderUI.h"

#include <d3dcompiler.h>
#include <dxgi.h>
#include <tchar.h>
#include <thread>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <sstream>

#include "AuthSystem.h"

#include "../MenuPath/imgui/imgui.h"
#include "../MenuPath/imgui/backends/imgui_impl_dx11.h"
#include "../MenuPath/imgui/backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API __int64 ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace {
    constexpr int kWindowWidth = 420;
    constexpr int kWindowHeight = 260;

    std::string CurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tmStruct;
        localtime_s(&tmStruct, &time);
        std::ostringstream oss;
        oss << std::put_time(&tmStruct, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string GetLogFilePath() {
        char buffer[MAX_PATH] = {};
        DWORD written = GetTempPathA(MAX_PATH, buffer);
        if (written == 0 || written > MAX_PATH) {
            return "loader_debug.log";
        }

        std::filesystem::path logDir(buffer);
        logDir /= "RustExternal";

        try {
            std::filesystem::create_directories(logDir);
        } catch (...) {
            // Fall back to temp root if creating directory fails
            logDir = std::filesystem::path(buffer);
        }

        return (logDir / "loader_debug.log").string();
    }

    void AppendDebugLog(const std::string& message) {
        std::string logPath = GetLogFilePath();
        std::ofstream logFile(logPath, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::string debugMsg = "Failed to open log file: " + logPath + "\n";
            OutputDebugStringA(debugMsg.c_str());
            return;
        }
        logFile << "[" << CurrentTimestamp() << "] " << message << "\n";
        logFile.flush();
    }

    void ShowWin32ErrorBox(const char* stage, DWORD error) {
        char buffer[256];
        sprintf_s(buffer, "Failed during %s. Win32 error code: %lu", stage, static_cast<unsigned long>(error));
        MessageBoxA(nullptr, buffer, "Rust External", MB_ICONERROR | MB_OK);
        AppendDebugLog(std::string(stage) + " failed with Win32 error " + std::to_string(error));
    }

    void ShowHRESULTErrorBox(const char* stage, HRESULT hr) {
        char buffer[256];
        sprintf_s(buffer, "Failed during %s. HRESULT: 0x%08lX", stage, static_cast<unsigned long>(hr));
        MessageBoxA(nullptr, buffer, "Rust External", MB_ICONERROR | MB_OK);
        std::ostringstream oss;
        oss << stage << " failed with HRESULT 0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << (static_cast<unsigned long>(hr) & 0xFFFFFFFFUL);
        AppendDebugLog(oss.str());
    }
}

void LoaderLog(const std::string& message) {
    AppendDebugLog(message);
}

static CLoaderUI* g_loaderUiInstance = nullptr;

CLoaderUI::CLoaderUI() {
    memset(m_licenseKeyBuffer, 0, sizeof(m_licenseKeyBuffer));
    g_loaderUiInstance = this;
}

CLoaderUI::~CLoaderUI() {
    CleanupImGui();
    CleanupDevice();
    if (m_hwnd && m_windowClass.lpszClassName) {
        DestroyWindow(m_hwnd);
        UnregisterClass(m_windowClass.lpszClassName, m_windowClass.hInstance);
    }
    g_loaderUiInstance = nullptr;
}

bool CLoaderUI::Initialize() {
    AppendDebugLog("CLoaderUI::Initialize - begin");
    if (!CreateRenderWindow()) {
        ShowWin32ErrorBox("CreateRenderWindow", GetLastError());
        return false;
    }
    AppendDebugLog("CreateRenderWindow succeeded");
    if (!InitializeDevice()) {
        return false;
    }
    AppendDebugLog("InitializeDevice succeeded");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.FramePadding = ImVec2(12.f, 8.f);
    style.ItemSpacing = ImVec2(12.f, 10.f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.10f, 0.98f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.18f, 0.55f, 0.33f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.21f, 0.65f, 0.40f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.45f, 0.28f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.32f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.21f, 0.65f, 0.40f, 0.80f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.23f, 0.72f, 0.44f, 0.90f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.16f, 0.48f, 0.30f, 1.00f);

    if (!ImGui_ImplWin32_Init(m_hwnd)) {
        ShowWin32ErrorBox("ImGui_ImplWin32_Init", GetLastError());
        return false;
    }
    AppendDebugLog("ImGui_ImplWin32_Init succeeded");
    if (!ImGui_ImplDX11_Init(m_device, m_context)) {
        ShowHRESULTErrorBox("ImGui_ImplDX11_Init", E_FAIL);
        return false;
    }
    AppendDebugLog("ImGui_ImplDX11_Init succeeded");

    m_initialized = true;
    AppendDebugLog("CLoaderUI::Initialize - completed successfully");
    return true;
}

LoaderLaunchResult CLoaderUI::Run() {
    AppendDebugLog("CLoaderUI::Run - begin");
    LoaderLaunchResult result;
    if (!m_initialized) {
        AppendDebugLog("Run aborted: not initialized");
        return result;
    }

    m_running = true;
    AppendDebugLog("Message loop started");

    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);

    MSG msg = {};
    while (m_running) {
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                m_running = false;
                AppendDebugLog("Received WM_QUIT, stopping message loop");
            }
        }

        if (!m_running) {
            AppendDebugLog("m_running false, breaking loop");
            break;
        }

        RenderFrame();

        if (m_shouldLaunch.load()) {
            result.shouldLaunch = true;
            result.cheat = std::move(m_cheat);
            m_running = false;
            m_shouldLaunch.store(false);
            AppendDebugLog("Launch requested, exiting loop");
        }
    }

    AppendDebugLog("CLoaderUI::Run - exiting");
    return result;
}

bool CLoaderUI::CreateRenderWindow() {
    AppendDebugLog("CreateRenderWindow - registering class");
    m_windowClass = {};
    m_windowClass.cbSize = sizeof(WNDCLASSEX);
    m_windowClass.style = CS_CLASSDC;
    m_windowClass.lpfnWndProc = &CLoaderUI::WndProc;
    m_windowClass.hInstance = GetModuleHandle(nullptr);
    m_windowClass.lpszClassName = _T("RustLoaderWindow");
    if (!RegisterClassEx(&m_windowClass)) {
        return false;
    }

    AppendDebugLog("CreateRenderWindow - creating window");
    m_hwnd = CreateWindow(m_windowClass.lpszClassName, _T("Aether Loader"),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, kWindowWidth, kWindowHeight, nullptr, nullptr, m_windowClass.hInstance, nullptr);

    if (!m_hwnd) {
        UnregisterClass(m_windowClass.lpszClassName, m_windowClass.hInstance);
        AppendDebugLog("CreateRenderWindow - CreateWindow failed");
        return false;
    }

    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    AppendDebugLog("CreateRenderWindow - success");
    return true;
}

bool CLoaderUI::InitializeDevice() {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = kWindowWidth;
    sd.BufferDesc.Height = kWindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, &featureLevel, &m_context);

    if (FAILED(hr)) {
        ShowHRESULTErrorBox("D3D11CreateDeviceAndSwapChain", hr);
        return false;
    }
    AppendDebugLog("InitializeDevice - swap chain created");

    ID3D11Texture2D* backBuffer = nullptr;
    if (SUCCEEDED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
        m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTarget);
        backBuffer->Release();
        AppendDebugLog("InitializeDevice - render target created");
    }
    return true;
}

void CLoaderUI::CleanupDevice() {
    if (m_renderTarget) {
        m_renderTarget->Release();
        m_renderTarget = nullptr;
    }
    if (m_swapChain) {
        m_swapChain->Release();
        m_swapChain = nullptr;
    }
    if (m_context) {
        m_context->Release();
        m_context = nullptr;
    }
    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }
}

void CLoaderUI::CleanupImGui() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void CLoaderUI::RenderFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    RenderUI();

    ImGui::Render();
    const float clearColor[4] = { 0.08f, 0.08f, 0.1f, 1.0f };
    m_context->OMSetRenderTargets(1, &m_renderTarget, nullptr);
    m_context->ClearRenderTargetView(m_renderTarget, clearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    m_swapChain->Present(1, 0);
}

void CLoaderUI::RenderUI() {
    ImGui::SetNextWindowPos(ImVec2(20.f, 20.f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(kWindowWidth - 40.f, kWindowHeight - 40.f), ImGuiCond_Always);

    ImGui::Begin("Aether Loader", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

    const ImVec4 accent = ImVec4(0.33f, 0.92f, 0.53f, 1.f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.33f, 0.92f, 0.53f, 1.f));
    ImGui::TextWrapped("Aether");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.80f, 0.82f, 0.88f, 1.f), "Secure Loader");
    ImGui::Spacing();
    ImGui::Text("License Key");

    bool busy = m_busy.load();
    float progressValue = m_progress.load();
    bool showProgress = m_showProgress.load();
    std::string statusCopy;
    std::string errorCopy;
    std::string progressLabelCopy;
    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        statusCopy = m_statusMessage;
        errorCopy = m_errorMessage;
        progressLabelCopy = m_progressLabel;
    }

    ImGui::InputText("##license", m_licenseKeyBuffer, IM_ARRAYSIZE(m_licenseKeyBuffer));

    if (busy) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button(busy ? "Working..." : "Authenticate", ImVec2(-1.f, 0.f))) {
        if (!busy) {
            LaunchCheat();
            busy = true;
        }
    }
    if (busy) {
        ImGui::EndDisabled();
    }

    if (showProgress) {
        ImGui::Spacing();
        if (!progressLabelCopy.empty()) {
            ImGui::TextColored(ImVec4(0.75f, 0.80f, 0.90f, 1.f), "%s", progressLabelCopy.c_str());
        }
        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_Text, accent);
        ImGui::TextUnformatted("<");
        ImGui::PopStyleColor();
        ImGui::SameLine(0.f, 0.f);
        const int segments = 13;
        int activeSegments = static_cast<int>(progressValue * segments + 0.01f);
        if (activeSegments < 0) activeSegments = 0;
        if (activeSegments > segments) activeSegments = segments;
        for (int i = 0; i < segments; ++i) {
            bool active = i < activeSegments;
            ImVec4 segmentColor = active ? accent : ImVec4(0.45f, 0.18f, 0.18f, 1.f);
            ImGui::PushStyleColor(ImGuiCol_Text, segmentColor);
            ImGui::TextUnformatted("-");
            ImGui::PopStyleColor();
            if (i < segments - 1) {
                ImGui::SameLine(0.f, 0.f);
            }
        }
        ImGui::SameLine(0.f, 0.f);
        ImGui::PushStyleColor(ImGuiCol_Text, accent);
        ImGui::TextUnformatted(">");
        ImGui::PopStyleColor();
        ImGui::EndGroup();
        ImGui::TextColored(ImVec4(0.75f, 0.80f, 0.90f, 1.f), "%.0f%%", progressValue * 100.f);
    }

    if (!statusCopy.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(accent, "Status: %s", statusCopy.c_str());
    }

    if (!errorCopy.empty()) {
        ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.f), "Error: %s", errorCopy.c_str());
    }

    ImGui::End();
}

void CLoaderUI::LaunchCheat() {
    if (m_busy.load()) {
        return;
    }

    std::string key = m_licenseKeyBuffer;
    if (key.empty()) {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        m_errorMessage = "Please enter a license key.";
        return;
    }

    m_busy.store(true);
    m_showProgress.store(true);
    m_progress.store(0.05f);
    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        m_statusMessage = "Loading...";
        m_errorMessage.clear();
        m_progressLabel = "Preparing";
    }

    std::thread([this, key]() {
        auto cheat = std::make_unique<App::CExternalCheat>();

        m_progress.store(0.25f);
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_progressLabel = "Validating license";
        }

        if (!cheat->Startup(key)) {
            {
                std::lock_guard<std::mutex> lock(m_stateMutex);
                m_errorMessage = "Authentication or initialization failed.";
                m_statusMessage.clear();
                m_progressLabel.clear();
            }
            m_showProgress.store(false);
            m_busy.store(false);
            return;
        }

        bool attached = cheat->IsAttachedToGame();
        bool waitingForGame = cheat->IsWaitingForGame();

        m_progress.store(attached ? 1.0f : 0.9f);
        m_showProgress.store(false);
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            if (waitingForGame && !attached) {
                m_statusMessage = "Waiting for RustClient.exe...";
                m_progressLabel.clear();
            } else {
                m_statusMessage = "Launching...";
                m_progressLabel.clear();
            }
        }

        m_cheat = std::move(cheat);
        m_shouldLaunch.store(true);
        m_busy.store(false);
    }).detach();
}

LRESULT CALLBACK CLoaderUI::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
        return true;
    }

    switch (msg) {
    case WM_NCDESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_DESTROY:
        if (g_loaderUiInstance) {
            g_loaderUiInstance->m_running = false;
        }
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

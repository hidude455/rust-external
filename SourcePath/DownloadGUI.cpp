#include "DownloadGUI.h"
#include <d3dcompiler.h>
#include <shlobj.h>

// Forward declare ImGui Win32 WndProc handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

CDownloadGUI::CDownloadGUI()
    : m_hInstance(nullptr)
    , m_hwnd(nullptr)
    , m_device(nullptr)
    , m_context(nullptr)
    , m_swapChain(nullptr)
    , m_renderTarget(nullptr)
    , m_downloader(std::make_unique<CDownloader>())
    , m_running(false)
    , m_initialized(false)
    , m_totalProgress(0.0f)
    , m_statusMessage("Ready to download P Client files...")
    , m_autoStart(true)
    , m_downloadComplete(false)
{
}

CDownloadGUI::~CDownloadGUI() {
    Shutdown();
}

bool CDownloadGUI::Initialize(HINSTANCE hInstance) {
    m_hInstance = hInstance;
    
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"DownloadGUIClass";
    
    if (!RegisterClassExW(&wc)) {
        return false;
    }
    
    RECT rect = { 0, 0, 600, 400 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    m_hwnd = CreateWindowExW(
        0, L"DownloadGUIClass", L"P Client - Setup",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, this
    );
    
    if (!m_hwnd) {
        return false;
    }
    
    if (!InitializeDirectX()) {
        return false;
    }
    
    if (!InitializeImGui()) {
        return false;
    }
    
    // Center window
    RECT screenRect;
    GetWindowRect(GetDesktopWindow(), &screenRect);
    int x = (screenRect.right - (rect.right - rect.left)) / 2;
    int y = (screenRect.bottom - (rect.bottom - rect.top)) / 2;
    SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE);
    
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    
    // Setup progress callback
    m_downloader->SetProgressCallback([this](float progress, const std::string& status) {
        m_totalProgress = progress;
        m_statusMessage = status;
    });
    
    // Setup download queue
    SetupDownloadQueue();
    
    // Auto-start download
    if (m_autoStart) {
        m_downloader->StartDownloads();
    }
    
    m_running = true;
    m_initialized = true;
    return true;
}

void CDownloadGUI::SetupDownloadQueue() {
    // Get installation directory
    char installPath[MAX_PATH];
    SHGetFolderPathA(nullptr, CSIDL_PROGRAM_FILES, nullptr, 0, installPath);
    std::string path = std::string(installPath) + "\\P Client\\";
    
    // Add files to download queue
    // These would be your actual download URLs
    m_downloader->AddFileToQueue(
        "https://example.com/RustInjector.exe",
        path + "RustInjector.exe",
        "RustInjector.exe"
    );
    
    m_downloader->AddFileToQueue(
        "https://example.com/EnhancedProject.dll",
        path + "EnhancedProject.dll",
        "EnhancedProject.dll"
    );
    
    m_downloader->AddFileToQueue(
        "https://example.com/icon.ico",
        path + "icon.ico",
        "icon.ico"
    );
}

bool CDownloadGUI::InitializeDirectX() {
    DXGI_SWAP_CHAIN_DESC sd = { 0 };
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &sd, &m_swapChain, &m_device, &featureLevel, &m_context
    );

    if (FAILED(hr)) {
        return false;
    }

    ID3D11Texture2D* pBackBuffer;
    m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    m_device->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTarget);
    pBackBuffer->Release();

    return true;
}

bool CDownloadGUI::InitializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ApplyPurpleTheme();
    
    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_device, m_context);
    
    return true;
}

void CDownloadGUI::ApplyPurpleTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    ImVec4 purple = ImVec4(0.5f, 0.0f, 0.5f, 1.0f);
    ImVec4 darkBg = ImVec4(0.1f, 0.1f, 0.15f, 1.0f);
    ImVec4 lightBg = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    
    style.WindowPadding = ImVec2(15, 15);
    style.WindowRounding = 8.0f;
    style.FramePadding = ImVec2(5, 5);
    style.ItemSpacing = ImVec2(10, 10);
    style.ItemInnerSpacing = ImVec2(5, 5);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 12.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabMinSize = 8.0f;
    style.GrabRounding = 4.0f;
    
    style.Colors[ImGuiCol_WindowBg] = darkBg;
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.15f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_Border] = purple;
    style.Colors[ImGuiCol_FrameBg] = lightBg;
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = purple;
    style.Colors[ImGuiCol_TitleBg] = purple;
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.6f, 0.0f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = purple;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.6f, 0.0f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.7f, 0.0f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = purple;
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.6f, 0.0f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_Button] = purple;
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.6f, 0.0f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.7f, 0.0f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_Header] = purple;
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.6f, 0.0f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.7f, 0.0f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = purple;
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.6f, 0.0f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.7f, 0.0f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
}

void CDownloadGUI::CleanupDirectX() {
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

void CDownloadGUI::CleanupImGui() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void CDownloadGUI::RenderFrame() {
    if (!m_initialized) return;
    
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    RenderUI();
    
    ImGui::Render();
    
    float clearColor[4] = { 0.1f, 0.1f, 0.15f, 1.0f };
    m_context->OMSetRenderTargets(1, &m_renderTarget, nullptr);
    m_context->ClearRenderTargetView(m_renderTarget, clearColor);
    
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
    m_swapChain->Present(1, 0);
}

void CDownloadGUI::RenderUI() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    
    ImGui::Begin("P Client Setup", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    // Header
    ImGui::Spacing();
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(1.5f);
    ImGui::TextColored(ImVec4(0.5f, 0.0f, 0.5f, 1.0f), "P Client Installation");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Status
    ImGui::Text("Status: %s", m_statusMessage.c_str());
    
    // Progress Bar
    ImGui::Spacing();
    ImGui::Text("Overall Progress:");
    ImGui::ProgressBar(m_totalProgress / 100.0f, ImVec2(550, 30), 
        (m_statusMessage.find("Complete") != std::string::npos) ? "Complete!" : "");
    
    // File List
    ImGui::Spacing();
    ImGui::SeparatorText("Download Queue");
    
    auto queue = m_downloader->GetDownloadQueue();
    if (ImGui::BeginChild("FileList", ImVec2(0, 150), true)) {
        for (const auto& file : queue) {
            ImGui::PushID(file.displayName.c_str());
            
            ImGui::Text("%s", file.displayName.c_str());
            ImGui::SameLine(300);
            
            if (file.completed) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Complete");
            } else if (file.failed) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed");
            } else {
                ImGui::ProgressBar(file.progress / 100.0f, ImVec2(150, 15));
            }
            
            ImGui::PopID();
        }
        ImGui::EndChild();
    }
    
    // Buttons
    ImGui::Spacing();
    
    if (m_downloader->IsDownloading()) {
        if (ImGui::Button("Cancel", ImVec2(150, 35))) {
            m_downloader->CancelDownloads();
            m_statusMessage = "Download cancelled";
        }
    } else {
        if (m_downloader->GetCompletedCount() == m_downloader->GetTotalCount() && m_downloader->GetTotalCount() > 0) {
            m_downloadComplete = true;
            if (ImGui::Button("Launch P Client", ImVec2(150, 35))) {
                // Launch the application
                char installPath[MAX_PATH];
                SHGetFolderPathA(nullptr, CSIDL_PROGRAM_FILES, nullptr, 0, installPath);
                std::string exePath = std::string(installPath) + "\\P Client\\RustInjector.exe";
                ShellExecuteA(nullptr, "open", exePath.c_str(), nullptr, nullptr, SW_SHOW);
                m_running = false;
            }
        } else {
            if (ImGui::Button("Start Download", ImVec2(150, 35))) {
                m_downloader->StartDownloads();
            }
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Exit", ImVec2(150, 35))) {
        m_running = false;
    }
    
    ImGui::End();
}

LRESULT CALLBACK CDownloadGUI::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CDownloadGUI* gui = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        gui = reinterpret_cast<CDownloadGUI*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(gui));
    } else {
        gui = reinterpret_cast<CDownloadGUI*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    
    if (gui && ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
        return true;
    }
    
    switch (uMsg) {
        case WM_SIZE:
            if (gui && gui->m_device && wParam != SIZE_MINIMIZED) {
                gui->CleanupDirectX();
                gui->InitializeDirectX();
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int CDownloadGUI::Run() {
    MSG msg = { 0 };
    while (m_running && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        RenderFrame();
    }
    return (int)msg.wParam;
}

void CDownloadGUI::Shutdown() {
    m_downloader->CancelDownloads();
    CleanupImGui();
    CleanupDirectX();
    m_running = false;
    m_initialized = false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CDownloadGUI gui;
    if (!gui.Initialize(hInstance)) {
        MessageBoxW(nullptr, L"Failed to initialize download GUI", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    return gui.Run();
}

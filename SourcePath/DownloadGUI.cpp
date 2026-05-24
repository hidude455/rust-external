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
    , m_animationTime(0.0f)
    , m_pulseIntensity(0.0f)
    , m_rotationAngle(0.0f)
    , m_maxLogMessages(8)
    , m_installingDependencies(false)
    , m_isInstaller(false)
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
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszClassName = L"DownloadGUIClass";
    
    if (!RegisterClassExW(&wc)) {
        return false;
    }
    
    RECT rect = { 0, 0, 700, 500 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    m_hwnd = CreateWindowExW(
        0, L"DownloadGUIClass", L"P Client - Setup",
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT,
        700, 500,
        nullptr, nullptr, hInstance, this
    );
    
    if (!m_hwnd) {
        return false;
    }
    
    // Center window on screen
    RECT screenRect;
    GetWindowRect(GetDesktopWindow(), &screenRect);
    int x = (screenRect.right - 700) / 2;
    int y = (screenRect.bottom - 500) / 2;
    SetWindowPos(m_hwnd, nullptr, x, y, 700, 500, SWP_NOZORDER);
    
    if (!InitializeDirectX()) {
        return false;
    }
    
    if (!InitializeImGui()) {
        return false;
    }
    
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    
    // Setup progress callback
    m_downloader->SetProgressCallback([this](float progress, const std::string& status) {
        m_totalProgress = progress;
        m_statusMessage = status;
        
        // Add to log
        m_logMessages.push_back(status);
        if (m_logMessages.size() > m_maxLogMessages) {
            m_logMessages.erase(m_logMessages.begin());
        }
    });
    
    // Setup download queue
    SetupDownloadQueue();
    
    // Check for Visual Studio installation
    if (!CheckVisualStudioInstalled()) {
        ShowVisualStudioWarning();
    }
    
    // Check for DirectX and add to install queue if needed
    if (!CheckDirectXInstalled()) {
        m_statusMessage = "DirectX not found, will install after download";
        m_logMessages.push_back("DirectX runtime will be installed");
    }
    
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
    
    // Create temp directory for installers
    std::string tempPath = std::string(installPath) + "\\P Client\\temp\\";
    
    // Add P Client files to download queue
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
    
    // Add DirectX End-User Runtime (web installer - ~1MB)
    m_downloader->AddFileToQueue(
        "https://download.microsoft.com/download/1/7/1/1718CCC4-3314-4815-8C2F-707C5B5E4B2F/directx_Jun2010_redist.exe",
        tempPath + "directx_redist.exe",
        "DirectX End-User Runtime"
    );
    
    // Add Visual C++ Redistributable (x64)
    m_downloader->AddFileToQueue(
        "https://aka.ms/vs/17/release/vc_redist.x64.exe",
        tempPath + "vc_redist.x64.exe",
        "Visual C++ Redistributable x64"
    );
    
    // Note: Visual Studio Build Tools is NOT downloaded here because:
    // - It's a massive installer (several GB)
    // - It requires admin privileges to install
    // - It's a complex installer that needs to run separately
    // - Users should install Visual Studio 2019/2022 with C++ development tools beforehand
    // - The DownloadGUI downloads smaller dependencies like DirectX and VC++ runtimes
}

bool CDownloadGUI::CheckVisualStudioInstalled() {
    // Check for Visual Studio 2022 (version 17)
    const char* vs2022Paths[] = {
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Tools\\MSVC",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Tools\\MSVC",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\Professional\\VC\\Tools\\MSVC",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Tools\\MSVC",
        "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC",
        "C:\\Program Files\\Microsoft Visual Studio\\18\\Professional\\VC\\Tools\\MSVC",
        "C:\\Program Files\\Microsoft Visual Studio\\18\\Enterprise\\VC\\Tools\\MSVC",
        nullptr
    };
    
    for (int i = 0; vs2022Paths[i] != nullptr; i++) {
        if (GetFileAttributesA(vs2022Paths[i]) != INVALID_FILE_ATTRIBUTES) {
            return true;
        }
    }
    
    // Check for Visual Studio 2019 (version 16)
    const char* vs2019Paths[] = {
        "C:\\Program Files\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC",
        "C:\\Program Files\\Microsoft Visual Studio\\2019\\Professional\\VC\\Tools\\MSVC",
        "C:\\Program Files\\Microsoft Visual Studio\\2019\\Enterprise\\VC\\Tools\\MSVC",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional\\VC\\Tools\\MSVC",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Enterprise\\VC\\Tools\\MSVC",
        nullptr
    };
    
    for (int i = 0; vs2019Paths[i] != nullptr; i++) {
        if (GetFileAttributesA(vs2019Paths[i]) != INVALID_FILE_ATTRIBUTES) {
            return true;
        }
    }
    
    return false;
}

void CDownloadGUI::ShowVisualStudioWarning() {
    m_statusMessage = "Warning: Visual Studio not found. P Client requires VS 2019/2022 with C++ tools.";
    m_logMessages.push_back("Visual Studio not detected");
    m_logMessages.push_back("Please install Visual Studio 2019/2022");
    m_logMessages.push_back("with C++ development tools");
}

bool CDownloadGUI::CheckDirectXInstalled() {
    // Check for DirectX by looking for d3d11.dll in System32
    char systemPath[MAX_PATH];
    GetSystemDirectoryA(systemPath, MAX_PATH);
    std::string d3d11Path = std::string(systemPath) + "\\d3d11.dll";
    
    if (GetFileAttributesA(d3d11Path.c_str()) != INVALID_FILE_ATTRIBUTES) {
        return true;
    }
    
    return false;
}

void CDownloadGUI::ExecuteInstaller(const std::string& installerPath) {
    m_statusMessage = "Installing " + installerPath;
    m_logMessages.push_back("Running installer: " + installerPath);
    
    // Execute installer silently
    SHELLEXECUTEINFOA sei = { 0 };
    sei.cbSize = sizeof(SHELLEXECUTEINFOA);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = nullptr;
    sei.lpVerb = "open";
    sei.lpFile = installerPath.c_str();
    sei.lpParameters = "/quiet /norestart";
    sei.lpDirectory = nullptr;
    sei.nShow = SW_SHOW;
    sei.hInstApp = GetModuleHandle(nullptr);
    
    if (ShellExecuteExA(&sei)) {
        // Wait for installer to complete
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);
        
        m_logMessages.push_back("Installation completed");
    } else {
        m_logMessages.push_back("Failed to run installer");
    }
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
    
    // Very strong purple colors to ensure visibility
    ImVec4 purple = ImVec4(0.8f, 0.3f, 0.9f, 1.0f);
    ImVec4 purpleDark = ImVec4(0.6f, 0.1f, 0.7f, 1.0f);
    ImVec4 purpleLight = ImVec4(0.9f, 0.5f, 1.0f, 1.0f);
    ImVec4 darkBg = ImVec4(0.3f, 0.15f, 0.4f, 1.0f); // Very strong purple
    ImVec4 panelBg = ImVec4(0.35f, 0.18f, 0.45f, 1.0f); // Very strong purple
    
    style.WindowPadding = ImVec2(15, 15);
    style.WindowRounding = 10.0f;
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 14.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 5.0f;
    style.WindowBorderSize = 1.0f;
    
    // Window and background colors - VERY purple
    style.Colors[ImGuiCol_WindowBg] = darkBg;
    style.Colors[ImGuiCol_ChildBg] = panelBg;
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.35f, 0.18f, 0.45f, 0.95f);
    style.Colors[ImGuiCol_Border] = purpleDark;
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Frame colors
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.35f, 0.2f, 0.45f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.45f, 0.25f, 0.55f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = purple;
    
    // Title bar
    style.Colors[ImGuiCol_TitleBg] = purpleDark;
    style.Colors[ImGuiCol_TitleBgActive] = purple;
    style.Colors[ImGuiCol_TitleBgCollapsed] = purpleDark;
    
    // Menu bar
    style.Colors[ImGuiCol_MenuBarBg] = panelBg;
    
    // Scrollbar
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.25f, 0.12f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = purpleDark;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = purple;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = purpleLight;
    
    // Check box
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Slider
    style.Colors[ImGuiCol_SliderGrab] = purpleDark;
    style.Colors[ImGuiCol_SliderGrabActive] = purple;
    
    // Button
    style.Colors[ImGuiCol_Button] = purpleDark;
    style.Colors[ImGuiCol_ButtonHovered] = purple;
    style.Colors[ImGuiCol_ButtonActive] = purpleLight;
    
    // Header
    style.Colors[ImGuiCol_Header] = purpleDark;
    style.Colors[ImGuiCol_HeaderHovered] = purple;
    style.Colors[ImGuiCol_HeaderActive] = purpleLight;
    
    // Resize grip
    style.Colors[ImGuiCol_ResizeGrip] = purpleDark;
    style.Colors[ImGuiCol_ResizeGripHovered] = purple;
    style.Colors[ImGuiCol_ResizeGripActive] = purpleLight;
    
    // Text
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    
    // Separator
    style.Colors[ImGuiCol_Separator] = purpleDark;
    style.Colors[ImGuiCol_SeparatorHovered] = purple;
    style.Colors[ImGuiCol_SeparatorActive] = purpleLight;
    
    // Plot/Histogram
    style.Colors[ImGuiCol_PlotLines] = purple;
    style.Colors[ImGuiCol_PlotLinesHovered] = purpleLight;
    style.Colors[ImGuiCol_PlotHistogram] = purpleDark;
    style.Colors[ImGuiCol_PlotHistogramHovered] = purple;
    
    // Tab
    style.Colors[ImGuiCol_Tab] = purpleDark;
    style.Colors[ImGuiCol_TabHovered] = purple;
    style.Colors[ImGuiCol_TabActive] = purple;
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.35f, 0.2f, 0.45f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = purpleDark;
    
    // Drag and drop
    style.Colors[ImGuiCol_DragDropTarget] = purpleLight;
    
    // Nav
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = purpleLight;
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
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
    
    // Clear with purple background
    float clearColor[4] = { 0.24f, 0.08f, 0.31f, 1.0f }; // Strong purple
    m_context->OMSetRenderTargets(1, &m_renderTarget, nullptr);
    m_context->ClearRenderTargetView(m_renderTarget, clearColor);
    
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
    m_swapChain->Present(1, 0);
}

void CDownloadGUI::RenderUI() {
    // Update animations
    m_animationTime += ImGui::GetIO().DeltaTime;
    m_pulseIntensity = (sin(m_animationTime * 2.0f) + 1.0f) * 0.5f;
    m_rotationAngle += ImGui::GetIO().DeltaTime * 50.0f;
    
    // Make window fill entire client area
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
    
    ImGui::Begin("P Client Setup", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
    
    ImGui::PopStyleVar(3);
    
    // Draw purple background - make it very visible
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImVec2(0, 0),
        ImGui::GetIO().DisplaySize,
        IM_COL32(60, 20, 80, 255)  // Much stronger purple
    );
    
    // Animated Header with gradient effect
    ImGui::Spacing();
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(1.8f);
    
    ImVec4 headerColor = ImVec4(0.5f + m_pulseIntensity * 0.2f, 0.0f, 0.5f + m_pulseIntensity * 0.2f, 1.0f);
    ImGui::TextColored(headerColor, "P Client Installation");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();
    
    // Subtitle with version info
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 200);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "v1.0.0");
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Current Status with icon
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 1.0f, 1.0f));
    ImGui::Text("Current Status:");
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    ImVec4 statusColor = ImVec4(0.5f, 0.8f, 1.0f, 1.0f);
    if (m_downloader->IsDownloading()) {
        statusColor = ImVec4(1.0f, 0.8f, 0.2f, 1.0f); // Orange for downloading
    } else if (m_downloadComplete) {
        statusColor = ImVec4(0.2f, 1.0f, 0.4f, 1.0f); // Green for complete
    }
    ImGui::TextColored(statusColor, "%s", m_statusMessage.c_str());
    
    // Animated Progress Bar
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 1.0f, 1.0f));
    ImGui::Text("Overall Progress:");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Custom progress bar with gradient effect
    ImVec2 progressBarSize(600, 35);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.5f + m_pulseIntensity * 0.1f, 0.0f, 0.5f + m_pulseIntensity * 0.1f, 1.0f));
    ImGui::ProgressBar(m_totalProgress / 100.0f, progressBarSize);
    ImGui::PopStyleColor(2);
    
    // Percentage display
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "%.1f%%", m_totalProgress);
    
    // Download Queue with animated indicators
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 1.0f, 1.0f));
    ImGui::SeparatorText("Download Queue");
    ImGui::PopStyleColor();
    
    auto queue = m_downloader->GetDownloadQueue();
    if (ImGui::BeginChild("FileList", ImVec2(0, 180), true, ImGuiWindowFlags_NoScrollbar)) {
        for (size_t i = 0; i < queue.size(); i++) {
            const auto& file = queue[i];
            ImGui::PushID(i);
            
            // File icon indicator
            float iconPulse = (file.progress > 0 && file.progress < 100) ? m_pulseIntensity : 0.0f;
            ImVec4 iconColor = ImVec4(0.5f + iconPulse * 0.3f, 0.0f, 0.5f + iconPulse * 0.3f, 1.0f);
            
            if (file.completed) {
                iconColor = ImVec4(0.2f, 1.0f, 0.4f, 1.0f);
                ImGui::TextColored(iconColor, "[✓]");
            } else if (file.failed) {
                iconColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
                ImGui::TextColored(iconColor, "[✗]");
            } else if (file.progress > 0) {
                ImGui::TextColored(iconColor, "[↓]");
            } else {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[ ]");
            }
            
            ImGui::SameLine(30);
            ImGui::Text("%s", file.displayName.c_str());
            
            // Progress bar for individual file
            if (file.progress > 0 && file.progress < 100) {
                ImGui::SameLine(200);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::ProgressBar(file.progress / 100.0f, ImVec2(250, 12), "");
                ImGui::PopStyleVar();
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "%.0f%%", file.progress);
            } else if (file.completed) {
                ImGui::SameLine(200);
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Downloaded successfully");
            } else if (file.failed) {
                ImGui::SameLine(200);
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Download failed");
            } else {
                ImGui::SameLine(200);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Waiting...");
            }
            
            ImGui::PopID();
        }
        ImGui::EndChild();
    }
    
    // Activity Log
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 1.0f, 1.0f));
    ImGui::SeparatorText("Activity Log");
    ImGui::PopStyleColor();
    
    if (ImGui::BeginChild("ActivityLog", ImVec2(0, 80), true, ImGuiWindowFlags_NoScrollbar)) {
        for (size_t i = 0; i < m_logMessages.size(); i++) {
            float alpha = 1.0f - (float)i / m_logMessages.size() * 0.5f;
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.9f, alpha));
            ImGui::Text("• %s", m_logMessages[i].c_str());
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
    }
    
    // Buttons with hover effects
    ImGui::Spacing();
    
    if (m_downloader->IsDownloading()) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Cancel Download", ImVec2(180, 40))) {
            m_downloader->CancelDownloads();
            m_statusMessage = "Download cancelled by user";
            m_logMessages.push_back("Download cancelled");
        }
        ImGui::PopStyleColor(2);
    } else {
        if (m_downloader->GetCompletedCount() == m_downloader->GetTotalCount() && m_downloader->GetTotalCount() > 0) {
            m_downloadComplete = true;
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 1.0f, 0.4f, 1.0f));
            if (ImGui::Button("Launch P Client", ImVec2(180, 40))) {
                char installPath[MAX_PATH];
                SHGetFolderPathA(nullptr, CSIDL_PROGRAM_FILES, nullptr, 0, installPath);
                std::string exePath = std::string(installPath) + "\\P Client\\RustInjector.exe";
                ShellExecuteA(nullptr, "open", exePath.c_str(), nullptr, nullptr, SW_SHOW);
                m_running = false;
            }
            ImGui::PopStyleColor(2);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.0f, 0.6f, 1.0f));
            if (ImGui::Button("Start Download", ImVec2(180, 40))) {
                m_downloader->StartDownloads();
                m_logMessages.push_back("Download started");
            }
            ImGui::PopStyleColor(2);
        }
    }
    
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.5f, 1.0f));
    if (ImGui::Button("Exit", ImVec2(180, 40))) {
        m_running = false;
    }
    ImGui::PopStyleColor(2);
    
    // Footer with animated dots
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 250);
    if (m_downloader->IsDownloading()) {
        const char* dots = "";
        int dotCount = (int)(m_animationTime * 2) % 4;
        for (int i = 0; i < dotCount; i++) dots = ".";
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Downloading%s", dots);
    } else if (m_downloadComplete) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "✓ Installation Complete!");
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Ready to install");
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
        case WM_ERASEBKGND:
            return 1; // Prevent Windows from erasing background
            
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

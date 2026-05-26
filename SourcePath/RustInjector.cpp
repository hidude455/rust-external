#include "RustInjector.h"
#include <d3dcompiler.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

CRustInjector::CRustInjector()
    : m_hInstance(nullptr)
    , m_hwnd(nullptr)
    , m_device(nullptr)
    , m_context(nullptr)
    , m_swapChain(nullptr)
    , m_renderTarget(nullptr)
    , m_injector(std::make_unique<CInjector>())
    , m_kernelInterface(std::make_unique<KernelInterface::CKernelInterface>())
    , m_memoryManager(nullptr)
    , m_renderer(nullptr)
    , m_esp(nullptr)
    , m_running(false)
    , m_initialized(false)
    , m_driverLoaded(false)
    , m_autoInject(false)
    , m_manualMap(false)
    , m_selectedTab(0)
    , m_animationTimer(0.0f)
    , m_espEnabled(true)
    , m_espShowCircle(true)
    , m_espShowInventory(true)
    , m_espShowChams(true)
    , m_espGalaxyMode(false)
    , m_espMaxDistance(500.0f)
    , m_espCircleRadius(30.0f)
{
    strcpy_s(m_dllPath, "EnhancedProject.dll");
    m_statusMessage = "Ready - Waiting for Rust process...";
}

CRustInjector::~CRustInjector() {
    Shutdown();
}

bool CRustInjector::Initialize(HINSTANCE hInstance) {
    m_hInstance = hInstance;
    
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"RustInjectorClass";
    
    if (!RegisterClassExW(&wc)) {
        return false;
    }
    
    RECT rect = { 0, 0, 800, 600 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    m_hwnd = CreateWindowExW(
        0, L"RustInjectorClass", L"P Client - Rust Injector",
        WS_OVERLAPPEDWINDOW,
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
    
    // Initialize kernel driver
    if (m_kernelInterface->Initialize("")) {
        m_driverLoaded = true;
        AppendLog("IntelPT driver loaded successfully");
        m_statusMessage = "Driver loaded - Ready for injection";
    } else {
        m_driverLoaded = false;
        AppendLog("IntelPT driver not found - using user-mode fallback");
        m_statusMessage = "No driver - User-mode fallback active";
    }
    
    // Initialize memory manager
    m_memoryManager = std::make_unique<MemoryManager>(m_kernelInterface.get());
    
    // Initialize renderer
    m_renderer = std::make_unique<Renderer>(m_device, m_context);
    
    // Initialize ESP system
    m_esp = std::make_unique<ESP>(m_memoryManager.get(), m_renderer.get());
    m_esp->Initialize();
    
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    
    m_running = true;
    m_initialized = true;
    return true;
}

bool CRustInjector::InitializeDirectX() {
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

bool CRustInjector::InitializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Load anti-aliased font
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2;
    fontConfig.OversampleV = 2;
    fontConfig.PixelSnapH = false;
    io.Fonts->AddFontFromFileTTF("MenuPath/imgui/misc/fonts/Roboto-Medium.ttf", 16.0f, &fontConfig);
    
    ApplyDarkTheme();
    
    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_device, m_context);
    
    return true;
}

void CRustInjector::ApplyDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Dark background colors
    ImVec4 darkBg = ImVec4(0.08f, 0.08f, 0.1f, 1.0f);
    ImVec4 darkerBg = ImVec4(0.05f, 0.05f, 0.07f, 1.0f);
    ImVec4 cardBg = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    
    // Red accent colors
    ImVec4 redAccent = ImVec4(0.85f, 0.15f, 0.15f, 1.0f);
    ImVec4 redHover = ImVec4(0.95f, 0.25f, 0.25f, 1.0f);
    ImVec4 redActive = ImVec4(0.75f, 0.05f, 0.05f, 1.0f);
    
    // Text colors
    ImVec4 textMain = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    ImVec4 textDim = ImVec4(0.6f, 0.6f, 0.65f, 1.0f);
    
    // Rounded corners for modern look
    style.WindowRounding = 12.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 6.0f;
    
    // Spacing
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(10, 8);
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 4);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 10.0f;
    style.GrabMinSize = 10.0f;
    
    // Window colors
    style.Colors[ImGuiCol_WindowBg] = darkBg;
    style.Colors[ImGuiCol_ChildBg] = cardBg;
    style.Colors[ImGuiCol_PopupBg] = darkerBg;
    style.Colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Frame colors
    style.Colors[ImGuiCol_FrameBg] = darkerBg;
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = redAccent;
    
    // Title bar
    style.Colors[ImGuiCol_TitleBg] = darkerBg;
    style.Colors[ImGuiCol_TitleBgActive] = darkerBg;
    style.Colors[ImGuiCol_TitleBgCollapsed] = darkerBg;
    
    // Menu bar
    style.Colors[ImGuiCol_MenuBarBg] = darkerBg;
    
    // Scrollbar
    style.Colors[ImGuiCol_ScrollbarBg] = darkerBg;
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = redAccent;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = redActive;
    
    // Check mark
    style.Colors[ImGuiCol_CheckMark] = redAccent;
    
    // Slider
    style.Colors[ImGuiCol_SliderGrab] = redAccent;
    style.Colors[ImGuiCol_SliderGrabActive] = redActive;
    
    // Button
    style.Colors[ImGuiCol_Button] = redAccent;
    style.Colors[ImGuiCol_ButtonHovered] = redHover;
    style.Colors[ImGuiCol_ButtonActive] = redActive;
    
    // Header
    style.Colors[ImGuiCol_Header] = darkerBg;
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = redAccent;
    
    // Selection
    style.Colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = redAccent;
    
    // Resize grip
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] = redAccent;
    style.Colors[ImGuiCol_ResizeGripActive] = redActive;
    
    // Text
    style.Colors[ImGuiCol_Text] = textMain;
    style.Colors[ImGuiCol_TextDisabled] = textDim;
    
    // Tab
    style.Colors[ImGuiCol_Tab] = darkerBg;
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = redAccent;
    style.Colors[ImGuiCol_TabUnfocused] = darkerBg;
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    
    // Docking
    style.Colors[ImGuiCol_DockingPreview] = redAccent;
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void CRustInjector::CleanupDirectX() {
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

void CRustInjector::CleanupImGui() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void CRustInjector::RenderFrame() {
    if (!m_initialized) return;
    
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    RenderUI();
    
    // Update and render ESP
    if (m_esp && m_espEnabled) {
        // Sync GUI settings to ESP config
        MIT::ESPConfig espConfig = m_esp->GetConfig();
        espConfig.enabled = m_espEnabled;
        espConfig.showCircleESP = m_espShowCircle;
        espConfig.showInventory = m_espShowInventory;
        espConfig.showWeaponChams = m_espShowChams;
        espConfig.galaxyMode = m_espGalaxyMode;
        espConfig.maxDistance = m_espMaxDistance;
        espConfig.circleRadius = m_espCircleRadius;
        m_esp->SetConfig(espConfig);
        
        m_esp->Update();
        m_esp->Render();
    }
    
    ImGui::Render();
    
    float clearColor[4] = { 0.1f, 0.1f, 0.15f, 1.0f };
    m_context->OMSetRenderTargets(1, &m_renderTarget, nullptr);
    m_context->ClearRenderTargetView(m_renderTarget, clearColor);
    
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
    m_swapChain->Present(1, 0);
}

void CRustInjector::RenderUI() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(900, 650), ImGuiCond_FirstUseEver);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("P Client - Rust Injector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar();
    
    // Menu bar
    if (ImGui::BeginMenuBar()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
        ImGui::Text("cheatstore.net");
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 20);
        ImGui::Text("Injector");
        ImGui::SameLine(0, 400);
        if (ImGui::MenuItem("Exit")) {
            m_running = false;
        }
        ImGui::EndMenuBar();
    }
    
    // Main content area with sidebar
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::BeginChild("MainContent", ImVec2(0, 0), false);
    ImGui::PopStyleVar();
    
    // Sidebar
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    ImGui::BeginChild("Sidebar", ImVec2(180, 0), true);
    ImGui::PopStyleVar();
    
    const char* tabs[] = { "Status", "DLL", "Options", "ESP", "Log" };
    for (int i = 0; i < 5; i++) {
        bool isSelected = (m_selectedTab == i);
        
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.05f, 0.05f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.07f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
        }
        
        if (ImGui::Button(tabs[i], ImVec2(156, 40))) {
            m_selectedTab = i;
            m_animationTimer = 0.0f;
        }
        
        ImGui::PopStyleColor(3);
        ImGui::Spacing();
    }
    
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Content area
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::BeginChild("Content", ImVec2(0, 0), true);
    ImGui::PopStyleVar();
    
    // Render selected tab content
    switch (m_selectedTab) {
        case 0: RenderStatusTab(); break;
        case 1: RenderDLLTab(); break;
        case 2: RenderOptionsTab(); break;
        case 3: RenderESPTab(); break;
        case 4: RenderLogTab(); break;
    }
    
    ImGui::EndChild();
    
    ImGui::EndChild();
    ImGui::End();
}

void CRustInjector::RenderStatusTab() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
    ImGui::Text("Status");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    // Status card
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("StatusCard", ImVec2(0, 100), true);
    ImGui::PopStyleVar();
    
    ImGui::Text("Current Status:");
    ImGui::Spacing();
    
    if (m_injector->IsProcessFound()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        ImGui::Text("Rust Found: %ls (PID: %d)", 
            m_injector->GetProcessName().c_str(), m_injector->GetProcessId());
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
        ImGui::Text("Rust Not Found - Waiting for game to launch...");
        ImGui::PopStyleColor();
    }
    
    ImGui::EndChild();
    ImGui::Spacing();
    
    // Inject button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.05f, 0.05f, 1.0f));
    if (ImGui::Button("Inject DLL", ImVec2(200, 45))) {
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, m_dllPath, -1, nullptr, 0);
        if (wideLen <= 1) {
            m_statusMessage = "Invalid DLL path";
            AppendLog("[ERROR] Provided DLL path is empty or invalid");
        } else {
            std::wstring dllPathW(static_cast<size_t>(wideLen - 1), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, m_dllPath, -1, dllPathW.data(), wideLen);

            if (m_injector->FindProcess(L"Rust.exe")) {
                bool success = m_manualMap ?
                    m_injector->InjectManualMap(dllPathW) :
                    m_injector->InjectDLL(dllPathW);

                if (success) {
                    m_statusMessage = "Injection successful!";
                    AppendLog("[SUCCESS] DLL injected into Rust.exe");
                } else {
                    m_statusMessage = "Injection failed!";
                    AppendLog("[ERROR] DLL injection routine returned failure");
                }
            } else {
                m_statusMessage = "Rust.exe not found!";
                AppendLog("[WARN] Rust.exe process not found");
            }
        }
    }
    ImGui::PopStyleColor(3);
}

void CRustInjector::RenderDLLTab() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
    ImGui::Text("DLL Selection");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("DLLCard", ImVec2(0, 150), true);
    ImGui::PopStyleVar();
    
    ImGui::Text("DLL Path:");
    ImGui::Spacing();
    ImGui::InputText("##dllpath", m_dllPath, MAX_PATH);
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.05f, 0.05f, 1.0f));
    if (ImGui::Button("Browse...", ImVec2(150, 35))) {
        OPENFILENAMEW ofn = { 0 };
        wchar_t szFile[MAX_PATH] = { 0 };
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hwnd;
        ofn.lpstrFilter = L"DLL Files\0*.dll\0All Files\0*.*\0";
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        
        if (GetOpenFileNameW(&ofn)) {
            WideCharToMultiByte(CP_UTF8, 0, szFile, -1, m_dllPath, MAX_PATH, nullptr, nullptr);
        }
    }
    ImGui::PopStyleColor(3);
    
    ImGui::EndChild();
}

void CRustInjector::RenderOptionsTab() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
    ImGui::Text("Injection Options");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("OptionsCard", ImVec2(0, 200), true);
    ImGui::PopStyleVar();
    
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
    if (ImGui::Checkbox("Auto-inject on game launch", &m_autoInject)) {
        m_animationTimer = 0.0f;
    }
    ImGui::Spacing();
    
    if (ImGui::Checkbox("Use Manual Mapping (Stealth)", &m_manualMap)) {
        m_animationTimer = 0.0f;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Manual mapping is more stealthy but may be less stable");
    }
    ImGui::PopStyleColor();
    
    ImGui::EndChild();
}

void CRustInjector::RenderESPTab() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
    ImGui::Text("ESP Settings");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("ESPCard", ImVec2(0, 350), true);
    ImGui::PopStyleVar();
    
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
    
    // Enable ESP
    if (ImGui::Checkbox("Enable ESP", &m_espEnabled)) {
        m_animationTimer = 0.0f;
    }
    ImGui::Spacing();
    
    // Circle ESP
    if (ImGui::Checkbox("Circle ESP (Player Outline)", &m_espShowCircle)) {
        m_animationTimer = 0.0f;
    }
    ImGui::Spacing();
    
    // Inventory ESP
    if (ImGui::Checkbox("Inventory ESP (Show Items)", &m_espShowInventory)) {
        m_animationTimer = 0.0f;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Shows player inventory items at top of screen");
    }
    ImGui::Spacing();
    
    // Weapon Chams
    if (ImGui::Checkbox("Weapon Chams (Gun Coloring)", &m_espShowChams)) {
        m_animationTimer = 0.0f;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Applies color to weapons");
    }
    ImGui::Spacing();
    
    // Galaxy Mode
    if (ImGui::Checkbox("Galaxy Mode (Animated Galaxy Effect)", &m_espGalaxyMode)) {
        m_animationTimer = 0.0f;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Applies animated galaxy effect to weapons and player outline");
    }
    ImGui::Spacing();
    
    ImGui::Separator();
    ImGui::Spacing();
    
    // Max Distance Slider
    ImGui::Text("Max Distance:");
    ImGui::SliderFloat("##maxdist", &m_espMaxDistance, 100.0f, 1000.0f, "%.0fm");
    ImGui::Spacing();
    
    // Circle Radius Slider
    ImGui::Text("Circle Radius:");
    ImGui::SliderFloat("##circleradius", &m_espCircleRadius, 10.0f, 100.0f, "%.0f");
    ImGui::Spacing();
    
    ImGui::PopStyleColor();
    
    ImGui::EndChild();
}

void CRustInjector::RenderLogTab() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.15f, 0.15f, 1.0f));
    ImGui::Text("Activity Log");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::BeginChild("LogCard", ImVec2(0, 400), true);
    ImGui::PopStyleVar();
    
    for (const auto& entry : m_logMessages) {
        // Color code log entries
        if (entry.find("[SUCCESS]") != std::string::npos) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        } else if (entry.find("[ERROR]") != std::string::npos) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
        } else if (entry.find("[WARN]") != std::string::npos) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.2f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        }
        ImGui::TextUnformatted(entry.c_str());
        ImGui::PopStyleColor();
    }
    
    // Auto-scroll to bottom
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

LRESULT CALLBACK CRustInjector::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CRustInjector* injector = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        injector = reinterpret_cast<CRustInjector*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(injector));
    } else {
        injector = reinterpret_cast<CRustInjector*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    
    if (injector && ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
        return true;
    }
    
    switch (uMsg) {
        case WM_SIZE:
            if (injector && injector->m_device && wParam != SIZE_MINIMIZED) {
                injector->CleanupDirectX();
                injector->InitializeDirectX();
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CRustInjector::AppendLog(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::tm localTm{};
    localtime_s(&localTm, &timeT);

    std::ostringstream oss;
    oss << "[" << std::put_time(&localTm, "%H:%M:%S") << "] " << message;
    m_logMessages.emplace_back(oss.str());

    constexpr size_t kMaxLogEntries = 200;
    if (m_logMessages.size() > kMaxLogEntries) {
        m_logMessages.erase(m_logMessages.begin(), m_logMessages.end() - kMaxLogEntries);
    }
}

int CRustInjector::Run() {
    MSG msg = { 0 };
    while (m_running && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        RenderFrame();
    }
    return (int)msg.wParam;
}

void CRustInjector::Shutdown() {
    CleanupImGui();
    CleanupDirectX();
    m_running = false;
    m_initialized = false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CRustInjector injector;
    if (!injector.Initialize(hInstance)) {
        MessageBoxW(nullptr, L"Failed to initialize injector", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    return injector.Run();
}

#include "RustInjector.h"
#include <d3dcompiler.h>
#include <fstream>

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
    , m_running(false)
    , m_initialized(false)
    , m_autoInject(false)
    , m_manualMap(false)
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
    
    ApplyPurpleTheme();
    
    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_device, m_context);
    
    return true;
}

void CRustInjector::ApplyPurpleTheme() {
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
    
    ImGui::Render();
    
    float clearColor[4] = { 0.1f, 0.1f, 0.15f, 1.0f };
    m_context->OMSetRenderTargets(1, &m_renderTarget, nullptr);
    m_context->ClearRenderTargetView(m_renderTarget, clearColor);
    
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
    m_swapChain->Present(1, 0);
}

void CRustInjector::RenderUI() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("P Client - Rust Injector", nullptr, ImGuiWindowFlags_MenuBar);
    
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                m_running = false;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                m_statusMessage = "P Client v1.0 - Rust Injector";
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    
    ImGui::Spacing();
    
    // Status Section
    ImGui::SeparatorText("Status");
    ImGui::Text("Status: %s", m_statusMessage.c_str());
    
    if (m_injector->IsProcessFound()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Rust Found: %ls (PID: %d)", 
            m_injector->GetProcessName().c_str(), m_injector->GetProcessId());
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Rust Not Found - Waiting for game to launch...");
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // DLL Selection
    ImGui::SeparatorText("DLL Selection");
    ImGui::InputText("DLL Path", m_dllPath, MAX_PATH);
    if (ImGui::Button("Browse...")) {
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
    
    ImGui::Spacing();
    
    // Injection Options
    ImGui::SeparatorText("Injection Options");
    ImGui::Checkbox("Auto-inject on game launch", &m_autoInject);
    ImGui::Checkbox("Use Manual Mapping (Stealth)", &m_manualMap);
    ImGui::SameLine();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Manual mapping is more stealthy but may be less stable");
    }
    
    ImGui::Spacing();
    
    // Inject Button
    if (ImGui::Button("Inject DLL", ImVec2(200, 40))) {
        std::wstring dllPathW(m_dllPath, m_dllPath + strlen(m_dllPath));
        
        if (m_injector->FindProcess(L"Rust.exe")) {
            bool success = m_manualMap ? 
                m_injector->InjectManualMap(dllPathW) : 
                m_injector->InjectDLL(dllPathW);
            
            if (success) {
                m_statusMessage = "Injection successful!";
                m_logMessages.push_back('\0');
                m_logMessages.push_back((char*)"[SUCCESS] DLL injected into Rust.exe\n");
            } else {
                m_statusMessage = "Injection failed!";
                m_logMessages.push_back('\0');
                m_logMessages.push_back((char*)"[ERROR] Failed to inject DLL\n");
            }
        } else {
            m_statusMessage = "Rust.exe not found!";
            m_logMessages.push_back('\0');
            m_logMessages.push_back((char*)"[ERROR] Rust.exe process not found\n");
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Log Section
    ImGui::SeparatorText("Log");
    if (ImGui::BeginChild("Log", ImVec2(0, 200), true)) {
        for (int i = 0; i < m_logMessages.size; i++) {
            ImGui::TextUnformatted(&m_logMessages[i]);
        }
        ImGui::EndChild();
    }
    
    ImGui::End();
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

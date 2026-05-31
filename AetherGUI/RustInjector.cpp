#include "RustInjector.h"
#include <d3dcompiler.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <cmath>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

namespace {
    // Aether-style flat palette (purple accent on near-black surface)
    static const ImVec4 kAccent        = ImVec4(0.545f, 0.361f, 0.965f, 1.0f);  // #8B5CF6 violet
    static const ImVec4 kAccentSoft    = ImVec4(0.545f, 0.361f, 0.965f, 0.55f);
    static const ImVec4 kSurfaceDark   = ImVec4(0.102f, 0.122f, 0.149f, 1.0f);  // #1A1F26 panel
    static const ImVec4 kSurfaceDeep   = ImVec4(0.063f, 0.078f, 0.098f, 1.0f);  // #101419 outer
    static const ImVec4 kSurfaceRow    = ImVec4(0.137f, 0.157f, 0.184f, 1.0f);  // #23282F input
    static const ImVec4 kDivider       = ImVec4(0.165f, 0.192f, 0.227f, 1.0f);  // #2A313A line
    static const ImVec4 kTextBright    = ImVec4(0.902f, 0.910f, 0.918f, 1.0f);  // #E6E8EA
    static const ImVec4 kTextMuted     = ImVec4(0.564f, 0.596f, 0.643f, 1.0f);  // #9098A4
    static const ImVec4 kTextDim       = ImVec4(0.40f, 0.44f, 0.49f, 1.0f);
}

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
    , m_configLoaded(false)
    , m_configPath("config.json")
    , m_soundEnabled(true)
    , m_soundVolume(0.5f)
    , m_showFPS(true)
    , m_showVersion(true)
    , m_minimizeToTray(false)
    , m_autoInjectOnLaunch(false)
    , m_injectionDelay(0.0f)
    , m_autoInject(false)
    , m_manualMap(false)
    , m_windowPos(ImVec2(100, 100))
    , m_windowSize(ImVec2(1200, 700))
    , m_isDragging(false)
    , m_isResizing(false)
    , m_dragOffset(ImVec2(0, 0))
    , m_resizeEdge(0)
    , m_minWindowSize(ImVec2(800, 600))
    , m_maxWindowSize(ImVec2(1920, 1080))
    , m_tooltipDelay(0.3f)
    , m_fontHeader(nullptr)
    , m_fontNormal(nullptr)
    , m_fontSmall(nullptr)
    , m_fontBold(nullptr)
    , m_globalAnimTime(0.0f)
    , m_lastFrameTime(0.0f)
    , m_espEnabled(true)
    , m_espShowCircle(true)
    , m_espShowInventory(true)
    , m_espShowChams(true)
    , m_espGalaxyMode(false)
    , m_espMaxDistance(500.0f)
    , m_espCircleRadius(30.0f)
    , m_espPlayerBox(false)
    , m_espPlayerSkeleton(false)
    , m_espPlayerHealth(true)
    , m_espPlayerName(true)
    , m_espPlayerDistance(true)
    , m_espPlayerWeapon(false)
    , m_espPlayerTeam(true)
    , m_espPlayerWounded(true)
    , m_espPlayerDead(true)
    , m_espTeamIndicator(true)
    , m_espShowLoot(true)
    , m_espShowResources(true)
    , m_espShowWeapons(true)
    , m_espShowTools(true)
    , m_espShowAmmo(true)
    , m_espShowMedical(true)
    , m_espShowNPCs(false)
    , m_espShowAnimals(false)
    , m_espShowVehicles(false)
    , m_espShowHelicopter(false)
    , m_espShowTurrets(false)
    , m_espTracerBulletPath(false)
    , m_espTracerLineOfSight(false)
    , m_espTracerTargetLine(false)
    , m_espTracerThickness(1.0f)
    , m_espTracerOpacity(1.0f)
    , m_noNightEnabled(false)
    , m_purpleSkyEnabled(false)
    , m_purpleSkyIntensity(0.5f)
    , m_galaxySkyEffect(false)
    , m_antiAFKEnabled(false)
    , m_antiAFKInterval(30.0f)
    , m_antiAFKAction(0)
    , m_autoHealEnabled(false)
    , m_autoHealThreshold(50.0f)
    , m_speedhackEnabled(false)
    , m_speedhackMultiplier(1.5f)
    , m_flyHackEnabled(false)
    , m_flyVerticalSpeed(5.0f)
    , m_flyHorizontalSpeed(10.0f)
    , m_noClipEnabled(false)
    , m_noClipSpeed(10.0f)
    , m_bunnyHopEnabled(false)
    , m_bunnyHopAutoJump(true)
    , m_bunnyHopTiming(0.1f)
    , m_chamsEnabled(false)
    , m_chamsOpacity(1.0f)
    , m_chamsStyle(0)
    , m_chamsThroughWalls(false)
    , m_glowEnabled(false)
    , m_glowIntensity(0.5f)
    , m_glowRadius(10.0f)
    , m_glowPlayers(true)
    , m_glowItems(false)
    , m_glowEntities(false)
    , m_ambientLight(1.0f)
    , m_fogControl(false)
    , m_fogDensity(0.5f)
    , m_nightModeRemoval(false)
    , m_aimbotEnabled(false)
    , m_aimbotFOV(90.0f)
    , m_aimbotSmoothness(5.0f)
    , m_aimbotVisibilityCheck(true)
    , m_aimbotTargetBone(0)
    , m_aimbotTargetPriority(0)
    , m_noRecoilEnabled(false)
    , m_noSpreadEnabled(false)
    , m_rapidFireEnabled(false)
    , m_rapidFireDelay(50.0f)
{
    strcpy_s(m_dllPath, "EnhancedProject.dll");
    m_statusMessage = "Ready - Waiting for Rust process...";
    
    m_selectedPage = MenuPage::Misc;
    m_miscLocal = {};
    m_miscLocal.bulletSize = 0.0f;
    m_miscLocal.handChamsColor[0] = 0.0f; m_miscLocal.handChamsColor[1] = 0.0f; m_miscLocal.handChamsColor[2] = 0.0f;
    m_miscLocal.gunChamsColor[0]  = 0.0f; m_miscLocal.gunChamsColor[1]  = 0.0f; m_miscLocal.gunChamsColor[2]  = 0.0f;
    m_miscLocal.handChamType = 0;
    m_miscLocal.gunChamType  = 0;
    m_miscLocal.hitEffect    = 0;

    m_miscAtmosphere = {};
    m_miscAtmosphere.time = 0.0f;
    m_miscAtmosphere.starSize = 0.0f;
    m_miscAtmosphere.rayleighAmount = 0.0f;
    m_miscAtmosphere.skyColor[0] = 0.0f; m_miscAtmosphere.skyColor[1] = 0.0f; m_miscAtmosphere.skyColor[2] = 0.0f;
    m_miscAtmosphere.cloudColor[0] = 0.0f; m_miscAtmosphere.cloudColor[1] = 0.0f; m_miscAtmosphere.cloudColor[2] = 0.0f;
    m_miscAtmosphere.sunMoonColorRGB[0] = 0.0f; m_miscAtmosphere.sunMoonColorRGB[1] = 0.0f; m_miscAtmosphere.sunMoonColorRGB[2] = 0.0f;
    m_selectedAimbotTab = AimbotTab::General;
    m_selectedVisualsTab = VisualsTab::Players;
    m_selectedWorldTab = WorldTab::Resources;
    m_selectedMiscTab = MiscTab::Movement;
    m_selectedCloudTab = CloudTab::Profiles;
    
    m_aimbotGeneral = {
        false,   // enabled
        true,    // silentAim
        true,    // prediction
        true,    // smoothing
        false,   // stickyAim
        true,    // drawFov
        false,   // predictionDot
        false,   // autoShoot
        false,   // waitForPowershot
        false,   // targetTeam
        true,    // visibleCheck
        90.0f,   // fovRadius
        65.0f,   // hitChance
        6.0f,    // smoothness
        0,       // aimType
        0,       // targetFilter
        0,       // targetBone
        1,       // reticleStyle
        false,   // alwaysAutomatic
        false,   // instantEoka
        false,   // instantBow
        false,   // shotgunNospead
        false,   // thickBullet
        false,   // bowThickOverride
        false,   // fastBullet
        3.5f,    // thickBulletRange
        ImGuiKey_MouseRight, // aimKey
        false    // listeningForAimKey
    };
    
    m_aimbotWeaponProfiles = {
        { "AK-47", true, 80.0f, 70.0f, 12.0f, 30.0f },
        { "LR-300", true, 70.0f, 60.0f, 10.0f, 25.0f },
        { "Thompson", false, 55.0f, 45.0f, 8.0f, 20.0f },
        { "Bolt Action", true, 20.0f, 15.0f, 24.0f, 60.0f }
    };
    m_selectedWeaponProfile = 0;
    
    m_visualPlayer = {
        true,  // enabled
        true,  // box2D
        false, // box3D
        true,  // skeleton
        true,  // healthBar
        true,  // showName
        true,  // showDistance
        true,  // showWeapon
        true,  // showAmmo
        true,  // sleeperESP
        true,  // chams
        0.65f, // chamsGlow
        true,  // showTeam
        true,  // showWounded
        true,  // showDead
        true   // teamIndicator
    };
    
    m_visualOverride = {
        true,  // enableGlow
        0.8f,  // glowIntensity
        true,  // enableOutlines
        1.5f,  // outlineThickness
        true,  // customReticle
        2      // reticleType
    };

    m_visualNpc = {
        true,  // enabled
        true,  // showName
        true,  // box
        false, // fill
        true,  // distance
        true,  // chams
        0.75f  // redGlow
    };
    
    m_worldResources = {
        true,  // sulfurNodes
        true,  // metalNodes
        true,  // stoneNodes
        250.0f // resourceDistance
    };
    
    m_worldLoot = {
        true,  // eliteCrates
        true,  // militaryCrates
        true,  // airDrops
        true,  // lockedCrates
        true,  // stashESP
        200.0f // lootDistance
    };
    
    m_worldBase = {
        true,  // toolCupboard
        true,  // authorizedPlayers
        true   // baseOutline
    };
    
    m_worldTraps = {
        true,  // landMines
        true,  // bearTraps
        true,  // autoTurrets
        true,  // flameTurrets
        true   // showRange
    };
    
    m_movement = {
        false, // spiderMan
        false, // flyHack
        false, // infiniteJump
        2.5f,  // flySpeed
        0.75f  // jumpSafety
    };
    
    m_exploits = {
        false, // thickBullets
        1.5f,  // bulletScale
        false, // noFallDamage
        false, // instantLoot
        false  // vehicleBoost
    };
    
    m_automation = {
        false, // autoFarm
        false, // autoPickup
        false, // autoHeal
        45.0f, // healThreshold
        false  // autoCraft
    };
    
    m_cloud.profiles = {
        { "Legit", true },
        { "Rage", false },
        { "Stream", false }
    };
    m_cloud.selectedProfile = 0;
    strcpy_s(m_cloud.newProfileName, "New Config");
    m_cloud.importString.clear();
    m_cloud.exportString = "LEGIT-BASE64-STRING";
    m_cloud.streamProof = false;
    m_cloud.menuKey = ImGuiKey_Insert;
    m_cloud.listeningForMenuKey = false;
    memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
    m_moduleSelectAnim.value = 1.0f;
    m_moduleSelectAnim.targetValue = 1.0f;
    m_moduleSelectAnim.active = false;
    m_contentFadeAnim.value = 1.0f;
    m_contentFadeAnim.targetValue = 1.0f;
    m_contentFadeAnim.active = false;
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
    
    RECT rect = { 0, 0, (LONG)m_windowSize.x, (LONG)m_windowSize.y };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    m_hwnd = CreateWindowExW(
        0, L"RustInjectorClass", L"P Client - Rust Injector",
        WS_OVERLAPPEDWINDOW,
        (int)m_windowPos.x, (int)m_windowPos.y,
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
    
    // Load fonts with different sizes and weights
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2;
    fontConfig.OversampleV = 2;
    fontConfig.PixelSnapH = false;
    
    // Try to load Inter font from Windows fonts or fallback to Segoe UI
    const char* fontPaths[] = {
        "C:\\Windows\\Fonts\\Inter-Regular.ttf",
        "C:\\Windows\\Fonts\\Inter.ttc",
        "C:\\Windows\\Fonts\\SegoeUIVariable.ttf",
        "C:\\Windows\\Fonts\\segoeuivar.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
        "MenuPath\\imgui\\misc\\fonts\\Roboto-Medium.ttf"
    };
    
    for (const char* fontPath : fontPaths) {
        m_fontHeader = io.Fonts->AddFontFromFileTTF(fontPath, 18.0f, &fontConfig);
        if (m_fontHeader) break;
    }
    if (!m_fontHeader) {
        m_fontHeader = io.Fonts->AddFontDefault();
    }
    
    for (const char* fontPath : fontPaths) {
        m_fontNormal = io.Fonts->AddFontFromFileTTF(fontPath, 14.0f, &fontConfig);
        if (m_fontNormal) break;
    }
    if (!m_fontNormal) {
        m_fontNormal = io.Fonts->AddFontDefault();
    }
    
    for (const char* fontPath : fontPaths) {
        m_fontSmall = io.Fonts->AddFontFromFileTTF(fontPath, 12.0f, &fontConfig);
        if (m_fontSmall) break;
    }
    if (!m_fontSmall) {
        m_fontSmall = io.Fonts->AddFontDefault();
    }
    
    for (const char* fontPath : fontPaths) {
        m_fontBold = io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, &fontConfig);
        if (m_fontBold) break;
    }
    if (!m_fontBold) {
        m_fontBold = io.Fonts->AddFontDefault();
    }
    
    // Set default font
    io.FontDefault = m_fontNormal;
    
    ApplyDarkTheme();
    
    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_device, m_context);
    
    return true;
}

void CRustInjector::ApplyDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Flat Disconnect-style background colors
    ImVec4 darkBg = kSurfaceDark;
    ImVec4 darkerBg = kSurfaceDeep;
    ImVec4 cardBg = kSurfaceDark;
    
    // Cyan accent colors
    ImVec4 pinkAccent = kAccent;
    ImVec4 pinkHover = ImVec4(0.20f, 0.62f, 1.0f, 1.0f);
    ImVec4 pinkActive = ImVec4(0.06f, 0.46f, 0.88f, 1.0f);
    
    // Text colors
    ImVec4 textMain = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    ImVec4 textDim = ImVec4(0.5f, 0.5f, 0.6f, 1.0f);
    
    // Larger rounded corners for modern look
    style.WindowRounding = 12.0f;
    style.ChildRounding = 10.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 8.0f;
    
    // More spacing for cleaner look
    style.WindowPadding = ImVec2(0, 0);
    style.FramePadding = ImVec2(15, 12);
    style.ItemSpacing = ImVec2(12, 12);
    style.ItemInnerSpacing = ImVec2(10, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 8.0f;
    style.GrabMinSize = 8.0f;
    
    // Window colors
    style.Colors[ImGuiCol_WindowBg] = darkBg;
    style.Colors[ImGuiCol_ChildBg] = cardBg;
    style.Colors[ImGuiCol_PopupBg] = darkerBg;
    style.Colors[ImGuiCol_Border] = ImVec4(0.15f, 0.15f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Frame colors
    style.Colors[ImGuiCol_FrameBg] = darkerBg;
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = pinkAccent;
    
    // Title bar
    style.Colors[ImGuiCol_TitleBg] = darkerBg;
    style.Colors[ImGuiCol_TitleBgActive] = darkerBg;
    style.Colors[ImGuiCol_TitleBgCollapsed] = darkerBg;
    
    // Menu bar
    style.Colors[ImGuiCol_MenuBarBg] = darkerBg;
    
    // Scrollbar
    style.Colors[ImGuiCol_ScrollbarBg] = darkerBg;
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = pinkAccent;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = pinkActive;
    
    // Check mark
    style.Colors[ImGuiCol_CheckMark] = pinkAccent;
    
    // Slider
    style.Colors[ImGuiCol_SliderGrab] = pinkAccent;
    style.Colors[ImGuiCol_SliderGrabActive] = pinkActive;
    
    // Button
    style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = pinkAccent;
    
    // Header
    style.Colors[ImGuiCol_Header] = darkerBg;
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = pinkAccent;
    
    // Selection
    style.Colors[ImGuiCol_Separator] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = pinkAccent;
    
    // Resize grip
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.15f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] = pinkAccent;
    style.Colors[ImGuiCol_ResizeGripActive] = pinkActive;
    
    // Text
    style.Colors[ImGuiCol_Text] = textMain;
    style.Colors[ImGuiCol_TextDisabled] = textDim;
    
    // Tab
    style.Colors[ImGuiCol_Tab] = darkerBg;
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = pinkAccent;
    style.Colors[ImGuiCol_TabUnfocused] = darkerBg;
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    
    // Docking
    style.Colors[ImGuiCol_DockingPreview] = pinkAccent;
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
    
    // Calculate delta time for animations
    auto currentTime = std::chrono::high_resolution_clock::now();
    static auto lastTime = currentTime;
    float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;
    
    // Update animations
    UpdateAnimations(deltaTime);
    
    // Update tooltip
    UpdateTooltip(deltaTime);
    
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
    
    // Render tooltip on top
    RenderTooltip();
    
    float clearColor[4] = { 0.1f, 0.1f, 0.15f, 1.0f };
    m_context->OMSetRenderTargets(1, &m_renderTarget, nullptr);
    m_context->ClearRenderTargetView(m_renderTarget, clearColor);
    
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
    m_swapChain->Present(1, 0);
}

void CRustInjector::RenderUI() {
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(m_windowSize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, kSurfaceDark);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::Begin("P Client - Rust Injector", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);

    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Flat dark fill (no gradient)
    drawList->AddRectFilled(windowPos,
        ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
        ImColor(kSurfaceDark));

    // Layout constants matching screenshot
    const float kRailWidth = 64.0f;
    const float kTabHeight = 46.0f;

    // Left icon rail
    RenderLeftIconRail();

    // Top tab bar (Players / World / Misc) sits to the right of rail
    ImGui::SetCursorPos(ImVec2(kRailWidth, 0.0f));
    RenderFlatTopTabs();

    // Divider under tab bar
    drawList->AddLine(
        ImVec2(windowPos.x + kRailWidth, windowPos.y + kTabHeight),
        ImVec2(windowPos.x + windowSize.x, windowPos.y + kTabHeight),
        ImColor(kDivider), 1.0f);
    // Vertical divider between rail and content
    drawList->AddLine(
        ImVec2(windowPos.x + kRailWidth, windowPos.y),
        ImVec2(windowPos.x + kRailWidth, windowPos.y + windowSize.y),
        ImColor(kDivider), 1.0f);

    // Content area
    ImVec2 contentPos = ImVec2(kRailWidth, kTabHeight);
    ImVec2 contentSize = ImVec2(windowSize.x - kRailWidth, windowSize.y - kTabHeight);
    ImGui::SetCursorPos(contentPos);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f + 0.6f * m_contentFadeAnim.value);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kSurfaceDark);
    ImGui::BeginChild("ContentArea", contentSize, false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

    switch (m_selectedPage) {
        case MenuPage::Aimbot:   RenderAimbotPage(); break;
        case MenuPage::Visuals:  RenderVisualsPage(); break;
        case MenuPage::World:    RenderWorldPage(); break;
        case MenuPage::Misc:     RenderMiscPage(); break;
        case MenuPage::Cloud:    RenderCloudPage(); break;
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    ImGui::End();
}

void CRustInjector::RenderTopNavigation() {
    struct NavItem {
        MenuPage page;
        const char* label;
    } navItems[] = {
        { MenuPage::Aimbot, "Aimbot" },
        { MenuPage::Visuals, "Visuals" },
        { MenuPage::Misc, "Misc" },
        { MenuPage::World, "World" },
        { MenuPage::Cloud, "Settings" }
    };

    const float navHeight = 68.0f;
    ImVec2 localStart = ImGui::GetCursorPos();
    ImVec2 start = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;

    ImGui::Dummy(ImVec2(width, navHeight));

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 end = ImVec2(start.x + width, start.y + navHeight);
    drawList->AddRectFilled(start, end, ImColor(0.10f, 0.10f, 0.15f, 0.92f), 26.0f, ImDrawFlags_RoundCornersTop);
    drawList->AddRect(start, end, ImColor(0.30f, 0.18f, 0.36f, 0.65f), 26.0f, ImDrawFlags_RoundCornersTop, 1.0f);
    drawList->AddRectFilled(ImVec2(start.x, end.y - 3.0f), end, ImColor(kAccent.x, kAccent.y, kAccent.z, 0.9f));

    ImU32 textDim = ImColor(0.70f, 0.70f, 0.82f, 1.0f);

    ImVec2 brandSize = m_fontHeader
        ? ImVec2(m_fontHeader->CalcTextSizeA(m_fontHeader->FontSize, FLT_MAX, 0.0f, "disconnect.wtf").x, m_fontHeader->FontSize)
        : ImGui::CalcTextSize("disconnect.wtf");
    ImVec2 brandPos = ImVec2(start.x + 34.0f, start.y + (navHeight - brandSize.y) * 0.5f - 4.0f);
    if (m_fontHeader) {
        drawList->AddText(m_fontHeader, m_fontHeader->FontSize, brandPos, ImColor(0.94f, 0.90f, 1.0f, 1.0f), "disconnect.wtf");
    } else {
        drawList->AddText(brandPos, ImColor(0.94f, 0.90f, 1.0f, 1.0f), "disconnect.wtf");
    }
    ImVec2 taglinePos = ImVec2(brandPos.x, brandPos.y + brandSize.y - 6.0f);
    drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.9f, taglinePos, textDim, "external");

    float cursorX = brandPos.x + brandSize.x + 90.0f;
    int hoveredIndex = -1;

    for (const auto& item : navItems) {
        int idx = static_cast<int>(item.page);
        const char* label = item.label;
        ImVec2 textSize = ImGui::CalcTextSize(label);
        float buttonWidth = textSize.x + 36.0f;

        ImVec2 btnPos = ImVec2(cursorX, start.y);
        ImGui::SetCursorScreenPos(btnPos);
        ImGui::PushID(label);
        ImGui::InvisibleButton("nav_btn", ImVec2(buttonWidth, navHeight));
        bool hovered = ImGui::IsItemHovered();
        bool selected = (m_selectedPage == item.page);

        if (selected) {
            m_iconHoverAnim[idx].value = 1.0f;
            m_iconHoverAnim[idx].targetValue = 1.0f;
        } else if (hovered) {
            StartIconHoverAnim(idx);
            hoveredIndex = idx;
        } else {
            m_iconHoverAnim[idx].targetValue = 0.0f;
            m_iconHoverAnim[idx].speed = 4.0f;
            m_iconHoverAnim[idx].active = true;
        }

        float glow = selected ? 1.0f : m_iconHoverAnim[idx].value;
        ImU32 col = ImColor(0.80f + glow * 0.18f, 0.78f, 0.94f + glow * 0.08f, 1.0f);

        ImVec2 textPos = ImVec2(btnPos.x + 18.0f, btnPos.y + (navHeight - textSize.y) * 0.5f - 1.0f);
        drawList->AddText(textPos, col, label);

        if (selected || hovered) {
            float thickness = selected ? 4.0f : 2.0f;
            float alpha = selected ? 0.95f : 0.65f;
            ImVec2 underlineStart = ImVec2(btnPos.x + 12.0f, end.y - 6.0f);
            ImVec2 underlineEnd = ImVec2(btnPos.x + buttonWidth - 12.0f, end.y - 6.0f);
            drawList->AddLine(underlineStart, underlineEnd, ImColor(kAccent.x, kAccent.y, kAccent.z, alpha), thickness);
        }

        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !selected) {
            m_selectedPage = item.page;
            StartModuleSelectAnim();
            StartContentFadeAnim();
        }

        ImGui::PopID();
        cursorX += buttonWidth + 16.0f;
    }

    if (hoveredIndex == -1) {
        for (int i = 0; i < 5; ++i) {
            if (m_selectedPage == static_cast<MenuPage>(i)) continue;
            m_iconHoverAnim[i].targetValue = 0.0f;
            m_iconHoverAnim[i].speed = 4.0f;
            m_iconHoverAnim[i].active = true;
        }
    }

    ImGui::SetCursorPos(ImVec2(localStart.x, localStart.y + navHeight + 22.0f));
}

void CRustInjector::RenderPageHeader(const PageHeaderData& data) {
    ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_NoPadOuterX;
    if (ImGui::BeginTable("PageHeaderTable", 2, flags)) {
        ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch, 3.0f);
        ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthFixed, 280.0f);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        if (m_fontHeader) ImGui::PushFont(m_fontHeader);
        ImGui::TextColored(kTextBright, "%s", data.title);
        if (m_fontHeader) ImGui::PopFont();
        ImGui::TextColored(kTextMuted, "%s", data.subtitle);
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.95f, 1.0f), "%s", data.narrativeTitle);
        ImGui::TextColored(ImVec4(0.68f, 0.70f, 0.82f, 1.0f), "%s", data.narrativeBody);
        ImGui::Spacing();
        RenderBulletList(data.bulletPoints);

        ImGui::TableSetColumnIndex(1);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.09f, 0.09f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.12f, 0.12f, 0.16f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(kAccent.x, kAccent.y, kAccent.z, 0.25f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.0f, 10.0f));
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##global_search", "Find something...", m_searchBuffer, IM_ARRAYSIZE(m_searchBuffer));
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

static void RenderTabButton(const char* label, bool active, std::function<void()> onClick) {
    ImGui::PushStyleColor(ImGuiCol_Button, active ? ImVec4(1.0f, 0.0f, 0.4f, 0.25f) : ImVec4(0.12f, 0.12f, 0.18f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.4f, active ? 0.35f : 0.4f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.4f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0f, 10.0f));
    if (ImGui::Button(label)) {
        if (onClick) onClick();
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

void CRustInjector::RenderAimbotPage() {
    // Synchronize struct view with legacy state before rendering
    m_aimbotGeneral.enabled = m_aimbotEnabled;
    m_aimbotGeneral.fovRadius = m_aimbotFOV;
    m_aimbotGeneral.smoothness = m_aimbotSmoothness;
    m_aimbotGeneral.targetBone = m_aimbotTargetBone;
    m_aimbotGeneral.targetFilter = m_aimbotTargetPriority;
    m_aimbotGeneral.visibleCheck = m_aimbotVisibilityCheck;

    if (ImGui::BeginTable("AimbotLayout", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoBordersInBody)) {
        ImGui::TableNextColumn();
        RenderSettingsCard("Aimbot", "Lock-on accuracy with ruthless precision", ImVec2(0.0f, 0.0f), [this]() {
            RenderToggleSwitch("✓", "Enable Module", &m_aimbotGeneral.enabled);
            RenderToggleSwitch("✓", "Silent Aim", &m_aimbotGeneral.silentAim);
            RenderToggleSwitch("✓", "Bullet Prediction", &m_aimbotGeneral.prediction);
            RenderToggleSwitch("✓", "Auto Shoot", &m_aimbotGeneral.autoShoot);
            RenderToggleSwitch("✓", "Target Team", &m_aimbotGeneral.targetTeam);
            RenderToggleSwitch("✓", "Visible Check", &m_aimbotGeneral.visibleCheck);
            RenderToggleSwitch("✓", "Sticky Tracking", &m_aimbotGeneral.stickyAim);
            RenderThinSlider("◎", "Aim FOV", &m_aimbotGeneral.fovRadius, 5.0f, 250.0f, "%.0f°");
            RenderThinSlider("%", "Hit Chance", &m_aimbotGeneral.hitChance, 0.0f, 100.0f, "%.0f%%");
            RenderThinSlider("↻", "Smoothness", &m_aimbotGeneral.smoothness, 0.5f, 20.0f, "%.1f");
            RenderToggleSwitch("⚡", "Recoil Control", &m_noRecoilEnabled);
            RenderToggleSwitch("⚡", "No Spread", &m_noSpreadEnabled);
            RenderToggleSwitch("⚡", "Rapid Fire", &m_rapidFireEnabled);
            RenderThinSlider("⏱", "Rapid Delay", &m_rapidFireDelay, 10.0f, 200.0f, "%.0fms");
            RenderKeybindInput("⌨", "Aim Key", &m_aimbotGeneral.aimKey, &m_aimbotGeneral.listeningForAimKey);
        });

        ImGui::TableNextColumn();
        RenderSettingsCard("Weapon Lab", "Tune projectiles & ballistic overrides", ImVec2(0.0f, 0.0f), [this]() {
            RenderToggleSwitch("⚙", "Always Automatic", &m_aimbotGeneral.alwaysAutomatic);
            RenderToggleSwitch("⚙", "Instant Eoka", &m_aimbotGeneral.instantEoka);
            RenderToggleSwitch("⚙", "Instant Bow", &m_aimbotGeneral.instantBow);
            RenderToggleSwitch("⚙", "Shotgun Nospread", &m_aimbotGeneral.shotgunNospead);
            RenderToggleSwitch("⚙", "Thick Bullet", &m_aimbotGeneral.thickBullet);
            RenderThinSlider("⌀", "Thick Bullet Range", &m_aimbotGeneral.thickBulletRange, 1.0f, 10.0f, "%.1fm");
            RenderToggleSwitch("⚙", "Bow Thick Override", &m_aimbotGeneral.bowThickOverride);
            RenderToggleSwitch("⚙", "Fast Bullet", &m_aimbotGeneral.fastBullet);
            RenderToggleSwitch("⚙", "Wait for Powershot", &m_aimbotGeneral.waitForPowershot);
            RenderInfoText("ℹ", "Weapon automation mirrors the Disconnect External presets shown on the product page.", ImVec4(0.6f, 0.6f, 0.75f, 1.0f));
        });

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        RenderSettingsCard("Target Suite", "Select priority logic & reticle style", ImVec2(0.0f, 0.0f), [this]() {
            const char* aimTypes[] = { "Field Lock", "Priority Cycle", "Smart Predict" };
            const char* filters[] = { "Visible Only", "Closest Distance", "Threat Level" };
            const char* bones[] = { "Head", "Chest", "Stomach" };
            const char* reticles[] = { "Dot", "Crosshair", "Hybrid" };
            RenderDropdown("🎯", "Aim Type", &m_aimbotGeneral.aimType, aimTypes, IM_ARRAYSIZE(aimTypes));
            RenderDropdown("☍", "Target Filter", &m_aimbotGeneral.targetFilter, filters, IM_ARRAYSIZE(filters));
            RenderDropdown("⚙", "Target Bone", &m_aimbotGeneral.targetBone, bones, IM_ARRAYSIZE(bones));
            RenderToggleSwitch("◎", "Draw FOV", &m_aimbotGeneral.drawFov);
            RenderToggleSwitch("◎", "Prediction Dot", &m_aimbotGeneral.predictionDot);
            RenderDropdown("✧", "Reticle Style", &m_aimbotGeneral.reticleStyle, reticles, IM_ARRAYSIZE(reticles));
        });

        ImGui::TableNextColumn();
        RenderSettingsCard("Weapon Profiles", "Per-weapon recoil & trigger tuning", ImVec2(0.0f, 0.0f), [this]() {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.09f, 0.09f, 0.13f, 0.95f));
            ImGui::BeginChild("WeaponProfileList", ImVec2(220.0f, 180.0f), true, ImGuiWindowFlags_NoScrollbar);
            for (size_t idx = 0; idx < m_aimbotWeaponProfiles.size(); ++idx) {
                bool selected = (m_selectedWeaponProfile == static_cast<int>(idx));
                ImGui::PushID(static_cast<int>(idx));
                if (ImGui::Selectable(m_aimbotWeaponProfiles[idx].name.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0.0f, 32.0f))) {
                    m_selectedWeaponProfile = static_cast<int>(idx);
                    StartContentFadeAnim();
                }
                ImGui::PopID();
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);

            ImGui::Spacing();
            if (ImGui::Button("+ Add Profile", ImVec2(-1.0f, 32.0f))) {
                AimbotWeaponProfile profile;
                profile.name = "Custom Weapon";
                profile.enabled = true;
                profile.recoilControl = 50.0f;
                profile.spreadControl = 50.0f;
                profile.predictionOffset = 10.0f;
                profile.triggerDelay = 30.0f;
                m_aimbotWeaponProfiles.push_back(profile);
                m_selectedWeaponProfile = static_cast<int>(m_aimbotWeaponProfiles.size()) - 1;
            }

            if (!m_aimbotWeaponProfiles.empty()) {
                m_selectedWeaponProfile = std::clamp(m_selectedWeaponProfile, 0, static_cast<int>(m_aimbotWeaponProfiles.size()) - 1);
                AimbotWeaponProfile& profile = m_aimbotWeaponProfiles[static_cast<size_t>(m_selectedWeaponProfile)];
                ImGui::Spacing();
                RenderToggleSwitch("✓", "Enable Profile", &profile.enabled);
                RenderThinSlider("%", "Recoil Control", &profile.recoilControl, 0.0f, 100.0f, "%.0f%%");
                RenderThinSlider("%", "Spread Control", &profile.spreadControl, 0.0f, 100.0f, "%.0f%%");
                RenderThinSlider("✶", "Prediction Offset", &profile.predictionOffset, 0.0f, 30.0f, "%.0fpx");
                RenderThinSlider("⏱", "Trigger Delay", &profile.triggerDelay, 0.0f, 120.0f, "%.0fms");
            }
        });

        ImGui::EndTable();
    }

    // Propagate struct edits back to legacy state for downstream systems
    m_aimbotEnabled = m_aimbotGeneral.enabled;
    m_aimbotFOV = m_aimbotGeneral.fovRadius;
    m_aimbotSmoothness = m_aimbotGeneral.smoothness;
    m_aimbotTargetBone = m_aimbotGeneral.targetBone;
    m_aimbotTargetPriority = m_aimbotGeneral.targetFilter;
    m_aimbotVisibilityCheck = m_aimbotGeneral.visibleCheck;
}

void CRustInjector::RenderVisualsPage() {
    // Sync structs with legacy state
    m_visualPlayer.enabled = m_espEnabled;
    m_visualPlayer.box2D = m_espPlayerBox;
    m_visualPlayer.skeleton = m_espPlayerSkeleton;
    m_visualPlayer.healthBar = m_espPlayerHealth;
    m_visualPlayer.showName = m_espPlayerName;
    m_visualPlayer.showDistance = m_espPlayerDistance;
    m_visualPlayer.showWeapon = m_espPlayerWeapon;
    m_visualPlayer.showAmmo = m_espShowAmmo;
    m_visualPlayer.showTeam = m_espPlayerTeam;
    m_visualPlayer.showWounded = m_espPlayerWounded;
    m_visualPlayer.showDead = m_espPlayerDead;
    m_visualPlayer.teamIndicator = m_espTeamIndicator;
    m_visualPlayer.chams = m_chamsEnabled;
    m_visualPlayer.chamsGlow = m_chamsOpacity;

    m_visualOverride.enableGlow = m_glowEnabled;
    m_visualOverride.glowIntensity = m_glowIntensity;
    m_visualNpc.enabled = m_espShowNPCs;

    if (ImGui::BeginTable("VisualLayout", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody)) {
        ImGui::TableNextColumn();
        RenderSettingsCard("Player", "Enemy silhouettes & telemetry", ImVec2(0.0f, 0.0f), [this]() {
            RenderToggleSwitch("◎", "Enable Player ESP", &m_visualPlayer.enabled);
            RenderToggleSwitch("☑", "2D Boxes", &m_visualPlayer.box2D);
            RenderToggleSwitch("☑", "3D Boxes", &m_visualPlayer.box3D);
            RenderToggleSwitch("☑", "Skeleton", &m_visualPlayer.skeleton);
            RenderToggleSwitch("❤", "Health Bar", &m_visualPlayer.healthBar);
            RenderToggleSwitch("✎", "Name ESP", &m_visualPlayer.showName);
            RenderToggleSwitch("📏", "Distance", &m_visualPlayer.showDistance);
            RenderToggleSwitch("⚔", "Weapon", &m_visualPlayer.showWeapon);
            RenderToggleSwitch("✶", "Ammo Counter", &m_visualPlayer.showAmmo);
            RenderToggleSwitch("👥", "Show Team", &m_visualPlayer.showTeam);
            RenderToggleSwitch("☄", "Show Wounded", &m_visualPlayer.showWounded);
            RenderToggleSwitch("☠", "Show Dead", &m_visualPlayer.showDead);
            RenderToggleSwitch("☄", "Sleeper ESP", &m_visualPlayer.sleeperESP);
            RenderToggleSwitch("⚑", "Team Indicator", &m_visualPlayer.teamIndicator);
        });

        ImGui::TableNextColumn();
        RenderSettingsCard("Awareness", "Chams, glow, and outline blending", ImVec2(0.0f, 0.0f), [this]() {
            RenderToggleSwitch("✧", "Neon Chams", &m_visualPlayer.chams);
            RenderThinSlider("✶", "Chams Glow", &m_visualPlayer.chamsGlow, 0.0f, 1.5f, "%.2f");
            RenderToggleSwitch("✧", "Global Glow", &m_visualOverride.enableGlow);
            RenderThinSlider("✧", "Glow Intensity", &m_visualOverride.glowIntensity, 0.0f, 1.5f, "%.2f");
            RenderToggleSwitch("☑", "Outline Shader", &m_visualOverride.enableOutlines);
            RenderThinSlider("▮", "Outline Thickness", &m_visualOverride.outlineThickness, 0.5f, 4.0f, "%.1f");
            RenderToggleSwitch("◎", "Custom Reticle", &m_visualOverride.customReticle);
            const char* reticles[] = { "Dot", "Chevron", "Holo" };
            RenderDropdown("◎", "Reticle Preset", &m_visualOverride.reticleType, reticles, IM_ARRAYSIZE(reticles));
        });

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        RenderSettingsCard("NPC", "Robotics & wildlife silhouette control", ImVec2(0.0f, 0.0f), [this]() {
            RenderToggleSwitch("◎", "Enable NPC ESP", &m_visualNpc.enabled);
            RenderToggleSwitch("☑", "Name Tag", &m_visualNpc.showName);
            RenderToggleSwitch("☑", "Box", &m_visualNpc.box);
            RenderToggleSwitch("▣", "Filled Box", &m_visualNpc.fill);
            RenderToggleSwitch("📏", "Distance", &m_visualNpc.distance);
            RenderToggleSwitch("✧", "Chams", &m_visualNpc.chams);
            RenderThinSlider("✶", "Red Glow", &m_visualNpc.redGlow, 0.0f, 1.5f, "%.2f");
        });

        ImGui::TableNextColumn();
        RenderSettingsCard("Visual Overview", "Unrivaled ESP and Wallhack Domination", ImVec2(0.0f, 0.0f), [this]() {
            RenderBulletList({
                "Player ESP",
                "Show Team",
                "Show Wounded",
                "Show Sleeper",
                "Show Dead",
                "Name ESP",
                "Box (Full / Corner)",
                "Healthbar (Side, Bottom, Top)"
            });
        });

        ImGui::EndTable();
    }

    // Propagate back to legacy variables
    m_espEnabled = m_visualPlayer.enabled;
    m_espPlayerBox = m_visualPlayer.box2D;
    m_espPlayerSkeleton = m_visualPlayer.skeleton;
    m_espPlayerHealth = m_visualPlayer.healthBar;
    m_espPlayerName = m_visualPlayer.showName;
    m_espPlayerDistance = m_visualPlayer.showDistance;
    m_espPlayerWeapon = m_visualPlayer.showWeapon;
    m_espShowAmmo = m_visualPlayer.showAmmo;
    m_espPlayerTeam = m_visualPlayer.showTeam;
    m_espPlayerWounded = m_visualPlayer.showWounded;
    m_espPlayerDead = m_visualPlayer.showDead;
    m_espTeamIndicator = m_visualPlayer.teamIndicator;
    m_chamsEnabled = m_visualPlayer.chams;
    m_chamsOpacity = m_visualPlayer.chamsGlow;

    m_glowEnabled = m_visualOverride.enableGlow;
    m_glowIntensity = m_visualOverride.glowIntensity;
    m_espShowNPCs = m_visualNpc.enabled;
}

void CRustInjector::RenderWorldPage() {
    // Sync structs with legacy state where applicable
    m_worldLoot.lootDistance = m_espMaxDistance;
    m_worldTraps.showRange = m_espShowCircle;

    if (ImGui::BeginTable("WorldGrid", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody)) {
        ImGui::TableNextColumn();
        RenderSettingsCard("Combat & Aiming", "Resource radar for flank routes", ImVec2(0.0f, 0.0f), [this]() {
            RenderToggleSwitch("⛏", "Sulfur Nodes", &m_worldResources.sulfurNodes);
            RenderToggleSwitch("⛏", "Metal Nodes", &m_worldResources.metalNodes);
            RenderToggleSwitch("⛏", "Stone Nodes", &m_worldResources.stoneNodes);
            RenderThinSlider("📡", "Scan Radius", &m_worldResources.resourceDistance, 50.0f, 500.0f, "%.0fm");
            RenderToggleSwitch("◎", "Show Range Rings", &m_worldTraps.showRange);
        });

        ImGui::TableNextColumn();
        RenderSettingsCard("ESP & World", "Crate tiers, drops, and hidden stashes", ImVec2(0.0f, 0.0f), [this]() {
            RenderToggleSwitch("📦", "Elite Crates", &m_worldLoot.eliteCrates);
            RenderToggleSwitch("📦", "Military Crates", &m_worldLoot.militaryCrates);
            RenderToggleSwitch("✈", "Air Drops", &m_worldLoot.airDrops);
            RenderToggleSwitch("🔐", "Locked Crates", &m_worldLoot.lockedCrates);
            RenderToggleSwitch("☠", "Buried Stashes", &m_worldLoot.stashESP);
            RenderThinSlider("📡", "Overlay Distance", &m_worldLoot.lootDistance, 50.0f, 500.0f, "%.0fm");
        });

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        RenderSettingsCard("Base Intel", "Cupboard privilege & outline tracing", ImVec2(0.0f, 0.0f), [this]() {
            RenderToggleSwitch("🏠", "Tool Cupboard", &m_worldBase.toolCupboard);
            RenderToggleSwitch("🛡", "Authorized Players", &m_worldBase.authorizedPlayers);
            RenderToggleSwitch("▣", "Base Outline", &m_worldBase.baseOutline);
            RenderInfoText("ℹ", "Draw raid-ready schematics with owner rosters.", ImVec4(0.6f, 0.6f, 0.75f, 1.0f));
        });

        ImGui::TableNextColumn();
        RenderSettingsCard("Trap Network", "Landmines, turrets, flame coverage", ImVec2(0.0f, 0.0f), [this]() {
            RenderToggleSwitch("☠", "Landmines", &m_worldTraps.landMines);
            RenderToggleSwitch("☠", "Bear Traps", &m_worldTraps.bearTraps);
            RenderToggleSwitch("☠", "Auto Turrets", &m_worldTraps.autoTurrets);
            RenderToggleSwitch("☠", "Flame Turrets", &m_worldTraps.flameTurrets);
        });

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        RenderSettingsCard("World Overview", "Master the Map with World ESP", ImVec2(0.0f, 0.0f), [this]() {
            RenderBulletList({
                "Ore Name & Distance",
                "Sulfur",
                "Metal",
                "Stone",
                "Collectable Name & Distance",
                "Vehicle Name & Distance",
                "Minicopter"
            });
        });

        ImGui::TableNextColumn();
        RenderSettingsCard("Movement & Exploits", "Fast traversal and map dominance", ImVec2(0.0f, 0.0f), [this]() {
            RenderBulletList({
                "No Fall Damage",
                "Infinite Jump",
                "Spider-man",
                "Flyhack (Toggle / Hold)",
                "Omni Sprint",
                "Fast Loot"
            });
        });

        ImGui::EndTable();
    }

    // Propagation to legacy fields
    m_espMaxDistance = m_worldLoot.lootDistance;
    m_espShowCircle = m_worldTraps.showRange;
}

void CRustInjector::RenderMiscPage() {
    // Two-column flat layout: Local | Atmosphere (matches Disconnect screenshot)
    ImVec2 avail = ImGui::GetContentRegionAvail();
    const float kColumnPad = 28.0f;
    const float colWidth = (avail.x - kColumnPad * 3.0f) * 0.5f;

    // Vertical center divider
    ImVec2 winPos = ImGui::GetWindowPos();
    float dividerX = winPos.x + kColumnPad + colWidth + kColumnPad * 0.5f;
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(dividerX, winPos.y + 12.0f),
        ImVec2(dividerX, winPos.y + avail.y - 12.0f),
        ImColor(kDivider), 1.0f);

    ImGui::SetCursorPos(ImVec2(kColumnPad, 18.0f));
    ImGui::BeginGroup();
    ImGui::PushItemWidth(colWidth);

    // ---- LOCAL column ----
    RenderFlatSectionHeader("Local");
    RenderFlatToggle("Reload Indicator", &m_miscLocal.reloadIndicator);
    RenderFlatToggle("Flyhack Indicator", &m_miscLocal.flyhackIndicator);
    RenderFlatToggle("Desync Indicator", &m_miscLocal.desyncIndicator);
    RenderFlatToggle("Only Show While Desyncing", &m_miscLocal.onlyShowWhileDesyncing);
    RenderFlatToggle("Anti Flash", &m_miscLocal.antiFlash);
    RenderFlatRowDivider();

    RenderFlatToggle("Local Movement Lines", &m_miscLocal.localMovementLines);
    RenderFlatToggle("Enemy Movement Lines", &m_miscLocal.enemyMovementLines);
    RenderFlatRowDivider();

    RenderFlatToggle("Bullet Trail", &m_miscLocal.bulletTrail);
    RenderFlatToggle("Bullet Tracers", &m_miscLocal.bulletTracers);
    RenderFlatToggle("Change Bullet Size", &m_miscLocal.changeBulletSize);
    RenderFlatSlider("Bullet Size:", &m_miscLocal.bulletSize, 0.0f, 100.0f, "%.0f");
    RenderFlatRowDivider();

    RenderFlatToggleWithColor("Hand Chams", &m_miscLocal.handChams, m_miscLocal.handChamsColor);
    RenderFlatToggleWithColor("Gun Chams", &m_miscLocal.gunChams, m_miscLocal.gunChamsColor);
    {
        const char* chamTypes[] = { "None", "Glow", "Flat", "Wireframe" };
        RenderFlatDropdown("Hand Cham Type", &m_miscLocal.handChamType, chamTypes, IM_ARRAYSIZE(chamTypes));
        RenderFlatDropdown("Gun Cham Type", &m_miscLocal.gunChamType, chamTypes, IM_ARRAYSIZE(chamTypes));
    }
    RenderFlatRowDivider();

    RenderFlatToggle("Change Hit Effect", &m_miscLocal.changeHitEffect);
    {
        const char* effects[] = { "None", "Spark", "Blood", "Custom" };
        RenderFlatDropdown("Hit Effect:", &m_miscLocal.hitEffect, effects, IM_ARRAYSIZE(effects));
    }

    ImGui::PopItemWidth();
    ImGui::EndGroup();

    // ---- ATMOSPHERE column ----
    ImGui::SetCursorPos(ImVec2(kColumnPad + colWidth + kColumnPad, 18.0f));
    ImGui::BeginGroup();
    ImGui::PushItemWidth(colWidth);

    RenderFlatSectionHeader("Atmosphere");
    RenderFlatToggle("Time Changer", &m_miscAtmosphere.timeChanger);
    RenderFlatToggle("Bright Stars", &m_miscAtmosphere.brightStars);
    RenderFlatToggle("Star Changer", &m_miscAtmosphere.starChanger);
    RenderFlatToggle("Rayleigh Changer", &m_miscAtmosphere.rayleighChanger);
    RenderFlatToggle("Ambient", &m_miscAtmosphere.ambient);
    RenderFlatToggle("No Fog", &m_miscAtmosphere.noFog);
    RenderFlatToggle("Draw Colliders", &m_miscAtmosphere.drawColliders);
    RenderFlatRowDivider();

    RenderFlatSlider("Time:", &m_miscAtmosphere.time, 0.0f, 24.0f, "%.0f");
    RenderFlatSlider("Star Size:", &m_miscAtmosphere.starSize, 0.0f, 100.0f, "%.0f");
    RenderFlatSlider("Rayleight Amount:", &m_miscAtmosphere.rayleighAmount, 0.0f, 100.0f, "%.0f");
    RenderFlatRowDivider();

    RenderFlatToggleWithColor("Change Sky Color", &m_miscAtmosphere.changeSkyColor, m_miscAtmosphere.skyColor);
    RenderFlatToggleWithColor("Change Cloud Color", &m_miscAtmosphere.changeCloudColor, m_miscAtmosphere.cloudColor);
    RenderFlatToggleWithColor("Sun/Moon Color", &m_miscAtmosphere.sunMoonColor, m_miscAtmosphere.sunMoonColorRGB);

    ImGui::PopItemWidth();
    ImGui::EndGroup();
}

void CRustInjector::RenderCloudPage() {
    if (ImGui::BeginTable("CloudGrid", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody)) {
        ImGui::TableNextColumn();
        RenderSettingsCard("Profiles", "Switch between preset cloud builds", ImVec2(0.0f, 0.0f), [this]() {
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.12f, 0.95f));
            ImGui::BeginChild("ProfileList", ImVec2(240.0f, 220.0f), true, ImGuiWindowFlags_NoScrollbar);
            for (size_t i = 0; i < m_cloud.profiles.size(); ++i) {
                bool selected = (m_cloud.selectedProfile == static_cast<int>(i));
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::Selectable(m_cloud.profiles[i].name.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0.0f, 32.0f))) {
                    m_cloud.selectedProfile = static_cast<int>(i);
                }
                ImGui::PopID();
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::Spacing();
            ImGui::InputTextWithHint("##new_profile", "New profile name", m_cloud.newProfileName, IM_ARRAYSIZE(m_cloud.newProfileName));
            if (ImGui::Button("Create Config", ImVec2(-1.0f, 34.0f))) {
                std::string name = m_cloud.newProfileName;
                if (!name.empty()) {
                    m_cloud.profiles.push_back({ name, false });
                    std::fill(std::begin(m_cloud.newProfileName), std::end(m_cloud.newProfileName), '\0');
                }
            }
        });

        ImGui::TableNextColumn();
        if (!m_cloud.profiles.empty()) {
            m_cloud.selectedProfile = std::clamp(m_cloud.selectedProfile, 0, static_cast<int>(m_cloud.profiles.size()) - 1);
            CloudProfile& profile = m_cloud.profiles[static_cast<size_t>(m_cloud.selectedProfile)];
            RenderSettingsCard(profile.name.c_str(), "Activate, duplicate, and schedule pushes", ImVec2(0.0f, 0.0f), [this, &profile]() {
                RenderToggleSwitch("★", "Set as Active", &profile.isActive);
                RenderInfoText("ℹ", "Active profile auto-syncs when the menu boots.", ImVec4(0.6f, 0.6f, 0.75f, 1.0f));
            });
        } else {
            RenderSettingsCard("No Configs", "Create a profile to start versioning settings", ImVec2(0.0f, 0.0f), []() {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.75f, 1.0f), "Add a profile name on the left to begin.");
            });
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        RenderSettingsCard("Import / Export", "Share presets via encrypted strings", ImVec2(0.0f, 0.0f), [this]() {
            char importBuffer[1024];
            strncpy_s(importBuffer, m_cloud.importString.c_str(), _TRUNCATE);
            if (ImGui::InputTextMultiline("##import_string", importBuffer, IM_ARRAYSIZE(importBuffer), ImVec2(-1.0f, 110.0f))) {
                m_cloud.importString = importBuffer;
            }
            ImGui::Spacing();
            char exportBuffer[1024];
            strncpy_s(exportBuffer, m_cloud.exportString.c_str(), _TRUNCATE);
            if (ImGui::InputTextMultiline("##export_string", exportBuffer, IM_ARRAYSIZE(exportBuffer), ImVec2(-1.0f, 110.0f))) {
                m_cloud.exportString = exportBuffer;
            }
            ImGui::Spacing();
            RenderInfoText("ℹ", "Paste or copy strings to sync profiles with teammates.", ImVec4(0.6f, 0.6f, 0.75f, 1.0f));
        });

        ImGui::TableNextColumn();
        RenderSettingsCard("Overlay & Hotkeys", "Stream-proofing and menu controls", ImVec2(0.0f, 0.0f), [this]() {
            RenderToggleSwitch("☁", "Streamproof Overlay", &m_cloud.streamProof);
            RenderKeybindInput("⌨", "Menu Toggle", &m_cloud.menuKey, &m_cloud.listeningForMenuKey);
            RenderToggleSwitch("⚙", "Auto-Inject on Launch", &m_autoInjectOnLaunch);
            RenderInfoText("ℹ", "Streamproof hides ESP layers from OBS / Discord capture.", ImVec4(0.6f, 0.6f, 0.75f, 1.0f));
        });

        ImGui::EndTable();
    }
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
        case WM_LBUTTONDOWN: {
            if (injector) {
                POINT pt = { LOWORD(lParam), HIWORD(lParam) };
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                // Check if clicking in title bar area for dragging
                if (pt.y < 30) {
                    injector->m_isDragging = true;
                    injector->m_dragOffset = ImVec2((float)pt.x, (float)pt.y);
                    SetCapture(hwnd);
                }
                
                // Check resize edges (10px from edges)
                const int edgeSize = 10;
                injector->m_resizeEdge = 0;
                if (pt.x < edgeSize) injector->m_resizeEdge |= 1; // Left
                if (pt.x > rect.right - edgeSize) injector->m_resizeEdge |= 2; // Right
                if (pt.y < edgeSize) injector->m_resizeEdge |= 4; // Top
                if (pt.y > rect.bottom - edgeSize) injector->m_resizeEdge |= 8; // Bottom
                
                if (injector->m_resizeEdge != 0) {
                    injector->m_isResizing = true;
                    SetCapture(hwnd);
                }
            }
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            if (injector) {
                POINT pt = { LOWORD(lParam), HIWORD(lParam) };
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                // Handle dragging
                if (injector->m_isDragging) {
                    POINT screenPt = pt;
                    ClientToScreen(hwnd, &screenPt);
                    injector->m_windowPos.x = screenPt.x - injector->m_dragOffset.x;
                    injector->m_windowPos.y = screenPt.y - injector->m_dragOffset.y;
                    SetWindowPos(hwnd, nullptr, (int)injector->m_windowPos.x, (int)injector->m_windowPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                }
                
                // Handle resizing
                if (injector->m_isResizing) {
                    RECT windowRect;
                    GetWindowRect(hwnd, &windowRect);
                    
                    int newWidth = windowRect.right - windowRect.left;
                    int newHeight = windowRect.bottom - windowRect.top;
                    
                    if (injector->m_resizeEdge & 1) { // Left
                        int delta = pt.x - injector->m_dragOffset.x;
                        newWidth -= delta;
                        injector->m_windowPos.x = windowRect.left + delta;
                    }
                    if (injector->m_resizeEdge & 2) { // Right
                        newWidth = pt.x;
                    }
                    if (injector->m_resizeEdge & 4) { // Top
                        int delta = pt.y - injector->m_dragOffset.y;
                        newHeight -= delta;
                        injector->m_windowPos.y = windowRect.top + delta;
                    }
                    if (injector->m_resizeEdge & 8) { // Bottom
                        newHeight = pt.y;
                    }
                    
                    // Clamp to min/max size
                    newWidth = max((int)injector->m_minWindowSize.x, min(newWidth, (int)injector->m_maxWindowSize.x));
                    newHeight = max((int)injector->m_minWindowSize.y, min(newHeight, (int)injector->m_maxWindowSize.y));
                    
                    injector->m_windowSize = ImVec2((float)newWidth, (float)newHeight);
                    SetWindowPos(hwnd, nullptr, (int)injector->m_windowPos.x, (int)injector->m_windowPos.y, newWidth, newHeight, SWP_NOZORDER);
                }
                
                // Update cursor for resize
                if (!injector->m_isDragging && !injector->m_isResizing) {
                    const int edgeSize = 10;
                    int resizeEdge = 0;
                    if (pt.x < edgeSize) resizeEdge |= 1;
                    if (pt.x > rect.right - edgeSize) resizeEdge |= 2;
                    if (pt.y < edgeSize) resizeEdge |= 4;
                    if (pt.y > rect.bottom - edgeSize) resizeEdge |= 8;
                    
                    LPCSTR cursor = IDC_ARROW;
                    switch (resizeEdge) {
                        case 1: case 2: cursor = IDC_SIZEWE; break;
                        case 4: case 8: cursor = IDC_SIZENS; break;
                        case 5: case 10: cursor = IDC_SIZENWSE; break;
                        case 6: case 9: cursor = IDC_SIZENESW; break;
                    }
                    SetCursor(LoadCursor(nullptr, cursor));
                }
            }
            return 0;
        }
        
        case WM_LBUTTONUP: {
            if (injector) {
                if (injector->m_isDragging || injector->m_isResizing) {
                    injector->m_isDragging = false;
                    injector->m_isResizing = false;
                    ReleaseCapture();
                    
                    // Save window position and size
                    RECT rect;
                    GetWindowRect(hwnd, &rect);
                    injector->m_windowPos = ImVec2((float)rect.left, (float)rect.top);
                    injector->m_windowSize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
                }
            }
            return 0;
        }
        
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

// Animation System Implementation
void CRustInjector::UpdateAnimations(float deltaTime) {
    m_globalAnimTime += deltaTime;
    
    // Update icon hover animations
    for (int i = 0; i < 5; i++) {
        if (m_iconHoverAnim[i].active) {
            float diff = m_iconHoverAnim[i].targetValue - m_iconHoverAnim[i].value;
            if (abs(diff) < 0.01f) {
                m_iconHoverAnim[i].value = m_iconHoverAnim[i].targetValue;
                m_iconHoverAnim[i].active = false;
            } else {
                m_iconHoverAnim[i].value += diff * m_iconHoverAnim[i].speed * deltaTime;
            }
        }
    }
    
    // Update module selection animation
    if (m_moduleSelectAnim.active) {
        float diff = m_moduleSelectAnim.targetValue - m_moduleSelectAnim.value;
        if (abs(diff) < 0.01f) {
            m_moduleSelectAnim.value = m_moduleSelectAnim.targetValue;
            m_moduleSelectAnim.active = false;
        } else {
            m_moduleSelectAnim.value += diff * m_moduleSelectAnim.speed * deltaTime;
        }
    }
    
    // Update content fade animation
    if (m_contentFadeAnim.active) {
        float diff = m_contentFadeAnim.targetValue - m_contentFadeAnim.value;
        if (abs(diff) < 0.01f) {
            m_contentFadeAnim.value = m_contentFadeAnim.targetValue;
            m_contentFadeAnim.active = false;
        } else {
            m_contentFadeAnim.value += diff * m_contentFadeAnim.speed * deltaTime;
        }
    }
}

float CRustInjector::EaseOutQuad(float t) {
    return t * (2.0f - t);
}

float CRustInjector::EaseInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

float CRustInjector::EaseOutBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    
    if (t < 1.0f / d1) {
        return n1 * t * t;
    } else if (t < 2.0f / d1) {
        return n1 * (t -= 1.5f / d1) * t + 0.75f;
    } else if (t < 2.5f / d1) {
        return n1 * (t -= 2.25f / d1) * t + 0.9375f;
    } else {
        return n1 * (t -= 2.625f / d1) * t + 0.984375f;
    }
}

void CRustInjector::StartIconHoverAnim(int index) {
    if (index >= 0 && index < 5) {
        m_iconHoverAnim[index].targetValue = 1.0f;
        m_iconHoverAnim[index].speed = 5.0f;
        m_iconHoverAnim[index].active = true;
    }
}

void CRustInjector::StartModuleSelectAnim() {
    m_moduleSelectAnim.value = 0.0f;
    m_moduleSelectAnim.targetValue = 1.0f;
    m_moduleSelectAnim.speed = 8.0f;
    m_moduleSelectAnim.active = true;
}

void CRustInjector::StartContentFadeAnim() {
    m_contentFadeAnim.value = 0.0f;
    m_contentFadeAnim.targetValue = 1.0f;
    m_contentFadeAnim.speed = 6.0f;
    m_contentFadeAnim.active = true;
}

// Custom Control Rendering Implementation
void CRustInjector::RenderCustomButton(const char* label, ImVec2 size, ImVec4 color, ImVec4 hoverColor, ImVec4 activeColor, bool* clicked) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 min = cursor;
    ImVec2 max = ImVec2(cursor.x + size.x, cursor.y + size.y);
    
    ImGui::SetCursorScreenPos(cursor);
    ImGui::InvisibleButton(label, size);
    
    bool isHovered = ImGui::IsItemHovered();
    bool isActive = ImGui::IsItemActive();
    
    ImVec4 finalColor = isActive ? activeColor : (isHovered ? hoverColor : color);
    
    // Draw rounded rectangle with gradient
    drawList->AddRectFilledMultiColor(
        min, max,
        ImColor(finalColor.x * 0.8f, finalColor.y * 0.8f, finalColor.z * 0.8f, finalColor.w),
        ImColor(finalColor),
        ImColor(finalColor),
        ImColor(finalColor.x * 0.8f, finalColor.y * 0.8f, finalColor.z * 0.8f, finalColor.w)
    );
    
    // Draw border
    drawList->AddRect(min, max, ImColor(1.0f, 1.0f, 1.0f, 0.1f), 8.0f, 0, 1.0f);
    
    // Draw text centered
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 textPos = ImVec2(
        cursor.x + (size.x - textSize.x) * 0.5f,
        cursor.y + (size.y - textSize.y) * 0.5f
    );
    drawList->AddText(textPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), label);
    
    if (clicked) {
        *clicked = ImGui::IsItemClicked();
    }
    
    ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y + size.y + ImGui::GetStyle().ItemSpacing.y));
}

void CRustInjector::RenderCustomCheckbox(const char* label, bool* value, ImVec4 checkColor) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    
    ImGui::PushStyleColor(ImGuiCol_CheckMark, checkColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::Checkbox(label, value);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void CRustInjector::RenderCustomSlider(const char* label, float* value, float minVal, float maxVal, const char* format) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.0f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0f, 0.0f, 0.3f, 1.0f));
    ImGui::SliderFloat(label, value, minVal, maxVal, format);
    ImGui::PopStyleColor(2);
}

void CRustInjector::RenderCustomScrollbar() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScrollbarRounding = 8.0f;
    style.ScrollbarSize = 10.0f;
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.07f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.0f, 0.0f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0f, 0.0f, 0.3f, 1.0f);
}

void CRustInjector::RenderCustomInput(const char* label, char* buffer, size_t bufferSize) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.07f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.1f, 0.1f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(1.0f, 0.0f, 0.4f, 0.3f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::InputText(label, buffer, bufferSize);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
}

void CRustInjector::RenderCustomCombo(const char* label, int* currentItem, const char** items, int itemsCount) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.07f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.1f, 0.1f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(1.0f, 0.0f, 0.4f, 0.3f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::Combo(label, currentItem, items, itemsCount);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
}

void CRustInjector::RenderToggleSwitch(const char* icon, const char* label, bool* value) {
    const float height = 32.0f;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    if (width <= 0.0f) width = 200.0f;

    ImGui::InvisibleButton(label, ImVec2(width, height));
    bool hovered = ImGui::IsItemHovered();
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        *value = !(*value);
    }

    ImVec4 activeColor = kAccent;
    ImVec4 inactiveColor = ImVec4(0.55f, 0.56f, 0.68f, 1.0f);
    ImVec4 textColor = *value ? activeColor : inactiveColor;

    ImVec2 iconSize = icon && icon[0] ? ImGui::CalcTextSize(icon) : ImVec2(0, 0);
    ImVec2 labelSize = ImGui::CalcTextSize(label);

    float textBaseline = pos.y + (height - labelSize.y) * 0.5f;
    float iconBaseline = pos.y + (height - iconSize.y) * 0.5f;

    ImVec2 textPos = ImVec2(pos.x + (iconSize.x > 0 ? iconSize.x + 12.0f : 2.0f), textBaseline);

    if (iconSize.x > 0) {
        drawList->AddText(pos + ImVec2(2.0f, iconBaseline), ImColor(textColor), icon);
    }

    drawList->AddText(textPos, ImColor(textColor.x, textColor.y, textColor.z, 1.0f), label);

    const float switchWidth = 48.0f;
    const float switchHeight = 20.0f;
    ImVec2 switchPos = ImVec2(pos.x + width - switchWidth - 8.0f, pos.y + (height - switchHeight) * 0.5f);

    ImVec4 trackOff = ImVec4(0.13f, 0.14f, 0.20f, 1.0f);
    ImVec4 trackOn = ImVec4(kAccent.x, kAccent.y, kAccent.z, *value ? 0.85f : 0.45f);

    drawList->AddRectFilled(switchPos, switchPos + ImVec2(switchWidth, switchHeight), ImColor(trackOff), switchHeight * 0.5f);
    if (*value) {
        drawList->AddRectFilledMultiColor(
            switchPos,
            switchPos + ImVec2(switchWidth, switchHeight),
            ImColor(trackOn.x, trackOn.y, trackOn.z, trackOn.w),
            ImColor(0.78f, 0.18f, 0.98f, trackOn.w),
            ImColor(0.78f, 0.18f, 0.98f, trackOn.w),
            ImColor(trackOn.x, trackOn.y, trackOn.z, trackOn.w),
            switchHeight * 0.5f
        );
    } else {
        drawList->AddRect(switchPos, switchPos + ImVec2(switchWidth, switchHeight), ImColor(0.26f, 0.28f, 0.36f, 1.0f), switchHeight * 0.5f, 0, 1.0f);
    }

    float knobRadius = (switchHeight * 0.5f) - 1.5f;
    float knobX = *value ? (switchPos.x + switchWidth - knobRadius - 2.0f) : (switchPos.x + knobRadius + 2.0f);
    ImVec2 knobCenter = ImVec2(knobX, switchPos.y + switchHeight * 0.5f);
    drawList->AddCircleFilled(knobCenter, knobRadius, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
    drawList->AddCircle(knobCenter, knobRadius, ImColor(0.92f, 0.92f, 1.0f, 0.35f), 24, 1.0f);

    if (hovered) {
        drawList->AddRect(switchPos - ImVec2(1.0f, 1.0f), switchPos + ImVec2(switchWidth + 1.0f, switchHeight + 1.0f), ImColor(kAccent.x, kAccent.y, kAccent.z, 0.45f), switchHeight * 0.5f, 0, 1.2f);
    }
}

void CRustInjector::RenderThinSlider(const char* icon, const char* label, float* value, float minVal, float maxVal, const char* format) {
    const float height = 38.0f;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    if (width <= 0.0f) width = 240.0f;

    ImGui::InvisibleButton(label, ImVec2(width, height));
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    ImVec2 iconSize = icon && icon[0] ? ImGui::CalcTextSize(icon) : ImVec2(0, 0);
    ImVec2 labelSize = ImGui::CalcTextSize(label);
    float textLeft = pos.x + (iconSize.x > 0 ? iconSize.x + 10.0f : 0.0f);
    float textBaseline = pos.y + 6.0f;

    if (iconSize.x > 0) {
        drawList->AddText(pos + ImVec2(0.0f, 4.0f), ImColor(kAccent), icon);
    }
    drawList->AddText(ImVec2(textLeft, textBaseline), ImColor(kTextBright), label);

    const float valueWidth = 70.0f;
    float trackStart = pos.x + std::max(160.0f, textLeft + labelSize.x + 24.0f);
    float trackEnd = pos.x + width - valueWidth;
    if (trackEnd <= trackStart) trackEnd = trackStart + 20.0f;
    float trackY = pos.y + height * 0.5f + 4.0f;
    float trackThickness = 2.0f;

    float t = (*value - minVal) / (maxVal - minVal);
    t = std::clamp(t, 0.0f, 1.0f);

    if ((hovered || active) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        float mouseX = ImGui::GetIO().MousePos.x;
        float newT = (mouseX - trackStart) / (trackEnd - trackStart);
        newT = std::clamp(newT, 0.0f, 1.0f);
        *value = minVal + (maxVal - minVal) * newT;
        t = newT;
    }

    drawList->AddLine(ImVec2(trackStart, trackY), ImVec2(trackEnd, trackY), ImColor(0.18f, 0.18f, 0.26f, 1.0f), trackThickness);
    drawList->AddLine(ImVec2(trackStart, trackY), ImVec2(trackStart + (trackEnd - trackStart) * t, trackY), ImColor(kAccent), trackThickness);

    float knobX = trackStart + (trackEnd - trackStart) * t;
    drawList->AddCircleFilled(ImVec2(knobX, trackY), 6.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
    drawList->AddCircle(ImVec2(knobX, trackY), 6.0f, ImColor(kAccent.x, kAccent.y, kAccent.z, 0.7f), 24, 1.0f);

    char valueBuffer[32];
    ImFormatString(valueBuffer, sizeof(valueBuffer), format, *value);
    drawList->AddText(ImVec2(trackEnd + 12.0f, trackY - 10.0f), ImColor(kTextMuted), valueBuffer);
}

void CRustInjector::RenderDropdown(const char* icon, const char* label, int* currentItem, const char** items, int itemsCount) {
    ImGui::PushID(label);
    const float height = 38.0f;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    if (width <= 0.0f) width = 220.0f;

    ImGui::InvisibleButton("##dropdown", ImVec2(width, height));
    bool hovered = ImGui::IsItemHovered();
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImGui::OpenPopup("##dropdown_popup");
    }

    ImVec4 accent = kAccent;
    ImVec4 textDim = ImVec4(0.6f, 0.6f, 0.75f, 1.0f);
    ImVec4 textMain = kTextBright;

    drawList->AddRectFilled(pos, pos + ImVec2(width, height), ImColor(kSurfaceDeep), 12.0f);
    drawList->AddRect(pos, pos + ImVec2(width, height), ImColor(0.2f, 0.2f, 0.28f, hovered ? 0.85f : 0.45f), 12.0f, 0, 1.0f);

    ImVec2 iconSize = icon && icon[0] ? ImGui::CalcTextSize(icon) : ImVec2(0, 0);
    if (iconSize.x > 0) {
        drawList->AddText(pos + ImVec2(12.0f, 6.0f), ImColor(accent), icon);
    }

    ImVec2 labelSize = ImGui::CalcTextSize(label);
    float labelX = pos.x + (iconSize.x > 0 ? iconSize.x + 18.0f : 12.0f);
    drawList->AddText(ImVec2(labelX, pos.y + 6.0f), ImColor(textDim), label);

    const char* currentText = (*currentItem >= 0 && *currentItem < itemsCount) ? items[*currentItem] : "";
    drawList->AddText(ImVec2(labelX, pos.y + height * 0.5f + 2.0f), ImColor(textMain), currentText);

    const char* arrow = "▼";
    ImVec2 arrowSize = ImGui::CalcTextSize(arrow);
    drawList->AddText(ImVec2(pos.x + width - arrowSize.x - 14.0f, pos.y + height * 0.5f - arrowSize.y * 0.5f), ImColor(textDim), arrow);

    if (ImGui::BeginPopup("##dropdown_popup")) {
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.05f, 0.05f, 0.08f, 0.98f));
        for (int i = 0; i < itemsCount; ++i) {
            bool selected = (*currentItem == i);
            if (ImGui::Selectable(items[i], selected)) {
                *currentItem = i;
            }
        }
        ImGui::PopStyleColor();
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void CRustInjector::RenderKeybindInput(const char* icon, const char* label, ImGuiKey* key, bool* listeningState) {
    ImGui::PushID(label);
    const float height = 38.0f;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    if (width <= 0.0f) width = 220.0f;

    ImGui::InvisibleButton("##keybind", ImVec2(width, height));
    bool hovered = ImGui::IsItemHovered();
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        *listeningState = true;
    }

    ImVec4 accent = ImVec4(1.0f, 0.0f, 0.4f, 1.0f);
    ImVec4 textDim = ImVec4(0.55f, 0.55f, 0.65f, 1.0f);
    ImVec4 textMain = ImVec4(0.9f, 0.9f, 0.95f, 1.0f);

    drawList->AddRectFilled(pos, pos + ImVec2(width, height), ImColor(0.09f, 0.09f, 0.12f, 1.0f), 10.0f);
    drawList->AddRect(pos, pos + ImVec2(width, height), ImColor(0.2f, 0.2f, 0.25f, hovered ? 0.7f : 0.4f), 10.0f, 0, 1.0f);

    ImVec2 iconSize = icon && icon[0] ? ImGui::CalcTextSize(icon) : ImVec2(0, 0);
    if (iconSize.x > 0) {
        drawList->AddText(pos + ImVec2(12.0f, 6.0f), ImColor(accent), icon);
    }

    float labelX = pos.x + (iconSize.x > 0 ? iconSize.x + 18.0f : 12.0f);
    drawList->AddText(ImVec2(labelX, pos.y + 6.0f), ImColor(textDim), label);

    const char* displayText = nullptr;
    if (*listeningState) {
        displayText = "Press any key...";
    } else {
        displayText = ImGui::GetKeyName(*key);
        if (!displayText || !displayText[0]) {
            displayText = "Unbound";
        }
    }
    drawList->AddText(ImVec2(labelX, pos.y + height * 0.5f + 2.0f), ImColor(textMain), displayText);

    if (*listeningState) {
        for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k) {
            ImGuiKey imguiKey = static_cast<ImGuiKey>(k);
            if (ImGui::IsKeyPressed(imguiKey)) {
                if (imguiKey == ImGuiKey_Escape) {
                    *listeningState = false;
                    break;
                }
                *key = imguiKey;
                *listeningState = false;
                break;
            }
        }
        for (int mouse = ImGuiKey_MouseLeft; mouse <= ImGuiKey_MouseRight; ++mouse) {
            ImGuiKey mouseKey = static_cast<ImGuiKey>(mouse);
            if (ImGui::IsKeyPressed(mouseKey)) {
                *key = mouseKey;
                *listeningState = false;
                break;
            }
        }
    }
    ImGui::PopID();
}

void CRustInjector::RenderInfoText(const char* icon, const char* text, ImVec4 color) {
    ImGui::BeginGroup();
    if (icon && icon[0]) {
        ImGui::TextColored(ImVec4(color.x, color.y, color.z, 0.9f), "%s", icon);
        ImGui::SameLine();
    }
    ImGui::TextColored(color, "%s", text);
    ImGui::EndGroup();
}

void CRustInjector::RenderBulletList(std::initializer_list<const char*> items) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.95f, 1.0f));
    for (const char* item : items) {
        ImGui::Bullet();
        ImGui::SameLine();
        ImGui::TextUnformatted(item);
    }
    ImGui::PopStyleColor();
}

void CRustInjector::RenderSettingsCard(const char* title, const char* subtitle, const ImVec2& size, const std::function<void()>& contentFn) {
    ImGui::PushID(title);
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImVec2 actual = size;
    if (actual.x <= 0.0f) {
        actual.x = avail.x;
    }
    if (actual.y < 120.0f) {
        actual.y = 160.0f;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 18.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kSurfaceDark);
    ImGui::BeginChild("##card", actual, true, ImGuiWindowFlags_NoScrollbar);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 childPos = ImGui::GetWindowPos();
    ImVec2 childSize = ImGui::GetWindowSize();
    // Flat thin divider rather than full border
    drawList->AddRect(childPos, ImVec2(childPos.x + childSize.x, childPos.y + childSize.y), ImColor(kDivider), 6.0f, 0, 1.0f);

    if (m_fontHeader) ImGui::PushFont(m_fontHeader);
    ImGui::TextColored(kTextBright, "%s", title);
    if (m_fontHeader) ImGui::PopFont();
    ImGui::TextColored(ImVec4(0.70f, 0.70f, 0.82f, 1.0f), "%s", subtitle);
    ImGui::Spacing();

    if (contentFn) {
        contentFn();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::PopID();
}

// Tooltip System Implementation
void CRustInjector::RenderTooltip() {
    if (!m_currentTooltip.visible) return;
    
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    ImVec2 textSize = ImGui::CalcTextSize(m_currentTooltip.text.c_str());
    ImVec2 padding = ImVec2(10.0f, 8.0f);
    ImVec2 tooltipSize = ImVec2(textSize.x + padding.x * 2, textSize.y + padding.y * 2);
    
    ImVec2 tooltipPos = m_currentTooltip.pos;
    
    // Smart positioning - avoid screen edges
    ImGuiIO& io = ImGui::GetIO();
    if (tooltipPos.x + tooltipSize.x > io.DisplaySize.x) {
        tooltipPos.x = io.DisplaySize.x - tooltipSize.x - 10.0f;
    }
    if (tooltipPos.y + tooltipSize.y > io.DisplaySize.y) {
        tooltipPos.y = io.DisplaySize.y - tooltipSize.y - 10.0f;
    }
    if (tooltipPos.x < 0) tooltipPos.x = 10.0f;
    if (tooltipPos.y < 0) tooltipPos.y = 10.0f;
    
    ImVec2 min = tooltipPos;
    ImVec2 max = ImVec2(tooltipPos.x + tooltipSize.x, tooltipPos.y + tooltipSize.y);
    
    // Draw tooltip background with shadow
    drawList->AddRectFilled(min, max, ImColor(0.1f, 0.1f, 0.12f, 0.95f), 8.0f);
    drawList->AddRect(min, max, ImColor(1.0f, 0.0f, 0.4f, 0.5f), 8.0f, 0, 1.0f);
    
    // Draw text
    drawList->AddText(ImVec2(min.x + padding.x, min.y + padding.y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), m_currentTooltip.text.c_str());
}

void CRustInjector::SetTooltip(const char* text, ImVec2 pos) {
    m_currentTooltip.text = text;
    m_currentTooltip.pos = pos;
    m_currentTooltip.timer = 0.0f;
    m_currentTooltip.visible = true;
}

void CRustInjector::UpdateTooltip(float deltaTime) {
    if (m_currentTooltip.visible) {
        m_currentTooltip.timer += deltaTime;
        
        // Hide tooltip after 5 seconds
        if (m_currentTooltip.timer > 5.0f) {
            m_currentTooltip.visible = false;
        }
    }
}

// Advanced Features Implementation
void CRustInjector::SaveConfig() {
    // Config saving implementation would go here
    // For now, just log it
    AppendLog("[INFO] Config saved");
}

void CRustInjector::LoadConfig() {
    // Config loading implementation would go here
    // For now, just log it
    AppendLog("[INFO] Config loaded");
}

void CRustInjector::RegisterHotkey(int key, bool ctrl, bool shift, bool alt, const char* action) {
    Hotkey hotkey;
    hotkey.key = key;
    hotkey.ctrl = ctrl;
    hotkey.shift = shift;
    hotkey.alt = alt;
    hotkey.action = action;
    m_hotkeys.push_back(hotkey);
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

// =====================================================================
//  Flat Disconnect-style helpers
// =====================================================================

void CRustInjector::RenderLeftIconRail() {
    const float kRailWidth = 64.0f;
    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Rail background (slightly different shade)
    dl->AddRectFilled(winPos,
        ImVec2(winPos.x + kRailWidth, winPos.y + winSize.y),
        ImColor(kSurfaceDeep));

    struct RailIcon { MenuPage page; const char* glyph; };
    RailIcon icons[] = {
        { MenuPage::Aimbot,  "+" },     // crosshair
        { MenuPage::Visuals, "o" },     // eye
        { MenuPage::Cloud,   "*" },     // gear/settings
        { MenuPage::Misc,    "|" },     // rifle / misc
    };

    const float startY = 70.0f;
    const float gap = 56.0f;
    const float iconBox = 36.0f;

    for (int i = 0; i < IM_ARRAYSIZE(icons); ++i) {
        ImVec2 center = ImVec2(winPos.x + kRailWidth * 0.5f,
                               winPos.y + startY + i * gap);
        ImVec2 boxMin = ImVec2(center.x - iconBox * 0.5f, center.y - iconBox * 0.5f);
        ImVec2 boxMax = ImVec2(center.x + iconBox * 0.5f, center.y + iconBox * 0.5f);

        ImGui::SetCursorScreenPos(boxMin);
        ImGui::PushID(i);
        ImGui::InvisibleButton("rail_btn", ImVec2(iconBox, iconBox));
        bool hovered = ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();
        ImGui::PopID();

        bool active = (m_selectedPage == icons[i].page);
        ImU32 col = active ? ImColor(kAccent)
                           : (hovered ? ImColor(0.55f, 0.70f, 0.85f, 1.0f)
                                      : ImColor(0.30f, 0.45f, 0.62f, 1.0f));

        // Draw simple primitive icons
        switch (i) {
            case 0: // crosshair
                dl->AddCircle(center, 12.0f, col, 24, 2.0f);
                dl->AddLine(ImVec2(center.x - 16.0f, center.y), ImVec2(center.x + 16.0f, center.y), col, 2.0f);
                dl->AddLine(ImVec2(center.x, center.y - 16.0f), ImVec2(center.x, center.y + 16.0f), col, 2.0f);
                dl->AddCircleFilled(center, 2.5f, col, 12);
                break;
            case 1: // eye
                dl->AddCircle(center, 8.0f, col, 24, 2.0f);
                dl->AddCircleFilled(center, 3.5f, col, 12);
                dl->AddBezierQuadratic(ImVec2(center.x - 16.0f, center.y),
                                       ImVec2(center.x, center.y - 12.0f),
                                       ImVec2(center.x + 16.0f, center.y), col, 2.0f);
                dl->AddBezierQuadratic(ImVec2(center.x - 16.0f, center.y),
                                       ImVec2(center.x, center.y + 12.0f),
                                       ImVec2(center.x + 16.0f, center.y), col, 2.0f);
                break;
            case 2: // gear
                dl->AddCircle(center, 9.0f, col, 24, 2.0f);
                dl->AddCircleFilled(center, 3.0f, col, 12);
                for (int t = 0; t < 8; ++t) {
                    float a = (float)t * (3.14159f / 4.0f);
                    ImVec2 p1 = ImVec2(center.x + cosf(a) * 10.0f, center.y + sinf(a) * 10.0f);
                    ImVec2 p2 = ImVec2(center.x + cosf(a) * 15.0f, center.y + sinf(a) * 15.0f);
                    dl->AddLine(p1, p2, col, 2.5f);
                }
                break;
            case 3: // rifle (stylized)
                dl->AddRectFilled(ImVec2(center.x - 14.0f, center.y - 3.0f),
                                  ImVec2(center.x + 12.0f, center.y + 3.0f), col);
                dl->AddRectFilled(ImVec2(center.x - 16.0f, center.y - 6.0f),
                                  ImVec2(center.x - 10.0f, center.y + 4.0f), col);
                dl->AddRectFilled(ImVec2(center.x + 4.0f, center.y + 3.0f),
                                  ImVec2(center.x + 8.0f, center.y + 12.0f), col);
                break;
        }

        if (clicked && m_selectedPage != icons[i].page) {
            m_selectedPage = icons[i].page;
            StartContentFadeAnim();
        }
    }
}

void CRustInjector::RenderFlatTopTabs() {
    struct Tab { MenuPage page; const char* label; };
    Tab tabs[] = {
        { MenuPage::Visuals, "Players" },
        { MenuPage::World,   "World" },
        { MenuPage::Misc,    "Misc" },
    };

    const float kTabHeight = 46.0f;
    ImVec2 start = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetWindowSize().x - start.x + ImGui::GetWindowPos().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Rail background
    dl->AddRectFilled(start,
        ImVec2(start.x + availW, start.y + kTabHeight),
        ImColor(kSurfaceDark));

    int n = IM_ARRAYSIZE(tabs);
    float tabW = availW / (float)n;

    // Animated underline tracker
    ImGuiStorage* st = ImGui::GetStateStorage();
    ImGuiID animXId = ImGui::GetID("##tab_uline_x");
    ImGuiID animWId = ImGui::GetID("##tab_uline_w");
    int activeIdx = 0;
    for (int i = 0; i < n; ++i) if (m_selectedPage == tabs[i].page) activeIdx = i;
    float targetX = start.x + activeIdx * tabW;
    float curX = st->GetFloat(animXId, targetX);
    float curW = st->GetFloat(animWId, tabW);
    float dt = ImGui::GetIO().DeltaTime;
    curX += (targetX - curX) * std::min(1.0f, dt * 14.0f);
    curW += (tabW    - curW) * std::min(1.0f, dt * 14.0f);
    st->SetFloat(animXId, curX);
    st->SetFloat(animWId, curW);

    for (int i = 0; i < n; ++i) {
        ImVec2 tabMin = ImVec2(start.x + i * tabW, start.y);
        ImVec2 tabMax = ImVec2(start.x + (i + 1) * tabW, start.y + kTabHeight);

        ImGui::SetCursorScreenPos(tabMin);
        ImGui::PushID(i);
        ImGui::InvisibleButton("tab_btn", ImVec2(tabW, kTabHeight));
        bool hovered = ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        ImGuiID hovId = ImGui::GetID("##tab_hov");
        float hov = st->GetFloat(hovId, 0.0f);
        hov += ((hovered ? 1.0f : 0.0f) - hov) * std::min(1.0f, dt * 10.0f);
        st->SetFloat(hovId, hov);
        ImGui::PopID();

        bool active = (m_selectedPage == tabs[i].page);
        ImVec4 base = active ? kAccent : kTextMuted;
        ImVec4 mix  = ImVec4(
            base.x + (kTextBright.x - base.x) * (active ? 0.0f : hov),
            base.y + (kTextBright.y - base.y) * (active ? 0.0f : hov),
            base.z + (kTextBright.z - base.z) * (active ? 0.0f : hov), 1.0f);

        ImVec2 ts = ImGui::CalcTextSize(tabs[i].label);
        ImVec2 tp = ImVec2(tabMin.x + (tabW - ts.x) * 0.5f,
                           tabMin.y + (kTabHeight - ts.y) * 0.5f);
        dl->AddText(tp, ImColor(mix), tabs[i].label);

        if (clicked && !active) {
            m_selectedPage = tabs[i].page;
            StartContentFadeAnim();
        }
    }

    // Animated underline below active tab
    dl->AddRectFilled(
        ImVec2(curX, start.y + kTabHeight - 2.0f),
        ImVec2(curX + curW, start.y + kTabHeight),
        ImColor(kAccent));
}

void CRustInjector::RenderFlatSectionHeader(const char* title) {
    ImGui::PushFont(m_fontHeader ? m_fontHeader : ImGui::GetFont());
    ImGui::TextColored(kTextBright, "%s", title);
    ImGui::PopFont();
    ImGui::Spacing();
}

void CRustInjector::RenderFlatRowDivider() {
    ImGui::Spacing();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddLine(p, ImVec2(p.x + w, p.y), ImColor(kDivider), 1.0f);
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
}

bool CRustInjector::RenderFlatToggle(const char* label, bool* value) {
    const float rowH = 28.0f;
    const float boxSize = 16.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;

    ImGui::PushID(label);
    ImGui::InvisibleButton("##row", ImVec2(availW, rowH));
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();

    // Animated fill state (smooth on/off)
    ImGuiStorage* st = ImGui::GetStateStorage();
    ImGuiID animId = ImGui::GetID("##fillAnim");
    ImGuiID hoverId = ImGui::GetID("##hovAnim");
    float fill = st->GetFloat(animId, *value ? 1.0f : 0.0f);
    float hov  = st->GetFloat(hoverId, 0.0f);
    float dt = ImGui::GetIO().DeltaTime;
    fill += ((*value ? 1.0f : 0.0f) - fill) * std::min(1.0f, dt * 12.0f);
    hov  += ((hovered ? 1.0f : 0.0f) - hov)  * std::min(1.0f, dt * 10.0f);
    st->SetFloat(animId, fill);
    st->SetFloat(hoverId, hov);
    ImGui::PopID();

    // Hover row background highlight
    if (hov > 0.001f) {
        dl->AddRectFilled(pos, ImVec2(pos.x + availW, pos.y + rowH),
            ImColor(kAccent.x, kAccent.y, kAccent.z, 0.05f * hov), 3.0f);
    }

    // Label left (color lerps to bright on hover)
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec4 lc = ImVec4(
        kTextMuted.x + (kTextBright.x - kTextMuted.x) * hov,
        kTextMuted.y + (kTextBright.y - kTextMuted.y) * hov,
        kTextMuted.z + (kTextBright.z - kTextMuted.z) * hov, 1.0f);
    dl->AddText(ImVec2(pos.x, pos.y + (rowH - textSize.y) * 0.5f), ImColor(lc), label);

    // Box right
    ImVec2 boxMin = ImVec2(pos.x + availW - boxSize - 2.0f, pos.y + (rowH - boxSize) * 0.5f);
    ImVec2 boxMax = ImVec2(boxMin.x + boxSize, boxMin.y + boxSize);
    dl->AddRect(boxMin, boxMax, ImColor(kDivider), 3.0f, 0, 1.5f);
    if (fill > 0.005f) {
        float pad = 3.0f + (1.0f - fill) * 4.0f;
        dl->AddRectFilled(ImVec2(boxMin.x + pad, boxMin.y + pad),
                          ImVec2(boxMax.x - pad, boxMax.y - pad),
                          ImColor(kAccent.x, kAccent.y, kAccent.z, fill), 2.0f);
    }

    if (clicked) { *value = !*value; return true; }
    return false;
}

bool CRustInjector::RenderFlatToggleWithColor(const char* label, bool* value, float colorRGB[3]) {
    const float rowH = 28.0f;
    const float boxSize = 16.0f;
    const float colorBoxSize = 16.0f;
    const float gap = 6.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;

    // Color square button (left of toggle)
    ImVec2 colorMin = ImVec2(pos.x + availW - boxSize - gap - colorBoxSize, pos.y + (rowH - colorBoxSize) * 0.5f);
    ImVec2 colorMax = ImVec2(colorMin.x + colorBoxSize, colorMin.y + colorBoxSize);

    ImGui::SetCursorScreenPos(colorMin);
    ImGui::PushID(label);
    ImGui::PushID("color");
    ImGui::InvisibleButton("##col", ImVec2(colorBoxSize, colorBoxSize));
    bool colorClicked = ImGui::IsItemClicked();
    ImGui::PopID();
    if (colorClicked) ImGui::OpenPopup("##colorpick");
    if (ImGui::BeginPopup("##colorpick")) {
        ImGui::ColorPicker3("##picker", colorRGB,
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    dl->AddRectFilled(colorMin, colorMax,
        ImColor(colorRGB[0], colorRGB[1], colorRGB[2], 1.0f), 2.0f);
    dl->AddRect(colorMin, colorMax, ImColor(kDivider), 2.0f, 0, 1.0f);

    // Toggle row (excluding the color box width)
    ImGui::SetCursorScreenPos(pos);
    ImGui::PushID(label);
    ImGui::PushID("tog");
    ImGui::InvisibleButton("##row", ImVec2(availW - colorBoxSize - gap, rowH));
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();
    ImGui::PopID();
    ImGui::PopID();

    ImVec2 textSize = ImGui::CalcTextSize(label);
    dl->AddText(ImVec2(pos.x, pos.y + (rowH - textSize.y) * 0.5f),
                ImColor(hovered ? kTextBright : kTextMuted), label);

    ImVec2 boxMin = ImVec2(pos.x + availW - boxSize, pos.y + (rowH - boxSize) * 0.5f);
    ImVec2 boxMax = ImVec2(boxMin.x + boxSize, boxMin.y + boxSize);
    dl->AddRect(boxMin, boxMax, ImColor(kDivider), 3.0f, 0, 1.5f);
    if (*value) {
        dl->AddRectFilled(ImVec2(boxMin.x + 3, boxMin.y + 3),
                          ImVec2(boxMax.x - 3, boxMax.y - 3),
                          ImColor(kAccent), 2.0f);
    }

    if (clicked) { *value = !*value; return true; }
    return false;
}

bool CRustInjector::RenderFlatSlider(const char* label, float* value, float minVal, float maxVal, const char* format) {
    const float rowH = 36.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;

    // Label + value text row
    ImVec2 textSize = ImGui::CalcTextSize(label);
    dl->AddText(ImVec2(pos.x, pos.y + 2.0f), ImColor(kTextMuted), label);

    char buf[32];
    snprintf(buf, sizeof(buf), format, *value);
    ImVec2 valSize = ImGui::CalcTextSize(buf);
    dl->AddText(ImVec2(pos.x + availW - valSize.x, pos.y + 2.0f), ImColor(kTextBright), buf);

    // Track
    float trackY = pos.y + rowH - 8.0f;
    dl->AddLine(ImVec2(pos.x, trackY), ImVec2(pos.x + availW, trackY), ImColor(kDivider), 2.0f);
    float t = (*value - minVal) / (maxVal - minVal);
    if (t < 0) t = 0; if (t > 1) t = 1;
    dl->AddLine(ImVec2(pos.x, trackY), ImVec2(pos.x + availW * t, trackY), ImColor(kAccent), 2.5f);

    ImGui::PushID(label);
    ImGui::SetCursorScreenPos(ImVec2(pos.x, trackY - 8.0f));
    ImGui::InvisibleButton("##sl", ImVec2(availW, 16.0f));
    bool changed = false;
    if (ImGui::IsItemActive()) {
        float mx = ImGui::GetIO().MousePos.x;
        float nt = (mx - pos.x) / availW;
        if (nt < 0) nt = 0; if (nt > 1) nt = 1;
        *value = minVal + nt * (maxVal - minVal);
        changed = true;
    }
    ImGui::PopID();

    ImGui::Dummy(ImVec2(availW, rowH));
    return changed;
}

bool CRustInjector::RenderFlatDropdown(const char* label, int* currentItem, const char** items, int itemsCount) {
    const float rowH = 30.0f;
    const float boxW = 140.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;

    ImVec2 textSize = ImGui::CalcTextSize(label);
    dl->AddText(ImVec2(pos.x, pos.y + (rowH - textSize.y) * 0.5f), ImColor(kTextMuted), label);

    ImVec2 boxMin = ImVec2(pos.x + availW - boxW, pos.y + 2.0f);
    ImVec2 boxMax = ImVec2(pos.x + availW, pos.y + rowH - 2.0f);
    dl->AddRectFilled(boxMin, boxMax, ImColor(kSurfaceRow), 3.0f);
    dl->AddRect(boxMin, boxMax, ImColor(kDivider), 3.0f, 0, 1.0f);

    const char* preview = (*currentItem >= 0 && *currentItem < itemsCount) ? items[*currentItem] : "None";
    dl->AddText(ImVec2(boxMin.x + 8.0f, boxMin.y + (rowH - 4.0f - textSize.y) * 0.5f),
                ImColor(kTextBright), preview);

    ImGui::PushID(label);
    ImGui::SetCursorScreenPos(boxMin);
    ImGui::InvisibleButton("##dd", ImVec2(boxW, rowH - 4.0f));
    bool clicked = ImGui::IsItemClicked();
    if (clicked) ImGui::OpenPopup("##ddpop");

    bool changed = false;
    ImGui::SetNextWindowPos(ImVec2(boxMin.x, boxMax.y + 2.0f));
    ImGui::SetNextWindowSize(ImVec2(boxW, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, kSurfaceRow);
    if (ImGui::BeginPopup("##ddpop")) {
        for (int i = 0; i < itemsCount; ++i) {
            if (ImGui::Selectable(items[i], i == *currentItem)) {
                *currentItem = i;
                changed = true;
            }
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor();
    ImGui::PopID();

    ImGui::Dummy(ImVec2(availW, rowH));
    return changed;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CRustInjector injector;
    if (!injector.Initialize(hInstance)) {
        MessageBoxW(nullptr, L"Failed to initialize injector", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    return injector.Run();
}

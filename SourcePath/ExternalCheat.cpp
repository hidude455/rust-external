#include "ExternalCheat.h"
#include <iostream>
#include <memory>

namespace App {

    CExternalCheat::CExternalCheat()
        : m_running(false), m_authenticated(false),
          m_gameState(GameState::Detached), m_shutdownRequested(false) {}

    CExternalCheat::~CExternalCheat() { Shutdown(); }

    bool CExternalCheat::Startup(const std::string& licenseKey) {
        std::cout << "[*] Starting Rust External v2.0...\n";

        m_auth = std::make_unique<Auth::CAuthSystem>();
        m_auth->Initialize("api.rustexternal.com", "PUBLIC_KEY_HERE", 12345);

        Auth::AuthStatus authStatus = m_auth->Authenticate(licenseKey);
        if (authStatus != Auth::AuthStatus::Valid) {
            std::cout << "[!] Authentication failed: " << m_auth->GetStatusString() << "\n";
            return false;
        }
        m_authenticated = true;
        std::cout << "[+] Authenticated successfully\n";
        std::cout << "[+] License tier: " << m_auth->GetLicenseInfo().tier << "\n";
        std::cout << "[+] Days remaining: " << m_auth->GetRemainingDays() << "\n";

        m_security = std::make_unique<Security::CSecurityLayer>();
        if (!m_security->Initialize()) {
            std::cout << "[!] Security check failed\n";
            std::cout << m_security->GetSecurityReport();
            return false;
        }
        std::cout << "[+] Security checks passed\n";

        m_memory = std::make_unique<Memory::CGameMemory>();
        if (m_memory->Initialize()) {
            m_gameState.store(GameState::Attached);
            std::cout << "[+] Attached to Rust (PID: " << m_memory->GetPID() << ")\n";
            std::cout << "[+] GameAssembly: 0x" << std::hex << m_memory->GetGameAssemblyBase() << std::dec << "\n";
        } else {
            m_gameState.store(GameState::WaitingForGame);
            std::cout << "[!] RustClient.exe not detected. Waiting for game launch...\n";
        }

        m_features = std::make_unique<Features::CFeatureManager>();
        if (!m_features->Initialize(m_memory.get())) {
            std::cout << "[!] Failed to initialize features\n";
            return false;
        }
        std::cout << "[+] Features initialized\n";

        m_config = std::make_unique<Config::CConfigSystem>();
        m_config->Initialize("rust_external");
        LoadConfiguration();
        std::cout << "[+] Configuration loaded\n";

        Render::OverlayConfig overlayCfg;
        overlayCfg.width = GetSystemMetrics(SM_CXSCREEN);
        overlayCfg.height = GetSystemMetrics(SM_CYSCREEN);
        overlayCfg.fullscreen = true;
        overlayCfg.transparent = true;
        overlayCfg.clickThrough = true;
        overlayCfg.targetFPS = 144.0f;
        overlayCfg.windowTitle = "Rust External Overlay";

        m_overlay = std::make_unique<Render::COverlayRenderer>();
        if (!m_overlay->Initialize(overlayCfg)) {
            std::cout << "[!] Failed to create overlay\n";
            return false;
        }

        m_overlay->SetRenderCallback([this](ID3D11DeviceContext* ctx, int w, int h) {
            RenderCallback(ctx, w, h);
        });

        m_menuManager = std::make_unique<CMenuManager>(m_features.get());
        if (!m_menuManager->Initialize(m_overlay->GetHWND(), m_overlay->GetDevice(), m_overlay->GetContext())) {
            std::cout << "[!] Failed to initialize menu\n";
            return false;
        }
        m_overlay->SetMenuManager(m_menuManager.get());

        // Initialize spoofer system
        if (!InitializeSpoofer()) {
            std::cout << "[!] Failed to initialize spoofer\n";
            return false;
        }

        // Connect spoofer to menu manager
        if (m_spoofer && m_spooferGUI && m_menuManager) {
            m_menuManager->SetSpoofer(m_spoofer.get());
            m_menuManager->SetSpooferGUI(m_spooferGUI.get());
            m_menuManager->SetRustEvasion(m_rustEvasion.get());
        }

        std::cout << "[+] Overlay created\n";
        std::cout << "[+] Menu initialized (Right Shift to toggle)\n";
        std::cout << "[+] Spoofer initialized\n";
        std::cout << "[+] Rust External ready!\n\n";

        m_shutdownRequested.store(false);
        m_running = true;
        m_updateThread = std::thread(&CExternalCheat::UpdateLoop, this);

        return true;
    }

    void CExternalCheat::Shutdown() {
        m_shutdownRequested.store(true);
        m_running = false;

        if (m_updateThread.joinable()) {
            m_updateThread.join();
        }

        SaveConfiguration();

        // Shutdown spoofer components first (they depend on nothing)
        if (m_spooferGUI) m_spooferGUI->Shutdown();
        if (m_rustEvasion) m_rustEvasion->Shutdown();
        if (m_spoofer) m_spoofer->Shutdown();

        if (m_menuManager) m_menuManager->Shutdown();
        if (m_overlay) m_overlay->Shutdown();
        if (m_features) m_features->Shutdown();
        if (m_memory) m_memory->Shutdown();
        if (m_security) m_security->Shutdown();

        m_authenticated = false;
        m_gameState.store(GameState::Detached);
        std::cout << "[*] Shutdown complete\n";
    }

    void CExternalCheat::Run() {
        if (m_overlay) {
            m_overlay->Run();
        }
    }

    void CExternalCheat::UpdateLoop() {
        constexpr DWORD kRetryDelayMs = 2000;

        while (m_running) {
            if (m_shutdownRequested.load()) {
                break;
            }

            GameState state = m_gameState.load();

            if (state == GameState::WaitingForGame || state == GameState::Attaching) {
                if (!m_memory) {
                    Sleep(kRetryDelayMs);
                    continue;
                }

                m_gameState.store(GameState::Attaching);
                if (m_memory->Initialize()) {
                    m_gameState.store(GameState::Attached);
                    std::cout << "[+] Attached to Rust (PID: " << m_memory->GetPID() << ")\n";
                    std::cout << "[+] GameAssembly: 0x" << std::hex << m_memory->GetGameAssemblyBase() << std::dec << "\n";
                } else {
                    m_gameState.store(GameState::WaitingForGame);
                    Sleep(kRetryDelayMs);
                    continue;
                }
            } else if (state == GameState::Attached) {
                if (m_memory && (!m_memory->IsAttached() || !m_memory->IsProcessAlive())) {
                    std::cout << "[!] Lost connection to Rust. Waiting for RustClient.exe...\n";
                    m_memory->Shutdown();
                    m_gameState.store(GameState::WaitingForGame);
                    Sleep(kRetryDelayMs);
                    continue;
                }
            }

            if (m_features && m_memory && m_memory->IsAttached()) {
                m_features->Update();
                Sleep(1);
            } else {
                Sleep(200);
            }
        }
    }

    void CExternalCheat::RenderCallback(ID3D11DeviceContext* ctx, int width, int height) {
        if (m_features && m_memory && m_memory->IsAttached()) {
            m_features->Render(ctx, width, height);
        }
        
        // Render spoofer GUI
        if (m_spooferGUI) {
            m_spooferGUI->Render();
        }
    }

    bool CExternalCheat::InitializeSpoofer() {
        // Initialize advanced spoofer
        m_spoofer = std::make_unique<Spoofer::CAdvancedSpoofer>();
        
        Spoofer::SpooferConfig spoofConfig;
        spoofConfig.randomizeAll = true;
        spoofConfig.enableWMIProtection = true;
        spoofConfig.enableRegistryProtection = true;
        spoofConfig.enableDriverProtection = true;
        spoofConfig.enableNetworkProtection = true;
        spoofConfig.enablePCISpoofing = true;
        spoofConfig.createBackup = true;
        spoofConfig.restoreOnExit = false;
        
        if (!m_spoofer->Initialize(spoofConfig)) {
            std::cout << "[!] Failed to initialize advanced spoofer\n";
            return false;
        }
        std::cout << "[+] Advanced spoofer initialized\n";
        
        // Initialize Rust-specific EAC evasion
        m_rustEvasion = std::make_unique<RustEvasion::CRustAntiCheatEvasion>();
        
        RustEvasion::RustEvasionConfig evasionConfig;
        evasionConfig.bypassEACKernel = true;
        evasionConfig.bypassEACUserMode = true;
        evasionConfig.hideFromEACScanner = true;
        evasionConfig.spoofEACIdentifiers = true;
        evasionConfig.bypassEACIntegrity = true;
        evasionConfig.bypassEACTiming = true;
        evasionConfig.bypassEACNetwork = true;
        evasionConfig.bypassEACBehavior = true;
        evasionConfig.enableKernelDriver = true;
        evasionConfig.enableUserModeHooking = true;
        evasionConfig.enableMemoryObfuscation = true;
        evasionConfig.enableProcessHiding = true;
        evasionConfig.enableThreadHiding = true;
        evasionConfig.enableModuleHiding = true;
        evasionConfig.stealthMode = true;
        evasionConfig.aggressiveMode = false;
        evasionConfig.paranoidMode = false;
        
        if (!m_rustEvasion->Initialize(evasionConfig)) {
            std::cout << "[!] Failed to initialize Rust EAC evasion\n";
            return false;
        }
        std::cout << "[+] Rust EAC evasion initialized\n";
        
        // Initialize spoofer GUI
        m_spooferGUI = std::make_unique<SpooferGUI::CSpooferGUI>(m_spoofer.get());
        if (!m_spooferGUI->Initialize()) {
            std::cout << "[!] Failed to initialize spoofer GUI\n";
            return false;
        }
        std::cout << "[+] Spoofer GUI initialized\n";
        
        return true;
    }

    void CExternalCheat::LoadConfiguration() {
        if (!m_config || !m_features) return;
        auto& aimbot = m_features->GetAimbotConfig();
        m_config->LoadBool("aimbot_enabled", aimbot.enabled);
        m_config->LoadFloat("aimbot_fov", aimbot.fov);
        m_config->LoadFloat("aimbot_smoothness", aimbot.smoothness);
        m_config->LoadFloat("aimbot_maxdist", aimbot.maxDistance);
        m_config->LoadBool("aimbot_silent", aimbot.silentAim);
        m_config->LoadFloat("aimbot_hitchance", aimbot.hitchance);

        auto& vis = m_features->GetVisualsConfig();
        m_config->LoadBool("vis_enabled", vis.enabled);
        m_config->LoadBool("vis_players", vis.showPlayers);
        m_config->LoadBool("vis_username", vis.username);
        m_config->LoadBool("vis_distance", vis.distance);
        m_config->LoadBool("vis_healthbars", vis.healthbars);
        m_config->LoadBool("vis_helditem", vis.heldItem);
        m_config->LoadBool("vis_skeletons", vis.skeletons);
        m_config->LoadBool("vis_chams", vis.chams);
        m_config->LoadFloat("vis_maxdist", vis.maxDistance);

        auto& exploits = m_features->GetExploitsConfig();
        m_config->LoadBool("exploits.fov_changer", exploits.fovChanger, false);
        m_config->LoadFloat("exploits.fov_value", exploits.fovValue, 90.0f);
        m_config->LoadBool("exploits.bright_night", exploits.brightNight, false);
        m_config->LoadBool("exploits.bright_cave", exploits.brightCave, false);
        m_config->LoadBool("exploits.zoom", exploits.zoom, false);

        auto& weapon = m_features->GetWeaponConfig();
        m_config->LoadBool("weapon.recoil", weapon.recoilModifier, false);
        m_config->LoadBool("weapon.spread", weapon.weaponSpread, false);
        m_config->LoadBool("weapon.rapid", weapon.rapidFire, false);
        m_config->LoadBool("weapon.thick", weapon.thickBullet, false);
        m_config->LoadBool("weapon.sway", weapon.weaponSway, false);

        auto& world = m_features->GetWorldVisualsConfig();
        m_config->LoadBool("world_enabled", world.enabled);
        m_config->LoadBool("world_ores", world.allOres.enabled);
        m_config->LoadBool("world_collectables", world.collectables.enabled);
        m_config->LoadBool("world_animals", world.allAnimals.enabled);
        m_config->LoadFloat("world_maxdist", world.globalMaxDistance);

        auto& move = m_features->GetMovementConfig();
        m_config->LoadBool("move_spiderman", move.spiderman);
        m_config->LoadBool("move_sprint", move.forceSprint);
        m_config->LoadBool("move_noFallDmg", move.noFallDamage);
        m_config->LoadBool("move_spinbot", move.spinbot);

        auto& ui = m_features->GetUIConfig();
        m_config->LoadBool("ui_watermark", ui.showWatermark);
        m_config->LoadBool("ui_fps", ui.showFPSCounter);
        m_config->LoadBool("ui_crosshair", ui.customCrosshairs);
        m_config->LoadFloat("ui_scale", ui.uiScale);
    }

    void CExternalCheat::SaveConfiguration() {
        if (!m_config || !m_features) return;
        const auto& aimbot = m_features->GetAimbotConfig();
        m_config->SaveBool("aimbot_enabled", aimbot.enabled);
        m_config->SaveFloat("aimbot_fov", aimbot.fov);
        m_config->SaveFloat("aimbot_smoothness", aimbot.smoothness);
        m_config->SaveFloat("aimbot_maxdist", aimbot.maxDistance);
        m_config->SaveBool("aimbot_silent", aimbot.silentAim);
        m_config->SaveFloat("aimbot_hitchance", aimbot.hitchance);

        const auto& vis = m_features->GetVisualsConfig();
        m_config->SaveBool("vis_enabled", vis.enabled);
        m_config->SaveBool("vis_players", vis.showPlayers);
        m_config->SaveBool("vis_username", vis.username);
        m_config->SaveBool("vis_distance", vis.distance);
        m_config->SaveBool("vis_healthbars", vis.healthbars);
        m_config->SaveBool("vis_helditem", vis.heldItem);
        m_config->SaveBool("vis_skeletons", vis.skeletons);
        m_config->SaveBool("vis_chams", vis.chams);
        m_config->SaveFloat("vis_maxdist", vis.maxDistance);

        const auto& exploits = m_features->GetExploitsConfig();
        m_config->SaveBool("exploits.fov_changer", exploits.fovChanger);
        m_config->SaveFloat("exploits.fov_value", exploits.fovValue);
        m_config->SaveBool("exploits.bright_night", exploits.brightNight);
        m_config->SaveBool("exploits.bright_cave", exploits.brightCave);
        m_config->SaveBool("exploits.zoom", exploits.zoom);

        const auto& weapon = m_features->GetWeaponConfig();
        m_config->SaveBool("weapon.recoil", weapon.recoilModifier);
        m_config->SaveBool("weapon.spread", weapon.weaponSpread);
        m_config->SaveBool("weapon.rapid", weapon.rapidFire);
        m_config->SaveBool("weapon.thick", weapon.thickBullet);
        m_config->SaveBool("weapon.sway", weapon.weaponSway);

        const auto& world = m_features->GetWorldVisualsConfig();
        m_config->SaveBool("world_enabled", world.enabled);
        m_config->SaveBool("world_ores", world.allOres.enabled);
        m_config->SaveBool("world_collectables", world.collectables.enabled);
        m_config->SaveBool("world_animals", world.allAnimals.enabled);
        m_config->SaveFloat("world_maxdist", world.globalMaxDistance);

        const auto& move = m_features->GetMovementConfig();
        m_config->SaveBool("move_spiderman", move.spiderman);
        m_config->SaveBool("move_sprint", move.forceSprint);
        m_config->SaveBool("move_noFallDmg", move.noFallDamage);
        m_config->SaveBool("move_spinbot", move.spinbot);

        const auto& ui = m_features->GetUIConfig();
        m_config->SaveBool("ui_watermark", ui.showWatermark);
        m_config->SaveBool("ui_fps", ui.showFPSCounter);
        m_config->SaveBool("ui_crosshair", ui.customCrosshairs);
        m_config->SaveFloat("ui_scale", ui.uiScale);
    }

}

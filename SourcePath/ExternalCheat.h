#pragma once
#include "GameMemory.h"
#include "OverlayRenderer.h"
#include "FeatureManager.h"
#include "ConfigSystem.h"
#include "SecurityLayer.h"
#include "AuthSystem.h"
#include "MenuManager.h"
#include "AdvancedSpoofer.h"
#include "SpooferGUI.h"
#include "RustAntiCheatEvasion.h"
#include <memory>
#include <thread>
#include <atomic>

namespace App {

    class CExternalCheat {
    private:
        enum class GameState {
            Detached,
            WaitingForGame,
            Attaching,
            Attached
        };

        std::unique_ptr<Auth::CAuthSystem> m_auth;
        std::unique_ptr<Security::CSecurityLayer> m_security;
        std::unique_ptr<Memory::CGameMemory> m_memory;
        std::unique_ptr<Render::COverlayRenderer> m_overlay;
        std::unique_ptr<Features::CFeatureManager> m_features;
        std::unique_ptr<Config::CConfigSystem> m_config;
        std::unique_ptr<CMenuManager> m_menuManager;
        
        // Spoofer components
        std::unique_ptr<Spoofer::CAdvancedSpoofer> m_spoofer;
        std::unique_ptr<SpooferGUI::CSpooferGUI> m_spooferGUI;
        std::unique_ptr<RustEvasion::CRustAntiCheatEvasion> m_rustEvasion;

        std::thread m_updateThread;
        std::atomic<bool> m_running;
        std::atomic<bool> m_authenticated;
        std::atomic<GameState> m_gameState;
        std::atomic<bool> m_shutdownRequested;

        bool InitializeAuth(const std::string& key);
        bool InitializeSecurity();
        bool InitializeMemory();
        bool InitializeOverlay();
        bool InitializeFeatures();
        bool InitializeConfig();
        bool InitializeSpoofer();

        void LoadConfiguration();
        void SaveConfiguration();
        void UpdateLoop();

        void RenderCallback(ID3D11DeviceContext* ctx, int width, int height);

    public:
        CExternalCheat();
        ~CExternalCheat();

        bool Startup(const std::string& licenseKey);
        void Shutdown();
        void Run();

        bool IsRunning() const { return m_running; }
        bool IsAuthenticated() const { return m_authenticated; }
        bool IsWaitingForGame() const { return m_gameState.load() == GameState::WaitingForGame; }
        bool IsAttachedToGame() const { return m_gameState.load() == GameState::Attached; }

        // Spoofer access
        Spoofer::CAdvancedSpoofer* GetSpoofer() const { return m_spoofer.get(); }
        SpooferGUI::CSpooferGUI* GetSpooferGUI() const { return m_spooferGUI.get(); }
        RustEvasion::CRustAntiCheatEvasion* GetRustEvasion() const { return m_rustEvasion.get(); }
    };

}

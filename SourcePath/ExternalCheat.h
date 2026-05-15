#pragma once
#include "GameMemory.h"
#include "OverlayRenderer.h"
#include "FeatureManager.h"
#include "ConfigSystem.h"
#include "SecurityLayer.h"
#include "AuthSystem.h"
#include "MenuManager.h"
#include <memory>
#include <thread>
#include <atomic>

namespace App {

    class CExternalCheat {
    private:
        std::unique_ptr<Auth::CAuthSystem> m_auth;
        std::unique_ptr<Security::CSecurityLayer> m_security;
        std::unique_ptr<Memory::CGameMemory> m_memory;
        std::unique_ptr<Render::COverlayRenderer> m_overlay;
        std::unique_ptr<Features::CFeatureManager> m_features;
        std::unique_ptr<Config::CConfigSystem> m_config;
        std::unique_ptr<CMenuManager> m_menuManager;

        std::thread m_updateThread;
        std::atomic<bool> m_running;
        std::atomic<bool> m_authenticated;

        bool InitializeAuth(const std::string& key);
        bool InitializeSecurity();
        bool InitializeMemory();
        bool InitializeOverlay();
        bool InitializeFeatures();
        bool InitializeConfig();

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
    };

}

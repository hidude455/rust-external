#pragma once
#include "Common.h"
#include "Renderer.h"
#include "../MenuPath/imgui/imgui.h"

namespace MIT {
    struct MenuConfig {
        bool showMenu = true;
        bool showESP = true;
        bool showPerformance = true;
        bool enableRecoil = false;
        bool enablePrediction = false;
        float espDistance = 500.0f;
        float smoothness = 0.5f;
        ImVec4 playerColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        ImVec4 oreColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 lootColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        int selectedTab = 0;
    };

    class Menu {
    private:
        MenuConfig config;
        Renderer* renderer;
        
        // Animation variables
        float menuAlpha = 0.0f;
        float tabTransitionProgress = 0.0f;
        int currentTab = 0;
        int targetTab = 0;
        std::chrono::high_resolution_clock::time_point lastTabChange;
        
        // Performance monitoring
        float fpsHistory[60] = {};
        int fpsIndex = 0;
        
        void RenderTabBar();
        void RenderESPSettings();
        void RenderCombatSettings();
        void RenderVisualSettings();
        void RenderPerformanceMonitor();
        void RenderColorPicker(const char* label, ImVec4& color);
        void AnimateTransitions();
        
        // Krypton-style effects
        void RenderBackgroundEffect();
        void RenderTabTransition();
        void DrawGlowEffect(ImVec2 pos, ImVec2 size, ImU32 color, float alpha = 1.0f);

    public:
        Menu(Renderer* renderer);
        ~Menu();
        
        void Initialize();
        void Render();
        void ToggleMenu() { config.showMenu = !config.showMenu; }
        bool IsMenuVisible() const { return config.showMenu; }
        
        MenuConfig& GetConfig() { return config; }
        const MenuConfig& GetConfig() const { return config; }
        
        // Input handling
        bool HandleInput(UINT msg, WPARAM wParam, LPARAM lParam);
        
        // Utility functions
        void UpdateFPS(float newFPS);
        void SaveConfig();
        void LoadConfig();
    };
}

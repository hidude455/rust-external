#pragma once
#include "Common.h"
#include "Renderer.h"
#include "VisualEffects.h"
#include "../MenuPath/imgui/imgui.h"
#include <memory>

namespace UI {
    
    // Advanced menu configuration with animation support
    struct AdvancedMenuConfig {
        bool showMenu = true;
        bool enableAnimations = true;
        bool enableParticles = true;
        bool enableBackground = true;
        bool enableGlow = true;
        
        // Animation settings
        float menuFadeSpeed = 2.0f;
        float tabTransitionSpeed = 3.0f;
        float buttonHoverSpeed = 1.5f;
        float notificationDuration = 3.0f;
        
        // Visual settings
        ImVec4 primaryColor = ImVec4(0.1f, 0.5f, 1.0f, 1.0f);
        ImVec4 secondaryColor = ImVec4(0.8f, 0.2f, 0.8f, 1.0f);
        ImVec4 backgroundColor = ImVec4(0.05f, 0.05f, 0.1f, 0.9f);
        ImVec4 accentColor = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
        
        // ESP settings
        bool showESP = true;
        bool showPlayers = true;
        bool showOre = true;
        bool showLoot = true;
        float espDistance = 500.0f;
        ImVec4 playerESPColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        ImVec4 oreESPColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
        ImVec4 lootESPColor = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);
        
        // Combat settings
        bool enableRecoil = false;
        bool enableAimbot = false;
        bool enablePrediction = false;
        float smoothness = 0.5f;
        float aimFOV = 15.0f;
        bool aimAtHead = true;
        
        int currentTab = 0;
        int previousTab = 0;
    };
    
    // Notification system
    struct Notification {
        std::string message;
        ImVec4 color;
        float startTime;
        float duration;
        bool isActive;
    };
    
    // Button animation state
    struct ButtonState {
        bool isHovered;
        bool isPressed;
        float hoverProgress;
        float pressProgress;
        ImVec2 size;
        ImVec2 position;
    };
    
    class CAdvancedMenu {
    private:
        std::unique_ptr<Graphics::CVisualEffects> m_visualEffects;
        AdvancedMenuConfig m_config;
        Renderer* m_renderer;
        
        // Animation state
        float m_menuAlpha;
        float m_tabTransitionProgress;
        std::chrono::high_resolution_clock::time_point m_lastUpdateTime;
        std::chrono::high_resolution_clock::time_point m_tabChangeTime;
        
        // Notification system
        std::vector<Notification> m_notifications;
        
        // Button states
        std::unordered_map<std::string, ButtonState> m_buttonStates;
        
        // Background animation
        ImVec2 m_menuPosition;
        ImVec2 m_menuSize;
        float m_backgroundOffset;
        
        // Internal helper functions
        void UpdateAnimations();
        void UpdateNotifications();
        void UpdateButtonStates();
        void RenderBackground();
        void RenderNotifications();
        void RenderTabBar();
        void RenderESPSettings();
        void RenderCombatSettings();
        void RenderVisualSettings();
        void RenderAdvancedSettings();
        
        // UI element rendering
        bool RenderAnimatedButton(const std::string& id, const char* label, const ImVec2& size = ImVec2(0, 0));
        bool RenderAnimatedCheckbox(const std::string& id, const char* label, bool* value);
        bool RenderAnimatedSlider(const std::string& id, const char* label, float* value, float min, float max);
        void RenderAnimatedColorPicker(const std::string& id, const char* label, ImVec4* color);
        
        // Animation utilities
        float SmoothStep(float edge0, float edge1, float x);
        float Lerp(float a, float b, float t);
        ImVec4 LerpColor(const ImVec4& a, const ImVec4& b, float t);
        
        // Notification utilities
        void AddNotification(const std::string& message, const ImVec4& color);
        void ClearNotifications();
        
    public:
        CAdvancedMenu(Renderer* renderer);
        ~CAdvancedMenu();
        
        // Initialization
        void Initialize();
        void Shutdown();
        
        // Main rendering
        void Render();
        
        // Menu control
        void ToggleMenu();
        bool IsMenuVisible() const;
        
        // Configuration
        AdvancedMenuConfig& GetConfig() { return m_config; }
        const AdvancedMenuConfig& GetConfig() const { return m_config; }
        
        // Visual effects access
        Graphics::CVisualEffects* GetVisualEffects() { return m_visualEffects.get(); }
        
        // Input handling
        bool HandleInput(UINT msg, WPARAM wParam, LPARAM lParam);
        
        // Configuration management
        void SaveConfig();
        void LoadConfig();
        
        // Utility functions
        void ShowNotification(const std::string& message, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        void SetMenuPosition(const ImVec2& position);
        void SetMenuSize(const ImVec2& size);
    };
    
} // namespace UI

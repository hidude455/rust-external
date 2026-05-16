#pragma once
#include "../MenuPath/imgui/imgui.h"
#include "AdvancedSpoofer.h"
#include <string>
#include <vector>
#include <chrono>
#include <functional>

namespace SpooferGUI {

    // GUI Configuration
    struct SpooferGUIConfig {
        ImVec4 accentColor = ImVec4(0.0f, 0.7f, 1.0f, 1.0f);  // Cyan accent
        ImVec4 successColor = ImVec4(0.0f, 1.0f, 0.5f, 1.0f);  // Green
        ImVec4 warningColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);  // Yellow
        ImVec4 errorColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);    // Red
        ImVec4 bgColor = ImVec4(0.08f, 0.08f, 0.1f, 1.0f);     // Dark background
        ImVec4 panelColor = ImVec4(0.12f, 0.12f, 0.15f, 0.95f); // Panel
        ImVec4 textColor = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);  // White text
        
        bool showAnimations = true;
        bool showStatusIndicators = true;
        bool showDetailedInfo = true;
    };

    // Animation states
    struct AnimationState {
        float spoofProgress = 0.0f;
        float restoreProgress = 0.0f;
        float pulseIntensity = 0.0f;
        float fadeIn = 0.0f;
        
        std::chrono::steady_clock::time_point lastUpdate;
    };

    // Spoofer GUI class
    class CSpooferGUI {
    private:
        Spoofer::CAdvancedSpoofer* m_spoofer;
        SpooferGUIConfig m_guiConfig;
        AnimationState m_animState;
        
        bool m_showWindow;
        bool m_showSettings;
        bool m_showLogs;
        
        std::vector<std::string> m_logMessages;
        int m_maxLogMessages = 50;
        
        // Custom widget rendering
        void RenderToggle(const char* label, bool* value, const char* tooltip = nullptr);
        void RenderButton(const char* label, const ImVec4& color, bool* clicked);
        void RenderProgressBar(float progress, const ImVec4& color);
        void RenderStatusIndicator(const char* label, bool active, const ImVec4& color);
        void RenderCard(const char* title, const std::function<void()>& content);
        
        // Main sections
        void RenderHeader();
        void RenderMainPanel();
        void RenderQuickActions();
        void RenderIdentifierToggles();
        void RenderAdvancedOptions();
        void RenderStatusPanel();
        void RenderSettingsPanel();
        void RenderLogsPanel();
        
        // Helper methods
        void AddLog(const std::string& message);
        void UpdateAnimations();
        ImVec4 GetStatusColor(Spoofer::SpoofStatus status);
        const char* GetStatusText(Spoofer::SpoofStatus status);
        
    public:
        CSpooferGUI(Spoofer::CAdvancedSpoofer* spoofer);
        ~CSpooferGUI();
        
        bool Initialize();
        void Shutdown();
        
        void Render();
        void ToggleWindow() { m_showWindow = !m_showWindow; }
        
        bool IsVisible() const { return m_showWindow; }
        void SetVisible(bool visible) { m_showWindow = visible; }
        
        const SpooferGUIConfig& GetConfig() const { return m_guiConfig; }
        void SetConfig(const SpooferGUIConfig& config) { m_guiConfig = config; }
    };

}

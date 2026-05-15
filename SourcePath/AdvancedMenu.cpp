#include "AdvancedMenu.h"
#include <algorithm>
#include <fstream>
#include <chrono>

namespace UI {
    
    CAdvancedMenu::CAdvancedMenu(Renderer* renderer) 
        : m_renderer(renderer), m_menuAlpha(0.0f), m_tabTransitionProgress(0.0f), 
          m_backgroundOffset(0.0f) {
        m_lastUpdateTime = std::chrono::high_resolution_clock::now();
        m_tabChangeTime = m_lastUpdateTime;
        
        // Initialize visual effects
        m_visualEffects = std::make_unique<Graphics::CVisualEffects>();
        
        // Set initial menu position and size
        m_menuPosition = ImVec2(100, 100);
        m_menuSize = ImVec2(600, 450);
        
        // Reserve space for notifications
        m_notifications.reserve(10);
    }
    
    CAdvancedMenu::~CAdvancedMenu() {
        Shutdown();
    }
    
    void CAdvancedMenu::Initialize() {
        if (m_visualEffects) {
            m_visualEffects->Initialize();
            m_visualEffects->SetBackgroundColors(m_config.backgroundColor, m_config.primaryColor);
            m_visualEffects->EnableBackgroundAnimation(m_config.enableBackground);
            m_visualEffects->SetEffectIntensity(1.0f);
        }
        
        LoadConfig();
        
        // Add welcome notification
        AddNotification("MIT Method Loaded Successfully", m_config.accentColor);
    }
    
    void CAdvancedMenu::Shutdown() {
        SaveConfig();
        
        if (m_visualEffects) {
            m_visualEffects->Shutdown();
        }
        
        ClearNotifications();
    }
    
    void CAdvancedMenu::Render() {
        if (!m_config.showMenu) return;
        
        UpdateAnimations();
        UpdateNotifications();
        UpdateButtonStates();
        
        // Set window flags for modern appearance
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        
        // Set window position and size
        ImGui::SetNextWindowPos(m_menuPosition, ImGuiCond_Always);
        ImGui::SetNextWindowSize(m_menuSize, ImGuiCond_Always);
        
        // Apply menu alpha for fade effect
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_menuAlpha);
        
        if (ImGui::Begin("##MITMethodMenu", nullptr, flags)) {
            // Render background effects
            if (m_config.enableBackground) {
                RenderBackground();
            }
            
            // Render main content
            RenderTabBar();
            
            // Render current tab content
            switch (m_config.currentTab) {
                case 0:
                    RenderESPSettings();
                    break;
                case 1:
                    RenderCombatSettings();
                    break;
                case 2:
                    RenderVisualSettings();
                    break;
                case 3:
                    RenderAdvancedSettings();
                    break;
                default:
                    RenderESPSettings();
                    break;
            }
        }
        ImGui::End();
        
        ImGui::PopStyleVar();
        
        // Render notifications
        RenderNotifications();
        
        // Update visual effects
        if (m_visualEffects) {
            m_visualEffects->Update();
            m_visualEffects->Render(m_menuPosition, m_menuSize);
        }
    }
    
    void CAdvancedMenu::UpdateAnimations() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - m_lastUpdateTime).count();
        m_lastUpdateTime = currentTime;
        
        // Menu fade animation
        if (m_config.showMenu && m_menuAlpha < 1.0f) {
            m_menuAlpha += deltaTime * m_config.menuFadeSpeed;
            m_menuAlpha = std::min(m_menuAlpha, 1.0f);
        } else if (!m_config.showMenu && m_menuAlpha > 0.0f) {
            m_menuAlpha -= deltaTime * m_config.menuFadeSpeed;
            m_menuAlpha = std::max(m_menuAlpha, 0.0f);
        }
        
        // Tab transition animation
        if (m_config.currentTab != m_config.previousTab) {
            auto tabElapsed = std::chrono::duration<float>(currentTime - m_tabChangeTime).count();
            m_tabTransitionProgress = std::min(1.0f, tabElapsed * m_config.tabTransitionSpeed);
            
            if (m_tabTransitionProgress >= 1.0f) {
                m_config.previousTab = m_config.currentTab;
                m_tabTransitionProgress = 0.0f;
            }
        }
        
        // Background animation offset
        if (m_config.enableBackground) {
            m_backgroundOffset += deltaTime * 0.5f;
            if (m_backgroundOffset > 1.0f) {
                m_backgroundOffset -= 1.0f;
            }
        }
    }
    
    void CAdvancedMenu::UpdateNotifications() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float currentFloat = std::chrono::duration<float>(currentTime.time_since_epoch()).count();
        
        // Remove expired notifications
        m_notifications.erase(
            std::remove_if(m_notifications.begin(), m_notifications.end(),
                [currentFloat](const Notification& notif) {
                    return !notif.isActive || (currentFloat - notif.startTime) > notif.duration;
                }),
            m_notifications.end()
        );
    }
    
    void CAdvancedMenu::UpdateButtonStates() {
        // Update button hover and press animations
        for (auto& [id, state] : m_buttonStates) {
            // Smooth hover animation
            float targetHoverProgress = state.isHovered ? 1.0f : 0.0f;
            state.hoverProgress += (targetHoverProgress - state.hoverProgress) * 0.1f;
            
            // Smooth press animation
            float targetPressProgress = state.isPressed ? 1.0f : 0.0f;
            state.pressProgress += (targetPressProgress - state.pressProgress) * 0.15f;
        }
    }
    
    void CAdvancedMenu::RenderBackground() {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        
        // Create animated gradient background
        ImVec4 gradientColor1 = LerpColor(m_config.backgroundColor, m_config.primaryColor, m_backgroundOffset);
        ImVec4 gradientColor2 = LerpColor(m_config.primaryColor, m_config.backgroundColor, m_backgroundOffset);
        
        ImU32 color1 = IM_COL32(
            static_cast<int>(gradientColor1.x * 255),
            static_cast<int>(gradientColor1.y * 255),
            static_cast<int>(gradientColor1.z * 255),
            static_cast<int>(gradientColor1.w * 255)
        );
        
        ImU32 color2 = IM_COL32(
            static_cast<int>(gradientColor2.x * 255),
            static_cast<int>(gradientColor2.y * 255),
            static_cast<int>(gradientColor2.z * 255),
            static_cast<int>(gradientColor2.w * 255)
        );
        
        // Draw gradient background
        drawList->AddRectFilledMultiColor(
            windowPos,
            ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
            color1, color2, color1, color2
        );
        
        // Add subtle glow effect
        if (m_config.enableGlow && m_visualEffects) {
            m_visualEffects->CreatePulsingGlow(
                windowPos,
                windowSize,
                m_config.accentColor,
                2.0f
            );
        }
    }
    
    void CAdvancedMenu::RenderNotifications() {
        if (m_notifications.empty()) return;
        
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 startPos = ImVec2(viewport->WorkPos.x + 10, viewport->WorkPos.y + 50);
        
        ImDrawList* drawList = ImGui::GetForegroundDrawList();
        
        for (size_t i = 0; i < m_notifications.size(); ++i) {
            const auto& notif = m_notifications[i];
            if (!notif.isActive) continue;
            
            ImVec2 notifPos = ImVec2(startPos.x, startPos.y + (i * 35));
            ImVec2 notifSize = ImVec2(250, 30);
            
            // Calculate notification alpha based on age
            auto currentTime = std::chrono::high_resolution_clock::now();
            float currentFloat = std::chrono::duration<float>(currentTime.time_since_epoch()).count();
            float age = currentFloat - notif.startTime;
            float alpha = 1.0f;
            
            if (age > notif.duration - 1.0f) {
                alpha = notif.duration - age;
            }
            
            // Draw notification background
            ImU32 bgColor = IM_COL32(
                static_cast<int>(notif.color.x * 255 * 0.2f),
                static_cast<int>(notif.color.y * 255 * 0.2f),
                static_cast<int>(notif.color.z * 255 * 0.2f),
                static_cast<int>(alpha * 200)
            );
            
            drawList->AddRectFilled(notifPos, ImVec2(notifPos.x + notifSize.x, notifPos.y + notifSize.y), bgColor, 5.0f);
            
            // Draw notification border
            ImU32 borderColor = IM_COL32(
                static_cast<int>(notif.color.x * 255),
                static_cast<int>(notif.color.y * 255),
                static_cast<int>(notif.color.z * 255),
                static_cast<int>(alpha * 255)
            );
            
            drawList->AddRect(notifPos, ImVec2(notifPos.x + notifSize.x, notifPos.y + notifSize.y), borderColor, 5.0f, 0, 2.0f);
            
            // Draw notification text
            ImU32 textColor = IM_COL32(255, 255, 255, static_cast<int>(alpha * 255));
            drawList->AddText(ImVec2(notifPos.x + 10, notifPos.y + 8), textColor, notif.message.c_str());
        }
    }
    
    void CAdvancedMenu::RenderTabBar() {
        const char* tabNames[] = { "ESP", "Combat", "Visuals", "Advanced" };
        const int tabCount = sizeof(tabNames) / sizeof(tabNames[0]);
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
        
        for (int i = 0; i < tabCount; ++i) {
            std::string buttonId = "tab_" + std::string(tabNames[i]);
            
            ImVec4 buttonColor = (i == m_config.currentTab) ? m_config.accentColor : m_config.primaryColor;
            
            if (RenderAnimatedButton(buttonId, tabNames[i], ImVec2(ImGui::GetContentRegionAvail().x / tabCount - 3, 35))) {
                if (m_config.currentTab != i) {
                    m_config.previousTab = m_config.currentTab;
                    m_config.currentTab = i;
                    m_tabChangeTime = std::chrono::high_resolution_clock::now();
                    m_tabTransitionProgress = 0.0f;
                    
                    AddNotification("Switched to " + std::string(tabNames[i]) + " tab", m_config.accentColor);
                }
            }
            
            if (i < tabCount - 1) {
                ImGui::SameLine();
            }
        }
        
        ImGui::PopStyleVar();
        ImGui::Separator();
    }
    
    void CAdvancedMenu::RenderESPSettings() {
        ImGui::Text("ESP Configuration");
        ImGui::Separator();
        
        if (RenderAnimatedCheckbox("esp_enabled", "Enable ESP", &m_config.showESP)) {
            AddNotification(m_config.showESP ? "ESP Enabled" : "ESP Disabled", 
                          m_config.showESP ? m_config.playerESPColor : ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
        }
        
        if (RenderAnimatedSlider("esp_distance", "Max Distance", &m_config.espDistance, 50.0f, 1000.0f)) {
            if (m_visualEffects) {
                m_visualEffects->EmitParticles(ImGui::GetCursorScreenPos(), m_config.playerESPColor, 5);
            }
        }
        
        ImGui::Text("Entity Filters:");
        RenderAnimatedCheckbox("esp_players", "Show Players", &m_config.showPlayers);
        ImGui::SameLine();
        RenderAnimatedCheckbox("esp_ore", "Show Ore", &m_config.showOre);
        ImGui::SameLine();
        RenderAnimatedCheckbox("esp_loot", "Show Loot", &m_config.showLoot);
        
        ImGui::Text("Entity Colors:");
        RenderAnimatedColorPicker("esp_player_color", "Players", &m_config.playerESPColor);
        RenderAnimatedColorPicker("esp_ore_color", "Ore", &m_config.oreESPColor);
        RenderAnimatedColorPicker("esp_loot_color", "Loot", &m_config.lootESPColor);
    }
    
    void CAdvancedMenu::RenderCombatSettings() {
        ImGui::Text("Combat Configuration");
        ImGui::Separator();
        
        if (RenderAnimatedCheckbox("combat_recoil", "Recoil Compensation", &m_config.enableRecoil)) {
            AddNotification(m_config.enableRecoil ? "Recoil Compensation Enabled" : "Recoil Compensation Disabled",
                          m_config.secondaryColor);
        }
        
        RenderAnimatedCheckbox("combat_aimbot", "Aim Assistance", &m_config.enableAimbot);
        RenderAnimatedCheckbox("combat_prediction", "Projectile Prediction", &m_config.enablePrediction);
        
        if (RenderAnimatedSlider("combat_smoothness", "Smoothness", &m_config.smoothness, 0.0f, 1.0f)) {
            // Create particles on slider change
            if (m_visualEffects) {
                m_visualEffects->EmitParticles(ImGui::GetCursorScreenPos(), m_config.secondaryColor, 3);
            }
        }
        
        RenderAnimatedSlider("combat_fov", "Aim FOV", &m_config.aimFOV, 5.0f, 50.0f);
        RenderAnimatedCheckbox("combat_head", "Aim at Head", &m_config.aimAtHead);
    }
    
    void CAdvancedMenu::RenderVisualSettings() {
        ImGui::Text("Visual Configuration");
        ImGui::Separator();
        
        RenderAnimatedCheckbox("visual_animations", "Enable Animations", &m_config.enableAnimations);
        RenderAnimatedCheckbox("visual_particles", "Enable Particles", &m_config.enableParticles);
        RenderAnimatedCheckbox("visual_background", "Enable Background", &m_config.enableBackground);
        RenderAnimatedCheckbox("visual_glow", "Enable Glow Effects", &m_config.enableGlow);
        
        ImGui::Text("Animation Speed:");
        RenderAnimatedSlider("visual_menu_fade", "Menu Fade", &m_config.menuFadeSpeed, 0.5f, 5.0f);
        RenderAnimatedSlider("visual_tab_transition", "Tab Transition", &m_config.tabTransitionSpeed, 0.5f, 5.0f);
        
        ImGui::Text("Color Scheme:");
        RenderAnimatedColorPicker("visual_primary", "Primary", &m_config.primaryColor);
        RenderAnimatedColorPicker("visual_secondary", "Secondary", &m_config.secondaryColor);
        RenderAnimatedColorPicker("visual_accent", "Accent", &m_config.accentColor);
    }
    
    void CAdvancedMenu::RenderAdvancedSettings() {
        ImGui::Text("Advanced Configuration");
        ImGui::Separator();
        
        ImGui::Text("Performance:");
        static bool optimizePerformance = false;
        if (RenderAnimatedCheckbox("advanced_optimize", "Optimize for Performance", &optimizePerformance)) {
            if (optimizePerformance && m_visualEffects) {
                m_visualEffects->OptimizeForPerformance();
                AddNotification("Performance optimization enabled", m_config.accentColor);
            }
        }
        
        ImGui::Text("Debug:");
        static bool showDebugInfo = false;
        RenderAnimatedCheckbox("advanced_debug", "Show Debug Info", &showDebugInfo);
        
        if (showDebugInfo) {
            ImGui::Text("Active Particles: %zu", m_visualEffects ? m_visualEffects->GetActiveParticleCount() : 0);
            ImGui::Text("Menu Alpha: %.2f", m_menuAlpha);
            ImGui::Text("Tab Transition: %.2f", m_tabTransitionProgress);
            ImGui::Text("Notifications: %zu", m_notifications.size());
        }
        
        if (RenderAnimatedButton("advanced_reset", "Reset to Defaults")) {
            // Reset configuration
            m_config = AdvancedMenuConfig();
            AddNotification("Configuration reset to defaults", ImVec4(1.0f, 1.0f, 0.2f, 1.0f));
        }
    }
    
    bool CAdvancedMenu::RenderAnimatedButton(const std::string& id, const char* label, const ImVec2& size) {
        // Get or create button state
        ButtonState& state = m_buttonStates[id];
        
        // Check hover state
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImVec2 buttonSize = size.x > 0 ? size : ImVec2(ImGui::GetContentRegionAvail().x, 0);
        state.isHovered = ImGui::IsMouseHoveringRect(cursorPos, ImVec2(cursorPos.x + buttonSize.x, cursorPos.y + buttonSize.y));
        state.isPressed = ImGui::IsMouseClicked(0) && state.isHovered;
        
        // Calculate animated color
        ImVec4 baseColor = m_config.primaryColor;
        ImVec4 hoverColor = m_config.accentColor;
        ImVec4 currentColor = LerpColor(baseColor, hoverColor, state.hoverProgress);
        
        // Apply press effect
        if (state.pressProgress > 0.0f) {
            currentColor = LerpColor(currentColor, ImVec4(0.2f, 0.2f, 0.2f, 1.0f), state.pressProgress);
        }
        
        // Push custom style
        ImGui::PushStyleColor(ImGuiCol_Button, currentColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, currentColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, currentColor);
        
        // Render button
        bool clicked = ImGui::Button(label, buttonSize);
        
        // Pop style
        ImGui::PopStyleColor(3);
        
        // Create particles on click
        if (clicked && m_visualEffects && m_config.enableParticles) {
            ImVec2 center = ImVec2(cursorPos.x + buttonSize.x / 2, cursorPos.y + buttonSize.y / 2);
            m_visualEffects->EmitParticles(center, m_config.accentColor, 10);
        }
        
        return clicked;
    }
    
    bool CAdvancedMenu::RenderAnimatedCheckbox(const std::string& id, const char* label, bool* value) {
        bool changed = ImGui::Checkbox(label, value);
        
        if (changed && m_visualEffects && m_config.enableParticles) {
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            m_visualEffects->EmitParticles(cursorPos, *value ? m_config.playerESPColor : m_config.primaryColor, 5);
        }
        
        return changed;
    }
    
    bool CAdvancedMenu::RenderAnimatedSlider(const std::string& id, const char* label, float* value, float min, float max) {
        return ImGui::SliderFloat(label, value, min, max);
    }
    
    void CAdvancedMenu::RenderAnimatedColorPicker(const std::string& id, const char* label, ImVec4* color) {
        ImGui::ColorEdit4(label, (float*)color, ImGuiColorEditFlags_NoInputs);
    }
    
    float CAdvancedMenu::SmoothStep(float edge0, float edge1, float x) {
        float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }
    
    float CAdvancedMenu::Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    
    ImVec4 CAdvancedMenu::LerpColor(const ImVec4& a, const ImVec4& b, float t) {
        return ImVec4(
            Lerp(a.x, b.x, t),
            Lerp(a.y, b.y, t),
            Lerp(a.z, b.z, t),
            Lerp(a.w, b.w, t)
        );
    }
    
    void CAdvancedMenu::AddNotification(const std::string& message, const ImVec4& color) {
        Notification notif;
        notif.message = message;
        notif.color = color;
        notif.startTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        notif.duration = m_config.notificationDuration;
        notif.isActive = true;
        
        m_notifications.push_back(notif);
        
        // Limit notification count
        if (m_notifications.size() > 5) {
            m_notifications.erase(m_notifications.begin());
        }
    }
    
    void CAdvancedMenu::ClearNotifications() {
        m_notifications.clear();
    }
    
    void CAdvancedMenu::ToggleMenu() {
        m_config.showMenu = !m_config.showMenu;
        
        if (!m_config.showMenu) {
            ClearNotifications();
        }
    }
    
    bool CAdvancedMenu::IsMenuVisible() const {
        return m_config.showMenu && m_menuAlpha > 0.1f;
    }
    
    bool CAdvancedMenu::HandleInput(UINT msg, WPARAM wParam, LPARAM lParam) {
        return ImGui_ImplWin32_WndProcHandler(nullptr, msg, wParam, lParam);
    }
    
    void CAdvancedMenu::SaveConfig() {
        std::ofstream file("advanced_config.ini");
        if (file.is_open()) {
            file << "showESP=" << m_config.showESP << "\n";
            file << "espDistance=" << m_config.espDistance << "\n";
            file << "smoothness=" << m_config.smoothness << "\n";
            file << "enableRecoil=" << m_config.enableRecoil << "\n";
            file << "enableAimbot=" << m_config.enableAimbot << "\n";
            file.close();
        }
    }
    
    void CAdvancedMenu::LoadConfig() {
        std::ifstream file("advanced_config.ini");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.find("showESP=") == 0) {
                    m_config.showESP = std::stoi(line.substr(8));
                } else if (line.find("espDistance=") == 0) {
                    m_config.espDistance = std::stof(line.substr(12));
                } else if (line.find("smoothness=") == 0) {
                    m_config.smoothness = std::stof(line.substr(11));
                } else if (line.find("enableRecoil=") == 0) {
                    m_config.enableRecoil = std::stoi(line.substr(13));
                } else if (line.find("enableAimbot=") == 0) {
                    m_config.enableAimbot = std::stoi(line.substr(13));
                }
            }
            file.close();
        }
    }
    
    void CAdvancedMenu::ShowNotification(const std::string& message, const ImVec4& color) {
        AddNotification(message, color);
    }
    
    void CAdvancedMenu::SetMenuPosition(const ImVec2& position) {
        m_menuPosition = position;
    }
    
    void CAdvancedMenu::SetMenuSize(const ImVec2& size) {
        m_menuSize = size;
    }
    
} // namespace UI

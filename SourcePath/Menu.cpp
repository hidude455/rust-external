#include "Menu.h"
#include <fstream>
#include <algorithm>

namespace MIT {
    Menu::Menu(Renderer* renderer) : renderer(renderer), currentTab(0), targetTab(0), 
                                    tabTransitionProgress(0.0f), fpsIndex(0) {
        lastTabChange = std::chrono::high_resolution_clock::now();
    }

    Menu::~Menu() {
    }

    void Menu::Initialize() {
        LoadConfig();
        LOG_INFO("Menu initialized");
    }

    void Menu::Render() {
        if (!config.showMenu) return;
        
        AnimateTransitions();
        
        // Set window flags for Krypton-style appearance
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
        
        // Main menu window
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menuAlpha);
        
        if (ImGui::Begin("MIT Method", nullptr, flags)) {
            RenderBackgroundEffect();
            RenderTabBar();
            RenderTabTransition();
            
            // Render selected tab content
            switch (currentTab) {
                case 0:
                    RenderESPSettings();
                    break;
                case 1:
                    RenderCombatSettings();
                    break;
                case 2:
                    RenderVisualSettings();
                    break;
                default:
                    RenderESPSettings();
                    break;
            }
        }
        ImGui::End();
        
        ImGui::PopStyleVar();
        
        // Render performance monitor if enabled
        if (config.showPerformance) {
            RenderPerformanceMonitor();
        }
    }

    void Menu::RenderTabBar() {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        
        const char* tabNames[] = { "ESP", "Combat", "Visuals" };
        const int tabCount = sizeof(tabNames) / sizeof(tabNames[0]);
        
        for (int i = 0; i < tabCount; ++i) {
            bool isActive = (i == currentTab);
            
            if (isActive) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.47f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.47f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.28f, 0.54f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.47f, 1.0f, 0.67f));
            }
            
            if (ImGui::Button(tabNames[i], ImVec2(ImGui::GetContentRegionAvail().x / tabCount - 4, 30))) {
                targetTab = i;
                lastTabChange = std::chrono::high_resolution_clock::now();
            }
            
            ImGui::PopStyleColor(2);
            
            if (i < tabCount - 1) {
                ImGui::SameLine();
            }
        }
        
        ImGui::PopStyleVar();
        ImGui::Separator();
    }

    void Menu::RenderESPSettings() {
        ImGui::Text("ESP Settings");
        ImGui::Separator();
        
        ImGui::Checkbox("Enable ESP", &config.showESP);
        ImGui::SliderFloat("Max Distance", &config.espDistance, 50.0f, 1000.0f);
        
        ImGui::Text("Entity Colors:");
        RenderColorPicker("Players", config.playerColor);
        RenderColorPicker("Ore", config.oreColor);
        RenderColorPicker("Loot", config.lootColor);
        
        ImGui::Separator();
        ImGui::Text("Filter Settings:");
        static bool filterPlayers = true;
        static bool filterOre = true;
        static bool filterLoot = true;
        
        ImGui::Checkbox("Show Players", &filterPlayers);
        ImGui::SameLine();
        ImGui::Checkbox("Show Ore", &filterOre);
        ImGui::SameLine();
        ImGui::Checkbox("Show Loot", &filterLoot);
    }

    void Menu::RenderCombatSettings() {
        ImGui::Text("Combat Settings");
        ImGui::Separator();
        
        ImGui::Checkbox("Enable Recoil Compensation", &config.enableRecoil);
        ImGui::Checkbox("Enable Projectile Prediction", &config.enablePrediction);
        
        ImGui::SliderFloat("Smoothness", &config.smoothness, 0.0f, 1.0f);
        
        ImGui::Separator();
        ImGui::Text("Weapon Configuration:");
        
        static int selectedWeapon = 0;
        const char* weapons[] = { "Assault Rifle", "SMG", "Sniper", "Pistol" };
        
        if (ImGui::Combo("Weapon", &selectedWeapon, weapons, IM_ARRAYSIZE(weapons))) {
            // Load weapon-specific settings
        }
        
        ImGui::Text("Recoil Pattern:");
        ImGui::Text("Bullet Velocity: %.1f m/s", 850.0f);
        ImGui::Text("Gravity Modifier: %.2f", 0.98f);
    }

    void Menu::RenderVisualSettings() {
        ImGui::Text("Visual Settings");
        ImGui::Separator();
        
        ImGui::Checkbox("Show Performance Monitor", &config.showPerformance);
        
        ImGui::Text("Menu Appearance:");
        static float menuScale = 1.0f;
        ImGui::SliderFloat("Menu Scale", &menuScale, 0.5f, 2.0f);
        
        static bool enableGlow = true;
        static bool enableAnimations = true;
        
        ImGui::Checkbox("Enable Glow Effects", &enableGlow);
        ImGui::Checkbox("Enable Animations", &enableAnimations);
        
        ImGui::Separator();
        ImGui::Text("Overlay Settings:");
        
        static bool showCrosshair = false;
        static bool showFOV = false;
        
        ImGui::Checkbox("Show Crosshair", &showCrosshair);
        ImGui::Checkbox("Show FOV Circle", &showFOV);
        
        if (showFOV) {
            static float fov = 90.0f;
            ImGui::SliderFloat("FOV", &fov, 60.0f, 120.0f);
        }
    }

    void Menu::RenderPerformanceMonitor() {
        // Performance monitor in corner
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 10, viewport->WorkPos.y + 10));
        ImGui::SetNextWindowBgAlpha(0.3f);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing | 
                                ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;
        
        if (ImGui::Begin("Performance", nullptr, flags)) {
            ImGui::Text("FPS: %.1f", renderer->GetFPS());
            ImGui::Text("Frame Time: %.2f ms", renderer->GetFrameTime());
            
            // Mini FPS graph
            float maxFPS = 120.0f;
            float graphHeight = 50.0f;
            float graphWidth = 200.0f;
            
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 graphPos = ImGui::GetCursorScreenPos();
            
            // Draw graph background
            drawList->AddRectFilled(graphPos, ImVec2(graphPos.x + graphWidth, graphPos.y + graphHeight), 
                                   IM_COL32(20, 20, 30, 180));
            
            // Draw FPS line
            for (int i = 0; i < 59; ++i) {
                float x1 = graphPos.x + (i * graphWidth / 60.0f);
                float x2 = graphPos.x + ((i + 1) * graphWidth / 60.0f);
                float y1 = graphPos.y + graphHeight - (fpsHistory[i] / maxFPS * graphHeight);
                float y2 = graphPos.y + graphHeight - (fpsHistory[i + 1] / maxFPS * graphHeight);
                
                drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(0, 255, 0, 255), 2.0f);
            }
            
            ImGui::Dummy(ImVec2(graphWidth, graphHeight));
        }
        ImGui::End();
    }

    void Menu::RenderColorPicker(const char* label, ImVec4& color) {
        ImGui::PushID(label);
        
        // Color preview button
        if (ImGui::ColorButton(label, color, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20))) {
            // Open color picker
        }
        
        ImGui::SameLine();
        ImGui::Text("%s", label);
        
        if (ImGui::BeginPopupContextItem(label)) {
            ImGui::ColorEdit4("##Color", (float*)&color, ImGuiColorEditFlags_NoInputs);
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        
        ImGui::PopID();
    }

    void Menu::AnimateTransitions() {
        // Menu fade-in animation
        if (config.showMenu && menuAlpha < 1.0f) {
            menuAlpha += 0.05f;
            if (menuAlpha > 1.0f) menuAlpha = 1.0f;
        } else if (!config.showMenu && menuAlpha > 0.0f) {
            menuAlpha -= 0.05f;
            if (menuAlpha < 0.0f) menuAlpha = 0.0f;
        }
        
        // Tab transition animation
        if (currentTab != targetTab) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTabChange).count();
            
            tabTransitionProgress = std::min(1.0f, elapsed / 200.0f); // 200ms transition
            
            if (tabTransitionProgress >= 1.0f) {
                currentTab = targetTab;
                tabTransitionProgress = 0.0f;
            }
        }
    }

    void Menu::RenderBackgroundEffect() {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        
        // Subtle gradient background
        for (int i = 0; i < 10; ++i) {
            float alpha = 0.02f * (10 - i);
            ImU32 color = IM_COL32(0, 120, 255, static_cast<int>(alpha * 255));
            drawList->AddRectFilled(
                ImVec2(windowPos.x + i, windowPos.y + i),
                ImVec2(windowPos.x + windowSize.x - i, windowPos.y + windowSize.y - i),
                color
            );
        }
    }

    void Menu::RenderTabTransition() {
        if (tabTransitionProgress > 0.0f && currentTab != targetTab) {
            // Smooth transition effect
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            
            float alpha = (1.0f - tabTransitionProgress) * 0.5f;
            ImU32 color = IM_COL32(0, 120, 255, static_cast<int>(alpha * 255));
            
            drawList->AddRectFilled(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), color);
        }
    }

    void Menu::DrawGlowEffect(ImVec2 pos, ImVec2 size, ImU32 color, float alpha) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Draw glow layers
        for (int i = 3; i > 0; --i) {
            float glowAlpha = alpha * (0.1f / i);
            ImU32 glowColor = (color & ~IM_COL32_A_MASK) | static_cast<ImU32>(glowAlpha * 255);
            
            drawList->AddRect(
                ImVec2(pos.x - i, pos.y - i),
                ImVec2(pos.x + size.x + i, pos.y + size.y + i),
                glowColor, 4.0f, 0, 2.0f
            );
        }
    }

    bool Menu::HandleInput(UINT msg, WPARAM wParam, LPARAM lParam) {
        // Forward input to ImGui
        return ImGui_ImplWin32_WndProcHandler(nullptr, msg, wParam, lParam);
    }

    void Menu::UpdateFPS(float newFPS) {
        fpsHistory[fpsIndex] = newFPS;
        fpsIndex = (fpsIndex + 1) % 60;
    }

    void Menu::SaveConfig() {
        std::ofstream file("config.ini");
        if (file.is_open()) {
            file << "showESP=" << config.showESP << "\n";
            file << "espDistance=" << config.espDistance << "\n";
            file << "smoothness=" << config.smoothness << "\n";
            file.close();
            LOG_INFO("Configuration saved");
        }
    }

    void Menu::LoadConfig() {
        std::ifstream file("config.ini");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.find("showESP=") == 0) {
                    config.showESP = std::stoi(line.substr(8));
                } else if (line.find("espDistance=") == 0) {
                    config.espDistance = std::stof(line.substr(12));
                } else if (line.find("smoothness=") == 0) {
                    config.smoothness = std::stof(line.substr(11));
                }
            }
            file.close();
            LOG_INFO("Configuration loaded");
        }
    }
}

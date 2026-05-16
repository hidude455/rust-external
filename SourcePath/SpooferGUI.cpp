#include "SpooferGUI.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace SpooferGUI {

    CSpooferGUI::CSpooferGUI(Spoofer::CAdvancedSpoofer* spoofer)
        : m_spoofer(spoofer), m_showWindow(true), m_showSettings(false), m_showLogs(false) {
        
        m_animState.lastUpdate = std::chrono::steady_clock::now();
        AddLog("Spoofer GUI initialized");
    }

    CSpooferGUI::~CSpooferGUI() {
        if (m_spoofer) {
            m_spoofer->Shutdown();
        }
    }

    bool CSpooferGUI::Initialize() {
        if (!m_spoofer) return false;
        
        // Initialize spoofer with default config
        Spoofer::SpooferConfig config;
        config.randomizeAll = true;
        config.enableWMIProtection = true;
        config.enableRegistryProtection = true;
        config.enableDriverProtection = true;
        config.enableNetworkProtection = true;
        config.enablePCISpoofing = true;
        config.createBackup = true;
        
        m_spoofer->Initialize(config);
        AddLog("Advanced spoofer initialized");
        
        return true;
    }

    void CSpooferGUI::Shutdown() {
        // Note: Spoofer shutdown is handled in destructor
        // This method is for GUI-specific cleanup if needed
        AddLog("Spoofer GUI shutdown");
    }

    void CSpooferGUI::Render() {
        if (!m_showWindow) return;
        if (!m_spoofer) return;
        
        UpdateAnimations();
        
        ImGui::SetNextWindowSize(ImVec2(700, 550), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, m_guiConfig.bgColor);
        ImGui::PushStyleColor(ImGuiCol_TitleBg, m_guiConfig.panelColor);
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, m_guiConfig.panelColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, m_guiConfig.panelColor);
        ImGui::PushStyleColor(ImGuiCol_Button, m_guiConfig.panelColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(m_guiConfig.accentColor.x * 0.7f, 
                                                              m_guiConfig.accentColor.y * 0.7f, 
                                                              m_guiConfig.accentColor.z * 0.7f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_guiConfig.accentColor);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, m_guiConfig.accentColor);
        ImGui::PushStyleColor(ImGuiCol_Text, m_guiConfig.textColor);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.0f);
        
        if (ImGui::Begin("HWID Spoofer - Rust Edition", &m_showWindow, ImGuiWindowFlags_AlwaysAutoResize)) {
            RenderHeader();
            ImGui::Separator();
            RenderMainPanel();
            
            if (m_showSettings) {
                ImGui::Separator();
                RenderSettingsPanel();
            }
            
            if (m_showLogs) {
                ImGui::Separator();
                RenderLogsPanel();
            }
        }
        
        ImGui::End();
        
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(9);
    }

    void CSpooferGUI::RenderHeader() {
        ImGui::PushStyleColor(ImGuiCol_Text, m_guiConfig.accentColor);
        ImGui::SetWindowFontScale(1.2f);
        ImGui::Text("🛡️ HWID Spoofer Pro");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        
        Spoofer::SpoofStatus status = m_spoofer->GetStatus();
        ImGui::PushStyleColor(ImGuiCol_Text, GetStatusColor(status));
        ImGui::Text("[%s]", GetStatusText(status));
        ImGui::PopStyleColor();
        
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        
        if (ImGui::Button("⚙️ Settings")) {
            m_showSettings = !m_showSettings;
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("📋 Logs")) {
            m_showLogs = !m_showLogs;
        }
    }

    void CSpooferGUI::RenderMainPanel() {
        // Quick Actions Section
        RenderCard("Quick Actions", [&]() {
            RenderQuickActions();
        });
        
        ImGui::Spacing();
        
        // Status Panel
        RenderCard("Spoof Status", [&]() {
            RenderStatusPanel();
        });
        
        ImGui::Spacing();
        
        // Identifier Toggles
        RenderCard("Hardware Identifiers", [&]() {
            RenderIdentifierToggles();
        });
        
        ImGui::Spacing();
        
        // Advanced Options
        RenderCard("Advanced Options", [&]() {
            RenderAdvancedOptions();
        });
    }

    void CSpooferGUI::RenderQuickActions() {
        ImVec2 buttonSize = ImVec2(150, 45);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        
        // Spoof All Button
        if (m_spoofer->GetStatus() == Spoofer::SpoofStatus::Spoofing) {
            RenderProgressBar(m_animState.spoofProgress, m_guiConfig.accentColor);
        } else {
            bool clicked = false;
            RenderButton("🚀 Spoof All", m_guiConfig.accentColor, &clicked);
            if (clicked) {
                AddLog("Starting full spoof...");
                m_spoofer->SpoofAll();
                AddLog("Spoof completed successfully");
                m_animState.spoofProgress = 1.0f;
            }
        }
        
        ImGui::SameLine();
        
        // Restore All Button
        bool clicked = false;
        RenderButton("🔄 Restore All", m_guiConfig.warningColor, &clicked);
        if (clicked) {
            AddLog("Restoring original identifiers...");
            m_spoofer->RestoreAll();
            AddLog("Restore completed");
            m_animState.spoofProgress = 0.0f;
        }
        
        ImGui::SameLine();
        
        // Rust-Specific Spoof
        bool rustClicked = false;
        RenderButton("🦀 Rust Mode", ImVec4(0.9f, 0.4f, 0.0f, 1.0f), &rustClicked);
        if (rustClicked) {
            AddLog("Applying Rust-specific spoofing...");
            m_spoofer->SpoofForRust();
            AddLog("Rust spoofing applied");
        }
        
        ImGui::PopStyleColor();
    }

    void CSpooferGUI::RenderIdentifierToggles() {
        Spoofer::SpooferConfig config = m_spoofer->GetConfig();
        
        ImGui::BeginGroup();
        
        RenderToggle("Motherboard Serial", &config.spoofMotherboard, "Spoof motherboard identifier");
        RenderToggle("BIOS Serial", &config.spoofBIOS, "Spoof BIOS identifier");
        RenderToggle("CPU ID", &config.spoofCPU, "Spoof processor identifier");
        RenderToggle("Disk Serial", &config.spoofDisk, "Spoof storage device identifier");
        
        ImGui::EndGroup();
        
        ImGui::SameLine(0, 30);
        
        ImGui::BeginGroup();
        
        RenderToggle("GPU Serial", &config.spoofGPU, "Spoof graphics card identifier");
        RenderToggle("MAC Address", &config.spoofMAC, "Spoof network adapter address");
        RenderToggle("System GUID", &config.spoofSystemGUID, "Spoof system globally unique identifier");
        RenderToggle("Machine GUID", &config.spoofMachineGUID, "Spoof machine GUID");
        
        ImGui::EndGroup();
        
        ImGui::SameLine(0, 30);
        
        ImGui::BeginGroup();
        
        RenderToggle("Product ID", &config.spoofProductID, "Spoof Windows product identifier");
        RenderToggle("Computer Name", &config.spoofComputerName, "Spoof system hostname");
        
        ImGui::Checkbox("Randomize All", &config.randomizeAll);
        ImGui::Checkbox("Use Custom Values", &config.useCustomValues);
        
        ImGui::EndGroup();
        
        m_spoofer->SetConfig(config);
    }

    void CSpooferGUI::RenderAdvancedOptions() {
        Spoofer::SpooferConfig config = m_spoofer->GetConfig();
        
        ImGui::BeginGroup();
        
        ImGui::Text("Protection Layers:");
        RenderToggle("WMI Protection", &config.enableWMIProtection, "Windows Management Instrumentation");
        RenderToggle("Registry Protection", &config.enableRegistryProtection, "System registry modifications");
        RenderToggle("Driver Protection", &config.enableDriverProtection, "Kernel-level driver integration");
        
        ImGui::EndGroup();
        
        ImGui::SameLine(0, 30);
        
        ImGui::BeginGroup();
        
        ImGui::Text("Network & Hardware:");
        RenderToggle("Network Protection", &config.enableNetworkProtection, "NDIS layer integration");
        RenderToggle("PCI Spoofing", &config.enablePCISpoofing, "PCI descriptor management");
        
        ImGui::EndGroup();
        
        ImGui::SameLine(0, 30);
        
        ImGui::BeginGroup();
        
        ImGui::Text("Safety Options:");
        RenderToggle("Create Backup", &config.createBackup, "Backup original identifiers");
        RenderToggle("Restore on Exit", &config.restoreOnExit, "Auto-restore when closing");
        
        ImGui::EndGroup();
        
        m_spoofer->SetConfig(config);
    }

    void CSpooferGUI::RenderStatusPanel() {
        Spoofer::SpoofStatus status = m_spoofer->GetStatus();
        
        // Status indicator
        RenderStatusIndicator("Current Status", status == Spoofer::SpoofStatus::Spoofed, 
                            GetStatusColor(status));
        
        ImGui::SameLine();
        ImGui::Text("%s", GetStatusText(status));
        
        ImGui::Spacing();
        
        // Spoofed identifiers list
        std::vector<std::string> spoofed = m_spoofer->GetSpoofedIdentifiers();
        
        if (spoofed.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, m_guiConfig.textColor);
            ImGui::Text("No identifiers spoofed yet");
            ImGui::PopStyleColor();
        } else {
            ImGui::Text("Spoofed Identifiers (%zu):", spoofed.size());
            for (const auto& id : spoofed) {
                ImGui::BulletText("%s", id.c_str());
            }
        }
        
        ImGui::Spacing();
        
        // Verification status
        bool isSpoofed = m_spoofer->IsSpoofed();
        RenderStatusIndicator("Verification", isSpoofed, 
                            isSpoofed ? m_guiConfig.successColor : m_guiConfig.errorColor);
        
        ImGui::SameLine();
        ImGui::Text(isSpoofed ? "Spoofing Active" : "Not Spoofed");
    }

    void CSpooferGUI::RenderSettingsPanel() {
        ImGui::Text("GUI Settings");
        
        ImGui::Checkbox("Show Animations", &m_guiConfig.showAnimations);
        ImGui::Checkbox("Show Status Indicators", &m_guiConfig.showStatusIndicators);
        ImGui::Checkbox("Show Detailed Info", &m_guiConfig.showDetailedInfo);
        
        ImGui::Spacing();
        
        ImGui::Text("Color Theme");
        
        ImGui::ColorEdit3("Accent Color", (float*)&m_guiConfig.accentColor);
        ImGui::ColorEdit3("Success Color", (float*)&m_guiConfig.successColor);
        ImGui::ColorEdit3("Warning Color", (float*)&m_guiConfig.warningColor);
        ImGui::ColorEdit3("Error Color", (float*)&m_guiConfig.errorColor);
    }

    void CSpooferGUI::RenderLogsPanel() {
        ImGui::Text("Activity Log");
        ImGui::Separator();
        
        ImGui::BeginChild("LogScroll", ImVec2(0, 150), true);
        
        for (const auto& log : m_logMessages) {
            ImGui::TextUnformatted(log.c_str());
        }
        
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        
        ImGui::EndChild();
        
        ImGui::Spacing();
        
        if (ImGui::Button("Clear Logs")) {
            m_logMessages.clear();
        }
    }

    void CSpooferGUI::RenderToggle(const char* label, bool* value, const char* tooltip) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        
        float height = ImGui::GetFrameHeight();
        float width = height * 1.8f;
        float radius = height * 0.5f;
        
        ImGui::InvisibleButton(label, ImVec2(width, height));
        
        if (ImGui::IsItemClicked()) {
            *value = !*value;
        }
        
        ImU32 color = *value ? 
            ImGui::ColorConvertFloat4ToU32(m_guiConfig.accentColor) :
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 0.4f, 0.45f, 1.0f));
        
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), color, radius);
        
        float circleOffset = *value ? width - radius - 2 : 2;
        draw_list->AddCircleFilled(ImVec2(p.x + circleOffset + radius, p.y + radius), radius - 2, 
                                    IM_COL32(255, 255, 255, 255));
        
        ImGui::SameLine();
        ImGui::Text("%s", label);
        
        if (tooltip && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", tooltip);
        }
    }

    void CSpooferGUI::RenderButton(const char* label, const ImVec4& color, bool* clicked) {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x * 0.8f, color.y * 0.8f, color.z * 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x * 0.6f, color.y * 0.6f, color.z * 0.6f, 1.0f));
        
        *clicked = ImGui::Button(label);
        
        ImGui::PopStyleColor(3);
    }

    void CSpooferGUI::RenderProgressBar(float progress, const ImVec4& color) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
        ImGui::ProgressBar(progress, ImVec2(150, 0));
        ImGui::PopStyleColor();
    }

    void CSpooferGUI::RenderStatusIndicator(const char* label, bool active, const ImVec4& color) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        
        float size = 8.0f;
        draw_list->AddCircleFilled(ImVec2(p.x + size, p.y + size), size, 
                                   ImGui::ColorConvertFloat4ToU32(color));
        
        if (active && m_guiConfig.showAnimations) {
            float pulse = sin(ImGui::GetTime() * 3.0f) * 0.3f + 0.7f;
            draw_list->AddCircleFilled(ImVec2(p.x + size, p.y + size), size * pulse * 1.5f,
                                       ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, 0.3f)));
        }
        
        ImGui::Dummy(ImVec2(size * 3, size * 2));
        ImGui::SameLine();
        ImGui::Text("%s", label);
    }

    void CSpooferGUI::RenderCard(const char* title, const std::function<void()>& content) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, m_guiConfig.panelColor);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        
        ImGui::BeginChild(title, ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysAutoResize);
        
        ImGui::PushStyleColor(ImGuiCol_Text, m_guiConfig.accentColor);
        ImGui::Text("%s", title);
        ImGui::PopStyleColor();
        
        ImGui::Separator();
        ImGui::Spacing();
        
        content();
        
        ImGui::EndChild();
        
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    void CSpooferGUI::AddLog(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        char timeStr[26];
        ctime_s(timeStr, sizeof(timeStr), &time);
        timeStr[24] = '\0'; // Remove newline
        
        std::string logEntry = std::string(timeStr) + " - " + message;
        m_logMessages.push_back(logEntry);
        
        if (m_logMessages.size() > m_maxLogMessages) {
            m_logMessages.erase(m_logMessages.begin());
        }
    }

    void CSpooferGUI::UpdateAnimations() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_animState.lastUpdate).count();
        
        if (m_guiConfig.showAnimations && elapsed > 16) {
            m_animState.lastUpdate = now;
            
            // Animate spoof progress
            if (m_spoofer->GetStatus() == Spoofer::SpoofStatus::Spoofing) {
                m_animState.spoofProgress += 0.02f;
                if (m_animState.spoofProgress > 1.0f) m_animState.spoofProgress = 1.0f;
            }
            
            // Pulse animation
            m_animState.pulseIntensity = sin(ImGui::GetTime() * 2.0f) * 0.5f + 0.5f;
            
            // Fade in animation
            if (m_animState.fadeIn < 1.0f) {
                m_animState.fadeIn += 0.05f;
                if (m_animState.fadeIn > 1.0f) m_animState.fadeIn = 1.0f;
            }
        }
    }

    ImVec4 CSpooferGUI::GetStatusColor(Spoofer::SpoofStatus status) {
        switch (status) {
            case Spoofer::SpoofStatus::NotSpoofed:
                return m_guiConfig.textColor;
            case Spoofer::SpoofStatus::Spoofing:
                return m_guiConfig.accentColor;
            case Spoofer::SpoofStatus::Spoofed:
                return m_guiConfig.successColor;
            case Spoofer::SpoofStatus::Failed:
                return m_guiConfig.errorColor;
            case Spoofer::SpoofStatus::Restored:
                return m_guiConfig.warningColor;
            default:
                return m_guiConfig.textColor;
        }
    }

    const char* CSpooferGUI::GetStatusText(Spoofer::SpoofStatus status) {
        switch (status) {
            case Spoofer::SpoofStatus::NotSpoofed: return "Not Spoofed";
            case Spoofer::SpoofStatus::Spoofing: return "Spoofing...";
            case Spoofer::SpoofStatus::Spoofed: return "Spoofed";
            case Spoofer::SpoofStatus::Failed: return "Failed";
            case Spoofer::SpoofStatus::Restored: return "Restored";
            default: return "Unknown";
        }
    }

}

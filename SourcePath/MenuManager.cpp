#include "MenuManager.h"
#include <algorithm>
#include <sstream>
#include <cstring>
#include <cstdio>

namespace {
    const ImVec4 kAccentColor = ImVec4(0.33f, 0.92f, 0.53f, 1.0f);
    const ImVec4 kDangerColor = ImVec4(0.95f, 0.32f, 0.32f, 1.0f);
    const ImVec4 kMutedText = ImVec4(0.76f, 0.80f, 0.88f, 1.0f);
}

CMenuManager::CMenuManager(CFeatureManager* features) : m_features(features), m_spoofer(nullptr), m_spooferGUI(nullptr), m_rustEvasion(nullptr) {}

CMenuManager::~CMenuManager() {
    Shutdown();
}

bool CMenuManager::Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* ctx) {
    m_hwnd = hwnd;
    m_device = device;
    m_ctx = ctx;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ApplyTheme();

    if (!ImGui_ImplWin32_Init(hwnd)) return false;
    if (!ImGui_ImplDX11_Init(device, ctx)) return false;

    return true;
}

void CMenuManager::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void CMenuManager::ApplyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(20.f, 20.f);
    style.FramePadding = ImVec2(12.f, 8.f);
    style.CellPadding = ImVec2(10.f, 6.f);
    style.ItemSpacing = ImVec2(14.f, 12.f);
    style.ItemInnerSpacing = ImVec2(10.f, 6.f);
    style.ScrollbarSize = 16.f;
    style.WindowRounding = 12.f;
    style.ChildRounding = 10.f;
    style.FrameRounding = 8.f;
    style.PopupRounding = 6.f;
    style.ScrollbarRounding = 12.f;
    style.GrabRounding = 8.f;
    style.TabRounding = 6.f;

    ImVec4 windowBg = ImVec4(0.06f, 0.07f, 0.11f, 0.97f);
    ImVec4 panelBg = ImVec4(0.10f, 0.12f, 0.16f, 0.94f);
    ImVec4 panelHover = ImVec4(0.14f, 0.20f, 0.22f, 0.98f);
    ImVec4 accent = ImVec4(0.21f, 0.55f, 0.35f, 1.0f);

    style.Colors[ImGuiCol_WindowBg] = windowBg;
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.11f, 0.15f, 0.88f);
    style.Colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.65f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.85f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(accent.x, accent.y, accent.z, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.16f, 0.23f, 0.18f, 0.90f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.33f, 0.24f, 0.95f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.28f, 0.20f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = panelBg;
    style.Colors[ImGuiCol_FrameBgHovered] = panelHover;
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(panelHover.x, panelHover.y, panelHover.z, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(accent.x, accent.y, accent.z, 0.9f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(accent.x, accent.y, accent.z, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(kAccentColor.x, kAccentColor.y, kAccentColor.z, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.14f, 0.13f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.20f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.16f, 0.14f, 0.95f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.26f, 0.20f, 0.95f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.36f, 0.28f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.35f, 0.32f, 0.6f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.94f, 0.95f, 0.97f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.60f, 0.68f, 1.0f);
}

void CMenuManager::Render() {
    if (!m_open) return;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    const char* sections[] = {
        "Overview",
        "Aimbot",
        "Visuals",
        "World",
        "Movement",
        "Spoofer",
        "Settings"
    };
    const char* sectionHints[] = {
        "Status & quick controls",
        "Targeting & aim assist",
        "Player and render visuals",
        "World exploits & weapon tweaks",
        "Mobility & physics edits",
        "Hardware spoofing & evasion",
        "Interface & preferences"
    };
    constexpr int sectionCount = IM_ARRAYSIZE(sections);
    m_activeSection = std::clamp(m_activeSection, 0, sectionCount - 1);

    ImGui::SetNextWindowSize(ImVec2(820, 560), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.97f);
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (ImGui::Begin("Aether Control Center", &m_open, windowFlags)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.f, 10.f));

        const float sidebarWidth = 210.0f;
        ImGui::BeginChild("##sidebar", ImVec2(sidebarWidth, 0.f), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::TextColored(kAccentColor, "Aether Modules");
        ImGui::TextColored(kMutedText, "Right Shift to toggle");
        ImGui::Spacing();
        for (int i = 0; i < sectionCount; ++i) {
            bool selected = (m_activeSection == i);
            ImVec4 base = selected ? ImVec4(0.16f, 0.26f, 0.20f, 1.0f) : ImVec4(0.12f, 0.14f, 0.18f, 0.95f);
            ImVec4 hover = ImVec4(0.20f, 0.36f, 0.26f, 0.95f);
            ImVec4 active = ImVec4(0.20f, 0.38f, 0.28f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, base);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, selected ? 1.5f : 0.2f);
            if (ImGui::Button(sections[i], ImVec2(-1, 40.f))) {
                m_activeSection = i;
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);

            ImGui::TextColored(kMutedText, "%s", sectionHints[i]);
            ImGui::Spacing();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##content", ImVec2(0.f, 0.f), false, ImGuiWindowFlags_NoScrollbar);
        ImGui::TextColored(kAccentColor, "Aether Control Center");
        ImGui::SameLine();
        ImGui::TextColored(kMutedText, "• Unified oversight for ESP, combat, movement, and spoofing");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        switch (m_activeSection) {
            case 0: RenderOverviewSection(); break;
            case 1: RenderAimbotTab(); break;
            case 2: RenderVisualSections(); break;
            case 3: RenderWorldSection(); break;
            case 4: RenderMovementTab(); break;
            case 5: RenderSpooferTab(); break;
            case 6: RenderSettingsTab(); break;
            default: RenderOverviewSection(); break;
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void CMenuManager::RenderOverviewSection() {
    auto& aimCfg = m_features->GetAimbotConfig();
    auto& visualsCfg = m_features->GetVisualsConfig();
    auto& worldVisualCfg = m_features->GetWorldVisualsConfig();
    auto& exploitsCfg = m_features->GetExploitsConfig();
    auto& movementCfg = m_features->GetMovementConfig();
    auto& weaponCfg = m_features->GetWeaponConfig();

    ImGui::BeginChild("##overview_grid", ImVec2(0.f, 220.f), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::Columns(3, nullptr, false);

    ImGui::TextColored(kAccentColor, "Combat Suite");
    ImGui::Separator();
    ImGui::Checkbox("Aimbot Enabled", &aimCfg.enabled);
    ImGui::Checkbox("Silent Aim", &aimCfg.silentAim);
    ImGui::Checkbox("Auto Fire", &aimCfg.autoFire);
    ImGui::Checkbox("Prediction", &aimCfg.showPrediction);
    ImGui::Checkbox("Highlight Target", &aimCfg.highlightTarget);

    ImGui::NextColumn();
    ImGui::TextColored(kAccentColor, "Visual Intelligence");
    ImGui::Separator();
    ImGui::Checkbox("Player ESP", &visualsCfg.enabled);
    ImGui::Checkbox("World ESP", &worldVisualCfg.enabled);
    ImGui::Checkbox("Inventory Overlay", &visualsCfg.inventoryOverlay);
    ImGui::Checkbox("Chams", &visualsCfg.chams);
    ImGui::Checkbox("Glow", &visualsCfg.playerGlow);

    ImGui::NextColumn();
    ImGui::TextColored(kAccentColor, "Mobility & Exploits");
    ImGui::Separator();
    ImGui::Checkbox("Flyhack", &exploitsCfg.flyhack);
    ImGui::Checkbox("Third Person", &exploitsCfg.thirdPerson);
    ImGui::Checkbox("Always Day", &exploitsCfg.alwaysDay);
    ImGui::Checkbox("Spiderman", &movementCfg.spiderman);
    ImGui::Checkbox("No Fall Damage", &movementCfg.noFallDamage);
    ImGui::Checkbox("Rapid Fire", &weaponCfg.rapidFire);

    ImGui::Columns(1);
    ImGui::EndChild();

    ImGui::Spacing();

    ImGui::BeginChild("##overview_status", ImVec2(0.f, 170.f), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextColored(kAccentColor, "System Status");
    ImGui::Separator();

    if (m_spoofer) {
        std::string text;
        ImVec4 color = kMutedText;
        switch (m_spoofer->GetStatus()) {
        case Spoofer::SpoofStatus::Spoofed: text = "Spoofed"; color = kAccentColor; break;
        case Spoofer::SpoofStatus::Spoofing: text = "Spoofing"; color = ImVec4(0.32f, 0.65f, 0.85f, 1.0f); break;
        case Spoofer::SpoofStatus::Failed: text = "Failed"; color = kDangerColor; break;
        case Spoofer::SpoofStatus::Restored: text = "Restored"; color = kMutedText; break;
        default: text = "Idle"; break;
        }
        ImGui::TextColored(color, "Spoofer: %s", text.c_str());
    } else {
        ImGui::TextColored(kMutedText, "Spoofer: Not attached");
    }

    if (m_rustEvasion) {
        std::string text;
        ImVec4 color = kMutedText;
        switch (m_rustEvasion->GetStatus()) {
        case RustEvasion::EACBypassStatus::Active: text = "EAC bypass active"; color = kAccentColor; break;
        case RustEvasion::EACBypassStatus::Initializing: text = "Initializing"; color = ImVec4(0.32f, 0.65f, 0.85f, 1.0f); break;
        case RustEvasion::EACBypassStatus::Partial: text = "Partial bypass"; color = ImVec4(0.90f, 0.72f, 0.25f, 1.0f); break;
        case RustEvasion::EACBypassStatus::Failed: text = "Failed"; color = kDangerColor; break;
        case RustEvasion::EACBypassStatus::Detected: text = "Detection flagged"; color = kDangerColor; break;
        default: text = "Inactive"; break;
        }
        ImGui::TextColored(color, "Evasion: %s", text.c_str());
    } else {
        ImGui::TextColored(kMutedText, "Evasion: Not attached");
    }

    ImGui::Separator();
    ImGui::Columns(3, nullptr, false);
    auto renderCard = [](const char* title, const char* subtitle, const ImVec4& color) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.15f, 0.19f, 0.9f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
        ImGui::BeginChild(title, ImVec2(0.f, 70.f), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::TextColored(color, "%s", title);
        ImGui::Spacing();
        ImGui::TextUnformatted(subtitle);
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    };

    renderCard("Combat", aimCfg.enabled ? "Aimbot engaged" : "Aimbot idle", aimCfg.enabled ? kAccentColor : kMutedText);
    ImGui::NextColumn();
    renderCard("Visuals", visualsCfg.enabled ? "ESP broadcasting" : "Visuals hidden", visualsCfg.enabled ? kAccentColor : kMutedText);
    ImGui::NextColumn();
    renderCard("World", (exploitsCfg.flyhack || exploitsCfg.alwaysDay || weaponCfg.rapidFire) ? "Custom modifiers active" : "World defaults", (exploitsCfg.flyhack || exploitsCfg.alwaysDay || weaponCfg.rapidFire) ? kAccentColor : kMutedText);
    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::TextColored(kMutedText, "Hotkey: Right Shift toggles this control center");
    ImGui::EndChild();
}

void CMenuManager::RenderVisualSections() {
    ImGui::BeginChild("##visual_sections", ImVec2(0.f, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::Columns(2, nullptr, false);

    ImGui::TextColored(kAccentColor, "Player Visuals");
    ImGui::Separator();
    RenderPlayerVisualsTab();

    ImGui::NextColumn();
    ImGui::TextColored(kAccentColor, "World Visuals");
    ImGui::Separator();
    RenderWorldVisualsTab();

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::TextColored(kMutedText, "Use inventory overlay via Overview or Visual panels.");
    ImGui::EndChild();
}

void CMenuManager::RenderWorldSection() {
    ImGui::BeginChild("##world_sections", ImVec2(0.f, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::Columns(2, nullptr, false);

    ImGui::TextColored(kAccentColor, "World Exploits");
    ImGui::Separator();
    RenderWorldExploitsTab();

    ImGui::NextColumn();
    ImGui::TextColored(kAccentColor, "Weapon Mods");
    ImGui::Separator();
    RenderWeaponModsTab();

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::TextColored(kMutedText, "Tune world visuals under the Visuals section");
    ImGui::EndChild();
}

void CMenuManager::RenderAimbotTab() {
    auto& cfg = m_features->GetAimbotConfig();
    ImGui::BeginChild("##aimbot_content", ImVec2(0.f, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::Columns(2, nullptr, false);

    ImGui::TextColored(kAccentColor, "Core Targeting");
    ImGui::Separator();
    ImGui::Checkbox("Enable Aimbot", &cfg.enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Silent Aim", &cfg.silentAim);
    ImGui::Checkbox("Memory Aim", &cfg.memoryAim);
    RenderKeybindPicker("Engage Key", cfg.aimKey);
    ImGui::Spacing();

    ImGui::TextColored(kMutedText, "Behaviour");
    ImGui::SliderFloat("Smoothness", &cfg.smoothness, 0.0f, 20.0f, "%.2f");
    ImGui::SliderFloat("FOV", &cfg.fov, 0.0f, 180.0f, "%.1f");
    ImGui::SliderFloat("Max Distance", &cfg.maxDistance, 0.0f, 500.0f, "%.0f");

    ImGui::Spacing();
    ImGui::TextColored(kMutedText, "Hitchance");
    ImGui::SliderFloat("Required %", &cfg.hitchance, 0.0f, 100.0f, "%.0f%%");

    ImGui::NextColumn();
    ImGui::TextColored(kMutedText, "Target Filtering");
    ImGui::Separator();
    ImGui::Combo("Preferred Hitbox", (int*)&cfg.hitbox, "Head\0Neck\0Chest\0Stomach\0Pelvis\0");
    ImGui::Checkbox("Target Players", &cfg.targetPlayers);
    ImGui::Checkbox("Target NPCs", &cfg.targetNPCs);
    ImGui::Checkbox("Target Animals", &cfg.targetAnimals);
    ImGui::Checkbox("Ignore Wounded", &cfg.ignoreWounded);
    ImGui::Checkbox("Ignore Teammates", &cfg.ignoreTeammates);

    ImGui::Spacing();
    ImGui::TextColored(kMutedText, "Visual aids");
    ImGui::Checkbox("Render FOV Circle", &cfg.fovCircle);
    ImGui::Checkbox("Render Target Line", &cfg.targetLine);
    ImGui::Checkbox("Show Prediction Marker", &cfg.showPrediction);
    ImGui::Checkbox("Bullet Tracers", &cfg.bulletTracers);
    ImGui::Checkbox("Highlight Target", &cfg.highlightTarget);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(kMutedText, "Active loadout presets");
    if (ImGui::BeginTable("##aimbot_presets", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody)) {
        ImGui::TableSetupColumn("Profile");
        ImGui::TableSetupColumn("Purpose");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();

        auto row = [&](const char* name, const char* purpose, bool active) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(name);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextColored(kMutedText, "%s", purpose);
            ImGui::TableSetColumnIndex(2);
            ImGui::TextColored(active ? kAccentColor : kMutedText, "%s", active ? "Loaded" : "Idle");
        };

        row("Legit", "Subtle humanized aim", cfg.enabled && cfg.smoothness > 5.0f && cfg.fov < 10.0f);
        row("Rage", "Aggressive snaps", cfg.enabled && cfg.smoothness < 2.0f && cfg.fov > 45.0f);
        const auto& visualsCfg = m_features->GetVisualsConfig();
        row("Inventory Overlay", "Auto center for inv ESP", visualsCfg.inventoryOverlay && cfg.enabled);
        ImGui::EndTable();
    }

    ImGui::Columns(1);
    ImGui::EndChild();
}

void CMenuManager::RenderPlayerVisualsTab() {
    auto& cfg = m_features->GetPlayerVisualsConfig();
    ImGui::BeginChild("##player_visuals", ImVec2(0.f, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::Columns(2, nullptr, false);

    ImGui::TextColored(kMutedText, "Core Overlays");
    ImGui::Separator();
    ImGui::Checkbox("Box ESP", &cfg.boxESP);
    ImGui::Combo("Box Style", (int*)&cfg.boxStyle, "Full\0Corner\0");
    ImGui::Checkbox("Skeletons", &cfg.skeletons);
    ImGui::Checkbox("Target Bone Lock", &cfg.targetBoneESP);
    ImGui::Checkbox("Chams", &cfg.chams);
    ImGui::Checkbox("Glow", &cfg.playerGlow);
    ImGui::Checkbox("Healthbars", &cfg.healthbars);
    ImGui::Checkbox("Distance Indicator", &cfg.distanceIndicator);
    ImGui::Checkbox("Username Display", &cfg.usernameDisplay);

    ImGui::NextColumn();
    ImGui::TextColored(kMutedText, "Supplementary");
    ImGui::Separator();
    ImGui::Checkbox("Held Item Display", &cfg.heldItemDisplay);
    ImGui::Checkbox("Inventory Overlay", &cfg.inventoryOverlay);
    ImGui::Checkbox("Team ID Colors", &cfg.teamIDColors);
    ImGui::Checkbox("Custom Team Palette", &cfg.customTeamColors);
    if (cfg.customTeamColors) {
        ImGui::Indent();
        ImVec4 teamColor = ImGui::ColorConvertU32ToFloat4(cfg.teammateCustomColor);
        if (ImGui::ColorEdit4("Teammates", &teamColor.x)) {
            cfg.teammateCustomColor = ImGui::ColorConvertFloat4ToU32(teamColor);
        }
        ImVec4 enemyColor = ImGui::ColorConvertU32ToFloat4(cfg.enemyCustomColor);
        if (ImGui::ColorEdit4("Enemies", &enemyColor.x)) {
            cfg.enemyCustomColor = ImGui::ColorConvertFloat4ToU32(enemyColor);
        }
        ImVec4 neutralColor = ImGui::ColorConvertU32ToFloat4(cfg.neutralCustomColor);
        if (ImGui::ColorEdit4("Neutrals", &neutralColor.x)) {
            cfg.neutralCustomColor = ImGui::ColorConvertFloat4ToU32(neutralColor);
        }
        ImGui::Unindent();
    }
    ImGui::Checkbox("Is Inside Building", &cfg.isInsideBuilding);
    ImGui::Checkbox("Hotbar Display", &cfg.hotbarDisplay);
    ImGui::Checkbox("Steam Avatar", &cfg.steamAvatar);
    ImGui::Checkbox("View Direction Arrow", &cfg.viewDirectionArrow);
    ImGui::Checkbox("Offscreen Arrows", &cfg.offscreenArrows);
    ImGui::Checkbox("Visibility Check", &cfg.visibilityCheck);
    ImGui::SliderFloat("Render Distance", &cfg.renderDistance, 0.0f, 1000.0f);
    ImGui::Checkbox("Custom ESP Color", &cfg.customESPColor);
    if (cfg.customESPColor) {
        ImVec4 customColor = ImGui::ColorConvertU32ToFloat4(cfg.customESPColorValue);
        if (ImGui::ColorEdit4("ESP Color", &customColor.x)) {
            cfg.customESPColorValue = ImGui::ColorConvertFloat4ToU32(customColor);
        }
        ImGui::TextColored(kMutedText, "Recent swatches:");
        for (size_t i = 0; i < cfg.recentESPColors.size(); ++i) {
            ImGui::SameLine(0.0f, i == 0 ? 0.0f : 6.0f);
            ImVec4 swatch = ImGui::ColorConvertU32ToFloat4(cfg.recentESPColors[i]);
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::ColorButton("##swatch", swatch)) {
                cfg.customESPColorValue = cfg.recentESPColors[i];
            }
            ImGui::PopID();
        }
        bool stored = false;
        for (uint32_t slot : cfg.recentESPColors) {
            if (slot == cfg.customESPColorValue) {
                stored = true;
                break;
            }
        }
        if (!stored) {
            for (size_t i = cfg.recentESPColors.size(); i-- > 1;) {
                cfg.recentESPColors[i] = cfg.recentESPColors[i - 1];
            }
            cfg.recentESPColors[0] = cfg.customESPColorValue;
        }
    }
    ImGui::Separator();
    ImGui::TextColored(kMutedText, "ESP Presets");
    ImGui::PushID("esp_presets_builtin");
    if (ImGui::Button("Stealth")) {
        cfg.customESPColor = true;
        cfg.customESPColorValue = 0xFF56A392;
        cfg.customTeamColors = true;
        cfg.teammateCustomColor = 0xFF3AA57C;
        cfg.enemyCustomColor = 0xFFBF4E63;
        cfg.neutralCustomColor = 0xFFB5A76F;
    }
    ImGui::SameLine();
    if (ImGui::Button("Arena")) {
        cfg.customESPColor = true;
        cfg.customESPColorValue = 0xFFFF5F5F;
        cfg.customTeamColors = true;
        cfg.teammateCustomColor = 0xFF40E0D0;
        cfg.enemyCustomColor = 0xFFFF1493;
        cfg.neutralCustomColor = 0xFFFFFF54;
    }
    ImGui::SameLine();
    if (ImGui::Button("Streamer")) {
        cfg.customESPColor = true;
        cfg.customESPColorValue = 0xFF9A4BFF;
        cfg.customTeamColors = true;
        cfg.teammateCustomColor = 0xFF5BD5FF;
        cfg.enemyCustomColor = 0xFFFF89B3;
        cfg.neutralCustomColor = 0xFFFFFF9C;
    }
    ImGui::PopID();
    ImGui::Separator();

    ImGui::TextColored(kMutedText, "Preset File");
    static char presetFilePath[260] = "esp_presets.cfg";
    static char presetStatus[160] = "";
    static ImVec4 presetStatusColor = kMutedText;
    ImGui::InputText("##preset_file", presetFilePath, IM_ARRAYSIZE(presetFilePath));
    if (ImGui::Button("Export Presets")) {
        if (m_features->ExportVisualPresets(presetFilePath)) {
            std::snprintf(presetStatus, sizeof(presetStatus), "Exported presets to %s", presetFilePath);
            presetStatusColor = kAccentColor;
        } else {
            std::snprintf(presetStatus, sizeof(presetStatus), "Failed to export presets to %s", presetFilePath);
            presetStatusColor = kDangerColor;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Import Presets")) {
        if (m_features->ImportVisualPresets(presetFilePath)) {
            std::snprintf(presetStatus, sizeof(presetStatus), "Imported presets from %s", presetFilePath);
            presetStatusColor = kAccentColor;
        } else {
            std::snprintf(presetStatus, sizeof(presetStatus), "Failed to import presets from %s", presetFilePath);
            presetStatusColor = kDangerColor;
        }
    }
    if (presetStatus[0] != '\0') {
        ImGui::TextColored(presetStatusColor, "%s", presetStatus);
    }

    ImGui::Separator();

    ImGui::TextColored(kMutedText, "Custom Presets");
    for (size_t i = 0; i < cfg.customPresets.size(); ++i) {
        auto& preset = cfg.customPresets[i];
        ImGui::PushID(static_cast<int>(i));
        if (!preset.occupied) {
            if (ImGui::Button("[Empty Slot] Save Current")) {
                preset.occupied = true;
                preset.customESP = cfg.customESPColor;
                preset.espColor = cfg.customESPColorValue;
                preset.customTeam = cfg.customTeamColors;
                preset.teammateColor = cfg.teammateCustomColor;
                preset.enemyColor = cfg.enemyCustomColor;
                preset.neutralColor = cfg.neutralCustomColor;
                std::snprintf(preset.name.data(), preset.name.size(), "Preset %d", static_cast<int>(i + 1));
            }
        } else {
            ImGui::PushItemWidth(160.f);
            ImGui::InputText("##preset_name", preset.name.data(), preset.name.size());
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::ColorButton("##preview", ImGui::ColorConvertU32ToFloat4(preset.espColor), 0, ImVec2(20, 20))) {
                cfg.customESPColor = preset.customESP;
                cfg.customESPColorValue = preset.espColor;
                cfg.customTeamColors = preset.customTeam;
                cfg.teammateCustomColor = preset.teammateColor;
                cfg.enemyCustomColor = preset.enemyColor;
                cfg.neutralCustomColor = preset.neutralColor;
            }
            ImGui::SameLine();
            if (ImGui::Button("Load")) {
                cfg.customESPColor = preset.customESP;
                cfg.customESPColorValue = preset.espColor;
                cfg.customTeamColors = preset.customTeam;
                cfg.teammateCustomColor = preset.teammateColor;
                cfg.enemyCustomColor = preset.enemyColor;
                cfg.neutralCustomColor = preset.neutralColor;
            }
            ImGui::SameLine();
            if (ImGui::Button("Overwrite")) {
                preset.customESP = cfg.customESPColor;
                preset.espColor = cfg.customESPColorValue;
                preset.customTeam = cfg.customTeamColors;
                preset.teammateColor = cfg.teammateCustomColor;
                preset.enemyColor = cfg.enemyCustomColor;
                preset.neutralColor = cfg.neutralCustomColor;
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear")) {
                preset = PlayerVisualsConfig::ESPColorPreset{};
            }
        }
        ImGui::PopID();
    }
    ImGui::Checkbox("Highlight ESP", &cfg.highlightESP);
    if (cfg.highlightESP) {
        ImVec4 color = ImGui::ColorConvertU32ToFloat4(cfg.highlightESPColor);
        if (ImGui::ColorEdit4("Highlight Color", &color.x)) {
            cfg.highlightESPColor = ImGui::ColorConvertFloat4ToU32(color);
        }
    }
    ImGui::Checkbox("Tracers", &cfg.tracers);
    if (cfg.tracers) {
        ImVec4 tracer = ImGui::ColorConvertU32ToFloat4(cfg.tracerLineColor);
        if (ImGui::ColorEdit4("Tracer Color", &tracer.x)) {
            cfg.tracerLineColor = ImGui::ColorConvertFloat4ToU32(tracer);
        }
    }

    ImGui::Columns(1);
    ImGui::EndChild();
}

void CMenuManager::RenderWorldExploitsTab() {
    auto& cfg = m_features->GetWorldExploitsConfig();
    ImGui::BeginChild("##world_exploits", ImVec2(0.f, ImGui::GetContentRegionAvail().y * 0.6f), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::Columns(2, nullptr, false);

    ImGui::TextColored(kMutedText, "Environment");
    ImGui::Separator();
    ImGui::Checkbox("FOV Changer", &cfg.fovChanger);
    ImGui::SliderFloat("FOV Value", &cfg.fovValue, 60.0f, 150.0f);
    ImGui::Checkbox("Bright Night", &cfg.brightNight);
    ImGui::SliderFloat("Night Brightness", &cfg.nightBrightness, 0.0f, 5.0f);
    ImGui::Checkbox("Bright Cave", &cfg.brightCave);
    ImGui::SliderFloat("Cave Brightness", &cfg.caveBrightness, 0.0f, 5.0f);
    ImGui::Checkbox("Bright Stars", &cfg.brightStars);
    ImGui::Checkbox("Always Day", &cfg.alwaysDay);
    ImGui::Checkbox("Remove Sun", &cfg.removeSun);
    ImGui::Checkbox("Remove Moon", &cfg.removeMoon);
    ImGui::Checkbox("Remove Layers", &cfg.removeLayers);

    ImGui::NextColumn();
    ImGui::TextColored(kMutedText, "Player Manipulation");
    ImGui::Separator();
    ImGui::Checkbox("Remove Screen Shake", &cfg.removeScreenShake);
    ImGui::Checkbox("Remove Flash", &cfg.removeFlash);
    ImGui::Checkbox("Zoom", &cfg.zoom);
    ImGui::SliderFloat("Zoom Value", &cfg.zoomValue, 1.0f, 10.0f);
    ImGui::Checkbox("Instant Revive", &cfg.instantRevive);
    ImGui::Checkbox("Remove Wear Restrictions", &cfg.removeWearRestrictions);
    ImGui::Checkbox("Fast Loot", &cfg.fastLoot);
    ImGui::Checkbox("Flyhack", &cfg.flyhack);
    ImGui::SliderFloat("Fly Speed", &cfg.flySpeed, 1.0f, 50.0f);
    RenderKeybindPicker("Flyhack Key", cfg.flyhackKey);
    ImGui::Checkbox("Ignore Player Collision", &cfg.ignorePlayerCollision);
    ImGui::Checkbox("Infinite Jump", &cfg.infiniteJump);
    ImGui::Checkbox("Debug Camera", &cfg.debugCamera);
    ImGui::Checkbox("Spectate", &cfg.spectate);
    ImGui::Checkbox("Third Person", &cfg.thirdPerson);
    ImGui::SliderFloat("Third Person Distance", &cfg.thirdPersonDistance, 1.0f, 20.0f);
    ImGui::Checkbox("Unlock Angles", &cfg.unlockAngles);
    ImGui::Checkbox("Show Buildings", &cfg.showBuildings);
    ImGui::Checkbox("No Fall Damage", &cfg.noFallDamage);

    ImGui::Columns(1);
    ImGui::EndChild();
}

void CMenuManager::RenderWeaponModsTab() {
    auto& cfg = m_features->GetWeaponModsConfig();
    ImGui::Checkbox("Recoil Modifier", &cfg.recoilModifier);
    ImGui::SliderFloat("Recoil X", &cfg.recoilX, 0.0f, 2.0f);
    ImGui::SliderFloat("Recoil Y", &cfg.recoilY, 0.0f, 2.0f);
    ImGui::Checkbox("Weapon Spread", &cfg.weaponSpread);
    ImGui::SliderFloat("Spread Value", &cfg.spreadValue, 0.0f, 1.0f);
    ImGui::Checkbox("Weapon Sway", &cfg.weaponSway);
    ImGui::SliderFloat("Sway Value", &cfg.swayValue, 0.0f, 1.0f);
    ImGui::Checkbox("Rapid Fire", &cfg.rapidFire);
    ImGui::Checkbox("Thick Bullet", &cfg.thickBullet);
    ImGui::Checkbox("Big Bullets", &cfg.bigBullets);
    ImGui::Checkbox("Hit Material Override", &cfg.hitMaterialOverride);
    ImGui::SliderInt("Hit Material", (int*)&cfg.hitMaterial, 0, 10);
    ImGui::Checkbox("Hitbox Override", &cfg.hitboxOverride);
    ImGui::SliderInt("Hitbox Override Value", (int*)&cfg.hitboxOverrideValue, 0, 10);
    ImGui::Checkbox("Smart Patrol Override", &cfg.smartPatrolOverride);
    ImGui::Checkbox("Ammo Indicator", &cfg.ammoIndicator);
    ImGui::Checkbox("Reload Indicator", &cfg.reloadIndicator);
}

void CMenuManager::RenderWorldVisualsTab() {
    auto& cfg = m_features->GetWorldVisualsConfig();
    ImGui::Checkbox("All Items Toggleable", &cfg.allItemsToggleable);
    ImGui::Checkbox("Custom Colors Per Item", &cfg.customColorsPerItem);
    ImGui::SliderFloat("Distance Slider", &cfg.distanceSlider, 0.0f, 1000.0f);
    ImGui::Checkbox("World Chams/Glow", &cfg.worldChams);
    ImGui::Checkbox("Blur Effect", &cfg.blurEffect);
    ImGui::Checkbox("Box ESP Toggle", &cfg.boxESP);
    ImGui::Checkbox("Collectables ESP", &cfg.collectablesESP);
    ImGui::Checkbox("Multi-Tier Crate ESP", &cfg.multiTierCrateESP);
    ImGui::Checkbox("Full Vehicle ESP", &cfg.fullVehicleESP);
    ImGui::Checkbox("Deployables ESP", &cfg.deployablesESP);
    ImGui::Checkbox("All Ores ESP", &cfg.allOresESP);
    ImGui::Checkbox("All Animals ESP", &cfg.allAnimalsESP);
    ImGui::Checkbox("Stashes & Hidden Items", &cfg.stashesHiddenItems);
    ImGui::Checkbox("Corpses & Backpacks", &cfg.corpsesBackpacks);
    ImGui::Checkbox("Supply Drops", &cfg.supplyDrops);
    ImGui::Checkbox("Hackable Crates", &cfg.hackableCrates);
    ImGui::Checkbox("Patrol Helicopter", &cfg.patrolHelicopter);
    ImGui::Checkbox("Bradley APC", &cfg.bradleyAPC);
}

void CMenuManager::RenderMovementTab() {
    auto& cfg = m_features->GetMovementConfig();
    ImGui::Checkbox("Spiderman", &cfg.spiderman);
    ImGui::SliderFloat("Spiderman Speed", &cfg.spiderManSpeed, 1.0f, 50.0f);
    RenderKeybindPicker("Spiderman Key", cfg.spidermanKey);
    ImGui::Checkbox("Force Sprint", &cfg.forceSprint);
    ImGui::Checkbox("Omni Sprint", &cfg.omnisprint);
    ImGui::Checkbox("No Fall Damage", &cfg.noFallDamage);
    ImGui::Checkbox("No Movement Slowdown", &cfg.noMovementSlowdown);
    ImGui::Checkbox("High Walk", &cfg.highWalk);
    ImGui::Checkbox("High Jump", &cfg.highJump);
    ImGui::SliderFloat("Jump Height", &cfg.jumpHeight, 1.0f, 20.0f);
    ImGui::Checkbox("Walk on Water", &cfg.walkOnWater);
    ImGui::Checkbox("Spinbot", &cfg.spinbot);
    ImGui::SliderFloat("Spin Speed", &cfg.spinSpeed, 1.0f, 50.0f);
    RenderKeybindPicker("Spinbot Key", cfg.spinbotKey);
    ImGui::Checkbox("Shoot In Air", &cfg.shootInAir);
    ImGui::Checkbox("Instant Suicide", &cfg.instantSuicide);
}

void CMenuManager::RenderSettingsTab() {
    auto& cfg = m_features->GetUIConfig();
    ImGui::Checkbox("Cloud Config Sync", &cfg.cloudConfigSync);
    ImGui::Checkbox("Local Config Profiles", &cfg.localConfigProfiles);
    ImGui::Checkbox("Import/Export Configs", &cfg.importExportConfigs);
    ImGui::Checkbox("Custom Crosshairs", &cfg.customCrosshairs);
    ImGui::Checkbox("Full Keybind Configuration", &cfg.fullKeybindConfiguration);
    ImGui::SliderFloat("UI Scale", &cfg.uiScale, 0.5f, 2.0f);
    ImGui::Checkbox("Show FPS Counter", &cfg.showFPSCounter);
    ImGui::Checkbox("Show Watermark", &cfg.showWatermark);
    ImGui::Text("Menu Key: Right Shift");
}

void CMenuManager::RenderKeybindPicker(const char* label, Keybind& keybind) {
    ImGui::Text("%s", label);
    ImGui::SameLine();
    char keyName[32];
    sprintf_s(keyName, sizeof(keyName), "Key: %d (Mode: %s)", keybind.key, keybind.mode == KeybindMode::Hold ? "Hold" : "Toggle");
    if (ImGui::Button(keyName)) {
        ImGui::OpenPopup("KeybindPopup");
    }
    if (ImGui::BeginPopup("KeybindPopup")) {
        static bool waitingForKey = true;
        ImGui::Text("Press a key...");
        if (waitingForKey) {
            for (int i = 0; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    keybind.key = i;
                    waitingForKey = false;
                    break;
                }
            }
        } else {
            ImGui::Combo("Mode", (int*)&keybind.mode, "Toggle\0Hold\0");
            if (ImGui::Button("Done")) {
                waitingForKey = true;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    } else {
        static bool waitingForKey = true;
        waitingForKey = true;
    }
}

void CMenuManager::SetSpoofer(Spoofer::CAdvancedSpoofer* spoofer) {
    m_spoofer = spoofer;
}

void CMenuManager::SetSpooferGUI(SpooferGUI::CSpooferGUI* spooferGUI) {
    m_spooferGUI = spooferGUI;
}

void CMenuManager::SetRustEvasion(RustEvasion::CRustAntiCheatEvasion* rustEvasion) {
    m_rustEvasion = rustEvasion;
}

void CMenuManager::RenderSpooferTab() {
    ImGui::Text("HWID Spoofer - Rust Edition");
    ImGui::Separator();
    
    if (!m_spoofer || !m_spooferGUI) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Spoofer not initialized");
        return;
    }
    
    // Quick Actions
    ImGui::Text("Quick Actions:");
    if (ImGui::Button("🚀 Spoof All")) {
        if (m_spoofer) m_spoofer->SpoofAll();
    }
    ImGui::SameLine();
    if (ImGui::Button("🔄 Restore All")) {
        if (m_spoofer) m_spoofer->RestoreAll();
    }
    ImGui::SameLine();
    if (ImGui::Button("🦀 Rust Mode")) {
        if (m_spoofer) m_spoofer->SpoofForRust();
    }
    
    ImGui::Separator();
    
    // Status
    Spoofer::SpoofStatus status = m_spoofer->GetStatus();
    const char* statusText = "Unknown";
    ImVec4 statusColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    
    switch (status) {
        case Spoofer::SpoofStatus::NotSpoofed:
            statusText = "Not Spoofed";
            statusColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
            break;
        case Spoofer::SpoofStatus::Spoofing:
            statusText = "Spoofing...";
            statusColor = ImVec4(0.0f, 0.7f, 1.0f, 1.0f);
            break;
        case Spoofer::SpoofStatus::Spoofed:
            statusText = "Spoofed";
            statusColor = ImVec4(0.0f, 1.0f, 0.5f, 1.0f);
            break;
        case Spoofer::SpoofStatus::Failed:
            statusText = "Failed";
            statusColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
            break;
        case Spoofer::SpoofStatus::Restored:
            statusText = "Restored";
            statusColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
            break;
    }
    
    ImGui::TextColored(statusColor, "Status: %s", statusText);
    
    ImGui::Separator();
    
    // Hardware Identifiers
    ImGui::Text("Hardware Identifiers:");
    Spoofer::SpooferConfig config = m_spoofer->GetConfig();
    
    ImGui::Checkbox("Motherboard Serial", &config.spoofMotherboard);
    ImGui::Checkbox("BIOS Serial", &config.spoofBIOS);
    ImGui::Checkbox("CPU ID", &config.spoofCPU);
    ImGui::Checkbox("Disk Serial", &config.spoofDisk);
    ImGui::Checkbox("GPU Serial", &config.spoofGPU);
    ImGui::Checkbox("MAC Address", &config.spoofMAC);
    ImGui::Checkbox("System GUID", &config.spoofSystemGUID);
    ImGui::Checkbox("Machine GUID", &config.spoofMachineGUID);
    ImGui::Checkbox("Product ID", &config.spoofProductID);
    
    m_spoofer->SetConfig(config);
    
    ImGui::Separator();
    
    // Rust EAC Evasion
    if (m_rustEvasion) {
        ImGui::Text("Rust EAC Evasion:");
        RustEvasion::RustEvasionConfig evasionConfig = m_rustEvasion->GetConfig();
        
        ImGui::Checkbox("Bypass EAC Kernel", &evasionConfig.bypassEACKernel);
        ImGui::Checkbox("Bypass EAC User Mode", &evasionConfig.bypassEACUserMode);
        ImGui::Checkbox("Hide from EAC Scanner", &evasionConfig.hideFromEACScanner);
        ImGui::Checkbox("Spoof EAC Identifiers", &evasionConfig.spoofEACIdentifiers);
        ImGui::Checkbox("Bypass EAC Integrity", &evasionConfig.bypassEACIntegrity);
        ImGui::Checkbox("Bypass EAC Timing", &evasionConfig.bypassEACTiming);
        ImGui::Checkbox("Bypass EAC Network", &evasionConfig.bypassEACNetwork);
        ImGui::Checkbox("Bypass EAC Behavior", &evasionConfig.bypassEACBehavior);
        
        ImGui::Separator();
        ImGui::Text("Evasion Mode:");
        
        if (ImGui::Button("Stealth Mode")) {
            m_rustEvasion->EnableStealthMode();
        }
        ImGui::SameLine();
        if (ImGui::Button("Aggressive Mode")) {
            m_rustEvasion->EnableAggressiveMode();
        }
        ImGui::SameLine();
        if (ImGui::Button("Paranoid Mode")) {
            m_rustEvasion->EnableParanoidMode();
        }
        
        m_rustEvasion->SetConfig(evasionConfig);
        
        ImGui::Separator();
        
        // EAC Status
        RustEvasion::EACBypassStatus eacStatus = m_rustEvasion->GetStatus();
        const char* eacStatusText = "Unknown";
        ImVec4 eacStatusColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        
        switch (eacStatus) {
            case RustEvasion::EACBypassStatus::NotActive:
                eacStatusText = "Not Active";
                eacStatusColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
                break;
            case RustEvasion::EACBypassStatus::Initializing:
                eacStatusText = "Initializing...";
                eacStatusColor = ImVec4(0.0f, 0.7f, 1.0f, 1.0f);
                break;
            case RustEvasion::EACBypassStatus::Active:
                eacStatusText = "Active";
                eacStatusColor = ImVec4(0.0f, 1.0f, 0.5f, 1.0f);
                break;
            case RustEvasion::EACBypassStatus::Partial:
                eacStatusText = "Partial";
                eacStatusColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
                break;
            case RustEvasion::EACBypassStatus::Failed:
                eacStatusText = "Failed";
                eacStatusColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
                break;
            case RustEvasion::EACBypassStatus::Detected:
                eacStatusText = "Detected";
                eacStatusColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
        }
        
        ImGui::TextColored(eacStatusColor, "EAC Bypass Status: %s", eacStatusText);
        
        ImGui::Separator();
        
        if (ImGui::Button("Activate EAC Bypass")) {
            if (m_rustEvasion) m_rustEvasion->ActivateBypass();
        }
        ImGui::SameLine();
        if (ImGui::Button("Deactivate EAC Bypass")) {
            if (m_rustEvasion) m_rustEvasion->DeactivateBypass();
        }
    }
    
    ImGui::Separator();
    
    // Open standalone spoofer GUI
    if (ImGui::Button("Open Spoofer GUI")) {
        if (m_spooferGUI) m_spooferGUI->ToggleWindow();
    }
}

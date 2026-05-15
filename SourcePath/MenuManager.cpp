#include "MenuManager.h"

CMenuManager::CMenuManager(CFeatureManager* features) : m_features(features) {}

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

    if (!ImGui_ImplWin32_Init(hwnd)) return false;
    if (!ImGui_ImplDX11_Init(device, ctx)) return false;

    return true;
}

void CMenuManager::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void CMenuManager::Render() {
    if (!m_open) return;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Rust External Premium", &m_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        static int currentTab = 0;
        const char* tabs[] = { "Aimbot", "Player Visuals", "World Exploits", "Weapon Mods", "World Visuals", "Movement", "Settings" };
        if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_None)) {
            for (int i = 0; i < 7; i++) {
                if (ImGui::BeginTabItem(tabs[i])) {
                    currentTab = i;
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::Separator();

        switch (currentTab) {
            case 0: RenderAimbotTab(); break;
            case 1: RenderPlayerVisualsTab(); break;
            case 2: RenderWorldExploitsTab(); break;
            case 3: RenderWeaponModsTab(); break;
            case 4: RenderWorldVisualsTab(); break;
            case 5: RenderMovementTab(); break;
            case 6: RenderSettingsTab(); break;
        }
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void CMenuManager::RenderAimbotTab() {
    auto& cfg = m_features->GetAimbotConfig();
    ImGui::Checkbox("Enable Aimbot", &cfg.enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Silent Aim", &cfg.silentAim);
    ImGui::Checkbox("Memory Aim", &cfg.memoryAim);
    ImGui::Checkbox("Hitchance", &cfg.hitchance);
    ImGui::SliderFloat("Hitchance Value", &cfg.hitchanceValue, 0.0f, 100.0f);
    ImGui::SliderFloat("Smoothing", &cfg.smoothing, 0.0f, 20.0f);
    ImGui::SliderFloat("Max Distance", &cfg.maxDistance, 0.0f, 500.0f);
    ImGui::SliderFloat("FOV", &cfg.fov, 0.0f, 180.0f);
    ImGui::Combo("Hitbox", (int*)&cfg.hitbox, "Head\0Neck\0Chest\0Stomach\0Pelvis\0");
    ImGui::Checkbox("Target Players", &cfg.targetPlayers);
    ImGui::Checkbox("Target NPCs", &cfg.targetNPCs);
    ImGui::Checkbox("Target Animals", &cfg.targetAnimals);
    ImGui::Checkbox("Ignore Wounded", &cfg.ignoreWounded);
    ImGui::Checkbox("Ignore Teammates", &cfg.ignoreTeammates);
    ImGui::Checkbox("FOV Circle", &cfg.fovCircle);
    ImGui::Checkbox("Target Line", &cfg.targetLine);
    ImGui::Checkbox("Show Prediction", &cfg.showPrediction);
    ImGui::Checkbox("Bullet Tracers", &cfg.bulletTracers);
    ImGui::Checkbox("Highlight Target", &cfg.highlightTarget);
    RenderKeybindPicker("Aimbot Key", cfg.aimKey);
}

void CMenuManager::RenderPlayerVisualsTab() {
    auto& cfg = m_features->GetPlayerVisualsConfig();
    ImGui::Checkbox("Box ESP", &cfg.boxESP);
    ImGui::Combo("Box Style", (int*)&cfg.boxStyle, "Full\0Corner\0");
    ImGui::Checkbox("Skeletons", &cfg.skeletons);
    ImGui::Checkbox("View Direction Arrow", &cfg.viewDirectionArrow);
    ImGui::Checkbox("Offscreen Arrows", &cfg.offscreenArrows);
    ImGui::Checkbox("Healthbars", &cfg.healthbars);
    ImGui::Checkbox("Distance Indicator", &cfg.distanceIndicator);
    ImGui::Checkbox("Username Display", &cfg.usernameDisplay);
    ImGui::Checkbox("Held Item Display", &cfg.heldItemDisplay);
    ImGui::Checkbox("Team ID Colors", &cfg.teamIDColors);
    ImGui::Checkbox("Is Inside Building", &cfg.isInsideBuilding);
    ImGui::Checkbox("Hotbar Display", &cfg.hotbarDisplay);
    ImGui::Checkbox("Steam Avatar", &cfg.steamAvatar);
    ImGui::Checkbox("Chams", &cfg.chams);
    ImGui::Checkbox("Visibility Check", &cfg.visibilityCheck);
    ImGui::SliderFloat("Render Distance", &cfg.renderDistance, 0.0f, 1000.0f);
}

void CMenuManager::RenderWorldExploitsTab() {
    auto& cfg = m_features->GetWorldExploitsConfig();
    ImGui::Checkbox("FOV Changer", &cfg.fovChanger);
    ImGui::SliderFloat("FOV Value", &cfg.fovValue, 60.0f, 150.0f);
    ImGui::Checkbox("Bright Night", &cfg.brightNight);
    ImGui::SliderFloat("Night Brightness", &cfg.nightBrightness, 0.0f, 5.0f);
    ImGui::Checkbox("Bright Cave", &cfg.brightCave);
    ImGui::SliderFloat("Cave Brightness", &cfg.caveBrightness, 0.0f, 5.0f);
    ImGui::Checkbox("Bright Stars", &cfg.brightStars);
    ImGui::Checkbox("Remove Sun", &cfg.removeSun);
    ImGui::Checkbox("Remove Moon", &cfg.removeMoon);
    ImGui::Checkbox("Remove Layers", &cfg.removeLayers);
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
    ImGui::Checkbox("Always Day", &cfg.alwaysDay);
    ImGui::Checkbox("No Fall Damage", &cfg.noFallDamage);
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
    ImGui::Checkbox("Menu Key: INSERT", nullptr);
}

void CMenuManager::RenderKeybindPicker(const char* label, Keybind& keybind) {
    ImGui::Text("%s", label);
    ImGui::SameLine();
    char keyName[32];
    sprintf_s(keyName, "Key: %d (Mode: %s)", keybind.key, keybind.mode == KeybindMode::Hold ? "Hold" : "Toggle");
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

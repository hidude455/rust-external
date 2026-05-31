#include "ESP.h"
#include "offsets.h"
#include <algorithm>

namespace MIT {

    ESP::ESP(MemoryManager* memoryManager, Renderer* renderer)
        : memoryManager(memoryManager), renderer(renderer), lastUpdate(std::chrono::high_resolution_clock::now()) {
    }

    ESP::~ESP() {
    }

    void ESP::Initialize() {
        // Initialization logic can be added here if needed.
    }

    void ESP::Update() {
        if (!config.enabled || !memoryManager->IsProcessValid()) return;

        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdate).count();

        if (elapsed >= updateInterval) {
            updateMatrices();
            cachedEntities = memoryManager->GetEntities();
            lastUpdate = currentTime;
        }
    }

    void ESP::Render() {
        if (!config.enabled) return;

        drawList = ImGui::GetBackgroundDrawList();
        if (!drawList) return;

        Vector3 localPos = memoryManager->GetLocalPlayerPosition();

        for (const auto& entity : cachedEntities) {
            if (entity.ptr == memoryManager->m_localPlayer) continue; // Skip local player

            float distance = localPos.Distance(entity.position);
            if (distance > 500.0f) continue; // General max distance

            ImVec2 screenPos;
            if (worldToScreen(entity.position, screenPos)) {
                ImU32 color = IM_COL32_WHITE;
                std::string text = entity.name;
                bool shouldRender = false;

                switch (entity.type) {
                    case EntityType::Player:
                    case EntityType::NPC:
                        if (config.playerESPEnabled) {
                            color = entity.is_npc ? ImGui::ColorConvertFloat4ToU32(config.npcColor) : ImGui::ColorConvertFloat4ToU32(config.playerColor);
                            text = entity.name;
                            shouldRender = true;
                        }
                        break;

                    case EntityType::StoneNode:
                        if (config.resourceStone && distance < config.resourceDistance) { color = IM_COL32(128, 128, 128, 255); text = "Stone"; shouldRender = true; }
                        break;
                    case EntityType::MetalNode:
                        if (config.resourceMetal && distance < config.resourceDistance) { color = IM_COL32(165, 42, 42, 255); text = "Metal"; shouldRender = true; }
                        break;
                    case EntityType::SulfurNode:
                        if (config.resourceSulfur && distance < config.resourceDistance) { color = IM_COL32(255, 255, 0, 255); text = "Sulfur"; shouldRender = true; }
                        break;

                    case EntityType::EliteCrate:
                        if (config.lootEliteCrates && distance < config.lootDistance) { color = IM_COL32(255, 215, 0, 255); text = "Elite Crate"; shouldRender = true; }
                        break;
                    case EntityType::MilitaryCrate:
                        if (config.lootMilitaryCrates && distance < config.lootDistance) { color = IM_COL32(0, 128, 0, 255); text = "Military Crate"; shouldRender = true; }
                        break;
                    case EntityType::Airdrop:
                        if (config.lootAirDrops && distance < config.lootDistance) { color = IM_COL32(0, 0, 255, 255); text = "Airdrop"; shouldRender = true; }
                        break;
                    case EntityType::LockedCrate:
                        if (config.lootLockedCrates && distance < config.lootDistance) { color = IM_COL32(255, 0, 0, 255); text = "Locked Crate"; shouldRender = true; }
                        break;
                    case EntityType::Stash:
                        if (config.lootStash && distance < config.lootDistance) { color = IM_COL32(139, 69, 19, 255); text = "Stash"; shouldRender = true; }
                        break;

                    case EntityType::AutoTurret:
                        if (config.trapAutoTurrets) { color = IM_COL32(255, 0, 0, 255); text = "Auto Turret"; shouldRender = true; }
                        break;
                    case EntityType::FlameTurret:
                        if (config.trapFlameTurrets) { color = IM_COL32(255, 165, 0, 255); text = "Flame Turret"; shouldRender = true; }
                        break;
                    case EntityType::BearTrap:
                        if (config.trapBearTraps) { color = IM_COL32(139, 69, 19, 255); text = "Bear Trap"; shouldRender = true; }
                        break;
                    case EntityType::LandMine:
                        if (config.trapLandMines) { color = IM_COL32(255, 0, 0, 255); text = "Land Mine"; shouldRender = true; }
                        break;

                    default:
                        break;
                }

                if (shouldRender) {
                    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
                    drawList->AddText(ImVec2(screenPos.x - textSize.x / 2, screenPos.y), color, text.c_str());

                    std::string distStr = std::to_string((int)distance) + "m";
                    ImVec2 distSize = ImGui::CalcTextSize(distStr.c_str());
                    drawList->AddText(ImVec2(screenPos.x - distSize.x / 2, screenPos.y + textSize.y), color, distStr.c_str());
                }
            }
        }
    }

    void ESP::updateMatrices() {
        if (!memoryManager || !memoryManager->m_gameAssemblyBase) return;

        uintptr_t camera_manager = memoryManager->ReadMemory<uintptr_t>(memoryManager->m_gameAssemblyBase + offsets::main_camera_c_o::klass);
        uintptr_t camera = memoryManager->ReadMemory<uintptr_t>(camera_manager + offsets::main_camera_o::static_fields);
        uintptr_t view_matrix_ptr = camera + 0x2FC; // This offset seems to be consistent

        viewMatrix = memoryManager->ReadMemory<Matrix4x4>(view_matrix_ptr);
        // We will assume projection matrix is right after view matrix for now, this might need adjustment.
        projectionMatrix = memoryManager->ReadMemory<Matrix4x4>(view_matrix_ptr + sizeof(Matrix4x4));
    }

    bool ESP::worldToScreen(const Vector3& worldPos, ImVec2& screenPos) {
        Matrix4x4 tempMatrix = viewMatrix; // Use a temporary matrix for calculations

        Vector3 trans = { tempMatrix.m[3][0], tempMatrix.m[3][1], tempMatrix.m[3][2] };
        Vector3 right = { tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2] };
        Vector3 up = { tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2] };

        float w = trans.x * worldPos.x + trans.y * worldPos.y + trans.z * worldPos.z + tempMatrix.m[3][3];

        if (w < 0.1f) {
            return false;
        }

        float x = right.x * worldPos.x + right.y * worldPos.y + right.z * worldPos.z + tempMatrix.m[0][3];
        float y = up.x * worldPos.x + up.y * worldPos.y + up.z * worldPos.z + tempMatrix.m[1][3];

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        float screenWidth = viewport->WorkSize.x;
        float screenHeight = viewport->WorkSize.y;

        screenPos.x = (screenWidth / 2.0f * (1.0f + x / w));
        screenPos.y = (screenHeight / 2.0f * (1.0f - y / w));

        return true;
    }

} // namespace MIT

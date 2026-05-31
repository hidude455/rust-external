#include "Aimbot.h"
#include "offsets.h"
#include <algorithm>

namespace MIT {

    Aimbot::Aimbot(MemoryManager* memManager) : memoryManager(memManager) {}

    Aimbot::~Aimbot() {}

    void Aimbot::SetConfig(const AimbotConfig& newConfig) {
        config = newConfig;
    }

    const AimbotConfig& Aimbot::GetConfig() const {
        return config;
    }

    void Aimbot::Update(const AimbotConfig& newConfig) {
        SetConfig(newConfig);
        if (!config.enabled) return;

        updateMatrices();
        auto entities = memoryManager->GetEntities();
        Entity target = findBestTarget(entities);

        if (target.ptr == 0) return; // No valid target found

        Vector3 targetPos = getTargetPosition(target);
        if (targetPos.x == 0 && targetPos.y == 0 && targetPos.z == 0) return;

        Vector3 aimAngles;
        calculateAimAngles(targetPos, aimAngles);

        if (config.smoothing) {
            applySmoothing(aimAngles);
        }

        setViewAngles(aimAngles);

        if (config.noRecoil) {
            applyRecoilControl();
        }
    }

    Entity Aimbot::findBestTarget(const std::vector<Entity>& entities) {
        Entity bestTarget = {};
        float minFov = config.fovRadius;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        float screenCenterX = viewport->WorkSize.x / 2.0f;
        float screenCenterY = viewport->WorkSize.y / 2.0f;

        for (const auto& entity : entities) {
            if (entity.type != EntityType::Player || entity.is_npc) continue;

            ImVec2 screenPos;
            if (worldToScreen(entity.position, screenPos)) {
                float fov = std::sqrt(std::pow(screenPos.x - screenCenterX, 2) + std::pow(screenPos.y - screenCenterY, 2));
                if (fov < minFov) {
                    minFov = fov;
                    bestTarget = entity;
                }
            }
        }

        return bestTarget;
    }

    Vector3 Aimbot::getTargetPosition(const Entity& target) {
        if (!target.ptr) return {};

        uintptr_t player_model = memoryManager->ReadMemory<uintptr_t>(target.ptr + offsets::base_player_o::player_model);
        if (!player_model) return {};

        uintptr_t bone_transforms = memoryManager->ReadMemory<uintptr_t>(player_model + offsets::player_model_o::bone_transforms);
        if (!bone_transforms) return {};

        int bone_index = 47; // Default to head bone
        switch (config.targetBone) {
            case 1: // Chest
                bone_index = 21;
                break;
            case 2: // Stomach
                bone_index = 1;
                break;
        }

        uintptr_t bone_transform = memoryManager->ReadMemory<uintptr_t>(bone_transforms + 0x20 + bone_index * 0x8);
        if (!bone_transform) return {};

        uintptr_t transform_visual_state = memoryManager->ReadMemory<uintptr_t>(bone_transform + 0x38);
        if (!transform_visual_state) return {};

        return memoryManager->ReadMemory<Vector3>(transform_visual_state + 0x90);
    }

    void Aimbot::calculateAimAngles(const Vector3& targetPos, Vector3& outAngles) {
        Vector3 localPos = memoryManager->GetLocalPlayerPosition();
        Vector3 delta = targetPos - localPos;
        float delta_length = delta.Distance(Vector3(0,0,0));

        float pitch = -asin(delta.y / delta_length) * (180.0f / 3.1415926535f);
        float yaw = atan2(delta.x, delta.z) * (180.0f / 3.1415926535f);

        // Normalize angles
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        while (yaw > 180.0f) yaw -= 360.0f;
        while (yaw < -180.0f) yaw += 360.0f;

        outAngles = { pitch, yaw, 0.0f };
    }

    void Aimbot::applySmoothing(Vector3& angles) {
        uintptr_t player_input = memoryManager->ReadMemory<uintptr_t>(memoryManager->m_localPlayer + offsets::base_player_o::input);
        if (!player_input) return;

        Vector3 current_angles = memoryManager->ReadMemory<Vector3>(player_input + offsets::player_input_o::body_angles);

        Vector3 delta = angles - current_angles;

        // Normalize delta
        while (delta.y > 180) { delta.y -= 360; }
        while (delta.y < -180) { delta.y += 360; }

        float smooth = config.smoothness > 1.0f ? config.smoothness : 1.0f;

        angles.x = current_angles.x + delta.x / smooth;
        angles.y = current_angles.y + delta.y / smooth;
    }

    void Aimbot::applyRecoilControl() {
        if (!memoryManager->m_localPlayer) return;

        uintptr_t player_input = memoryManager->ReadMemory<uintptr_t>(memoryManager->m_localPlayer + offsets::base_player_o::input);
        if (!player_input) return;

        Vector3 current_recoil = memoryManager->ReadMemory<Vector3>(player_input + offsets::player_input_o::recoil_angles);
        Vector3 current_angles = memoryManager->ReadMemory<Vector3>(player_input + offsets::player_input_o::body_angles);

        float recoil_reduction = config.recoilControl / 100.0f;

        Vector3 new_angles;
        new_angles.x = current_angles.x - (current_recoil.x * recoil_reduction);
        new_angles.y = current_angles.y - (current_recoil.y * recoil_reduction);
        new_angles.z = 0; // Roll is not affected

        setViewAngles(new_angles);
    }

    void Aimbot::setViewAngles(const Vector3& angles) {
        uintptr_t player_input = memoryManager->ReadMemory<uintptr_t>(memoryManager->m_localPlayer + offsets::base_player_o::input);
        if (!player_input) return;

        memoryManager->WriteMemory<Vector3>(player_input + offsets::player_input_o::body_angles, angles);
    }

    void Aimbot::updateMatrices() {
        if (!memoryManager || !memoryManager->m_gameAssemblyBase) return;

        uintptr_t camera_manager = memoryManager->ReadMemory<uintptr_t>(memoryManager->m_gameAssemblyBase + offsets::main_camera_c_o::klass);
        uintptr_t camera = memoryManager->ReadMemory<uintptr_t>(camera_manager + offsets::main_camera_o::static_fields);
        uintptr_t view_matrix_ptr = camera + 0x2FC; // This offset seems to be consistent

        viewMatrix = memoryManager->ReadMemory<Matrix4x4>(view_matrix_ptr);
    }

    bool Aimbot::worldToScreen(const Vector3& worldPos, ImVec2& screenPos) {
        Matrix4x4 tempMatrix = viewMatrix;

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

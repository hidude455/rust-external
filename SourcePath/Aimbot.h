#pragma once
#include "Common.h"
#include "MemoryManager.h"

namespace MIT {

    struct AimbotConfig {
        // General
        bool enabled = false;
        bool silentAim = false;
        bool prediction = true;
        bool smoothing = true;
        bool visibleCheck = true;
        float fovRadius = 90.0f;
        float smoothness = 5.0f;
        int targetBone = 0; // 0: Head, 1: Chest, 2: Stomach

        // Weapon Specific
        bool noRecoil = false;
        bool noSpread = false;
        float recoilControl = 80.0f;
        float spreadControl = 70.0f;
    };

    class Aimbot {
    public:
        Aimbot(MemoryManager* memManager);
        ~Aimbot();

        void Update(const AimbotConfig& config);
        void SetConfig(const AimbotConfig& config);
        const AimbotConfig& GetConfig() const;

    private:
        MemoryManager* memoryManager;
        AimbotConfig config;
        Matrix4x4 viewMatrix;

        void updateMatrices();
        bool worldToScreen(const Vector3& worldPos, ImVec2& screenPos);
        Entity findBestTarget(const std::vector<Entity>& entities);
        Vector3 getTargetPosition(const Entity& target);
        void calculateAimAngles(const Vector3& targetPos, Vector3& outAngles);
        void applySmoothing(Vector3& angles);
        void applyRecoilControl();
        void setViewAngles(const Vector3& angles);
    };

} // namespace MIT

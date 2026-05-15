#pragma once
#include "Common.h"
#include "MemoryManager.h"
#include "../MenuPath/imgui/imgui.h"

namespace MIT {
    struct CombatConfig {
        bool enableRecoil = false;
        bool enablePrediction = false;
        bool enableAimbot = false;
        float smoothness = 0.5f;
        float fov = 15.0f;
        bool aimAtHead = true;
        bool visibleOnly = true;
        int priorityMode = 0; // 0 = Distance, 1 = Health, 2 = Crosshair
    };

    struct TrajectoryData {
        Vector3 startPosition;
        Vector3 targetPosition;
        Vector3 predictedPosition;
        float travelTime;
        float bulletDrop;
        bool isValid;
    };

    class Combat {
    private:
        MemoryManager* memoryManager;
        CombatConfig config;
        
        // Recoil patterns for different weapons
        std::unordered_map<std::string, std::vector<Vector3>> recoilPatterns;
        Vector3 currentRecoilOffset;
        int currentBulletCount;
        
        // Input smoothing
        Vector3 targetAimPosition;
        Vector3 currentAimPosition;
        std::chrono::high_resolution_clock::time_point lastUpdateTime;
        
        // Weapon data cache
        WeaponData currentWeapon;
        std::chrono::high_resolution_clock::time_point lastWeaponUpdate;
        
        // Trajectory calculation
        TrajectoryData calculateTrajectory(const Vector3& start, const Vector3& target, 
                                          float bulletVelocity, float gravityModifier);
        Vector3 predictTargetMovement(const Vector3& targetPos, const Vector3& targetVelocity, 
                                    float time);
        
        // Aim assistance
        Vector3 getBestTarget(const std::vector<Entity>& entities);
        float getFOVDistance(const Vector3& screenPos);
        void applySmoothAim(const Vector3& targetPos, float deltaTime);
        
        // Recoil compensation
        void updateRecoilPattern();
        void applyRecoilCompensation();
        Vector3 calculateRecoilOffset(const std::string& weaponName, int bulletCount);
        
        // Input simulation
        void simulateMouseMove(int deltaX, int deltaY);
        void simulateMouseClick(int button);

    public:
        Combat(MemoryManager* memoryManager);
        ~Combat();
        
        void Initialize();
        void Update();
        void Render();
        
        void SetConfig(const CombatConfig& newConfig) { config = newConfig; }
        const CombatConfig& GetConfig() const { return config; }
        
        // Combat functions
        void handleRecoil();
        void handleAimAssistance();
        void handleProjectilePrediction();
        
        // Utility functions
        void ToggleRecoil() { config.enableRecoil = !config.enableRecoil; }
        void TogglePrediction() { config.enablePrediction = !config.enablePrediction; }
        void ToggleAimbot() { config.enableAimbot = !config.enableAimbot; }
        
        // Weapon management
        void updateWeaponData();
        const WeaponData& getCurrentWeapon() const { return currentWeapon; }
        
        // Performance monitoring
        float getAveragePredictionTime() const;
        int getRecoilCompensationsPerSecond() const;
    };
}

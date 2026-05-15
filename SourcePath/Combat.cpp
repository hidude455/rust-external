#include "Combat.h"
#include <cmath>
#include <algorithm>
#include <unordered_map>

namespace MIT {
    Combat::Combat(MemoryManager* memoryManager) 
        : memoryManager(memoryManager), currentRecoilOffset{0, 0, 0}, currentBulletCount(0),
          targetAimPosition{0, 0, 0}, currentAimPosition{0, 0, 0} {
        lastUpdateTime = std::chrono::high_resolution_clock::now();
        lastWeaponUpdate = std::chrono::high_resolution_clock::now();
        
        // Initialize recoil patterns for common weapons
        recoilPatterns["AssaultRifle"] = {
            {0.0f, 0.5f, 0.0f}, {0.1f, 0.6f, 0.0f}, {-0.1f, 0.7f, 0.0f},
            {0.2f, 0.8f, 0.0f}, {-0.2f, 0.9f, 0.0f}, {0.0f, 1.0f, 0.0f}
        };
        
        recoilPatterns["SMG"] = {
            {0.0f, 0.3f, 0.0f}, {0.05f, 0.4f, 0.0f}, {-0.05f, 0.5f, 0.0f},
            {0.1f, 0.6f, 0.0f}, {-0.1f, 0.7f, 0.0f}, {0.0f, 0.8f, 0.0f}
        };
        
        recoilPatterns["Sniper"] = {
            {0.0f, 0.2f, 0.0f}, {0.0f, 0.3f, 0.0f}
        };
    }

    Combat::~Combat() {
    }

    void Combat::Initialize() {
        LOG_INFO("Combat system initialized");
    }

    void Combat::Update() {
        if (!memoryManager->IsProcessValid()) return;
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
        lastUpdateTime = currentTime;
        
        // Update weapon data periodically
        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastWeaponUpdate).count() > 100) {
            updateWeaponData();
            lastWeaponUpdate = currentTime;
        }
        
        // Handle combat features
        if (config.enableRecoil) {
            handleRecoil();
        }
        
        if (config.enableAimbot) {
            handleAimAssistance();
        }
        
        if (config.enablePrediction) {
            handleProjectilePrediction();
        }
        
        // Apply smooth aim transitions
        applySmoothAim(targetAimPosition, deltaTime);
    }

    void Combat::Render() {
        if (!ImGui::Begin("Combat Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::End();
            return;
        }
        
        ImGui::Text("Weapon: %s", currentWeapon.name.c_str());
        ImGui::Text("Bullet Velocity: %.1f m/s", currentWeapon.bulletVelocity);
        ImGui::Text("Gravity Modifier: %.2f", currentWeapon.gravityModifier);
        
        ImGui::Separator();
        ImGui::Text("Recoil Offset: %.2f, %.2f", currentRecoilOffset.x, currentRecoilOffset.y);
        ImGui::Text("Bullet Count: %d", currentBulletCount);
        
        ImGui::Separator();
        ImGui::Text("Aim Position: %.2f, %.2f", targetAimPosition.x, targetAimPosition.y);
        ImGui::Text("Current Position: %.2f, %.2f", currentAimPosition.x, currentAimPosition.y);
        
        ImGui::End();
    }

    void Combat::handleRecoil() {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            currentBulletCount++;
            updateRecoilPattern();
            applyRecoilCompensation();
        } else {
            currentBulletCount = 0;
            currentRecoilOffset = {0, 0, 0};
        }
    }

    void Combat::handleAimAssistance() {
        if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) return;
        
        std::vector<Entity> entities = memoryManager->GetEntities();
        Vector3 bestTarget = getBestTarget(entities);
        
        if (bestTarget.x != 0 || bestTarget.y != 0 || bestTarget.z != 0) {
            targetAimPosition = bestTarget;
        }
    }

    void Combat::handleProjectilePrediction() {
        if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) return;
        
        // Get current weapon and target
        Vector3 localPos = memoryManager->GetLocalPlayerPosition();
        std::vector<Entity> entities = memoryManager->GetEntities();
        
        if (entities.empty()) return;
        
        Entity target = entities[0]; // Simplified - get closest target
        
        TrajectoryData trajectory = calculateTrajectory(
            localPos, 
            target.position, 
            currentWeapon.bulletVelocity, 
            currentWeapon.gravityModifier
        );
        
        if (trajectory.isValid) {
            // Adjust aim to compensate for bullet drop and target movement
            targetAimPosition = trajectory.predictedPosition;
        }
    }

    TrajectoryData Combat::calculateTrajectory(const Vector3& start, const Vector3& target, 
                                               float bulletVelocity, float gravityModifier) {
        TrajectoryData data;
        data.startPosition = start;
        data.targetPosition = target;
        data.isValid = false;
        
        if (bulletVelocity <= 0) return data;
        
        Vector3 direction = target - start;
        float distance = direction.Distance({0, 0, 0});
        
        if (distance <= 0) return data;
        
        // Calculate time to target
        data.travelTime = distance / bulletVelocity;
        
        // Calculate bullet drop
        const float gravity = 9.81f;
        data.bulletDrop = 0.5f * gravity * gravityModifier * data.travelTime * data.travelTime;
        
        // Predict target position (simplified - assumes no movement)
        data.predictedPosition = target;
        data.predictedPosition.y += data.bulletDrop; // Compensate for drop
        
        data.isValid = true;
        return data;
    }

    Vector3 Combat::predictTargetMovement(const Vector3& targetPos, const Vector3& targetVelocity, 
                                         float time) {
        // Simple linear prediction
        return targetPos + targetVelocity * time;
    }

    Vector3 Combat::getBestTarget(const std::vector<Entity>& entities) {
        Vector3 bestTarget{0, 0, 0};
        float bestScore = FLT_MAX;
        
        for (const auto& entity : entities) {
            if (entity.type != EntityType::Player) continue;
            if (!entity.isVisible && config.visibleOnly) continue;
            
            float score = 0;
            
            switch (config.priorityMode) {
                case 0: // Distance
                    score = entity.distance;
                    break;
                case 1: // Health (placeholder)
                    score = 100; // Would need health data
                    break;
                case 2: // Crosshair distance
                    ImVec2 screenPos;
                    if (worldToScreen(entity.position, screenPos)) {
                        ImVec2 center = ImGui::GetIO().DisplaySize;
                        center.x *= 0.5f;
                        center.y *= 0.5f;
                        score = sqrt(pow(screenPos.x - center.x, 2) + pow(screenPos.y - center.y, 2));
                    }
                    break;
            }
            
            if (score < bestScore) {
                bestScore = score;
                bestTarget = entity.position;
            }
        }
        
        return bestTarget;
    }

    float Combat::getFOVDistance(const Vector3& screenPos) {
        ImVec2 center = ImGui::GetIO().DisplaySize;
        center.x *= 0.5f;
        center.y *= 0.5f;
        
        return sqrt(pow(screenPos.x - center.x, 2) + pow(screenPos.y - center.y, 2));
    }

    void Combat::applySmoothAim(const Vector3& targetPos, float deltaTime) {
        if (config.smoothness <= 0.0f) {
            currentAimPosition = targetPos;
            return;
        }
        
        // Smooth interpolation
        float smoothingFactor = 1.0f - pow(1.0f - config.smoothness, deltaTime * 60.0f);
        
        currentAimPosition.x += (targetPos.x - currentAimPosition.x) * smoothingFactor;
        currentAimPosition.y += (targetPos.y - currentAimPosition.y) * smoothingFactor;
        currentAimPosition.z += (targetPos.z - currentAimPosition.z) * smoothingFactor;
        
        // Calculate mouse movement needed
        Vector3 delta = targetPos - currentAimPosition;
        
        if (abs(delta.x) > 0.1f || abs(delta.y) > 0.1f) {
            simulateMouseMove(static_cast<int>(delta.x), static_cast<int>(delta.y));
        }
    }

    void Combat::updateRecoilPattern() {
        Vector3 offset = calculateRecoilOffset(currentWeapon.name, currentBulletCount);
        currentRecoilOffset = currentRecoilOffset + offset;
    }

    void Combat::applyRecoilCompensation() {
        if (currentRecoilOffset.x == 0 && currentRecoilOffset.y == 0) return;
        
        // Apply negative recoil to compensate
        int compensationX = static_cast<int>(-currentRecoilOffset.x * 10);
        int compensationY = static_cast<int>(-currentRecoilOffset.y * 10);
        
        simulateMouseMove(compensationX, compensationY);
    }

    Vector3 Combat::calculateRecoilOffset(const std::string& weaponName, int bulletCount) {
        auto it = recoilPatterns.find(weaponName);
        if (it == recoilPatterns.end()) {
            return {0, 0, 0};
        }
        
        const auto& pattern = it->second;
        if (bulletCount >= pattern.size()) {
            // Repeat pattern or use last value
            int index = bulletCount % pattern.size();
            return pattern[index];
        }
        
        return pattern[bulletCount];
    }

    void Combat::simulateMouseMove(int deltaX, int deltaY) {
        // This would use Windows API to move mouse
        // For now, just log the movement
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dx = deltaX;
        input.mi.dy = deltaY;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.mouseData = 0;
        input.mi.dwExtraInfo = 0;
        input.mi.time = 0;
        
        SendInput(1, &input, sizeof(INPUT));
    }

    void Combat::simulateMouseClick(int button) {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        
        if (button == 0) { // Left click
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            SendInput(1, &input, sizeof(INPUT));
            
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(1, &input, sizeof(INPUT));
        } else if (button == 1) { // Right click
            input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            SendInput(1, &input, sizeof(INPUT));
            
            input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            SendInput(1, &input, sizeof(INPUT));
        }
    }

    void Combat::updateWeaponData() {
        currentWeapon = memoryManager->GetCurrentWeapon();
    }

    float Combat::getAveragePredictionTime() const {
        return 1.0f; // Placeholder
    }

    int Combat::getRecoilCompensationsPerSecond() const {
        return static_cast<int>(currentBulletCount);
    }

    bool Combat::worldToScreen(const Vector3& worldPos, ImVec2& screenPos) {
        // Simplified world-to-screen conversion
        // In a real implementation, this would use game's view/projection matrices
        screenPos.x = worldPos.x + 400; // Offset to center
        screenPos.y = worldPos.y + 300;
        return true;
    }
}

#include "ESP.h"
#include <algorithm>
#include <cmath>
#include <map>

namespace MIT {
    
    // Bone definitions for skeleton ESP
    enum class Bone {
        Head = 0,
        Neck = 1,
        Chest = 2,
        Pelvis = 3,
        LeftShoulder = 4,
        RightShoulder = 5,
        LeftElbow = 6,
        RightElbow = 7,
        LeftHand = 8,
        RightHand = 9,
        LeftHip = 10,
        RightHip = 11,
        LeftKnee = 12,
        RightKnee = 13,
        LeftFoot = 14,
        RightFoot = 15
    };
    
    // ESP configuration structure
    struct ESPConfig {
        // General settings
        bool enabled = true;
        bool showHealthBars = true;
        bool showBoneESP = true;
        bool showBoundingBoxes = true;
        bool showNames = true;
        bool showDistance = true;
        bool showWeapon = true;
        bool showTracers = true;
        bool showSkeleton = true;
        
        // Color settings
        ImVec4 enemyColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        ImVec4 friendlyColor = ImVec4(0.0f, 0.8f, 0.2f, 1.0f);
        ImVec4 neutralColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 healthBarColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 armorBarColor = ImVec4(0.8f, 0.4f, 0.0f, 1.0f);
        ImVec4 tracerColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        // Health bar settings
        float healthBarWidth = 80.0f;
        float healthBarHeight = 6.0f;
        float healthBarOffset = 30.0f;
        bool healthBarBackground = true;
        
        // Bone ESP settings
        bool showHeadBone = true;
        bool showNeckBone = true;
        bool showChestBone = true;
        bool showPelvisBone = true;
        bool showSpineBones = true;
        bool showArmBones = true;
        bool showLegBones = true;
        
        // Tracer settings
        float tracerLength = 200.0f;
        float tracerWidth = 1.5f;
        bool showHeadTracer = true;
        bool showChestTracer = true;
        
        // Distance settings
        float maxRenderDistance = 500.0f;
        bool fadeWithDistance = true;
        float fadeStartDistance = 200.0f;
        float fadeEndDistance = 400.0f;
        
        // Toggle settings
        bool toggleHealthBars = true;
        bool toggleBoneESP = true;
        bool toggleBoundingBoxes = true;
        bool toggleTracers = true;
        bool toggleSkeleton = true;
        bool toggleColors = true;
    };
    
    // Entity structure for enhanced ESP
    struct EnhancedEntity {
        Entity baseEntity;
        
        // Extended information
        float healthPercentage;
        float armorPercentage;
        std::string weaponName;
        int ammoCount;
        float lastSeenTime;
        bool isVisible;
        bool isFriendly;
        bool isNPC;
        bool isAnimal;
        
        // ESP data
        ImVec2 screenPosition;
        ImVec2 headScreenPosition;
        ImVec2 chestScreenPosition;
        ImVec2 pelvisScreenPosition;
        BoundingBox boundingBox;
        
        // Bone positions
        std::map<Bone, ImVec2> bonePositions;
        
        // Health bar data
        float healthBarWidth;
        float healthBarHeight;
        ImVec2 healthBarPosition;
        
        // Tracer data
        std::vector<ImVec2> tracerPoints;
        int tracerAlpha;
    };
    
    ESP::ESP(MemoryManager* memoryManager, Renderer* renderer) 
        : memoryManager(memoryManager), renderer(renderer), lastUpdate(std::chrono::high_resolution_clock::now()) {
    }

    ESP::~ESP() {
    }

    void ESP::Initialize() {
        LOG_INFO("ESP system initialized");
        
        // Initialize ESP configuration with default values
        config = ESPConfig();
        config.enabled = true;
        config.showHealthBars = true;
        config.showBoneESP = true;
        config.showBoundingBoxes = true;
        config.showNames = true;
        config.showDistance = true;
        config.showWeapon = true;
        config.showTracers = true;
        config.showSkeleton = true;
        
        // Set default colors
        config.enemyColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        config.friendlyColor = ImVec4(0.0f, 0.8f, 0.2f, 1.0f);
        config.neutralColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        config.healthBarColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        config.armorBarColor = ImVec4(0.8f, 0.4f, 0.0f, 1.0f);
        config.tracerColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        // Initialize bone visibility
        config.showHeadBone = true;
        config.showNeckBone = true;
        config.showChestBone = true;
        config.showPelvisBone = true;
        config.showSpineBones = true;
        config.showArmBones = true;
        config.showLegBones = true;
        
        // Initialize tracer settings
        config.showHeadTracer = true;
        config.showChestTracer = true;
        config.showPelvisTracer = true;
        config.showLimbTracers = true;
        
        // Initialize distance settings
        config.maxRenderDistance = 500.0f;
        config.fadeWithDistance = true;
        config.fadeStartDistance = 200.0f;
        config.fadeEndDistance = 400.0f;
        
        // Initialize toggle settings
        config.toggleHealthBars = true;
        config.toggleBoneESP = true;
        config.toggleBoundingBoxes = true;
        config.toggleTracers = true;
        config.toggleSkeleton = true;
        config.toggleColors = true;
        
        // Load saved configuration if exists
        LoadConfiguration();
    }

    void ESP::Update() {
        if (!config.enabled || !memoryManager->IsProcessValid()) return;
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdate).count();
        
        if (elapsed >= updateInterval) {
            // Update matrices for world-to-screen conversion
            updateMatrices();
            
            // Get entities from memory
            std::vector<Entity> allEntities = memoryManager->GetEntities();
            
            // Filter entities based on configuration
            cachedEntities = filterEntities(allEntities);
            
            // Update entity data for ESP features
            UpdateEntityData(cachedEntities);
            
            lastUpdate = currentTime;
        }
    }

    void ESP::Render() {
        if (!config.enabled || cachedEntities.empty()) return;
        
        drawList = ImGui::GetBackgroundDrawList();
        if (!drawList) return;
        
        // Render all enhanced entities
        for (const auto& entity : enhancedEntities) {
            RenderEnhancedEntity(entity);
        }
    }
    
    void ESP::UpdateEntityData(std::vector<Entity>& entities) {
        enhancedEntities.clear();
        enhancedEntities.reserve(entities.size());
        
        for (const auto& entity : entities) {
            EnhancedEntity enhanced = {};
            enhanced.baseEntity = entity;
            
            // Calculate extended information
            enhanced.healthPercentage = entity.health / entity.maxHealth * 100.0f;
            enhanced.armorPercentage = entity.armor / entity.maxArmor * 100.0f;
            enhanced.weaponName = entity.weaponName;
            enhanced.ammoCount = entity.ammo;
            enhanced.lastSeenTime = GetTickCount64();
            enhanced.isVisible = IsEntityVisible(entity);
            enhanced.isFriendly = entity.team == 0; // Assuming team 0 is friendly
            enhanced.isNPC = entity.isNPC;
            enhanced.isAnimal = entity.isAnimal;
            
            // Calculate screen positions
            Vector3 worldPos = entity.position;
            worldToScreen(worldPos, enhanced.screenPosition);
            
            // Calculate bone positions
            CalculateBonePositions(entity, enhanced);
            
            // Calculate bounding box
            enhanced.boundingBox = calculateBoundingBox(worldPos, entity.height, entity.width);
            
            // Calculate health bar data
            CalculateHealthBarData(enhanced);
            
            // Calculate tracer points
            CalculateTracerPoints(enhanced);
            
            enhancedEntities.push_back(enhanced);
        }
    }
    
    void ESP::CalculateBonePositions(const Entity& entity, EnhancedEntity& enhanced) {
        if (!config.showBoneESP) return;
        
        // Get bone positions from memory (simplified)
        Vector3 headPos = GetBonePosition(entity, Bone::Head);
        Vector3 neckPos = GetBonePosition(entity, Bone::Neck);
        Vector3 chestPos = GetBonePosition(entity, Bone::Chest);
        Vector3 pelvisPos = GetBonePosition(entity, Bone::Pelvis);
        Vector3 leftShoulderPos = GetBonePosition(entity, Bone::LeftShoulder);
        Vector3 rightShoulderPos = GetBonePosition(entity, Bone::RightShoulder);
        Vector3 leftElbowPos = GetBonePosition(entity, Bone::LeftElbow);
        Vector3 rightElbowPos = GetBonePosition(entity, Bone::RightElbow);
        Vector3 leftHandPos = GetBonePosition(entity, Bone::LeftHand);
        Vector3 rightHandPos = GetBonePosition(entity, Bone::RightHand);
        Vector3 leftHipPos = GetBonePosition(entity, Bone::LeftHip);
        Vector3 rightHipPos = GetBonePosition(entity, Bone::RightHip);
        Vector3 leftKneePos = GetBonePosition(entity, Bone::LeftKnee);
        Vector3 rightKneePos = GetBonePosition(entity, Bone::RightKnee);
        Vector3 leftFootPos = GetBonePosition(entity, Bone::LeftFoot);
        Vector3 rightFootPos = GetBonePosition(entity, Bone::RightFoot);
        
        // Convert to screen coordinates
        worldToScreen(headPos, enhanced.headScreenPosition);
        worldToScreen(neckPos, enhanced.bonePositions[Bone::Neck]);
        worldToScreen(chestPos, enhanced.bonePositions[Bone::Chest]);
        worldToScreen(pelvisPos, enhanced.bonePositions[Bone::Pelvis]);
        worldToScreen(leftShoulderPos, enhanced.bonePositions[Bone::LeftShoulder]);
        worldToScreen(rightShoulderPos, enhanced.bonePositions[Bone::RightShoulder]);
        worldToScreen(leftElbowPos, enhanced.bonePositions[Bone::LeftElbow]);
        worldToScreen(rightElbowPos, enhanced.bonePositions[Bone::RightElbow]);
        worldToScreen(leftHandPos, enhanced.bonePositions[Bone::LeftHand]);
        worldToScreen(rightHandPos, enhanced.bonePositions[Bone::RightHand]);
        worldToScreen(leftHipPos, enhanced.bonePositions[Bone::LeftHip]);
        worldToScreen(rightHipPos, enhanced.bonePositions[Bone::RightHip]);
        worldToScreen(leftKneePos, enhanced.bonePositions[Bone::LeftKnee]);
        worldToScreen(rightKneePos, enhanced.bonePositions[Bone::RightKnee]);
        worldToScreen(leftFootPos, enhanced.bonePositions[Bone::LeftFoot]);
        worldToScreen(rightFootPos, enhanced.bonePositions[Bone::RightFoot]);
    }
    
    void ESP::CalculateHealthBarData(EnhancedEntity& entity) {
        if (!config.showHealthBars) return;
        
        // Calculate health bar position
        entity.healthBarPosition = ImVec2(
            entity.screenPosition.x - config.healthBarWidth / 2.0f,
            entity.screenPosition.y - config.healthBarOffset
        );
        
        // Calculate health bar dimensions
        entity.healthBarWidth = config.healthBarWidth * (entity.healthPercentage / 100.0f);
        entity.healthBarHeight = config.healthBarHeight;
    }
    
    void ESP::CalculateTracerPoints(EnhancedEntity& entity) {
        if (!config.showTracers) return;
        
        entity.tracerPoints.clear();
        
        // Calculate tracer from local player to entity
        ImVec2 localPlayerScreen;
        Vector3 localPlayerPos = GetLocalPlayerPosition();
        if (worldToScreen(localPlayerPos, localPlayerScreen)) {
            entity.tracerPoints.push_back(localPlayerScreen);
            entity.tracerPoints.push_back(entity.screenPosition);
            
            // Add intermediate points for curved tracers
            if (config.showHeadTracer) {
                entity.tracerPoints.push_back(entity.headScreenPosition);
            }
            if (config.showChestTracer) {
                entity.tracerPoints.push_back(entity.bonePositions[Bone::Chest]);
            }
            if (config.showPelvisTracer) {
                entity.tracerPoints.push_back(entity.bonePositions[Bone::Pelvis]);
            }
        }
        
        // Calculate tracer alpha based on distance
        float distance = GetDistance(localPlayerPos, entity.baseEntity.position);
        entity.tracerAlpha = CalculateAlphaFromDistance(distance);
    }
    
    float ESP::CalculateAlphaFromDistance(float distance) {
        if (!config.fadeWithDistance) return 255;
        
        if (distance <= config.fadeStartDistance) return 255;
        if (distance >= config.fadeEndDistance) return 0;
        
        // Linear interpolation
        float fadeRange = config.fadeEndDistance - config.fadeStartDistance;
        float fadeProgress = (distance - config.fadeStartDistance) / fadeRange;
        return 255 * (1.0f - fadeProgress);
    }
    
    void ESP::RenderEnhancedEntity(const EnhancedEntity& entity) {
        // Check distance filter
        float distance = GetDistance(GetLocalPlayerPosition(), entity.baseEntity.position);
        if (distance > config.maxRenderDistance) return;
        
        // Calculate alpha for distance fading
        float alpha = CalculateAlphaFromDistance(distance);
        
        // Get appropriate color based on entity type
        ImVec4 entityColor = GetEntityColor(entity, alpha);
        
        // Render bounding box
        if (config.toggleBoundingBoxes && config.showBoundingBoxes) {
            drawBoundingBox(entity.boundingBox, entityColor);
        }
        
        // Render health bar
        if (config.toggleHealthBars && config.showHealthBars) {
            RenderHealthBar(entity);
        }
        
        // Render bone ESP
        if (config.toggleBoneESP && config.showBoneESP) {
            RenderBoneESP(entity);
        }
        
        // Render tracers
        if (config.toggleTracers && config.showTracers) {
            RenderTracers(entity);
        }
        
        // Render skeleton
        if (config.toggleSkeleton && config.showSkeleton) {
            RenderSkeleton(entity);
        }
        
        // Render name
        if (config.showNames) {
            RenderEntityName(entity, entityColor);
        }
        
        // Render distance
        if (config.showDistance) {
            RenderEntityDistance(entity, entityColor);
        }
        
        // Render weapon
        if (config.showWeapon) {
            RenderEntityWeapon(entity, entityColor);
        }
    }
    
    ImVec4 ESP::GetEntityColor(const EnhancedEntity& entity, float alpha) {
        ImVec4 baseColor;
        
        if (entity.isFriendly) {
            baseColor = config.friendlyColor;
        } else if (entity.isNPC) {
            baseColor = config.neutralColor;
        } else if (entity.isAnimal) {
            baseColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f); // Green for animals
        } else {
            baseColor = config.enemyColor;
        }
        
        // Apply alpha
        return ImVec4(baseColor.x, baseColor.y, baseColor.z, baseColor.w * (alpha / 255.0f));
    }
    
    void ESP::RenderHealthBar(const EnhancedEntity& entity) {
        if (!config.showHealthBars) return;
        
        ImVec2 barPos = entity.healthBarPosition;
        float barWidth = entity.healthBarWidth;
        float barHeight = entity.healthBarHeight;
        
        // Draw background
        if (config.healthBarBackground) {
            ImVec4 bgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
            drawList->AddRectFilled(
                ImVec2(barPos.x - 1, barPos.y - 1),
                ImVec2(barPos.x + barWidth + 2, barPos.y + barHeight + 2),
                bgColor
            );
        }
        
        // Draw health bar
        drawList->AddRectFilled(
            barPos,
            ImVec2(barPos.x + barWidth, barPos.y + barHeight),
            config.healthBarColor
        );
        
        // Draw armor bar if applicable
        if (entity.armorPercentage > 0) {
            float armorWidth = barWidth * (entity.armorPercentage / 100.0f);
            ImVec2 armorPos = ImVec2(barPos.x, barPos.y + barHeight + 2);
            
            drawList->AddRectFilled(
                armorPos,
                ImVec2(armorPos.x + armorWidth, armorPos.y + 3),
                config.armorBarColor
            );
        }
        
        // Draw health text
        std::string healthText = std::to_string(static_cast<int>(entity.healthPercentage)) + "%";
        drawList->AddText(
            ImVec2(barPos.x + barWidth + 5, barPos.y),
            ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
            healthText.c_str()
        );
    }
    
    void ESP::RenderBoneESP(const EnhancedEntity& entity) {
        if (!config.showBoneESP) return;
        
        // Draw individual bones based on configuration
        if (config.showHeadBone) {
            drawList->AddCircleFilled(entity.headScreenPosition, 3.0f, ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 8);
        }
        
        if (config.showNeckBone) {
            DrawBoneLine(entity.bonePositions[Bone::Head], entity.bonePositions[Bone::Neck], ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        }
        
        if (config.showChestBone) {
            DrawBoneLine(entity.bonePositions[Bone::Neck], entity.bonePositions[Bone::Chest], ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
            drawList->AddCircleFilled(entity.bonePositions[Bone::Chest], 4.0f, ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 8);
        }
        
        if (config.showPelvisBone) {
            DrawBoneLine(entity.bonePositions[Bone::Chest], entity.bonePositions[Bone::Pelvis], ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
            drawList->AddCircleFilled(entity.bonePositions[Bone::Pelvis], 3.0f, ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 8);
        }
        
        if (config.showArmBones) {
            // Draw arm bones
            DrawBoneLine(entity.bonePositions[Bone::Pelvis], entity.bonePositions[Bone::LeftHip], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            DrawBoneLine(entity.bonePositions[Bone::LeftHip], entity.bonePositions[Bone::LeftShoulder], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            DrawBoneLine(entity.bonePositions[Bone::LeftShoulder], entity.bonePositions[Bone::LeftElbow], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            DrawBoneLine(entity.bonePositions[Bone::LeftElbow], entity.bonePositions[Bone::LeftHand], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            
            DrawBoneLine(entity.bonePositions[Bone::Pelvis], entity.bonePositions[Bone::RightHip], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            DrawBoneLine(entity.bonePositions[Bone::RightHip], entity.bonePositions[Bone::RightShoulder], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            DrawBoneLine(entity.bonePositions[Bone::RightShoulder], entity.bonePositions[Bone::RightElbow], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            DrawBoneLine(entity.bonePositions[Bone::RightElbow], entity.bonePositions[Bone::RightHand], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        }
        
        if (config.showLegBones) {
            // Draw leg bones
            DrawBoneLine(entity.bonePositions[Bone::Pelvis], entity.bonePositions[Bone::LeftHip], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            DrawBoneLine(entity.bonePositions[Bone::LeftHip], entity.bonePositions[Bone::LeftKnee], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            DrawBoneLine(entity.bonePositions[Bone::LeftKnee], entity.bonePositions[Bone::LeftFoot], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            
            DrawBoneLine(entity.bonePositions[Bone::Pelvis], entity.bonePositions[Bone::RightHip], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            DrawBoneLine(entity.bonePositions[Bone::RightHip], entity.bonePositions[Bone::RightKnee], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            DrawBoneLine(entity.bonePositions[Bone::RightKnee], entity.bonePositions[Bone::RightFoot], ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        }
    }
    
    void ESP::DrawBoneLine(const ImVec2& from, const ImVec2& to, const ImVec4& color) {
        drawList->AddLine(from, to, color, 2.0f);
    }
    
    void ESP::RenderTracers(const EnhancedEntity& entity) {
        if (!config.showTracers || entity.tracerPoints.size() < 2) return;
        
        ImVec4 tracerColor = config.tracerColor;
        tracerColor.w = entity.tracerAlpha / 255.0f;
        
        // Draw tracer lines
        for (size_t i = 0; i < entity.tracerPoints.size() - 1; i++) {
            drawList->AddLine(entity.tracerPoints[i], entity.tracerPoints[i + 1], tracerColor, config.tracerWidth);
        }
    }
    
    void ESP::RenderSkeleton(const EnhancedEntity& entity) {
        if (!config.showSkeleton) return;
        
        ImVec4 boneColor = ImVec4(1.0f, 1.0f, 1.0f, 0.8f);
        
        // Draw skeleton connections
        std::vector<std::pair<Bone, Bone>> skeletonConnections = {
            {Bone::Head, Bone::Neck},
            {Bone::Neck, Bone::Chest},
            {Bone::Chest, Bone::Pelvis},
            {Bone::Pelvis, Bone::LeftHip},
            {Bone::Pelvis, Bone::RightHip},
            {Bone::LeftHip, Bone::LeftKnee},
            {Bone::RightHip, Bone::RightKnee},
            {Bone::LeftKnee, Bone::LeftFoot},
            {Bone::RightKnee, Bone::RightFoot},
            {Bone::LeftShoulder, Bone::LeftElbow},
            {Bone::RightShoulder, Bone::RightElbow},
            {Bone::LeftElbow, Bone::LeftHand},
            {Bone::RightElbow, Bone::RightHand}
        };
        
        for (const auto& connection : skeletonConnections) {
            auto it1 = entity.bonePositions.find(connection.first);
            auto it2 = entity.bonePositions.find(connection.second);
            
            if (it1 != entity.bonePositions.end() && it2 != entity.bonePositions.end()) {
                drawList->AddLine(it1->second, it2->second, boneColor, 1.5f);
            }
        }
    }
    
    void ESP::RenderEntityName(const EnhancedEntity& entity, const ImVec4& color) {
        if (!config.showNames) return;
        
        std::string name = entity.baseEntity.name;
        ImVec2 textPos = ImVec2(entity.screenPosition.x, entity.screenPosition.y - 15);
        
        drawList->AddText(textPos, color, name.c_str());
    }
    
    void ESP::RenderEntityDistance(const EnhancedEntity& entity, const ImVec4& color) {
        if (!config.showDistance) return;
        
        float distance = GetDistance(GetLocalPlayerPosition(), entity.baseEntity.position);
        std::string distanceText = std::to_string(static_cast<int>(distance)) + "m";
        ImVec2 textPos = ImVec2(entity.screenPosition.x, entity.screenPosition.y + 5);
        
        drawList->AddText(textPos, color, distanceText.c_str());
    }
    
    void ESP::RenderEntityWeapon(const EnhancedEntity& entity, const ImVec4& color) {
        if (!config.showWeapon || entity.weaponName.empty()) return;
        
        std::string weaponText = entity.weaponName;
        ImVec2 textPos = ImVec2(entity.screenPosition.x, entity.screenPosition.y + 20);
        
        drawList->AddText(textPos, color, weaponText.c_str());
    }
    
    Vector3 ESP::GetBonePosition(const Entity& entity, Bone bone) {
        // This would read from game memory
        // For now, return simplified positions
        Vector3 basePos = entity.position;
        
        switch (bone) {
            case Bone::Head:
                return Vector3(basePos.x, basePos.y + entity.height, basePos.z);
            case Bone::Neck:
                return Vector3(basePos.x, basePos.y + entity.height * 0.9f, basePos.z);
            case Bone::Chest:
                return Vector3(basePos.x, basePos.y + entity.height * 0.7f, basePos.z);
            case Bone::Pelvis:
                return Vector3(basePos.x, basePos.y + entity.height * 0.5f, basePos.z);
            case Bone::LeftShoulder:
                return Vector3(basePos.x - entity.width * 0.3f, basePos.y + entity.height * 0.8f, basePos.z);
            case Bone::RightShoulder:
                return Vector3(basePos.x + entity.width * 0.3f, basePos.y + entity.height * 0.8f, basePos.z);
            case Bone::LeftElbow:
                return Vector3(basePos.x - entity.width * 0.3f, basePos.y + entity.height * 0.6f, basePos.z);
            case Bone::RightElbow:
                return Vector3(basePos.x + entity.width * 0.3f, basePos.y + entity.height * 0.6f, basePos.z);
            case Bone::LeftHand:
                return Vector3(basePos.x - entity.width * 0.4f, basePos.y + entity.height * 0.4f, basePos.z);
            case Bone::RightHand:
                return Vector3(basePos.x + entity.width * 0.4f, basePos.y + entity.height * 0.4f, basePos.z);
            case Bone::LeftHip:
                return Vector3(basePos.x - entity.width * 0.2f, basePos.y + entity.height * 0.5f, basePos.z);
            case Bone::RightHip:
                return Vector3(basePos.x + entity.width * 0.2f, basePos.y + entity.height * 0.5f, basePos.z);
            case Bone::LeftKnee:
                return Vector3(basePos.x - entity.width * 0.2f, basePos.y + entity.height * 0.3f, basePos.z);
            case Bone::RightKnee:
                return Vector3(basePos.x + entity.width * 0.2f, basePos.y + entity.height * 0.3f, basePos.z);
            case Bone::LeftFoot:
                return Vector3(basePos.x - entity.width * 0.2f, basePos.y, basePos.z);
            case Bone::RightFoot:
                return Vector3(basePos.x + entity.width * 0.2f, basePos.y, basePos.z);
            default:
                return basePos;
        }
    }
    
    Vector3 ESP::GetLocalPlayerPosition() {
        // This would read from game memory
        // For now, return a placeholder
        return Vector3(0, 0, 0);
    }
    
    float ESP::GetDistance(const Vector3& pos1, const Vector3& pos2) {
        float dx = pos1.x - pos2.x;
        float dy = pos1.y - pos2.y;
        float dz = pos1.z - pos2.z;
        return sqrt(dx * dx + dy * dy + dz * dz);
    }
    
    bool ESP::IsEntityVisible(const Entity& entity) {
        // This would perform line of sight checks
        // For now, return true
        return true;
    }
    
    void ESP::LoadConfiguration() {
        // Load configuration from file
        std::ifstream configFile("esp_config.ini");
        if (configFile.is_open()) {
            std::string line;
            while (std::getline(configFile, line)) {
                // Parse configuration options
                ParseConfigLine(line);
            }
            configFile.close();
        }
    }
    
    void ESP::ParseConfigLine(const std::string& line) {
        // Parse individual configuration lines
        // Format: key=value
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Apply configuration based on key
            if (key == "showHealthBars") {
                config.showHealthBars = (value == "true");
            } else if (key == "showBoneESP") {
                config.showBoneESP = (value == "true");
            } else if (key == "enemyColor") {
                ParseColor(value, config.enemyColor);
            } else if (key == "healthBarColor") {
                ParseColor(value, config.healthBarColor);
            }
            // Add more configuration options as needed
        }
    }
    
    void ESP::ParseColor(const std::string& colorString, ImVec4& color) {
        // Parse color from string format "R,G,B,A"
        std::stringstream ss(colorString);
        std::string token;
        
        if (std::getline(ss, token, ',')) {
            color.x = std::stof(token);
        }
        if (std::getline(ss, token, ',')) {
            color.y = std::stof(token);
        }
        if (std::getline(ss, token, ',')) {
            color.z = std::stof(token);
        }
        if (std::getline(ss, token, ',')) {
            color.w = std::stof(token);
        }
    }
    
    void ESP::SaveConfiguration() {
        // Save configuration to file
        std::ofstream configFile("esp_config.ini");
        if (configFile.is_open()) {
            configFile << "showHealthBars=" << (config.showHealthBars ? "true" : "false") << std::endl;
            configFile << "showBoneESP=" << (config.showBoneESP ? "true" : "false") << std::endl;
            configFile << "showBoundingBoxes=" << (config.showBoundingBoxes ? "true" : "false") << std::endl;
            configFile << "showTracers=" << (config.showTracers ? "true" : "false") << std::endl;
            configFile << "showSkeleton=" << (config.showSkeleton ? "true" : "false") << std::endl;
            
            // Save colors
            configFile << "enemyColor=" << config.enemyColor.x << "," << config.enemyColor.y << "," << config.enemyColor.z << "," << config.enemyColor.w << std::endl;
            configFile << "friendlyColor=" << config.friendlyColor.x << "," << config.friendlyColor.y << "," << config.friendlyColor.z << "," << config.friendlyColor.w << std::endl;
            configFile << "healthBarColor=" << config.healthBarColor.x << "," << config.healthBarColor.y << "," << config.healthBarColor.z << "," << config.healthBarColor.w << std::endl;
            
            configFile.close();
        }
    }
    
    void ESP::ToggleFeature(const std::string& feature) {
        if (feature == "healthBars") {
            config.toggleHealthBars = !config.toggleHealthBars;
            config.showHealthBars = config.toggleHealthBars;
        } else if (feature == "boneESP") {
            config.toggleBoneESP = !config.toggleBoneESP;
            config.showBoneESP = config.toggleBoneESP;
        } else if (feature == "boundingBoxes") {
            config.toggleBoundingBoxes = !config.toggleBoundingBoxes;
            config.showBoundingBoxes = config.toggleBoundingBoxes;
        } else if (feature == "tracers") {
            config.toggleTracers = !config.toggleTracers;
            config.showTracers = config.toggleTracers;
        } else if (feature == "skeleton") {
            config.toggleSkeleton = !config.toggleSkeleton;
            config.showSkeleton = config.toggleSkeleton;
        }
        
        // Save configuration
        SaveConfiguration();
        
        // Log toggle
        LOG_INFO("Toggled " + feature + ": " + (config.showHealthBars ? "ON" : "OFF"));
    }

    void ESP::updateMatrices() {
        // This would typically read from game memory
        // For now, using placeholder matrices
        viewMatrix = {};
        projectionMatrix = {};
        
        // Initialize identity matrices
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                viewMatrix.m[i][j] = (i == j) ? 1.0f : 0.0f;
                projectionMatrix.m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }

    bool ESP::worldToScreen(const Vector3& worldPos, ImVec2& screenPos) {
        // Get screen dimensions
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        float screenWidth = viewport->WorkSize.x;
        float screenHeight = viewport->WorkSize.y;
        
        // Transform world position to clip space
        float x = worldPos.x * viewMatrix.m[0][0] + worldPos.y * viewMatrix.m[0][1] + worldPos.z * viewMatrix.m[0][2] + viewMatrix.m[0][3];
        float y = worldPos.x * viewMatrix.m[1][0] + worldPos.y * viewMatrix.m[1][1] + worldPos.z * viewMatrix.m[1][2] + viewMatrix.m[1][3];
        float z = worldPos.x * viewMatrix.m[2][0] + worldPos.y * viewMatrix.m[2][1] + worldPos.z * viewMatrix.m[2][2] + viewMatrix.m[2][3];
        float w = worldPos.x * viewMatrix.m[3][0] + worldPos.y * viewMatrix.m[3][1] + worldPos.z * viewMatrix.m[3][2] + viewMatrix.m[3][3];
        
        // Apply projection matrix
        float clipX = x * projectionMatrix.m[0][0] + y * projectionMatrix.m[0][1] + z * projectionMatrix.m[0][2] + w * projectionMatrix.m[0][3];
        float clipY = x * projectionMatrix.m[1][0] + y * projectionMatrix.m[1][1] + z * projectionMatrix.m[1][2] + w * projectionMatrix.m[1][3];
        float clipZ = x * projectionMatrix.m[2][0] + y * projectionMatrix.m[2][1] + z * projectionMatrix.m[2][2] + w * projectionMatrix.m[2][3];
        float clipW = x * projectionMatrix.m[3][0] + y * projectionMatrix.m[3][1] + z * projectionMatrix.m[3][2] + w * projectionMatrix.m[3][3];
        
        // Perspective division
        if (clipW <= 0.0f) return false;
        
        float ndcX = clipX / clipW;
        float ndcY = clipY / clipW;
        
        // Convert to screen coordinates
        screenPos.x = (ndcX + 1.0f) * screenWidth * 0.5f;
        screenPos.y = (1.0f - ndcY) * screenHeight * 0.5f;
        
        // Check if within screen bounds
        return screenPos.x >= 0 && screenPos.x <= screenWidth && 
               screenPos.y >= 0 && screenPos.y <= screenHeight;
    }

    ESP::BoundingBox ESP::calculateBoundingBox(const Vector3& position, float height, float width) {
        BoundingBox box;
        
        // Calculate 8 corners of the 3D bounding box
        Vector3 corners[8] = {
            {position.x - width/2, position.y - height, position.z - width/2},
            {position.x + width/2, position.y - height, position.z - width/2},
            {position.x + width/2, position.y, position.z - width/2},
            {position.x - width/2, position.y, position.z - width/2},
            {position.x - width/2, position.y - height, position.z + width/2},
            {position.x + width/2, position.y - height, position.z + width/2},
            {position.x + width/2, position.y, position.z + width/2},
            {position.x - width/2, position.y, position.z + width/2}
        };
        
        // Convert to screen coordinates
        bool validCorners = false;
        for (int i = 0; i < 8; ++i) {
            if (worldToScreen(corners[i], box.corners[i])) {
                validCorners = true;
            }
        }
        
        if (!validCorners) return box;
        
        // Find min and max screen coordinates
        box.min = box.corners[0];
        box.max = box.corners[0];
        
        for (int i = 1; i < 8; ++i) {
            box.min.x = std::min(box.min.x, box.corners[i].x);
            box.min.y = std::min(box.min.y, box.corners[i].y);
            box.max.x = std::max(box.max.x, box.corners[i].x);
            box.max.y = std::max(box.max.y, box.corners[i].y);
        }
        
        return box;
    }

    void ESP::drawBoundingBox(const BoundingBox& box, ImU32 color) {
        if (!config.showBoundingBoxes) return;
        
        // Draw rectangle outline
        drawList->AddRect(box.min, box.max, color, 0.0f, 0, 2.0f);
        
        // Draw corner lines for better visibility
        float cornerLength = 10.0f;
        
        // Top-left corner
        drawList->AddLine(box.min, ImVec2(box.min.x + cornerLength, box.min.y), color, 2.0f);
        drawList->AddLine(box.min, ImVec2(box.min.x, box.min.y + cornerLength), color, 2.0f);
        
        // Top-right corner
        drawList->AddLine(ImVec2(box.max.x, box.min.y), ImVec2(box.max.x - cornerLength, box.min.y), color, 2.0f);
        drawList->AddLine(ImVec2(box.max.x, box.min.y), ImVec2(box.max.x, box.min.y + cornerLength), color, 2.0f);
        
        // Bottom-left corner
        drawList->AddLine(ImVec2(box.min.x, box.max.y), ImVec2(box.min.x + cornerLength, box.max.y), color, 2.0f);
        drawList->AddLine(ImVec2(box.min.x, box.max.y), ImVec2(box.min.x, box.max.y - cornerLength), color, 2.0f);
        
        // Bottom-right corner
        drawList->AddLine(box.max, ImVec2(box.max.x - cornerLength, box.max.y), color, 2.0f);
        drawList->AddLine(box.max, ImVec2(box.max.x, box.max.y - cornerLength), color, 2.0f);
    }

    void ESP::drawEntityInfo(const Entity& entity, const ImVec2& screenPos) {
        ImVec4 color;
        switch (entity.type) {
            case EntityType::Player:
                color = config.playerColor;
                break;
            case EntityType::Ore:
                color = config.oreColor;
                break;
            case EntityType::Loot:
                color = config.lootColor;
                break;
            default:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                break;
        }
        
        ImU32 textColor = IM_COL32(
            static_cast<int>(color.x * 255),
            static_cast<int>(color.y * 255),
            static_cast<int>(color.z * 255),
            static_cast<int>(color.w * 255)
        );
        
        float textY = screenPos.y;
        
        // Draw entity name
        if (config.showNames && !entity.name.empty()) {
            drawList->AddText(ImVec2(screenPos.x, textY), textColor, entity.name.c_str());
            textY += 15.0f;
        }
        
        // Draw distance
        if (config.showDistance) {
            std::string distanceStr = std::to_string(static_cast<int>(entity.distance)) + "m";
            drawList->AddText(ImVec2(screenPos.x, textY), textColor, distanceStr.c_str());
        }
    }

    std::vector<Entity> ESP::filterEntities(const std::vector<Entity>& entities) {
        std::vector<Entity> filtered;
        filtered.reserve(entities.size());
        
        for (const auto& entity : entities) {
            if (shouldRenderEntity(entity)) {
                filtered.push_back(entity);
            }
        }
        
        // Sort by distance for better rendering order
        std::sort(filtered.begin(), filtered.end(), 
                 [](const Entity& a, const Entity& b) { return a.distance < b.distance; });
        
        return filtered;
    }

    bool ESP::shouldRenderEntity(const Entity& entity) {
        // Distance filter
        if (entity.distance > config.maxDistance) return false;
        
        // Type filters
        switch (entity.type) {
            case EntityType::Player:
                return config.showPlayers;
            case EntityType::Ore:
                return config.showOre;
            case EntityType::Loot:
                return config.showLoot;
            default:
                return false;
        }
    }

    void ESP::batchRenderEntities(const std::vector<Entity>& entities) {
        for (const auto& entity : entities) {
            ImVec2 screenPos;
            if (!worldToScreen(entity.position, screenPos)) continue;
            
            // Calculate bounding box
            BoundingBox box = calculateBoundingBox(entity.position);
            
            // Determine color based on entity type
            ImU32 color;
            switch (entity.type) {
                case EntityType::Player:
                    color = IM_COL32(
                        static_cast<int>(config.playerColor.x * 255),
                        static_cast<int>(config.playerColor.y * 255),
                        static_cast<int>(config.playerColor.z * 255),
                        static_cast<int>(config.playerColor.w * 255)
                    );
                    break;
                case EntityType::Ore:
                    color = IM_COL32(
                        static_cast<int>(config.oreColor.x * 255),
                        static_cast<int>(config.oreColor.y * 255),
                        static_cast<int>(config.oreColor.z * 255),
                        static_cast<int>(config.oreColor.w * 255)
                    );
                    break;
                case EntityType::Loot:
                    color = IM_COL32(
                        static_cast<int>(config.lootColor.x * 255),
                        static_cast<int>(config.lootColor.y * 255),
                        static_cast<int>(config.lootColor.z * 255),
                        static_cast<int>(config.lootColor.w * 255)
                    );
                    break;
                default:
                    color = IM_COL32(255, 255, 255, 255);
                    break;
            }
            
            // Draw bounding box
            drawBoundingBox(box, color);
            
            // Draw entity info
            ImVec2 infoPos = ImVec2(box.min.x, box.min.y - 30.0f);
            drawEntityInfo(entity, infoPos);
        }
    }

    int ESP::getRenderedEntityCount() const {
        return static_cast<int>(cachedEntities.size());
    }

    float ESP::getAverageUpdateTime() const {
        return static_cast<float>(updateInterval);
    }
}

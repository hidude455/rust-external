#pragma once
#include "Common.h"
#include "MemoryManager.h"
#include "Renderer.h"
#include "../MenuPath/imgui/imgui.h"

namespace MIT {
        struct ESPConfig {
        // Master switch
        bool enabled = true;

        // Player ESP
        bool playerESPEnabled = true;
        bool playerBox2D = true;
        bool playerBox3D = false;
        bool playerSkeleton = true;
        bool playerHealthBar = true;
        bool playerName = true;
        bool playerDistance = true;
        bool playerWeapon = true;
        bool playerAmmo = true;
        bool playerSleeper = true;
        bool playerShowTeam = true;
        bool playerShowWounded = true;
        bool playerShowDead = true;
        bool playerTeamIndicator = true;

        // NPC ESP
        bool npcESPEnabled = true;
        bool npcName = true;
        bool npcBox = true;
        bool npcFill = false;
        bool npcDistance = true;
        bool npcChams = true;
        float npcRedGlow = 0.75f;

        // World ESP - Resources
        bool resourceSulfur = true;
        bool resourceMetal = true;
        bool resourceStone = true;
        float resourceDistance = 250.0f;

        // World ESP - Loot
        bool lootEliteCrates = true;
        bool lootMilitaryCrates = true;
        bool lootAirDrops = true;
        bool lootLockedCrates = true;
        bool lootStash = true;
        float lootDistance = 200.0f;

        // World ESP - Bases & Traps
        bool baseToolCupboard = true;
        bool baseAuthorizedPlayers = true;
        bool baseOutline = true;
        bool trapLandMines = true;
        bool trapBearTraps = true;
        bool trapAutoTurrets = true;
        bool trapFlameTurrets = true;
        bool trapShowRange = true;

        // Visual Overrides
        bool chamsEnabled = true;
        float chamsGlow = 0.65f;
        bool glowEnabled = true;
        float glowIntensity = 0.8f;
        bool outlinesEnabled = true;
        float outlineThickness = 1.5f;
        bool customReticle = true;
        int reticleType = 2;
        bool galaxyMode = false;

        // Colors
        ImVec4 playerColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        ImVec4 teamColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 woundedColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 deadColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        ImVec4 npcColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
        ImVec4 chamsColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
    };

    class ESP {
    private:
        MemoryManager* memoryManager;
        Renderer* renderer;
        ESPConfig config;
        
        // Performance optimization
        std::vector<Entity> cachedEntities;
        std::chrono::high_resolution_clock::time_point lastUpdate;
        const int updateInterval = 16; // ~60 FPS updates
        
        // World to screen conversion
        struct Matrix4x4 {
            float m[4][4];
        };
        
        Matrix4x4 viewMatrix;
        Matrix4x4 projectionMatrix;
        bool worldToScreen(const Vector3& worldPos, ImVec2& screenPos);
        void updateMatrices();
        
        // Bounding box calculations
        struct BoundingBox {
            ImVec2 min;
            ImVec2 max;
            ImVec2 corners[8];
        };
        
        BoundingBox calculateBoundingBox(const Vector3& position, float height = 1.8f, float width = 0.6f);
        void drawBoundingBox(const BoundingBox& box, ImU32 color);
        void drawCircleESP(const ImVec2& screenPos, float radius, ImU32 color);
        void drawEntityInfo(const Entity& entity, const ImVec2& screenPos);
        void drawInventoryESP(const Entity& entity, const ImVec2& screenPos);
        void applyWeaponChams(const Entity& entity);
        void applyGalaxyEffect(const Entity& entity);
        ImU32 getGalaxyColor(float time, float offset);
        
        // Filtering system
        std::vector<Entity> filterEntities(const std::vector<Entity>& entities);
        bool shouldRenderEntity(const Entity& entity);
        
        // High-performance rendering
        ImDrawList* drawList;
        void batchRenderEntities(const std::vector<Entity>& entities);

    public:
        ESP(MemoryManager* memoryManager, Renderer* renderer);
        ~ESP();
        
        void Initialize();
        void Update();
        void Render();
        
        void SetConfig(const ESPConfig& newConfig) { config = newConfig; }
        const ESPConfig& GetConfig() const { return config; }
        void SetGalaxyMode(bool enabled) { config.galaxyMode = enabled; }
        bool GetGalaxyMode() const { return config.galaxyMode; }
        
        // Utility functions
        void Toggle() { config.enabled = !config.enabled; }
        bool IsEnabled() const { return config.enabled; }
        
        // Performance monitoring
        int getRenderedEntityCount() const;
        float getAverageUpdateTime() const;
    };
}

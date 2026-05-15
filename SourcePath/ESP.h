#pragma once
#include "Common.h"
#include "MemoryManager.h"
#include "Renderer.h"
#include "../MenuPath/imgui/imgui.h"

namespace MIT {
    struct ESPConfig {
        bool enabled = true;
        bool showPlayers = true;
        bool showOre = true;
        bool showLoot = true;
        bool showBoundingBoxes = true;
        bool showNames = true;
        bool showDistance = true;
        float maxDistance = 500.0f;
        ImVec4 playerColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        ImVec4 oreColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 lootColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
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
        void drawEntityInfo(const Entity& entity, const ImVec2& screenPos);
        
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
        
        // Utility functions
        void Toggle() { config.enabled = !config.enabled; }
        bool IsEnabled() const { return config.enabled; }
        
        // Performance monitoring
        int getRenderedEntityCount() const;
        float getAverageUpdateTime() const;
    };
}

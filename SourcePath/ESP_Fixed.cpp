#include "ESP.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <thread>
#include <mutex>

namespace MIT {
    
    // Memory pool for EnhancedEntity objects
    class EnhancedEntityPool {
    private:
        std::vector<EnhancedEntity> m_pool;
        std::queue<EnhancedEntity*> m_available;
        std::mutex m_poolMutex;
        size_t m_poolSize;
        
    public:
        EnhancedEntityPool(size_t poolSize = 1000) : m_poolSize(poolSize) {
            m_pool.resize(poolSize);
            for (size_t i = 0; i < poolSize; i++) {
                m_available.push(&m_pool[i]);
            }
        }
        
        EnhancedEntity* Acquire() {
            std::lock_guard<std::mutex> lock(m_poolMutex);
            if (m_available.empty()) {
                // Pool exhausted, create temporary entity
                return new EnhancedEntity();
            }
            
            EnhancedEntity* entity = m_available.front();
            m_available.pop();
            
            // Reset entity to default state
            ResetEntity(entity);
            return entity;
        }
        
        void Release(EnhancedEntity* entity) {
            if (!entity) return;
            
            std::lock_guard<std::mutex> lock(m_poolMutex);
            
            // Check if this entity is from our pool
            if (entity >= &m_pool[0] && entity <= &m_pool[m_poolSize - 1]) {
                m_available.push(entity);
            } else {
                // This was a temporary entity, delete it
                delete entity;
            }
        }
        
        void Cleanup() {
            std::lock_guard<std::mutex> lock(m_poolMutex);
            while (!m_available.empty()) {
                m_available.pop();
            }
            m_pool.clear();
        }
        
    private:
        void ResetEntity(EnhancedEntity* entity) {
            if (!entity) return;
            
            entity->baseEntity = Entity();
            entity->healthPercentage = 0.0f;
            entity->armorPercentage = 0.0f;
            entity->weaponName.clear();
            entity->ammoCount = 0;
            entity->lastSeenTime = 0;
            entity->isVisible = false;
            entity->isFriendly = false;
            entity->isNPC = false;
            entity->isAnimal = false;
            
            entity->screenPosition = ImVec2(0, 0);
            entity->headScreenPosition = ImVec2(0, 0);
            entity->chestScreenPosition = ImVec2(0, 0);
            entity->pelvisScreenPosition = ImVec2(0, 0);
            entity->boundingBox = BoundingBox();
            
            entity->bonePositions.clear();
            entity->healthBarWidth = 0.0f;
            entity->healthBarHeight = 0.0f;
            entity->healthBarPosition = ImVec2(0, 0);
            
            entity->tracerPoints.clear();
            entity->tracerAlpha = 255;
        }
    };
    
    // Thread-safe matrix cache
    class MatrixCache {
    private:
        Matrix4x4 m_viewMatrix;
        Matrix4x4 m_projectionMatrix;
        uint64_t m_cacheTime;
        std::mutex m_cacheMutex;
        static const uint64_t CACHE_DURATION = 16; // 16ms cache
        
    public:
        MatrixCache() : m_cacheTime(0) {}
        
        void UpdateMatrices(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            m_viewMatrix = viewMatrix;
            m_projectionMatrix = projectionMatrix;
            m_cacheTime = GetTickCount64();
        }
        
        bool GetMatrices(Matrix4x4& viewMatrix, Matrix4x4& projectionMatrix) {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            
            uint64_t currentTime = GetTickCount64();
            if (currentTime - m_cacheTime > CACHE_DURATION) {
                return false; // Cache expired
            }
            
            viewMatrix = m_viewMatrix;
            projectionMatrix = m_projectionMatrix;
            return true;
        }
        
        bool IsCacheValid() {
            uint64_t currentTime = GetTickCount64();
            return (currentTime - m_cacheTime <= CACHE_DURATION);
        }
    };
    
    // Spatial grid for entity culling
    class SpatialGrid {
    private:
        struct GridCell {
            std::vector<EnhancedEntity*> entities;
        };
        
        std::vector<GridCell> m_grid;
        int m_gridSize;
        int m_cellSize;
        float m_maxDistance;
        std::mutex m_gridMutex;
        
    public:
        SpatialGrid(int gridSize = 100, int cellSize = 50, float maxDistance = 500.0f) 
            : m_gridSize(gridSize), m_cellSize(cellSize), m_maxDistance(maxDistance) {
            m_grid.resize(gridSize * gridSize);
        }
        
        void UpdateGrid(const std::vector<EnhancedEntity*>& entities) {
            std::lock_guard<std::mutex> lock(m_gridMutex);
            
            // Clear grid
            for (auto& cell : m_grid) {
                cell.entities.clear();
            }
            
            // Add entities to grid
            for (auto* entity : entities) {
                if (!entity) continue;
                
                int gridX = static_cast<int>(entity->baseEntity.position.x / m_cellSize);
                int gridZ = static_cast<int>(entity->baseEntity.position.z / m_cellSize);
                
                // Clamp to grid bounds
                gridX = std::max(0, std::min(m_gridSize - 1, gridX));
                gridZ = std::max(0, std::min(m_gridSize - 1, gridZ));
                
                int index = gridZ * m_gridSize + gridX;
                m_grid[index].entities.push_back(entity);
            }
        }
        
        std::vector<EnhancedEntity*> GetNearbyEntities(const Vector3& position, float radius) {
            std::lock_guard<std::mutex> lock(m_gridMutex);
            
            std::vector<EnhancedEntity*> nearby;
            nearby.reserve(100); // Pre-allocate
            
            int gridX = static_cast<int>(position.x / m_cellSize);
            int gridZ = static_cast<int>(position.z / m_cellSize);
            
            // Check surrounding cells
            int cellRadius = static_cast<int>(radius / m_cellSize) + 1;
            
            for (int dx = -cellRadius; dx <= cellRadius; dx++) {
                for (int dz = -cellRadius; dz <= cellRadius; dz++) {
                    int checkX = gridX + dx;
                    int checkZ = gridZ + dz;
                    
                    if (checkX >= 0 && checkX < m_gridSize && 
                        checkZ >= 0 && checkZ < m_gridSize) {
                        
                        int index = checkZ * m_gridSize + checkX;
                        
                        for (auto* entity : m_grid[index].entities) {
                            if (!entity) continue;
                            
                            float distance = CalculateDistance(position, entity->baseEntity.position);
                            if (distance <= radius) {
                                nearby.push_back(entity);
                            }
                        }
                    }
                }
            }
            
            return nearby;
        }
        
    private:
        float CalculateDistance(const Vector3& a, const Vector3& b) {
            float dx = a.x - b.x;
            float dy = a.y - b.y;
            float dz = a.z - b.z;
            return sqrt(dx * dx + dy * dy + dz * dz);
        }
    };
    
    // Fixed ESP implementation with memory management
    class ESPFixed {
    private:
        MemoryManager* memoryManager;
        Renderer* renderer;
        ESPConfig config;
        
        // Memory management
        std::unique_ptr<EnhancedEntityPool> m_entityPool;
        std::vector<EnhancedEntity*> m_activeEntities;
        std::mutex m_entitiesMutex;
        
        // Performance optimizations
        std::unique_ptr<MatrixCache> m_matrixCache;
        std::unique_ptr<SpatialGrid> m_spatialGrid;
        
        // Timing
        std::chrono::high_resolution_clock::time_point lastUpdate;
        const int updateInterval = 16; // ~60 FPS
        
        // Thread safety
        std::mutex m_renderMutex;
        std::atomic<bool> m_isUpdating{false};
        
        // ImGui draw list
        ImDrawList* drawList;
        
    public:
        ESPFixed(MemoryManager* memoryManager, Renderer* renderer) 
            : memoryManager(memoryManager), renderer(renderer) {
            
            // Initialize memory management
            m_entityPool = std::make_unique<EnhancedEntityPool>(500);
            m_matrixCache = std::make_unique<MatrixCache>();
            m_spatialGrid = std::make_unique<SpatialGrid>(100, 50, config.maxRenderDistance);
            
            lastUpdate = std::chrono::high_resolution_clock::now();
            drawList = nullptr;
        }
        
        ~ESPFixed() {
            Cleanup();
        }
        
        void Cleanup() {
            // Release all active entities
            std::lock_guard<std::mutex> lock(m_entitiesMutex);
            for (auto* entity : m_activeEntities) {
                if (entity) {
                    m_entityPool->Release(entity);
                }
            }
            m_activeEntities.clear();
            
            // Cleanup pools
            if (m_entityPool) {
                m_entityPool->Cleanup();
            }
            
            // Clear draw list reference
            drawList = nullptr;
        }
        
        void Initialize() {
            LOG_INFO("ESP system initialized with memory management");
            
            // Initialize configuration
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
            
            // Load configuration
            LoadConfiguration();
        }
        
        void Update() {
            if (!config.enabled || !memoryManager || !memoryManager->IsProcessValid()) return;
            
            // Prevent concurrent updates
            if (m_isUpdating.exchange(true)) return;
            
            try {
                auto currentTime = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdate).count();
                
                if (elapsed >= updateInterval) {
                    // Update matrices if needed
                    UpdateMatricesIfNeeded();
                    
                    // Get entities from memory
                    std::vector<Entity> allEntities = memoryManager->GetEntities();
                    
                    // Apply distance culling
                    std::vector<Entity> culledEntities = ApplyDistanceCulling(allEntities);
                    
                    // Update entity data with memory management
                    UpdateEntityDataOptimized(culledEntities);
                    
                    // Update spatial grid
                    UpdateSpatialGrid();
                    
                    lastUpdate = currentTime;
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("ESP Update error: " + std::string(e.what()));
            }
            
            m_isUpdating = false;
        }
        
        void Render() {
            if (!config.enabled) return;
            
            std::lock_guard<std::mutex> lock(m_renderMutex);
            
            drawList = ImGui::GetBackgroundDrawList();
            if (!drawList) return;
            
            try {
                // Get nearby entities for rendering
                Vector3 playerPos = GetLocalPlayerPosition();
                auto nearbyEntities = m_spatialGrid->GetNearbyEntities(playerPos, config.maxRenderDistance);
                
                // Render entities
                for (auto* entity : nearbyEntities) {
                    if (entity && IsEntityVisible(*entity)) {
                        RenderEnhancedEntity(*entity);
                    }
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("ESP Render error: " + std::string(e.what()));
            }
        }
        
    private:
        void UpdateMatricesIfNeeded() {
            Matrix4x4 viewMatrix, projectionMatrix;
            
            if (!m_matrixCache->GetMatrices(viewMatrix, projectionMatrix)) {
                // Update matrices from game
                if (memoryManager) {
                    viewMatrix = memoryManager->GetViewMatrix();
                    projectionMatrix = memoryManager->GetProjectionMatrix();
                    m_matrixCache->UpdateMatrices(viewMatrix, projectionMatrix);
                }
            }
        }
        
        std::vector<Entity> ApplyDistanceCulling(const std::vector<Entity>& entities) {
            std::vector<Entity> culled;
            culled.reserve(entities.size());
            
            Vector3 playerPos = GetLocalPlayerPosition();
            
            for (const auto& entity : entities) {
                float distance = CalculateDistance(playerPos, entity.position);
                if (distance <= config.maxRenderDistance) {
                    culled.push_back(entity);
                }
            }
            
            return culled;
        }
        
        void UpdateEntityDataOptimized(const std::vector<Entity>& entities) {
            std::lock_guard<std::mutex> lock(m_entitiesMutex);
            
            // Release previous entities
            for (auto* entity : m_activeEntities) {
                if (entity) {
                    m_entityPool->Release(entity);
                }
            }
            m_activeEntities.clear();
            
            // Reserve space
            m_activeEntities.reserve(entities.size());
            
            // Create enhanced entities
            for (const auto& entity : entities) {
                EnhancedEntity* enhanced = m_entityPool->Acquire();
                if (!enhanced) continue;
                
                // Copy base entity data
                enhanced->baseEntity = entity;
                
                // Calculate extended information
                enhanced->healthPercentage = (entity.maxHealth > 0) ? 
                    (entity.health / entity.maxHealth * 100.0f) : 0.0f;
                enhanced->armorPercentage = (entity.maxArmor > 0) ? 
                    (entity.armor / entity.maxArmor * 100.0f) : 0.0f;
                enhanced->weaponName = entity.weaponName;
                enhanced->ammoCount = entity.ammo;
                enhanced->lastSeenTime = GetTickCount64();
                enhanced->isVisible = IsEntityVisible(entity);
                enhanced->isFriendly = (entity.team == 0);
                enhanced->isNPC = entity.isNPC;
                enhanced->isAnimal = entity.isAnimal;
                
                // Calculate screen positions (using cached matrices)
                CalculateScreenPositions(*enhanced);
                
                // Calculate bone positions only if needed
                if (config.showBoneESP) {
                    CalculateBonePositionsOptimized(*enhanced);
                }
                
                // Calculate bounding box
                enhanced->boundingBox = CalculateBoundingBox(entity.position, entity.height, entity.width);
                
                // Calculate health bar data
                if (config.showHealthBars) {
                    CalculateHealthBarData(*enhanced);
                }
                
                // Calculate tracer points
                if (config.showTracers) {
                    CalculateTracerPoints(*enhanced);
                }
                
                m_activeEntities.push_back(enhanced);
            }
        }
        
        void UpdateSpatialGrid() {
            m_spatialGrid->UpdateGrid(m_activeEntities);
        }
        
        void CalculateScreenPositions(EnhancedEntity& enhanced) {
            Matrix4x4 viewMatrix, projectionMatrix;
            if (m_matrixCache->GetMatrices(viewMatrix, projectionMatrix)) {
                // Use cached matrices for faster calculation
                worldToScreenCached(enhanced.baseEntity.position, enhanced.screenPosition, viewMatrix, projectionMatrix);
            } else {
                // Fallback to direct calculation
                worldToScreen(enhanced.baseEntity.position, enhanced.screenPosition);
            }
        }
        
        void CalculateBonePositionsOptimized(EnhancedEntity& enhanced) {
            if (!config.showBoneESP) return;
            
            // Only calculate visible bones
            std::vector<Bone> visibleBones = GetVisibleBones();
            
            Matrix4x4 viewMatrix, projectionMatrix;
            bool hasCachedMatrices = m_matrixCache->GetMatrices(viewMatrix, projectionMatrix);
            
            for (Bone bone : visibleBones) {
                Vector3 bonePos = GetBonePosition(enhanced.baseEntity, bone);
                ImVec2 screenPos;
                
                if (hasCachedMatrices) {
                    worldToScreenCached(bonePos, screenPos, viewMatrix, projectionMatrix);
                } else {
                    worldToScreen(bonePos, screenPos);
                }
                
                enhanced.bonePositions[bone] = screenPos;
            }
        }
        
        std::vector<Bone> GetVisibleBones() {
            std::vector<Bone> visible;
            
            if (config.showHeadBone) visible.push_back(Bone::Head);
            if (config.showNeckBone) visible.push_back(Bone::Neck);
            if (config.showChestBone) visible.push_back(Bone::Chest);
            if (config.showPelvisBone) visible.push_back(Bone::Pelvis);
            if (config.showSpineBones) {
                visible.push_back(Bone::Neck);
                visible.push_back(Bone::Chest);
                visible.push_back(Bone::Pelvis);
            }
            if (config.showArmBones) {
                visible.push_back(Bone::LeftShoulder);
                visible.push_back(Bone::RightShoulder);
                visible.push_back(Bone::LeftElbow);
                visible.push_back(Bone::RightElbow);
                visible.push_back(Bone::LeftHand);
                visible.push_back(Bone::RightHand);
            }
            if (config.showLegBones) {
                visible.push_back(Bone::LeftHip);
                visible.push_back(Bone::RightHip);
                visible.push_back(Bone::LeftKnee);
                visible.push_back(Bone::RightKnee);
                visible.push_back(Bone::LeftFoot);
                visible.push_back(Bone::RightFoot);
            }
            
            return visible;
        }
        
        bool IsEntityVisible(const EnhancedEntity& entity) {
            if (!config.fadeWithDistance) return true;
            
            Vector3 playerPos = GetLocalPlayerPosition();
            float distance = CalculateDistance(playerPos, entity.baseEntity.position);
            
            return distance <= config.fadeEndDistance;
        }
        
        void RenderEnhancedEntity(const EnhancedEntity& entity) {
            if (!drawList) return;
            
            // Apply distance fading
            float alpha = 1.0f;
            if (config.fadeWithDistance) {
                Vector3 playerPos = GetLocalPlayerPosition();
                float distance = CalculateDistance(playerPos, entity.baseEntity.position);
                
                if (distance >= config.fadeStartDistance) {
                    float fadeRange = config.fadeEndDistance - config.fadeStartDistance;
                    float fadeProgress = (distance - config.fadeStartDistance) / fadeRange;
                    alpha = std::max(0.0f, 1.0f - fadeProgress);
                }
            }
            
            // Get entity color
            ImVec4 entityColor = GetEntityColor(entity);
            entityColor.w *= alpha; // Apply alpha
            
            // Render based on configuration
            if (config.showBoundingBoxes) {
                RenderBoundingBox(entity.boundingBox, entityColor);
            }
            
            if (config.showHealthBars) {
                RenderHealthBar(entity, alpha);
            }
            
            if (config.showBoneESP && !entity.bonePositions.empty()) {
                RenderBones(entity, entityColor);
            }
            
            if (config.showNames) {
                RenderEntityName(entity, entityColor);
            }
            
            if (config.showDistance) {
                RenderDistance(entity, entityColor);
            }
            
            if (config.showWeapon && !entity.weaponName.empty()) {
                RenderWeapon(entity, entityColor);
            }
            
            if (config.showTracers && !entity.tracerPoints.empty()) {
                RenderTracers(entity, entityColor, alpha);
            }
        }
        
        ImVec4 GetEntityColor(const EnhancedEntity& entity) {
            if (entity.isFriendly) {
                return config.friendlyColor;
            } else if (entity.isNPC) {
                return config.neutralColor;
            } else if (entity.isAnimal) {
                return config.neutralColor;
            } else {
                return config.enemyColor;
            }
        }
        
        void RenderBoundingBox(const BoundingBox& box, const ImVec4& color) {
            if (!drawList) return;
            
            ImU32 col = ImGui::ColorConvertFloat4ToU32(color);
            
            // Draw rectangle
            drawList->AddRect(ImVec2(box.min.x, box.min.y), 
                             ImVec2(box.max.x, box.max.y), 
                             col, 0.0f, 0, 1.5f);
        }
        
        void RenderHealthBar(const EnhancedEntity& entity, float alpha) {
            if (!drawList || entity.healthBarWidth <= 0) return;
            
            ImVec4 bgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.5f * alpha);
            ImVec4 healthColor = config.healthBarColor;
            healthColor.w *= alpha;
            
            ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(bgColor);
            ImU32 healthCol = ImGui::ColorConvertFloat4ToU32(healthColor);
            
            // Background
            ImVec2 bgMin = ImVec2(entity.healthBarPosition.x, entity.healthBarPosition.y);
            ImVec2 bgMax = ImVec2(entity.healthBarPosition.x + entity.healthBarWidth, 
                                 entity.healthBarPosition.y + entity.healthBarHeight);
            drawList->AddRectFilled(bgMin, bgMax, bgCol, 0.0f);
            
            // Health fill
            float healthWidth = entity.healthBarWidth * (entity.healthPercentage / 100.0f);
            ImVec2 healthMax = ImVec2(entity.healthBarPosition.x + healthWidth, 
                                     entity.healthBarPosition.y + entity.healthBarHeight);
            drawList->AddRectFilled(bgMin, healthMax, healthCol, 0.0f);
            
            // Border
            drawList->AddRect(bgMin, bgMax, healthCol, 0.0f, 0, 1.0f);
        }
        
        void RenderBones(const EnhancedEntity& entity, const ImVec4& color) {
            if (!drawList) return;
            
            ImU32 col = ImGui::ColorConvertFloat4ToU32(color);
            
            // Draw bone connections
            DrawBoneConnection(entity, Bone::Head, Bone::Neck, col);
            DrawBoneConnection(entity, Bone::Neck, Bone::Chest, col);
            DrawBoneConnection(entity, Bone::Chest, Bone::Pelvis, col);
            DrawBoneConnection(entity, Bone::Chest, Bone::LeftShoulder, col);
            DrawBoneConnection(entity, Bone::Chest, Bone::RightShoulder, col);
            DrawBoneConnection(entity, Bone::LeftShoulder, Bone::LeftElbow, col);
            DrawBoneConnection(entity, Bone::LeftElbow, Bone::LeftHand, col);
            DrawBoneConnection(entity, Bone::RightShoulder, Bone::RightElbow, col);
            DrawBoneConnection(entity, Bone::RightElbow, Bone::RightHand, col);
            DrawBoneConnection(entity, Bone::Pelvis, Bone::LeftHip, col);
            DrawBoneConnection(entity, Bone::LeftHip, Bone::LeftKnee, col);
            DrawBoneConnection(entity, Bone::LeftKnee, Bone::LeftFoot, col);
            DrawBoneConnection(entity, Bone::Pelvis, Bone::RightHip, col);
            DrawBoneConnection(entity, Bone::RightHip, Bone::RightKnee, col);
            DrawBoneConnection(entity, Bone::RightKnee, Bone::RightFoot, col);
        }
        
        void DrawBoneConnection(const EnhancedEntity& entity, Bone bone1, Bone bone2, ImU32 color) {
            auto it1 = entity.bonePositions.find(bone1);
            auto it2 = entity.bonePositions.find(bone2);
            
            if (it1 != entity.bonePositions.end() && it2 != entity.bonePositions.end()) {
                drawList->AddLine(it1->second, it2->second, color, 1.0f);
            }
        }
        
        void RenderEntityName(const EnhancedEntity& entity, const ImVec4& color) {
            if (!drawList) return;
            
            ImU32 col = ImGui::ColorConvertFloat4ToU32(color);
            
            // Simple name rendering (would need font handling in real implementation)
            ImVec2 namePos = ImVec2(entity.screenPosition.x, entity.screenPosition.y - 20);
            drawList->AddText(namePos, col, "Player"); // Simplified
        }
        
        void RenderDistance(const EnhancedEntity& entity, const ImVec4& color) {
            if (!drawList) return;
            
            Vector3 playerPos = GetLocalPlayerPosition();
            float distance = CalculateDistance(playerPos, entity.baseEntity.position);
            
            ImU32 col = ImGui::ColorConvertFloat4ToU32(color);
            
            std::string distanceStr = std::to_string(static_cast<int>(distance)) + "m";
            ImVec2 distPos = ImVec2(entity.screenPosition.x, entity.screenPosition.y - 5);
            drawList->AddText(distPos, col, distanceStr.c_str());
        }
        
        void RenderWeapon(const EnhancedEntity& entity, const ImVec4& color) {
            if (!drawList || entity.weaponName.empty()) return;
            
            ImU32 col = ImGui::ColorConvertFloat4ToU32(color);
            
            ImVec2 weaponPos = ImVec2(entity.screenPosition.x, entity.screenPosition.y + 15);
            drawList->AddText(weaponPos, col, entity.weaponName.c_str());
        }
        
        void RenderTracers(const EnhancedEntity& entity, const ImVec4& color, float alpha) {
            if (!drawList || entity.tracerPoints.empty()) return;
            
            ImVec4 tracerColor = config.tracerColor;
            tracerColor.w *= alpha;
            ImU32 col = ImGui::ColorConvertFloat4ToU32(tracerColor);
            
            for (size_t i = 1; i < entity.tracerPoints.size(); i++) {
                drawList->AddLine(entity.tracerPoints[i-1], entity.tracerPoints[i], col, config.tracerWidth);
            }
        }
        
        // Utility methods
        Vector3 GetLocalPlayerPosition() {
            if (!memoryManager) return Vector3(0, 0, 0);
            return memoryManager->GetLocalPlayerPosition();
        }
        
        float CalculateDistance(const Vector3& a, const Vector3& b) {
            float dx = a.x - b.x;
            float dy = a.y - b.y;
            float dz = a.z - b.z;
            return sqrt(dx * dx + dy * dy + dz * dz);
        }
        
        void worldToScreen(const Vector3& worldPos, ImVec2& screenPos) {
            // Simplified world-to-screen conversion
            // In real implementation, this would use proper matrix transformations
            screenPos.x = worldPos.x;
            screenPos.y = worldPos.y;
        }
        
        void worldToScreenCached(const Vector3& worldPos, ImVec2& screenPos, 
                                const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
            // Optimized world-to-screen using cached matrices
            // In real implementation, this would use the cached matrices
            screenPos.x = worldPos.x;
            screenPos.y = worldPos.y;
        }
        
        bool IsEntityVisible(const Entity& entity) {
            // Simplified visibility check
            return true;
        }
        
        Vector3 GetBonePosition(const Entity& entity, Bone bone) {
            // Simplified bone position calculation
            // In real implementation, this would read from game memory
            Vector3 pos = entity.position;
            
            switch (bone) {
                case Bone::Head:
                    pos.y += entity.height * 0.9f;
                    break;
                case Bone::Neck:
                    pos.y += entity.height * 0.8f;
                    break;
                case Bone::Chest:
                    pos.y += entity.height * 0.6f;
                    break;
                case Bone::Pelvis:
                    pos.y += entity.height * 0.4f;
                    break;
                default:
                    break;
            }
            
            return pos;
        }
        
        BoundingBox CalculateBoundingBox(const Vector3& position, float height, float width) {
            BoundingBox box;
            // Simplified bounding box calculation
            box.min = ImVec2(position.x - width/2, position.y - height);
            box.max = ImVec2(position.x + width/2, position.y);
            return box;
        }
        
        void CalculateHealthBarData(EnhancedEntity& entity) {
            if (!config.showHealthBars) return;
            
            entity.healthBarWidth = config.healthBarWidth;
            entity.healthBarHeight = config.healthBarHeight;
            entity.healthBarPosition = ImVec2(entity.screenPosition.x - config.healthBarWidth/2, 
                                            entity.screenPosition.y - config.healthBarOffset);
        }
        
        void CalculateTracerPoints(EnhancedEntity& entity) {
            if (!config.showTracers) return;
            
            entity.tracerPoints.clear();
            
            // Add tracer points from player to entity
            Vector3 playerPos = GetLocalPlayerPosition();
            ImVec2 playerScreen;
            worldToScreen(playerPos, playerScreen);
            
            entity.tracerPoints.push_back(playerScreen);
            entity.tracerPoints.push_back(entity.headScreenPosition);
            
            entity.tracerAlpha = 255;
        }
        
        void LoadConfiguration() {
            // Configuration loading implementation
        }
    };
    
} // namespace MIT

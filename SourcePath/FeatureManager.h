#pragma once
#include "GameMemory.h"
#include <d3d11.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <random>

namespace Features {

    enum class AimbotMode { Off = 0, Legit = 1, Rage = 2, Silent = 3, Memory = 4 };
    enum class ESPBoxStyle { Box2D = 0, Box3D = 1, Corner = 2, Filled = 3, Rounded = 4 };
    enum class TargetBone { Head = 0, Neck = 1, Chest = 2, Pelvis = 3, LeftFoot = 4, RightFoot = 5, Random = 6 };
    enum class KeybindMode { Toggle = 0, Hold = 1, Always = 2 };
    enum class CrosshairStyle { Cross = 0, Dot = 1, Circle = 2, TShape = 3, Chevron = 4, Custom = 5 };

    struct Keybind {
        int key = 0;
        KeybindMode mode = KeybindMode::Toggle;
        bool enabled = false;
    };

    struct AimbotConfig {
        bool enabled = false;
        AimbotMode mode = AimbotMode::Legit;
        TargetBone targetBone = TargetBone::Head;
        float fov = 15.0f;
        float smoothness = 0.3f;
        float aimSpeed = 2.0f;
        bool visibilityCheck = true;
        bool targetPlayers = true;
        bool targetNPCs = false;
        bool targetAnimals = false;
        bool ignoreSleepers = true;
        bool ignoreTeammates = true;
        bool ignoreWounded = true;
        bool autoFire = false;
        bool autoScope = false;
        bool autoWall = false;
        float maxDistance = 500.0f;
        float predictionTime = 0.08f;
        bool humanizeAim = true;
        float reactionTime = 180.0f;
        float aimJitter = 2.0f;
        bool categorizeAim = true;
        bool silentAim = false;
        float hitchance = 65.0f;
        bool memoryAim = false;
        bool fovCircle = true;
        uint32_t fovCircleColor = 0x40FFFFFF;
        bool targetLine = true;
        uint32_t targetLineColor = 0xFFFF3030;
        bool showPrediction = true;
        uint32_t predictionColor = 0x80FFFF00;
        bool bulletTracers = false;
        uint32_t tracerColor = 0xFFFFFF00;
        float tracerDuration = 1.0f;
        bool highlightTarget = true;
        uint32_t highlightColor = 0x40FF0000;
        Keybind aimKey;
        Keybind autoFireKey;
    };

    struct PlayerVisualsConfig {
        bool enabled = true;
        bool showPlayers = true;
        bool showNPCs = true;
        ESPBoxStyle boxStyle = ESPBoxStyle::Corner;
        uint32_t boxColor = 0xFFFFFFFF;
        float boxThickness = 1.5f;
        bool cornerBox = true;
        float cornerLength = 0.25f;
        bool skeletons = false;
        uint32_t skeletonColor = 0xFFFFFFFF;
        float skeletonThickness = 1.0f;
        bool fullBodySkeleton = true;
        bool viewDirectionArrow = false;
        uint32_t arrowColor = 0xFFFFFFFF;
        float arrowSize = 15.0f;
        bool offscreenArrows = false;
        uint32_t offscreenColor = 0xFFFFFFFF;
        float offscreenSize = 20.0f;
        float offscreenDistance = 40.0f;
        bool insideBuilding = true;
        uint32_t insideBuildingColor = 0xFF00FFFF;
        bool username = true;
        uint32_t usernameColor = 0xFFFFFFFF;
        bool steamAvatar = false;
        float avatarSize = 24.0f;
        bool chams = false;
        uint32_t chamColor = 0x40FF0000;
        bool playerGlow = false;
        uint32_t glowColor = 0x40FF3030;
        bool healthbars = true;
        bool colorCodedHealth = true;
        float healthbarWidth = 4.0f;
        bool distance = true;
        uint32_t distanceColor = 0xFFFFFFFF;
        bool heldItem = true;
        uint32_t heldItemColor = 0xFFFFAA30;
        bool showAttachments = true;
        bool teamID = true;
        bool colorizeTeams = true;
        uint32_t teammateColor = 0xFF30A0FF;
        uint32_t enemyColor = 0xFFFF3030;
        uint32_t neutralColor = 0xFFFFFF30;
        bool hotbar = false;
        bool visibilityCheck = true;
        float maxDistance = 500.0f;
        bool fadeWithDistance = true;
        bool copySteamID = true;
        bool showSnaplines = false;
        uint32_t snaplineColor = 0x80FFFFFF;
        bool showHeadDot = true;
        uint32_t headDotColor = 0xFFFF0000;
        float headDotSize = 4.0f;
    };

    struct WorldExploitsConfig {
        bool fovChanger = false;
        float fovValue = 90.0f;
        bool brightNight = false;
        float nightBrightness = 1.0f;
        bool brightCave = false;
        float caveBrightness = 1.0f;
        bool brightStars = false;
        bool removeSun = false;
        bool removeMoon = false;
        bool removeLayers = false;
        bool zoom = false;
        float zoomValue = 1.5f;
        float zoomSpeed = 0.1f;
        bool aspectRatio = false;
        float aspectRatioValue = 1.777f;
        bool removeScreenShake = false;
        bool removeFlash = false;
        bool instantRevive = false;
        bool removeWearRestrictions = false;
        bool fastLoot = false;
        bool flyhack = false;
        float flySpeed = 10.0f;
        bool ignorePlayerCollision = false;
        bool infiniteJump = false;
        bool debugCamera = false;
        float debugCamSpeed = 5.0f;
        bool spectate = false;
        bool thirdPerson = false;
        float thirdPersonDistance = 3.0f;
        bool unlockAngles = false;
        bool showBuildings = false;
        bool alwaysDay = false;
        bool noFallDamage = false;
        Keybind flyhackKey;
        Keybind debugCamKey;
        Keybind zoomKey;
    };

    struct WeaponModsConfig {
        bool recoilModifier = false;
        float recoilX = 0.0f;
        float recoilY = 0.0f;
        bool weaponSpread = false;
        float spreadValue = 0.0f;
        bool projectileSpread = false;
        float projectileSpreadValue = 0.0f;
        bool weaponSway = false;
        float swayValue = 0.0f;
        bool rapidFire = false;
        float rapidFireMultiplier = 2.0f;
        bool thickBullet = false;
        float thickBulletValue = 2.0f;
        bool bigBullets = false;
        float bigBulletSize = 2.0f;
        bool hitMaterialOverride = false;
        int hitMaterial = 0;
        bool hitboxOverride = false;
        float hitboxMultiplier = 2.0f;
        bool smartPatrolOverride = false;
        bool ammoIndicator = true;
        uint32_t ammoColor = 0xFFFFFFFF;
        bool reloadIndicator = true;
        uint32_t reloadColor = 0xFFFFAA00;
    };

    struct WorldVisualsConfig {
        bool enabled = true;
        struct ItemVisual {
            bool enabled = true;
            uint32_t color = 0xFFFFFFFF;
            bool chams = false;
            uint32_t chamColor = 0x40FFFFFF;
            bool glow = false;
            uint32_t glowColor = 0x40FFFFFF;
            bool box = true;
            float maxDistance = 300.0f;
        };
        ItemVisual collectables;
        ItemVisual multiTierCrates;
        ItemVisual vehicles;
        ItemVisual deployables;
        ItemVisual allOres;
        ItemVisual allAnimals;
        ItemVisual stashes;
        ItemVisual corpses;
        ItemVisual backpacks;
        ItemVisual supplyDrops;
        ItemVisual hackableCrates;
        ItemVisual patrolHelicopter;
        ItemVisual bradleyAPC;
        ItemVisual cargoShip;
        ItemVisual lockedCrates;
        ItemVisual oilRig;
        ItemVisual toolCupboard;
        bool blurEffect = false;
        float blurStrength = 2.0f;
        bool boxESP = true;
        float globalMaxDistance = 500.0f;
    };

    struct MovementConfig {
        bool spiderman = false;
        float spiderManSpeed = 5.0f;
        bool forceSprint = false;
        bool omnisprint = false;
        bool noFallDamage = false;
        bool noMovementSlowdown = false;
        bool highWalk = false;
        float walkHeight = 2.0f;
        bool highJump = false;
        float jumpHeight = 5.0f;
        bool walkOnWater = false;
        bool spinbot = false;
        float spinSpeed = 10.0f;
        bool shootInAir = false;
        bool instantSuicide = false;
        Keybind spidermanKey;
        Keybind spinbotKey;
    };

    struct UIConfig {
        bool cloudConfigSync = false;
        std::string cloudUsername;
        std::string cloudPassword;
        bool localConfigProfiles = true;
        std::string currentProfile = "default";
        std::vector<std::string> profiles;
        bool importExportConfigs = true;
        bool customCrosshairs = false;
        CrosshairStyle crosshairStyle = CrosshairStyle::Cross;
        uint32_t crosshairColor = 0xFF00FF00;
        float crosshairSize = 10.0f;
        float crosshairThickness = 1.5f;
        float crosshairGap = 4.0f;
        bool crosshairOutline = true;
        bool fullKeybindConfig = true;
        float uiScale = 1.0f;
        int themeIndex = 0;
        uint32_t accentColor = 0xFF0078FF;
        uint32_t bgColor = 0xFF14141E;
        uint32_t textColor = 0xFFFFFFFF;
        bool showWatermark = true;
        bool showFPSCounter = true;
    };

    class CFeatureManager {
    private:
        Memory::CGameMemory* m_memory;
        AimbotConfig m_aimbotCfg;
        PlayerVisualsConfig m_visualsCfg;
        WorldExploitsConfig m_exploitsCfg;
        WeaponModsConfig m_weaponCfg;
        WorldVisualsConfig m_worldVisualsCfg;
        MovementConfig m_movementCfg;
        UIConfig m_uiCfg;
        Memory::GameEntity* m_currentTarget;
        std::chrono::high_resolution_clock::time_point m_lastAimTime;
        std::mt19937 m_rng;
        std::vector<std::pair<Memory::Vector2, std::chrono::steady_clock::time_point>> m_tracers;

        ID3D11Device* m_d3dDevice;
        ID3D11VertexShader* m_vs;
        ID3D11PixelShader* m_ps;
        ID3D11InputLayout* m_inputLayout;
        ID3D11Buffer* m_vb;
        ID3D11Buffer* m_ib;
        ID3D11BlendState* m_blendState;
        bool m_renderInit;

        struct Vertex { float x, y, z, rhw; uint32_t color; };
        bool InitRenderResources(ID3D11Device* device);
        void BeginBatch(ID3D11DeviceContext* ctx);
        void FlushBatch(ID3D11DeviceContext* ctx);
        void AddVertex(ID3D11DeviceContext* ctx, float x, float y, uint32_t color);
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;

        Memory::GameEntity* SelectAimbotTarget();
        Memory::Vector2 CalculateAimAngles(const Memory::Vector3& from, const Memory::Vector3& to);
        Memory::Vector3 PredictPosition(const Memory::GameEntity& target, float time);
        void ApplyAimbot(const Memory::Vector2& aimAngles);
        float GetFOV(const Memory::Vector2& viewAngles, const Memory::Vector2& targetAngles);
        float CalculateHitchance(const Memory::GameEntity& target, const Memory::Vector2& aimAngles);
        void ApplySilentAim(const Memory::Vector2& aimAngles);
        void ApplyMemoryAim(const Memory::Vector2& aimAngles);

        void RenderPlayerESP(ID3D11DeviceContext* ctx, const Memory::GameEntity& entity, int w, int h);
        void RenderSkeleton(ID3D11DeviceContext* ctx, const Memory::GameEntity& entity, int w, int h);
        void RenderViewArrow(ID3D11DeviceContext* ctx, const Memory::GameEntity& entity, int w, int h);
        void RenderOffscreenArrow(ID3D11DeviceContext* ctx, const Memory::GameEntity& entity, int w, int h);
        void RenderChams(ID3D11DeviceContext* ctx, const Memory::GameEntity& entity, int w, int h);
        void RenderHotbar(ID3D11DeviceContext* ctx, const Memory::GameEntity& entity, int w, int h);
        void RenderAvatar(ID3D11DeviceContext* ctx, const Memory::GameEntity& entity, int w, int h);
        void RenderItemESP(ID3D11DeviceContext* ctx, const Memory::GameEntity& entity, int w, int h, const WorldVisualsConfig::ItemVisual& cfg);
        void RenderFOVCircle(ID3D11DeviceContext* ctx, int w, int h);
        void RenderTargetLine(ID3D11DeviceContext* ctx, int w, int h);
        void RenderPrediction(ID3D11DeviceContext* ctx, int w, int h);
        void RenderTracers(ID3D11DeviceContext* ctx, int w, int h);
        void RenderHighlight(ID3D11DeviceContext* ctx, int w, int h);
        void RenderCrosshair(ID3D11DeviceContext* ctx, int w, int h);
        void RenderWatermark(ID3D11DeviceContext* ctx, int w, int h);

        void ApplyWorldExploits();
        void ApplyWeaponMods();
        void ApplyMovement();

        uint32_t GetEntityColor(const Memory::GameEntity& entity) const;
        uint32_t GetHealthColor(float health, float maxHealth) const;

        void DrawBox(ID3D11DeviceContext* ctx, float x, float y, float w, float h, uint32_t color, float thickness);
        void DrawCornerBox(ID3D11DeviceContext* ctx, float x, float y, float w, float h, uint32_t color, float thickness, float cornerLen);
        void DrawLine(ID3D11DeviceContext* ctx, float x1, float y1, float x2, float y2, uint32_t color, float thickness);
        void DrawFilledRect(ID3D11DeviceContext* ctx, float x, float y, float w, float h, uint32_t color);
        void DrawCircle(ID3D11DeviceContext* ctx, float x, float y, float radius, uint32_t color, float thickness, bool filled);
        void DrawText(ID3D11DeviceContext* ctx, const std::string& text, float x, float y, uint32_t color, bool centered, float scale);
        void DrawTriangle(ID3D11DeviceContext* ctx, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color, bool filled);

    public:
        CFeatureManager();
        ~CFeatureManager();
        bool Initialize(Memory::CGameMemory* memory);
        void Shutdown();
        void Update();
        void Render(ID3D11DeviceContext* ctx, int width, int height);

        AimbotConfig& GetAimbotConfig() { return m_aimbotCfg; }
        PlayerVisualsConfig& GetVisualsConfig() { return m_visualsCfg; }
        WorldExploitsConfig& GetExploitsConfig() { return m_exploitsCfg; }
        WeaponModsConfig& GetWeaponConfig() { return m_weaponCfg; }
        WorldVisualsConfig& GetWorldVisualsConfig() { return m_worldVisualsCfg; }
        MovementConfig& GetMovementConfig() { return m_movementCfg; }
        UIConfig& GetUIConfig() { return m_uiCfg; }
    };

}

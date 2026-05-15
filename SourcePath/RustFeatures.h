/*
 * Rust Advanced Features System
 * Complete cheat functionality with anti-detection
 * Author: Anonymous
 * Last Modified: 2026
 * 
 * This module implements all major Rust cheat features including
 * ESP, aimbot, resource gathering, and advanced combat systems.
 */

#pragma once
#include "RustAntiCheat.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace RustFeatures {
    
    // Advanced ESP rendering modes
    enum class ESPMode {
        BASIC = 0,              // Simple boxes and text
        ADVANCED = 1,            // 3D boxes with distance
        SKELETON = 2,            // Skeleton ESP
        CHAMS = 3,                // Color models
        GLOW = 4,                 // Glow ESP
        RADAR = 5,                // 2D radar
        FULL = 6                   // All ESP features
    };
    
    // Aimbot modes
    enum class AimbotMode {
        OFF = 0,
        LEGIT = 1,               // Legit-looking aimbot
        RAGE = 2,                // Instant aimbot
        SILENT = 3,              // Silent aim (no view angle change)
        PSILENT = 4,             // Perfect silent aim
        SMOOTH = 5,               // Smooth aimbot
        PREDICTIVE = 6            // Predictive aimbot
    };
    
    // Resource types for gathering
    enum class ResourceType {
        WOOD = 0,
        STONE = 1,
        METAL = 2,
        SULFUR = 3,
        HQM = 4,
        CLOTH = 5,
        FOOD = 6,
        MEDICAL = 7,
        AMMO = 8,
        WEAPONS = 9,
        TOOLS = 10,
        COMPONENTS = 11,
        TRAPS = 12,
        EXPLOSIVES = 13
    };
    
    // Resource node structure
    struct ResourceNode {
        Vec3 position;
        ResourceType type;
        float amount;
        bool isGathered;
        std::string name;
        uint64_t entityPtr;
        float respawnTime;
        bool isHighTier;
    };
    
    // Advanced player data
    struct AdvancedPlayer {
        RustPlayer basePlayer;
        
        // Combat information
        float lastShotTime;
        Vec3 lastKnownPosition;
        Vec3 velocity;
        Vec3 acceleration;
        bool isInCombat;
        float healthPercentage;
        bool isAimingAtMe;
        bool isReloading;
        float reloadTime;
        
        // Equipment information
        RustWeapon primaryWeapon;
        RustWeapon secondaryWeapon;
        std::vector<RustWeapon> inventory;
        int armorLevel;
        float armorCondition;
        
        // Movement information
        bool isSprinting;
        bool isCrouching;
        bool isProne;
        bool isJumping;
        bool isSwimming;
        bool isClimbing;
        float stamina;
        
        // Building information
        bool isBuilding;
        bool isCrafting;
        std::string currentBuilding;
        float craftingProgress;
        
        // Team information
        std::vector<int> teammateIds;
        bool isInTeam;
        std::string teamName;
        bool isTeamLeader;
        
        // Voice chat information
        bool isTalking;
        float voiceVolume;
        std::string lastVoiceMessage;
    };
    
    // ESP configuration
    struct ESPConfig {
        ESPMode mode = ESPMode::ADVANCED;
        bool showPlayers = true;
        bool showResources = true;
        bool showContainers = true;
        bool showTraps = true;
        bool showVehicles = true;
        bool showHelicopters = true;
        bool showCargoShips = true;
        bool showBradley = true;
        bool showTurrets = true;
        
        // Player ESP options
        bool showHealth = true;
        bool showArmor = true;
        bool showWeapon = true;
        bool showDistance = true;
        bool showName = true;
        bool showSkeleton = false;
        bool showBox = true;
        bool showChams = false;
        bool showGlow = false;
        bool showRadar = false;
        
        // Resource ESP options
        bool showWood = true;
        bool showStone = true;
        bool showMetal = true;
        bool showSulfur = true;
        bool showHQM = true;
        bool showCloth = true;
        bool showFood = true;
        bool showMedical = true;
        bool showAmmo = true;
        bool showWeapons = true;
        
        // Visual options
        float maxRenderDistance = 800.0f;
        bool onlyVisible = true;
        bool fadeWithDistance = true;
        bool dynamicColors = true;
        ImVec4 playerColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        ImVec4 resourceColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
        ImVec4 containerColor = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);
        ImVec4 enemyColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        ImVec4 teammateColor = ImVec4(0.0f, 0.5f, 1.0f, 1.0f);
    };
    
    // Weapon-specific recoil patterns
    struct WeaponRecoilProfile {
        std::string weaponName;
        float recoilX;
        float recoilY;
        float recoilZ;
        float spread;
        float fireRate;
        float recoilPattern[50]; // Full recoil pattern data
        int patternLength;
        float aimbotStrength; // Individual aimbot strength per weapon
        bool isAutomatic;
        float bulletSpeed;
        float damage;
    };
    
    // Aimbot configuration
    struct AimbotConfig {
        AimbotMode mode = AimbotMode::LEGIT;
        bool enabled = false;
        bool autoFire = false;
        bool autoScope = false;
        bool autoSwitch = false;
        
        // Targeting options
        bool targetPlayers = true;
        bool targetNPCs = false;
        bool targetAnimals = false;
        bool ignoreTeammates = true;
        bool ignoreInvisible = true;
        bool ignoreSleepers = true;
        bool prioritizeLowHealth = false;
        bool prioritizeClosest = true;
        bool prioritizeThreat = false;
        
        // Aim options
        float aimFOV = 15.0f;
        float aimSpeed = 2.0f;
        float aimSmoothness = 0.3f;
        bool aimAtHead = true;
        bool aimAtChest = false;
        bool aimAtPelvis = false;
        float headshotChance = 0.8f;
        
        // Prediction options
        bool enablePrediction = true;
        bool predictMovement = true;
        bool predictBullets = true;
        bool predictRecoil = true;
        float predictionTime = 0.1f;
        
        // Humanization options
        bool humanizeAim = true;
        bool addImperfections = true;
        float reactionTime = 180.0f;
        float aimJitter = 2.0f;
        float aimDelay = 50.0f;
        bool randomizeDelay = true;
        
        // Silent aim options
        bool silentAim = false;
        bool perfectSilent = false;
        float silentFOV = 5.0f;
        bool silentHeadshot = true;
        
        // NEW: Automatic weapon detection and consistent aimbot levels
        bool autoWeaponDetection = true;
        bool consistentAimbotLevels = true;
        float globalAimbotStrength = 0.8f; // Global strength override
        bool maintainRecoilControl = true;
        bool adaptiveRecoil = true;
        std::string currentWeapon;
        std::string lastWeapon;
        float weaponChangeDelay = 0.1f;
        bool smoothWeaponTransition = true;
        
        // NEW: Advanced stealth aimbot techniques
        bool circularAimbot = true;
        bool invisibleAimbot = true;
        bool sizeBasedLocking = true;
        bool antiDetectionMode = true;
        float lockRadius = 100.0f;
        float minPlayerSize = 50.0f;
        float maxPlayerSize = 200.0f;
        bool randomizeTargetBone = true;
        bool breakLineOfSight = false;
        float stealthLevel = 0.9f;
        int detectionEvasionCount = 0;
        bool humanMovementSimulation = true;
        float aimbotInvisibility = 0.95f;
        bool serverSideBypass = true;
        bool behavioralMasking = true;
    };
    
    // Resource gathering configuration
    struct ResourceConfig {
        bool enabled = false;
        bool autoGather = false;
        bool autoDeposit = false;
        bool autoCraft = false;
        
        // Gathering options
        bool gatherWood = true;
        bool gatherStone = true;
        bool gatherMetal = true;
        bool gatherSulfur = true;
        bool gatherHQM = true;
        float gatherRadius = 50.0f;
        bool avoidPlayers = true;
        bool avoidRadiation = true;
        float playerAvoidDistance = 100.0f;
        
        // Crafting options
        bool craftEssential = true;
        bool craftWeapons = true;
        bool craftArmor = true;
        bool craftTools = true;
        bool craftBuilding = true;
        std::vector<std::string> priorityItems;
        
        // Deposit options
        bool depositToTC = true;
        bool depositToBase = true;
        float depositThreshold = 0.8f; // Deposit when 80% full
        bool autoSortInventory = true;
    };
    
    // Combat assistance configuration
    struct CombatConfig {
        bool enabled = false;
        bool autoHeal = false;
        bool autoReload = false;
        bool autoSwitch = false;
        
        // Healing options
        float healThreshold = 0.5f; // Heal when below 50% health
        bool useMedkits = true;
        bool useBandages = true;
        bool useFood = true;
        float healDelay = 1.0f;
        
        // Reload options
        bool reloadWhenEmpty = true;
        bool reloadDuringCombat = false;
        float reloadThreshold = 0.2f; // Reload when below 20% ammo
        
        // Weapon switching options
        bool switchToBetter = true;
        bool switchWhenEmpty = true;
        bool prioritizeRange = false;
        bool prioritizeDamage = true;
        
        // Movement options
        bool autoDodge = false;
        bool autoStrafe = false;
        bool autoJump = false;
        float dodgeChance = 0.3f;
        float strafeChance = 0.5f;
        float jumpChance = 0.2f;
    };
    
    class CRustFeatures {
    private:
        // Feature configurations
        ESPConfig m_espConfig;
        AimbotConfig m_aimbotConfig;
        ResourceConfig m_resourceConfig;
        CombatConfig m_combatConfig;
        
        // Feature state
        bool m_featuresEnabled;
        uint64_t m_lastUpdateTime;
        
        // Player data
        std::vector<AdvancedPlayer> m_players;
        AdvancedPlayer m_localPlayer;
        
        // Resource data
        std::vector<ResourceNode> m_resources;
        std::unordered_map<ResourceType, std::vector<ResourceNode>> m_resourceMap;
        
        // Targeting data
        AdvancedPlayer* m_currentTarget;
        Vec3 m_aimPosition;
        bool m_isAiming;
        uint64_t m_lastAimTime;
        
        // Gathering data
        ResourceNode* m_currentResource;
        bool m_isGathering;
        uint64_t m_lastGatherTime;
        
        // Combat data
        bool m_inCombat;
        uint64_t m_lastCombatTime;
        float m_lastHealth;
        
        // Rust anti-cheat integration
        std::unique_ptr<RustAC::CRustAntiCheat> m_antiCheat;
        
        // NEW: Weapon profiles for automatic detection and consistent aimbot
        std::vector<WeaponRecoilProfile> m_weaponProfiles;
        
        // Private methods
        void UpdatePlayers();
        void UpdateResources();
        void UpdateCombat();
        void UpdateTargeting();
        void UpdateGathering();
        
        // ESP rendering methods
        void RenderPlayerESP(const AdvancedPlayer& player);
        void RenderResourceESP(const ResourceNode& resource);
        void RenderSkeletonESP(const AdvancedPlayer& player);
        void RenderChamsESP(const AdvancedPlayer& player);
        void RenderGlowESP(const AdvancedPlayer& player);
        void RenderRadar();
        void Render3DBox(const Vec3& position, const Vec3& size, const ImVec4& color);
        void Render2DBox(const Vec3& position, const ImVec4& color);
        
        // Aimbot methods
        void UpdateAimbot();
        AdvancedPlayer* SelectTarget();
        float CalculateAimAngle(const AdvancedPlayer& target);
        void ApplyAim(const Vec3& aimAngle);
        void PredictMovement(AdvancedPlayer& target, float time);
        void PredictBulletDrop(Vec3& aimPosition, float distance);
        
        // Resource gathering methods
        void UpdateResourceNodes();
        ResourceNode* FindNearestResource(ResourceType type);
        void GatherResource(ResourceNode* resource);
        void DepositResources();
        void CraftItems();
        
        // Combat assistance methods
        void AutoHeal();
        void AutoReload();
        void AutoSwitch();
        void AutoDodge();
        void AutoStrafe();
        
        // NEW: Weapon detection and consistent aimbot methods
        void DetectCurrentWeapon();
        void LoadWeaponProfiles();
        void ApplyConsistentAimbot();
        void HandleWeaponSwitch();
        WeaponRecoilProfile* GetWeaponProfile(const std::string& weaponName);
        void UpdateWeaponSpecificSettings();
        void MaintainRecoilControl();
        void SmoothWeaponTransition();
        
        // NEW: Advanced stealth aimbot methods
        void ApplyCircularAimbot();
        void ApplyInvisibleAimbot();
        void ApplySizeBasedLocking();
        void ApplyAntiDetectionTechniques();
        void SimulateHumanMovement();
        void ApplyServerSideBypass();
        void RandomizeTargetBone();
        void BreakLineOfSight();
        void UpdateStealthMetrics();
        
        // Utility methods
        Vec3 GetBonePosition(const AdvancedPlayer& player, int boneId);
        bool IsLineOfSight(const Vec3& start, const Vec3& end);
        float GetDistance(const Vec3& pos1, const Vec3& pos2);
        ImVec4 GetPlayerColor(const AdvancedPlayer& player);
        std::string GetResourceName(ResourceType type);
        
    public:
        CRustFeatures();
        ~CRustFeatures();
        
        // Initialization
        bool Initialize();
        void Shutdown();
        bool IsInitialized() const;
        
        // Main update loop
        void Update();
        void Render();
        
        // Configuration
        void SetESPConfig(const ESPConfig& config);
        void SetAimbotConfig(const AimbotConfig& config);
        void SetResourceConfig(const ResourceConfig& config);
        void SetCombatConfig(const CombatConfig& config);
        
        const ESPConfig& GetESPConfig() const;
        const AimbotConfig& GetAimbotConfig() const;
        const ResourceConfig& GetResourceConfig() const;
        const CombatConfig& GetCombatConfig() const;
        
        // Feature control
        void EnableFeatures(bool enable = true);
        void EnableESP(bool enable = true);
        void EnableAimbot(bool enable = true);
        void EnableResourceGathering(bool enable = true);
        void EnableCombatAssistance(bool enable = true);
        
        bool AreFeaturesEnabled() const;
        bool IsESPEnabled() const;
        bool IsAimbotEnabled() const;
        bool IsResourceGatheringEnabled() const;
        bool IsCombatAssistanceEnabled() const;
        
        // Target management
        void SetTarget(AdvancedPlayer* target);
        void ClearTarget();
        AdvancedPlayer* GetCurrentTarget() const;
        
        // Resource management
        void AddResourceNode(const ResourceNode& resource);
        void RemoveResourceNode(const ResourceNode& resource);
        std::vector<ResourceNode> GetResourcesByType(ResourceType type);
        
        // Combat management
        void SetInCombat(bool inCombat);
        bool IsInCombat() const;
        
        // Statistics
        int GetPlayerCount() const;
        int GetResourceCount() const;
        float GetAverageFPS() const;
        std::vector<std::string> GetFeatureStatus() const;
    };
    
} // namespace RustFeatures

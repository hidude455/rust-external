#pragma once
#include <windows.h>
#include <d3d11.h>
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <algorithm>
#include <initializer_list>
#include "Injector.h"
#include "KernelInterface.h"
#include "MemoryManager.h"
#include "ESP.h"
#include "Renderer.h"
#include "RendererDX.h"
#include "../MenuPath/imgui/imgui.h"

struct PageHeaderData {
    const char* title;
    const char* subtitle;
    const char* narrativeTitle;
    const char* narrativeBody;
    std::initializer_list<const char*> bulletPoints;
};

class CRustInjector {
public:
    CRustInjector();
    ~CRustInjector();

    bool Initialize(HINSTANCE hInstance);
    int Run();
    void Shutdown();

private:
    bool CreateMainWindow();
    bool InitializeDirectX();
    bool InitializeImGui();
    void CleanupDirectX();
    void CleanupImGui();
    
    void RenderFrame();
    void RenderUI();
    void RenderDLLTab();
    void RenderOptionsTab();
    void RenderESPTab();
    void RenderLogTab();
    
    // Page rendering
    void RenderTopNavigation();
    void RenderPageHeader(const PageHeaderData& data);
    void RenderAimbotPage();
    void RenderVisualsPage();
    void RenderWorldPage();
    void RenderMiscPage();
    void RenderCloudPage();
    void ApplyDarkTheme();

    // Flat (Disconnect-style) layout helpers
    void RenderLeftIconRail();
    void RenderFlatTopTabs();
    bool RenderFlatToggle(const char* label, bool* value);
    bool RenderFlatToggleWithColor(const char* label, bool* value, float colorRGB[3]);
    bool RenderFlatSlider(const char* label, float* value, float minVal, float maxVal, const char* format);
    bool RenderFlatDropdown(const char* label, int* currentItem, const char** items, int itemsCount);
    void RenderFlatSectionHeader(const char* title);
    void RenderFlatRowDivider();
    void AppendLog(const std::string& message);
    
    // Animation functions
    void UpdateAnimations(float deltaTime);
    float EaseOutQuad(float t);
    float EaseInOutCubic(float t);
    float EaseOutBounce(float t);
    void StartIconHoverAnim(int index);
    void StartModuleSelectAnim();
    void StartContentFadeAnim();
    
    // Custom control rendering
    void RenderCustomButton(const char* label, ImVec2 size, ImVec4 color, ImVec4 hoverColor, ImVec4 activeColor, bool* clicked);
    void RenderCustomCheckbox(const char* label, bool* value, ImVec4 checkColor);
    void RenderCustomSlider(const char* label, float* value, float minVal, float maxVal, const char* format);
    void RenderCustomScrollbar();
    void RenderCustomInput(const char* label, char* buffer, size_t bufferSize);
    void RenderCustomCombo(const char* label, int* currentItem, const char** items, int itemsCount);
    void RenderToggleSwitch(const char* icon, const char* label, bool* value);
    void RenderThinSlider(const char* icon, const char* label, float* value, float minVal, float maxVal, const char* format);
    void RenderDropdown(const char* icon, const char* label, int* currentItem, const char** items, int itemsCount);
    void RenderKeybindInput(const char* icon, const char* label, ImGuiKey* key, bool* listeningState);
    void RenderInfoText(const char* icon, const char* text, ImVec4 color);
    void RenderSettingsCard(const char* title, const char* subtitle, const ImVec2& size, const std::function<void()>& contentFn);
    void RenderBulletList(std::initializer_list<const char*> items);
    
    // Tooltip functions
    void RenderTooltip();
    void SetTooltip(const char* text, ImVec2 pos);
    void UpdateTooltip(float deltaTime);
    
    // Advanced features functions
    void SaveConfig();
    void LoadConfig();
    void RegisterHotkey(int key, bool ctrl, bool shift, bool alt, const char* action);
    void RenderWatermark();
    void RenderFPSCounter();
    
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    IDXGISwapChain* m_swapChain;
    ID3D11RenderTargetView* m_renderTarget;
    
    std::unique_ptr<CInjector> m_injector;
    std::unique_ptr<KernelInterface::CKernelInterface> m_kernelInterface;
    std::unique_ptr<MemoryManager> m_memoryManager;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<ESP> m_esp;
    bool m_running;
    bool m_initialized;
    bool m_driverLoaded;
    
    // Tooltip System
    struct Tooltip {
        std::string text;
        ImVec2 pos;
        float timer;
        bool visible;
        
        Tooltip() : text(""), pos(ImVec2(0, 0)), timer(0.0f), visible(false) {}
    };
    
    Tooltip m_currentTooltip;
    float m_tooltipDelay;
    
    // Font System
    ImFont* m_fontHeader;
    ImFont* m_fontNormal;
    ImFont* m_fontSmall;
    ImFont* m_fontBold;
    
    // Animation System
    struct AnimationState {
        float value;
        float targetValue;
        float speed;
        bool active;
        
        AnimationState() : value(0.0f), targetValue(0.0f), speed(1.0f), active(false) {}
        AnimationState(float v, float t, float s) : value(v), targetValue(t), speed(s), active(true) {}
    };
    
    AnimationState m_iconHoverAnim[5];  // One per category
    AnimationState m_moduleSelectAnim;
    AnimationState m_contentFadeAnim;
    float m_globalAnimTime;
    float m_lastFrameTime;
    
    // Window Management
    ImVec2 m_windowPos;
    ImVec2 m_windowSize;
    bool m_isDragging;
    bool m_isResizing;
    ImVec2 m_dragOffset;
    int m_resizeEdge;
    ImVec2 m_minWindowSize;
    ImVec2 m_maxWindowSize;
    
    // Advanced Features
    bool m_configLoaded;
    std::string m_configPath;
    bool m_soundEnabled;
    float m_soundVolume;
    bool m_showFPS;
    bool m_showVersion;
    bool m_minimizeToTray;
    
    // Hotkey System
    struct Hotkey {
        int key;
        bool ctrl;
        bool shift;
        bool alt;
        std::string action;
    };
    std::vector<Hotkey> m_hotkeys;
    bool m_autoInjectOnLaunch;
    float m_injectionDelay;
    std::vector<std::string> m_recentDLLs;
    char m_dllPath[MAX_PATH];
    bool m_autoInject;
    bool m_manualMap;
    std::string m_statusMessage;
    std::vector<std::string> m_logMessages;
    
    enum class MenuPage : int {
        Aimbot = 0,
        Visuals,
        World,
        Misc,
        Cloud
    };
    
    enum class AimbotTab : int {
        General = 0,
        Weapon
    };
    
    enum class VisualsTab : int {
        Players = 0,
        Overrides,
        Chams
    };
    
    enum class WorldTab : int {
        Resources = 0,
        Loot,
        Bases,
        Traps
    };
    
    enum class MiscTab : int {
        Movement = 0,
        Exploits,
        Automation
    };
    
    enum class CloudTab : int {
        Profiles = 0,
        ImportExport,
        Overlay
    };
    
    struct ToggleItem {
        const char* icon;
        const char* label;
        bool* value;
    };
    
    struct AimbotGeneralSettings {
        bool enabled;
        bool silentAim;
        bool prediction;
        bool smoothing;
        bool stickyAim;
        bool drawFov;
        bool predictionDot;
        bool autoShoot;
        bool waitForPowershot;
        bool targetTeam;
        bool visibleCheck;
        float fovRadius;
        float hitChance;
        float smoothness;
        int aimType;
        int targetFilter;
        int targetBone;
        int reticleStyle;
        bool alwaysAutomatic;
        bool instantEoka;
        bool instantBow;
        bool shotgunNospead;
        bool thickBullet;
        bool bowThickOverride;
        bool fastBullet;
        float thickBulletRange;
        ImGuiKey aimKey;
        bool listeningForAimKey;
    };
    
    struct AimbotWeaponProfile {
        std::string name;
        bool enabled;
        float recoilControl;
        float spreadControl;
        float predictionOffset;
        float triggerDelay;
    };
    
    struct VisualPlayerSettings {
        bool enabled;
        bool box2D;
        bool box3D;
        bool skeleton;
        bool healthBar;
        bool showName;
        bool showDistance;
        bool showWeapon;
        bool showAmmo;
        bool sleeperESP;
        bool chams;
        float chamsGlow;
        bool showTeam;
        bool showWounded;
        bool showDead;
        bool teamIndicator;
    };
    
    struct VisualOverrideSettings {
        bool enableGlow;
        float glowIntensity;
        bool enableOutlines;
        float outlineThickness;
        bool customReticle;
        int reticleType;
    };

    struct VisualNPCSettings {
        bool enabled;
        bool showName;
        bool box;
        bool fill;
        bool distance;
        bool chams;
        float redGlow;
    };
    
    struct WorldResourceSettings {
        bool sulfurNodes;
        bool metalNodes;
        bool stoneNodes;
        float resourceDistance;
    };
    
    struct WorldLootSettings {
        bool eliteCrates;
        bool militaryCrates;
        bool airDrops;
        bool lockedCrates;
        bool stashESP;
        float lootDistance;
    };
    
    struct WorldBaseSettings {
        bool toolCupboard;
        bool authorizedPlayers;
        bool baseOutline;
    };
    
    struct WorldTrapSettings {
        bool landMines;
        bool bearTraps;
        bool autoTurrets;
        bool flameTurrets;
        bool showRange;
    };
    
    struct MovementSettings {
        bool spiderMan;
        bool flyHack;
        bool infiniteJump;
        float flySpeed;
        float jumpSafety;
    };
    
    struct ExploitSettings {
        bool thickBullets;
        float bulletScale;
        bool noFallDamage;
        bool instantLoot;
        bool vehicleBoost;
    };
    
    struct AutomationSettings {
        bool autoFarm;
        bool autoPickup;
        bool autoHeal;
        float healThreshold;
        bool autoCraft;
    };
    
    struct CloudProfile {
        std::string name;
        bool isActive;
    };
    
    struct MiscLocalSettings {
        bool reloadIndicator;
        bool flyhackIndicator;
        bool desyncIndicator;
        bool onlyShowWhileDesyncing;
        bool antiFlash;
        bool localMovementLines;
        bool enemyMovementLines;
        bool bulletTrail;
        bool bulletTracers;
        bool changeBulletSize;
        float bulletSize;
        bool handChams;
        bool gunChams;
        float handChamsColor[3];
        float gunChamsColor[3];
        int handChamType;
        int gunChamType;
        bool changeHitEffect;
        int hitEffect;
    };

    struct MiscAtmosphereSettings {
        bool timeChanger;
        bool brightStars;
        bool starChanger;
        bool rayleighChanger;
        bool ambient;
        bool noFog;
        bool drawColliders;
        float time;
        float starSize;
        float rayleighAmount;
        bool changeSkyColor;
        bool changeCloudColor;
        bool sunMoonColor;
        float skyColor[3];
        float cloudColor[3];
        float sunMoonColorRGB[3];
    };

    struct CloudSettings {
        std::vector<CloudProfile> profiles;
        int selectedProfile;
        char newProfileName[64];
        std::string importString;
        std::string exportString;
        bool streamProof;
        ImGuiKey menuKey;
        bool listeningForMenuKey;
    };
    
    MenuPage m_selectedPage;
    AimbotTab m_selectedAimbotTab;
    VisualsTab m_selectedVisualsTab;
    WorldTab m_selectedWorldTab;
    MiscTab m_selectedMiscTab;
    CloudTab m_selectedCloudTab;
    char m_searchBuffer[64];
    int m_selectedWeaponProfile;
    
    AimbotGeneralSettings m_aimbotGeneral;
    std::vector<AimbotWeaponProfile> m_aimbotWeaponProfiles;
    VisualPlayerSettings m_visualPlayer;
    VisualOverrideSettings m_visualOverride;
    VisualNPCSettings m_visualNpc;
    WorldResourceSettings m_worldResources;
    WorldLootSettings m_worldLoot;
    WorldBaseSettings m_worldBase;
    WorldTrapSettings m_worldTraps;
    MovementSettings m_movement;
    ExploitSettings m_exploits;
    AutomationSettings m_automation;
    CloudSettings m_cloud;
    MiscLocalSettings m_miscLocal;
    MiscAtmosphereSettings m_miscAtmosphere;
    
    // ESP Settings
    bool m_espEnabled;
    bool m_espShowCircle;
    bool m_espShowInventory;
    bool m_espShowChams;
    bool m_espGalaxyMode;
    float m_espMaxDistance;
    float m_espCircleRadius;
    
    // Player ESP Settings
    bool m_espPlayerBox;
    bool m_espPlayerSkeleton;
    bool m_espPlayerHealth;
    bool m_espPlayerName;
    bool m_espPlayerDistance;
    bool m_espPlayerWeapon;
    bool m_espPlayerTeam;
    bool m_espPlayerWounded;
    bool m_espPlayerDead;
    bool m_espTeamIndicator;
    
    // Item ESP Settings
    bool m_espShowLoot;
    bool m_espShowResources;
    bool m_espShowWeapons;
    bool m_espShowTools;
    bool m_espShowAmmo;
    bool m_espShowMedical;
    
    // Entity ESP Settings
    bool m_espShowNPCs;
    bool m_espShowAnimals;
    bool m_espShowVehicles;
    bool m_espShowHelicopter;
    bool m_espShowTurrets;
    
    // Tracer Settings
    bool m_espTracerBulletPath;
    bool m_espTracerLineOfSight;
    bool m_espTracerTargetLine;
    float m_espTracerThickness;
    float m_espTracerOpacity;
    
    // Visual Settings
    bool m_noNightEnabled;
    bool m_purpleSkyEnabled;
    float m_purpleSkyIntensity;
    bool m_galaxySkyEffect;
    
    // Misc Settings
    bool m_antiAFKEnabled;
    float m_antiAFKInterval;
    int m_antiAFKAction;  // 0=Move, 1=Jump, 2=Look
    bool m_autoHealEnabled;
    float m_autoHealThreshold;
    bool m_speedhackEnabled;
    float m_speedhackMultiplier;
    bool m_flyHackEnabled;
    float m_flyVerticalSpeed;
    float m_flyHorizontalSpeed;
    bool m_noClipEnabled;
    float m_noClipSpeed;
    bool m_bunnyHopEnabled;
    bool m_bunnyHopAutoJump;
    float m_bunnyHopTiming;
    
    // Weapon Chams Settings
    bool m_chamsEnabled;
    float m_chamsOpacity;
    int m_chamsStyle;  // 0=Solid, 1=Gradient, 2=Wireframe
    bool m_chamsThroughWalls;
    
    // Glow Settings
    bool m_glowEnabled;
    float m_glowIntensity;
    float m_glowRadius;
    bool m_glowPlayers;
    bool m_glowItems;
    bool m_glowEntities;
    
    // World Lighting Settings
    float m_ambientLight;
    bool m_fogControl;
    float m_fogDensity;
    bool m_nightModeRemoval;
    
    // Combat Settings
    bool m_aimbotEnabled;
    float m_aimbotFOV;
    float m_aimbotSmoothness;
    bool m_aimbotVisibilityCheck;
    int m_aimbotTargetBone;  // 0=Head, 1=Chest, 2=Stomach
    int m_aimbotTargetPriority;  // 0=Distance, 1=Crosshair
    bool m_noRecoilEnabled;
    bool m_noSpreadEnabled;
    bool m_rapidFireEnabled;
    float m_rapidFireDelay;
};

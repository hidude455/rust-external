#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <functional>

#pragma comment(lib, "psapi.lib")

namespace Memory {

    struct Vector2 { float x, y; };
    struct Vector3 { float x, y, z; };
    struct Vector4 { float x, y, z, w; };

    struct Matrix4x4 {
        float m[4][4];
        float* operator[](int i) { return m[i]; }
        const float* operator[](int i) const { return m[i]; }
    };

    struct BoundingBox {
        Vector3 min;
        Vector3 max;
    };

    enum class EntityType : uint32_t {
        Player = 0,
        Scientist = 1,
        Animal = 2,
        Ore = 3,
        Collectible = 4,
        LootContainer = 5,
        Stash = 6,
        Trap = 7,
        Turret = 8,
        Helicopter = 9,
        BradleyAPC = 10,
        CargoShip = 11,
        SupplyDrop = 12,
        HackableCrate = 13,
        LockedCrate = 14,
        MultiTierCrate = 15,
        Vehicle = 16,
        Deployable = 17,
        Corpse = 18,
        Backpack = 19,
        ToolCupboard = 20,
        OilRig = 21,
        BaseEntity = 99
    };

    struct GameEntity {
        uint64_t address;
        EntityType type;
        Vector3 position;
        Vector3 velocity;
        Vector2 viewAngles;
        float health;
        float maxHealth;
        std::string name;
        std::string weaponName;
        float distance;
        bool isVisible;
        bool isSleeping;
        bool isWounded;
        bool isLocalPlayer;
        bool isTeammate;
        bool isInsideBuilding;
        uint64_t teamID;
        uint64_t steamID;
        BoundingBox bounds;
        uint64_t boneTransforms;
        Vector3 bonePositions[64];
        std::string heldItemName;
        std::vector<std::string> hotbarItems;
        std::vector<std::string> attachments;
        int ammoCount;
        int maxAmmo;
        bool isReloading;
        float reloadProgress;
        uint32_t flags;
    };

    struct GameCamera {
        Vector3 position;
        Vector2 viewAngles;
        Matrix4x4 viewMatrix;
        Matrix4x4 projectionMatrix;
        float fov;
        float nearPlane;
        float farPlane;
    };

    struct GameData {
        std::vector<GameEntity> entities;
        GameEntity localPlayer;
        GameCamera camera;
        uint64_t entityListAddress;
        uint32_t entityCount;
        uint64_t gameTime;
        bool isInGame;
    };

    class CGameMemory {
    private:
        uint32_t m_pid;
        uint64_t m_baseAddress;
        uint64_t m_gameAssemblyBase;
        uint64_t m_unityPlayerBase;
        HANDLE m_processHandle;
        bool m_attached;

        std::unordered_map<std::string, uint64_t> m_offsetCache;
        std::mutex m_mutex;

        GameData m_gameData;

        bool OpenProcess();
        void CloseProcess();
        uint64_t GetModuleBase(const std::string& moduleName);

        bool ReadRaw(uint64_t address, void* buffer, size_t size);
        bool WriteRaw(uint64_t address, const void* buffer, size_t size);

        uint64_t ResolveOffset(const std::string& name, const std::vector<uint64_t>& chain);
        uint64_t ReadChain(uint64_t base, const std::vector<uint64_t>& offsets);

        void UpdateEntityList();
        void UpdateLocalPlayer();
        void UpdateCamera();
        void UpdateEntityData(GameEntity& entity);

    public:
        bool WorldToScreen(const Vector3& worldPos, Vector2& screenPos, int screenW, int screenH);
        CGameMemory();
        ~CGameMemory();

        bool Initialize();
        void Shutdown();
        bool IsAttached() const { return m_attached; }
        bool IsProcessAlive() const;

        template<typename T>
        T Read(uint64_t address) {
            T value{};
            ReadRaw(address, &value, sizeof(T));
            return value;
        }

        template<typename T>
        bool Write(uint64_t address, const T& value) {
            return WriteRaw(address, &value, sizeof(T));
        }

        uint64_t ReadPtr(uint64_t address) { return Read<uint64_t>(address); }
        int32_t ReadInt(uint64_t address) { return Read<int32_t>(address); }
        float ReadFloat(uint64_t address) { return Read<float>(address); }
        Vector3 ReadVector3(uint64_t address) { return Read<Vector3>(address); }
        std::string ReadString(uint64_t address, size_t maxLen = 256);

        void Update();
        const GameData& GetGameData() const { return m_gameData; }

        uint32_t GetPID() const { return m_pid; }
        uint64_t GetBaseAddress() const { return m_baseAddress; }
        uint64_t GetGameAssemblyBase() const { return m_gameAssemblyBase; }

        bool IsInGame() const { return m_gameData.isInGame; }
        const GameEntity& GetLocalPlayer() const { return m_gameData.localPlayer; }
        const std::vector<GameEntity>& GetEntities() const { return m_gameData.entities; }
        const GameCamera& GetCamera() const { return m_gameData.camera; }

        bool WorldToScreen(const Vector3& worldPos, Vector2& screenPos) {
            return WorldToScreen(worldPos, screenPos, 1920, 1080);
        }
    };

}

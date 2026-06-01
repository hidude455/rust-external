#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <mutex>
#include <thread>
#include "../MenuPath/imgui/imgui.h"

#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl

namespace MIT {
    constexpr auto SOURCE_PATH = "c:/Users/lundk/OneDrive/Desktop/rust/SourcePath";
    constexpr auto MENU_PATH = "c:/Users/lundk/OneDrive/Desktop/rust/MenuPath";
    constexpr auto LOGS_PATH = "c:/Users/lundk/OneDrive/Desktop/rust/Logs";

    struct Vector3 {
        float x, y, z;
        Vector3() : x(0), y(0), z(0) {}
        Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
        Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
        Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
        Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
        float Distance(const Vector3& other) const {
            return sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y) + (z - other.z) * (z - other.z));
        }
    };

    enum class EntityType {
        Player,
        NPC,

        // Resources
        StoneNode,
        MetalNode,
        SulfurNode,

        // Loot Containers
        EliteCrate,
        MilitaryCrate,
        NormalCrate,
        Airdrop,
        LockedCrate,
        Stash,
        Toolbox,
        Corpse,

        // Traps
        AutoTurret,
        FlameTurret,
        BearTrap,
        LandMine,
        SAMSite,

        // Bases
        ToolCupboard,

        // Vehicles
        Helicopter,
        BradleyAPC,
        Minicopter,
        ScrapHeli,

        // Other
        DroppedItem,
        Unknown
    };

    struct Entity {
        uintptr_t ptr; // Pointer to the entity object
        Vector3 position;
        EntityType type;
        float distance;
        std::string name;
        std::string className;
        bool isVisible;
        bool is_npc;
    };

    struct WeaponData {
        float recoilX;
        float recoilY;
        float bulletVelocity;
        float gravityModifier;
        std::string name;
    };

    namespace Colors {
        constexpr ImU32 ESP_PLAYER = IM_COL32(255, 0, 0, 255);
        constexpr ImU32 ESP_ORE = IM_COL32(0, 255, 0, 255);
        constexpr ImU32 ESP_LOOT = IM_COL32(255, 255, 0, 255);
        constexpr ImU32 MENU_BACKGROUND = IM_COL32(20, 20, 30, 240);
        constexpr ImU32 MENU_ACCENT = IM_COL32(0, 120, 255, 255);
    }
}

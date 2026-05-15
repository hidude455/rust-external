/*
 * MIT Method Core System
 * Author: Anonymous
 * Last Modified: 2026
 * 
 * This is the main header file for our game enhancement utility.
 * Contains all the core definitions and structures needed for the system.
 */

#pragma once

// Windows includes - we need these for low-level system access
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

// Standard library includes
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

// Project namespace - keeps everything organized
namespace GameEnhance {
    
    // Important paths for our system
    const char* const BASE_DIR = "c:/Users/lundk/OneDrive/Desktop/rust/SourcePath";
    const char* const UI_DIR = "c:/Users/lundk/OneDrive/Desktop/rust/MenuPath";
    const char* const LOG_DIR = "c:/Users/lundk/OneDrive/Desktop/rust/Logs";
    
    // Basic 3D vector for positions and calculations
    struct Vec3 {
        float x, y, z;
        
        // Constructors
        Vec3() : x(0), y(0), z(0) {}
        Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
        
        // Vector operations
        Vec3 operator+(const Vec3& other) const { 
            return Vec3(x + other.x, y + other.y, z + other.z); 
        }
        
        Vec3 operator-(const Vec3& other) const { 
            return Vec3(x - other.x, y - other.y, z - other.z); 
        }
        
        Vec3 operator*(float scale) const { 
            return Vec3(x * scale, y * scale, z * scale); 
        }
        
        // Distance calculation - useful for ESP filtering
        float GetDistance(const Vec3& other) const {
            float dx = x - other.x;
            float dy = y - other.y;
            float dz = z - other.z;
            return sqrt(dx * dx + dy * dy + dz * dz);
        }
    };
    
    // Entity types we can detect in the game
    enum class EntityType {
        PLAYER = 0,    // Other players
        ORE = 1,       // Mining resources
        LOOT = 2,      // Items on ground
        UNKNOWN = 99    // Anything else
    };
    
    // Structure to hold information about game entities
    struct GameEntity {
        Vec3 position;          // Where the entity is in 3D space
        EntityType type;         // What type of entity this is
        float distance;          // How far away from player
        std::string name;       // Entity name if available
        bool isVisible;          // Can we actually see this entity
        int health;             // Health points if applicable
        
        GameEntity() : type(EntityType::UNKNOWN), distance(0), isVisible(false), health(0) {}
    };
    
    // Weapon information for recoil and prediction
    struct WeaponInfo {
        float recoilX;           // Horizontal recoil pattern
        float recoilY;           // Vertical recoil pattern
        float bulletSpeed;       // How fast bullets travel
        float gravityFactor;     // How much gravity affects bullets
        std::string weaponName;  // Name of the weapon
        int fireRate;            // Shots per second
        float damage;            // Damage per shot
        
        WeaponInfo() : recoilX(0), recoilY(0), bulletSpeed(0), gravityFactor(1.0f), 
                      fireRate(0), damage(0) {}
    };
    
    // Color definitions for our UI and ESP
    namespace Colors {
        const unsigned int ESP_PLAYER_COLOR = 0xFF0000FF;    // Red for players
        const unsigned int ESP_ORE_COLOR = 0xFF00FF00;      // Green for ore
        const unsigned int ESP_LOOT_COLOR = 0xFFFFFF00;      // Yellow for loot
        const unsigned int MENU_BG = 0xFF14141E;            // Dark background
        const unsigned int MENU_ACCENT = 0xFF0078FF;         // Blue accent
    }
    
    // Configuration structure for our main system
    struct SystemConfig {
        // ESP settings
        bool espEnabled = true;
        bool showPlayers = true;
        bool showOre = true;
        bool showLoot = true;
        float maxEspDistance = 500.0f;
        
        // Combat settings
        bool recoilControl = false;
        bool aimAssist = false;
        bool bulletPrediction = false;
        float aimSmoothness = 0.5f;
        float aimFov = 15.0f;
        
        // Visual settings
        bool showMenu = true;
        bool showPerformance = true;
        bool enableAnimations = true;
        
        // Protection settings
        bool enableProtection = true;
        bool stealthMode = true;
    };
    
    // Utility functions
    inline float Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    
    inline float Clamp(float value, float min, float max) {
        return (value < min) ? min : (value > max) ? max : value;
    }
    
    // Random number generator for obfuscation
    class RandomGen {
    private:
        static std::random_device rd;
        static std::mt19937 gen;
        static std::uniform_int_distribution<> dis;
        
    public:
        static int GetRandomInt(int min = 0, int max = 1000) {
            std::uniform_int_distribution<> range(min, max);
            return range(gen);
        }
        
        static float GetRandomFloat(float min = 0.0f, float max = 1.0f) {
            std::uniform_real_distribution<float> range(min, max);
            return range(gen);
        }
    };
    
    // Static member definitions
    std::random_device RandomGen::rd;
    std::mt19937 RandomGen::gen(RandomGen::rd());
    std::uniform_int_distribution<> RandomGen::dis(0, 1000);
    
} // namespace GameEnhance

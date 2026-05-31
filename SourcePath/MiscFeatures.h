#pragma once
#include "Common.h"
#include "MemoryManager.h"

namespace MIT {

    struct MiscConfig {
        // Movement
        bool spiderMan = false;
        bool flyHack = false;
        bool infiniteJump = false;
        float flySpeed = 2.5f;

        // Exploits
        bool noFallDamage = false;
        bool instantLoot = false;

        // Automation
        bool autoHeal = false;
        float healThreshold = 45.0f;
    };

    class MiscFeatures {
    public:
        MiscFeatures(MemoryManager* memManager);
        ~MiscFeatures();

        void Update(const MiscConfig& config);

    private:
        MemoryManager* memoryManager;
        MiscConfig config;

        void handleFlyHack();
        void handleNoFallDamage();
        void handleInfiniteJump();
    };

} // namespace MIT

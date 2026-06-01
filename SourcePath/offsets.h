#ifndef OFFSET_H
#define OFFSET_H

#include <cstdint>
#include <cstring>
#include <immintrin.h>

// Forward-declare MemoryManager to resolve circular dependency
namespace MIT {
    class MemoryManager;
}

// Global instance, to be defined in a .cpp file
extern MIT::MemoryManager* g_pMemoryManager;

#define ROL32(v,s) (((v)<<(s))|((v)>>(32-(s))))
#define ROR32(v,s) (((v)>>(s))|((v)<<(32-(s))))

namespace offsets {
    namespace base_networkable_o {
        constexpr uintptr_t klass = 0xE1598D0;
        constexpr uintptr_t entity = 0x10;
        constexpr uintptr_t parent_static_fields = 0x10;
        constexpr uintptr_t static_fields = 0xB8;
        constexpr uintptr_t wrapper_class_ptr = 0x18;
    }

    namespace entity_list {
        constexpr uintptr_t array = 0x10;
        constexpr uintptr_t size = 0x18;
    }

    namespace base_player_o {
        constexpr uintptr_t player_model = 0x2C8;
        constexpr uintptr_t model_state = 0x378;
    }

    namespace model_state_o {
        constexpr uintptr_t flags = 0x70;
    }

    namespace player_model_o {
        constexpr uintptr_t position = 0x2b8; // This seems to be the same, but let's keep it for clarity
        constexpr uintptr_t is_npc = 0x3c8;
    }

    namespace main_camera_c_o {
        constexpr uintptr_t klass = 0xE1CB9D0;
    }

    namespace main_camera_o {
        constexpr uintptr_t static_fields = 0xB8;
    }

} // namespace offsets

namespace decryptions {
    uint64_t Il2cppGetHandle(uint64_t ObjectHandleID);
    uint64_t decrypt_base_networkable(uint64_t address);
    uint64_t decrypt_base_networkable_list(uint64_t address);
} // namespace decryptions

#endif // OFFSET_H

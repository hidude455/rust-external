#include "offsets.h"
#include "MemoryManager.h"

// Define the global MemoryManager instance
MIT::MemoryManager* g_pMemoryManager = nullptr;

namespace decryptions {

    uint64_t Il2cppGetHandle(uint64_t ObjectHandleID) {
        if (!g_pMemoryManager) return 0;

        uint64_t page_base = ObjectHandleID & 0xFFFFFFFFFFFFE000;

        uint8_t type = g_pMemoryManager->ReadMemory<uint8_t>(page_base + 0x20);
        if (type >= 4)
            return 0;

        int64_t slot = (int64_t)(ObjectHandleID - page_base - 0x28) >> 3;

        uint32_t size = g_pMemoryManager->ReadMemory<uint32_t>(page_base + 0x1C);
        if ((uint32_t)slot >= size)
            return 0;

        uint64_t bitmap_ptr = g_pMemoryManager->ReadMemory<uint64_t>(page_base + 0x10);
        uint32_t bitmask = g_pMemoryManager->ReadMemory<uint32_t>(bitmap_ptr + (4 * ((uint32_t)slot >> 5)));

        if (!((bitmask >> (slot & 0x1F)) & 1))
            return 0;

        uint64_t entry = g_pMemoryManager->ReadMemory<uint64_t>(page_base + (8 * ((uint32_t)slot + 5)));

        return (type > 1) ? entry : ~entry;
    }

    uint64_t decrypt_base_networkable(uint64_t address) {
        if (!g_pMemoryManager || !address) return 0;
        uint64_t buf = g_pMemoryManager->ReadMemory<uint64_t>(address + 0x18);
        if (!buf) return 0;
        uint32_t* ptr = (uint32_t*)&buf;
        for (int i = 0; i < 2; i++) {
            uint32_t val = ptr[i];
            val += 0x4BA08177;
            val = ROL32(val, 24);
            val -= 0x5DF21DC4;
            ptr[i] = val;
        }
        return Il2cppGetHandle(buf);
    }

    uint64_t decrypt_base_networkable_list(uint64_t address) {
        if (!g_pMemoryManager || !address) return 0;
        uint64_t buf = g_pMemoryManager->ReadMemory<uint64_t>(address + 0x18);
        if (!buf) return 0;
        uint32_t* ptr = (uint32_t*)&buf;
        for (int i = 0; i < 2; i++) {
            uint32_t val = ptr[i];
            val = ROL32(val, 7);
            val ^= 0x1714CF61;
            val = ROL32(val, 4);
            ptr[i] = val;
        }
        return Il2cppGetHandle(buf);
    }

} // namespace decryptions

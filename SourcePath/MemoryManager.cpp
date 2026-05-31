#include "MemoryManager.h"
#include "offsets.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>

namespace MIT {
    MemoryManager::MemoryManager() : processHandle(nullptr), processId(0), isAttached(false), m_gameAssemblyBase(0), m_localPlayer(0), gen(rd()), dis(0, 1000) {
        g_pMemoryManager = this; // Initialize global pointer
    }

    MemoryManager::~MemoryManager() {
        Detach();
    }

    void MemoryManager::GenerateJunkCode() {
        // Simple junk code generation for obfuscation
        volatile int junk = dis(gen);
        for (int i = 0; i < junk % 5; ++i) {
            junk = (junk * 1103515245 + 12345) & 0x7fffffff;
        }
        
        // Add some meaningless operations
        if (junk % 2 == 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(junk % 100));
        }
    }

    bool MemoryManager::AttachToProcess(const std::string& processName) {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;

        bool found = false;
        if (Process32First(snapshot, &entry)) {
            do {
                std::wstring wProcessName(entry.szExeFile);
                std::string currentProcessName(wProcessName.begin(), wProcessName.end());
                
                if (currentProcessName == processName) {
                    processId = entry.th32ProcessID;
                    found = true;
                    break;
                }
            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);

        if (!found) return false;

        processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
        if (processHandle == nullptr) return false;

        isAttached = true;
        m_gameAssemblyBase = GetModuleBaseAddress("GameAssembly.dll");
        return true;
    }

    void MemoryManager::Detach() {
        if (processHandle && processHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(processHandle);
            processHandle = nullptr;
        }
        isAttached = false;
        processId = 0;
    }

    uintptr_t MemoryManager::GetModuleBaseAddress(const std::string& moduleName) {
        if (!isAttached) return 0;

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;

        MODULEENTRY32 entry;
        entry.dwSize = sizeof(MODULEENTRY32);
        uintptr_t baseAddress = 0;

        if (Module32First(snapshot, &entry)) {
            do {
                std::wstring wModuleName(entry.szModule);
                std::string currentModuleName(wModuleName.begin(), wModuleName.end());
                
                if (currentModuleName == moduleName) {
                    baseAddress = (uintptr_t)entry.modBaseAddr;
                    break;
                }
            } while (Module32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return baseAddress;
    }

    uintptr_t MemoryManager::CalculateAddress(uintptr_t base, const std::vector<uintptr_t>& offsets) {
        uintptr_t address = base;
        for (size_t i = 0; i < offsets.size(); ++i) {
            address = ReadMemory<uintptr_t>(address);
            if (address == 0) return 0;
            address += offsets[i];
        }
        return address;
    }

    uintptr_t MemoryManager::FollowPointerChain(uintptr_t base, const std::vector<uintptr_t>& offsets) {
        return CalculateAddress(base, offsets);
    }

    std::vector<uintptr_t> MemoryManager::ScanPattern(const std::string& pattern, const std::string& mask) {
        std::vector<uintptr_t> results;
        
        if (!isAttached) return results;

        MODULEINFO moduleInfo;
        if (!GetModuleInformation(processHandle, GetModuleHandle(nullptr), &moduleInfo, sizeof(moduleInfo))) {
            return results;
        }

        uintptr_t base = (uintptr_t)moduleInfo.lpBaseOfDll;
        uintptr_t size = moduleInfo.SizeOfImage;

        std::vector<uint8_t> buffer(size);
        SIZE_T bytesRead;
        
        if (!ReadProcessMemory(processHandle, (LPCVOID)base, buffer.data(), size, &bytesRead)) {
            return results;
        }

        for (size_t i = 0; i <= bytesRead - pattern.length(); ++i) {
            bool found = true;
            for (size_t j = 0; j < pattern.length(); ++j) {
                if (mask[j] != '?' && buffer[i + j] != (uint8_t)pattern[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                results.push_back(base + i);
            }
        }

        return results;
    }

    EntityType MemoryManager::GetEntityType(const std::string& className) {
        if (className.find("player.prefab") != std::string::npos) return EntityType::Player;
        if (className.find("scientist") != std::string::npos) return EntityType::NPC;
        if (className.find("stone-ore") != std::string::npos) return EntityType::StoneNode;
        if (className.find("metal-ore") != std::string::npos) return EntityType::MetalNode;
        if (className.find("sulfur-ore") != std::string::npos) return EntityType::SulfurNode;
        if (className.find("crate_elite") != std::string::npos) return EntityType::EliteCrate;
        if (className.find("crate_military") != std::string::npos) return EntityType::MilitaryCrate;
        if (className.find("crate_normal") != std::string::npos) return EntityType::NormalCrate;
        if (className.find("supply_drop") != std::string::npos) return EntityType::Airdrop;
        if (className.find("codelockedhackablecrate") != std::string::npos) return EntityType::LockedCrate;
        if (className.find("stash-small") != std::string::npos) return EntityType::Stash;
        if (className.find("tool_box") != std::string::npos) return EntityType::Toolbox;
        if (className.find("player_corpse") != std::string::npos) return EntityType::Corpse;
        if (className.find("autoturret_deployed") != std::string::npos) return EntityType::AutoTurret;
        if (className.find("flameturret.deployed") != std::string::npos) return EntityType::FlameTurret;
        if (className.find("beartrap") != std::string::npos) return EntityType::BearTrap;
        if (className.find("landmine") != std::string::npos) return EntityType::LandMine;
        if (className.find("samsite_deployed") != std::string::npos) return EntityType::SAMSite;
        if (className.find("building_privilege") != std::string::npos) return EntityType::ToolCupboard;
        if (className.find("patrolhelicopter") != std::string::npos) return EntityType::Helicopter;
        if (className.find("bradleyapc") != std::string::npos) return EntityType::BradleyAPC;
        if (className.find("minicopter.entity") != std::string::npos) return EntityType::Minicopter;
        if (className.find("scraptransporthelicopter") != std::string::npos) return EntityType::ScrapHeli;
        if (className.find("worlditem.prefab") != std::string::npos) return EntityType::DroppedItem;
        return EntityType::Unknown;
    }

    std::vector<Entity> MemoryManager::GetEntities() {
        std::vector<Entity> entities;
        if (!isAttached || m_gameAssemblyBase == 0) return entities;

        uintptr_t base_networkable_static = m_gameAssemblyBase + offsets::base_networkable_o::klass;
        uintptr_t wrapper_enc = ReadMemory<uintptr_t>(base_networkable_static + offsets::base_networkable_o::static_fields);
        uintptr_t wrapper_dec = decryptions::decrypt_base_networkable(wrapper_enc);
        uintptr_t parent_static_enc = ReadMemory<uintptr_t>(wrapper_dec + offsets::base_networkable_o::parent_static_fields);
        uintptr_t parent_static_dec = decryptions::decrypt_base_networkable_list(parent_static_enc);
        uintptr_t entity_list_ptr = ReadMemory<uintptr_t>(parent_static_dec + offsets::base_networkable_o::entity);

        uintptr_t list_ptr = ReadMemory<uintptr_t>(entity_list_ptr + offsets::entity_list::array);
        int list_size = ReadMemory<int>(entity_list_ptr + offsets::entity_list::size);

        if (list_size <= 0 || list_size > 1024) return entities;

        for (int i = 0; i < list_size; ++i) {
            uintptr_t current_entity = ReadMemory<uintptr_t>(list_ptr + 0x20 + (i * 0x8));
            if (!current_entity) continue;

            uintptr_t base_object = ReadMemory<uintptr_t>(current_entity + 0x10);
            if (!base_object) continue;

            uintptr_t game_object = ReadMemory<uintptr_t>(base_object + 0x30);
            if (!game_object) continue;

            uintptr_t object_class = ReadMemory<uintptr_t>(game_object + 0x30);
            if (!object_class) continue;

            uintptr_t entity_name_ptr = ReadMemory<uintptr_t>(object_class + 0x60);
            if (!entity_name_ptr) continue;

            char entity_name_buf[128] = {};
            ReadProcessMemory(processHandle, (LPCVOID)entity_name_ptr, entity_name_buf, sizeof(entity_name_buf), nullptr);
            std::string className(entity_name_buf);

            EntityType type = GetEntityType(className);
            if (type == EntityType::Unknown) continue;

            Entity entity;
            entity.ptr = current_entity;
            entity.className = className;
            entity.type = type;

            uintptr_t player_visual = ReadMemory<uintptr_t>(game_object + 0x8);
            uintptr_t visual_state = ReadMemory<uintptr_t>(player_visual + 0x38);
            entity.position = ReadMemory<Vector3>(visual_state + 0x90);

            if (type == EntityType::Player) {
                uintptr_t player = ReadMemory<uintptr_t>(base_object + 0x28);
                if (!player) continue;

                uintptr_t model_state = ReadMemory<uintptr_t>(player + offsets::base_player_o::model_state);
                uint32_t player_flags = ReadMemory<uint32_t>(model_state + offsets::model_state_o::flags);
                bool is_local_player = (player_flags & 256); // PlayerFlags::IsLocalPlayer

                if (is_local_player) {
                    m_localPlayer = player;
                }

                uintptr_t player_model = ReadMemory<uintptr_t>(player + offsets::base_player_o::player_model);
                entity.is_npc = ReadMemory<bool>(player_model + offsets::player_model_o::is_npc);

                uintptr_t display_name_ptr = ReadMemory<uintptr_t>(player + 0x6E0); // _displayName
                if (display_name_ptr) {
                    wchar_t buffer[64];
                    ReadProcessMemory(processHandle, (LPCVOID)display_name_ptr, buffer, sizeof(buffer), nullptr);
                    std::wstring wstr(buffer);
                    entity.name = std::string(wstr.begin(), wstr.end());
                }
            } else {
                uintptr_t object_name_ptr = ReadMemory<uintptr_t>(game_object + 0x60);
                if (object_name_ptr) {
                    char object_name_buf[128] = {};
                    ReadProcessMemory(processHandle, (LPCVOID)object_name_ptr, object_name_buf, sizeof(object_name_buf), nullptr);
                    entity.name = std::string(object_name_buf);
                }
            }

            entities.push_back(entity);
        }

        return entities;
    }

    WeaponData MemoryManager::GetCurrentWeapon() {
        WeaponData weapon{};
        
        // This would be implemented based on the specific game's memory structure
        // For now, returning empty weapon data as placeholder
        
        return weapon;
    }

    Vector3 MemoryManager::GetLocalPlayerPosition() {
        if (!m_localPlayer) return Vector3{};

        uintptr_t player_model = ReadMemory<uintptr_t>(m_localPlayer + offsets::base_player::playerModel);
        if (!player_model) return Vector3{};

        return ReadMemory<Vector3>(player_model + offsets::player_model::position);
    }

    bool MemoryManager::IsProcessValid() {
        if (!isAttached || !processHandle) return false;
        
        DWORD exitCode;
        return GetExitCodeProcess(processHandle, &exitCode) && exitCode == STILL_ACTIVE;
    }
}

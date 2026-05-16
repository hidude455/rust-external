#include "GameMemory.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>

namespace Memory {

    CGameMemory::CGameMemory()
        : m_pid(0), m_baseAddress(0), m_gameAssemblyBase(0),
          m_unityPlayerBase(0), m_processHandle(nullptr), m_attached(false) {}

    CGameMemory::~CGameMemory() { Shutdown(); }

    bool CGameMemory::Initialize() {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;

        PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
        if (Process32FirstW(snapshot, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"RustClient.exe") == 0) {
                    m_pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &pe));
        }
        CloseHandle(snapshot);

        if (m_pid == 0) return false;

        if (!OpenProcess()) return false;

        m_baseAddress = GetModuleBase("RustClient.exe");
        m_gameAssemblyBase = GetModuleBase("GameAssembly.dll");
        m_unityPlayerBase = GetModuleBase("UnityPlayer.dll");

        m_attached = true;
        return true;
    }

    void CGameMemory::Shutdown() {
        CloseProcess();
        m_attached = false;
    }

    bool CGameMemory::OpenProcess() {
        m_processHandle = ::OpenProcess(
            PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION |
            PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD,
            FALSE, m_pid);
        return m_processHandle != nullptr;
    }

    void CGameMemory::CloseProcess() {
        if (m_processHandle) {
            CloseHandle(m_processHandle);
            m_processHandle = nullptr;
        }
    }

    uint64_t CGameMemory::GetModuleBase(const std::string& moduleName) {
        if (!m_processHandle) return 0;

        HMODULE hMods[1024];
        DWORD cbNeeded;

        if (EnumProcessModules(m_processHandle, hMods, sizeof(hMods), &cbNeeded)) {
            for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                char modName[MAX_PATH] = {};
                if (GetModuleFileNameExA(m_processHandle, hMods[i], modName, sizeof(modName))) {
                    std::string name(modName);
                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                    std::string search = moduleName;
                    std::transform(search.begin(), search.end(), search.begin(), ::tolower);

                    if (name.find(search) != std::string::npos) {
                        MODULEINFO modInfo;
                        if (GetModuleInformation(m_processHandle, hMods[i], &modInfo, sizeof(modInfo))) {
                            return (uint64_t)modInfo.lpBaseOfDll;
                        }
                    }
                }
            }
        }
        return 0;
    }

    bool CGameMemory::ReadRaw(uint64_t address, void* buffer, size_t size) {
        if (!m_processHandle || !address) return false;
        SIZE_T bytesRead = 0;
        return ReadProcessMemory(m_processHandle, (LPCVOID)address, buffer, size, &bytesRead) && bytesRead == size;
    }

    bool CGameMemory::WriteRaw(uint64_t address, const void* buffer, size_t size) {
        if (!m_processHandle || !address) return false;
        SIZE_T bytesWritten = 0;
        return WriteProcessMemory(m_processHandle, (LPVOID)address, buffer, size, &bytesWritten) && bytesWritten == size;
    }

    std::string CGameMemory::ReadString(uint64_t address, size_t maxLen) {
        if (!address) return "";
        std::vector<char> buf(maxLen + 1, 0);
        if (ReadRaw(address, buf.data(), maxLen)) {
            return std::string(buf.data());
        }
        return "";
    }

    uint64_t CGameMemory::ResolveOffset(const std::string& name, const std::vector<uint64_t>& chain) {
        auto it = m_offsetCache.find(name);
        if (it != m_offsetCache.end()) return it->second;

        uint64_t resolved = ReadChain(m_baseAddress, chain);
        m_offsetCache[name] = resolved;
        return resolved;
    }

    uint64_t CGameMemory::ReadChain(uint64_t base, const std::vector<uint64_t>& offsets) {
        uint64_t current = base;
        for (size_t i = 0; i < offsets.size(); i++) {
            if (i == offsets.size() - 1) {
                current += offsets[i];
            } else {
                current = Read<uint64_t>(current + offsets[i]);
                if (!current) return 0;
            }
        }
        return current;
    }

    void CGameMemory::Update() {
        if (!m_attached) return;

        std::lock_guard<std::mutex> lock(m_mutex);

        UpdateLocalPlayer();
        UpdateCamera();
        UpdateEntityList();

        m_gameData.isInGame = m_gameData.localPlayer.address != 0;
    }

    void CGameMemory::UpdateEntityList() {
        m_gameData.entities.clear();

        uint64_t entityList = Read<uint64_t>(m_baseAddress + 0x31F0058);
        if (!entityList) return;

        uint64_t entityListBase = Read<uint64_t>(entityList + 0xB8);
        if (!entityListBase) return;

        uint32_t entityCount = Read<uint32_t>(entityList + 0xC0);
        m_gameData.entityCount = entityCount;
        m_gameData.entityListAddress = entityList;

        for (uint32_t i = 0; i < std::min(entityCount, 512u); i++) {
            uint64_t entityPtr = Read<uint64_t>(entityListBase + 0x20 + i * 0x8);
            if (!entityPtr) continue;

            uint64_t entity = Read<uint64_t>(entityPtr + 0x10);
            if (!entity) continue;

            uint64_t object = Read<uint64_t>(entity + 0x30);
            if (!object) continue;

            GameEntity ge;
            ge.address = object;
            UpdateEntityData(ge);

            if (ge.position.x == 0 && ge.position.y == 0 && ge.position.z == 0) continue;
            if (ge.health <= 0 && ge.type == EntityType::Player) continue;

            Vector3 delta = {
                ge.position.x - m_gameData.localPlayer.position.x,
                ge.position.y - m_gameData.localPlayer.position.y,
                ge.position.z - m_gameData.localPlayer.position.z
            };
            ge.distance = sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

            m_gameData.entities.push_back(ge);
        }
    }

    void CGameMemory::UpdateLocalPlayer() {
        uint64_t localPlayer = Read<uint64_t>(m_baseAddress + 0x31F0058);
        if (!localPlayer) return;

        localPlayer = Read<uint64_t>(localPlayer + 0xB8);
        if (!localPlayer) return;

        localPlayer = Read<uint64_t>(localPlayer);
        if (!localPlayer) return;

        localPlayer = Read<uint64_t>(localPlayer + 0x10);
        if (!localPlayer) return;

        localPlayer = Read<uint64_t>(localPlayer + 0x30);
        if (!localPlayer) return;

        m_gameData.localPlayer.address = localPlayer;
        m_gameData.localPlayer.isLocalPlayer = true;
        UpdateEntityData(m_gameData.localPlayer);
    }

    void CGameMemory::UpdateCamera() {
        uint64_t cameraPtr = Read<uint64_t>(m_baseAddress + 0x31F0058);
        if (!cameraPtr) return;

        cameraPtr = Read<uint64_t>(cameraPtr + 0xB8);
        if (!cameraPtr) return;

        cameraPtr = Read<uint64_t>(cameraPtr);
        if (!cameraPtr) return;

        m_gameData.camera.viewMatrix = Read<Matrix4x4>(cameraPtr + 0x2E4);
        m_gameData.camera.position = Read<Vector3>(cameraPtr + 0x42C);
        m_gameData.camera.fov = Read<float>(cameraPtr + 0x160);
        m_gameData.camera.nearPlane = Read<float>(cameraPtr + 0x164);
        m_gameData.camera.farPlane = Read<float>(cameraPtr + 0x168);
    }

    void CGameMemory::UpdateEntityData(GameEntity& entity) {
        if (!entity.address) return;
        entity.position = Read<Vector3>(entity.address + 0x90);
        entity.velocity = Read<Vector3>(entity.address + 0x9C);
        entity.health = Read<float>(entity.address + 0x200);
        entity.maxHealth = Read<float>(entity.address + 0x204);
        entity.flags = Read<uint32_t>(entity.address + 0x1E0);
        entity.isSleeping = (entity.flags & 16) != 0;
        entity.isWounded = (entity.flags & 2) != 0;
        entity.isInsideBuilding = (entity.flags & 0x2000) != 0;

        uint64_t namePtr = Read<uint64_t>(entity.address + 0x5A8);
        if (namePtr) entity.name = ReadString(namePtr + 0x14, 32);

        uint64_t weaponPtr = Read<uint64_t>(entity.address + 0x5D0);
        if (weaponPtr) {
            uint64_t weaponInfo = Read<uint64_t>(weaponPtr + 0x28);
            if (weaponInfo) entity.weaponName = ReadString(weaponInfo + 0x18, 32);
            entity.ammoCount = Read<int>(weaponPtr + 0x2C);
            entity.maxAmmo = Read<int>(weaponPtr + 0x30);
            entity.isReloading = Read<bool>(weaponPtr + 0x34);
            entity.reloadProgress = Read<float>(weaponPtr + 0x38);
        }

        uint64_t heldItemPtr = Read<uint64_t>(entity.address + 0x5D8);
        if (heldItemPtr) {
            uint64_t itemInfo = Read<uint64_t>(heldItemPtr + 0x28);
            if (itemInfo) entity.heldItemName = ReadString(itemInfo + 0x18, 32);
        }

        entity.teamID = Read<uint64_t>(entity.address + 0x5B0);
        entity.steamID = Read<uint64_t>(entity.address + 0x5B8);
        entity.type = EntityType::Player;
    }

    bool CGameMemory::WorldToScreen(const Vector3& worldPos, Vector2& screenPos, int screenW, int screenH) {
        const Matrix4x4& view = m_gameData.camera.viewMatrix;
        float w = view[0][3] * worldPos.x + view[1][3] * worldPos.y + view[2][3] * worldPos.z + view[3][3];

        if (w < 0.01f) return false;

        float invW = 1.0f / w;
        float x = (view[0][0] * worldPos.x + view[1][0] * worldPos.y + view[2][0] * worldPos.z + view[3][0]) * invW;
        float y = (view[0][1] * worldPos.x + view[1][1] * worldPos.y + view[2][1] * worldPos.z + view[3][1]) * invW;

        screenPos.x = (screenW * 0.5f) + (x * screenW * 0.5f);
        screenPos.y = (screenH * 0.5f) - (y * screenH * 0.5f);

        return (screenPos.x >= 0 && screenPos.x <= screenW &&
                screenPos.y >= 0 && screenPos.y <= screenH);
    }

}

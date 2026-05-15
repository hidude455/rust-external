#pragma once
#include "../MenuPath/imgui/imgui.h"
#include "../MenuPath/imgui/backends/imgui_impl_win32.h"
#include "../MenuPath/imgui/backends/imgui_impl_dx11.h"
#include <d3d11.h>
#include <Windows.h>
#include "FeatureManager.h"

class CMenuManager {
public:
    CMenuManager(CFeatureManager* features);
    ~CMenuManager();

    bool Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* ctx);
    void Shutdown();
    void Render();
    bool IsOpen() const { return m_open; }
    void Toggle() { m_open = !m_open; }

private:
    void RenderAimbotTab();
    void RenderPlayerVisualsTab();
    void RenderWorldExploitsTab();
    void RenderWeaponModsTab();
    void RenderWorldVisualsTab();
    void RenderMovementTab();
    void RenderSettingsTab();
    void RenderKeybindPicker(const char* label, Keybind& keybind);

    CFeatureManager* m_features;
    bool m_open = true;
    HWND m_hwnd = nullptr;
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_ctx = nullptr;
};

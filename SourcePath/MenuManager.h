#pragma once
#include "../MenuPath/imgui/imgui.h"
#include "../MenuPath/imgui/backends/imgui_impl_win32.h"
#include "../MenuPath/imgui/backends/imgui_impl_dx11.h"
#include <d3d11.h>
#include <Windows.h>
#include "FeatureManager.h"
#include "AdvancedSpoofer.h"
#include "SpooferGUI.h"
#include "RustAntiCheatEvasion.h"

class CMenuManager {
public:
    CMenuManager(Features::CFeatureManager* features);
    ~CMenuManager();

    bool Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* ctx);
    void Shutdown();
    void Render();
    bool IsOpen() const { return m_open; }
    void Toggle() { m_open = !m_open; }
    
    // Spoofer integration
    void SetSpoofer(Spoofer::CAdvancedSpoofer* spoofer);
    void SetSpooferGUI(SpooferGUI::CSpooferGUI* spooferGUI);
    void SetRustEvasion(RustEvasion::CRustAntiCheatEvasion* rustEvasion);

private:
    void ApplyTheme();
    void RenderOverviewSection();
    void RenderVisualSections();
    void RenderWorldSection();
    void RenderAimbotTab();
    void RenderPlayerVisualsTab();
    void RenderWorldExploitsTab();
    void RenderWeaponModsTab();
    void RenderWorldVisualsTab();
    void RenderMovementTab();
    void RenderSpooferTab();
    void RenderSettingsTab();
    void RenderKeybindPicker(const char* label, Features::Keybind& keybind);

    Features::CFeatureManager* m_features;
    Spoofer::CAdvancedSpoofer* m_spoofer;
    SpooferGUI::CSpooferGUI* m_spooferGUI;
    RustEvasion::CRustAntiCheatEvasion* m_rustEvasion;
    
    bool m_open = true;
    HWND m_hwnd = nullptr;
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_ctx = nullptr;
    int m_activeSection = 0;
};

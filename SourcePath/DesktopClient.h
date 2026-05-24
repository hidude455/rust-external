#pragma once
#include <windows.h>
#include <string>
#include <memory>
#include "DriverInterface.h"

class CDesktopClient {
public:
    CDesktopClient();
    ~CDesktopClient();

    bool Initialize(HINSTANCE hInstance);
    int Run();
    void Shutdown();

private:
    bool CreateMainWindow();
    void CreateControls();
    void UpdateDriverStatus();
    void HandleCommand(WPARAM wParam, LPARAM lParam);
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    HWND m_statusLabel;
    HWND m_connectButton;
    HWND m_disconnectButton;
    HWND m_espCheckbox;
    HWND m_aimbotCheckbox;
    HWND m_logEdit;
    
    std::unique_ptr<CDriverInterface> m_driver;
    bool m_connected;
    bool m_running;
};

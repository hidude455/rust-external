#include "DesktopClient.h"
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

CDesktopClient::CDesktopClient()
    : m_hInstance(nullptr)
    , m_hwnd(nullptr)
    , m_statusLabel(nullptr)
    , m_connectButton(nullptr)
    , m_disconnectButton(nullptr)
    , m_espCheckbox(nullptr)
    , m_aimbotCheckbox(nullptr)
    , m_logEdit(nullptr)
    , m_driver(std::make_unique<CDriverInterface>())
    , m_connected(false)
    , m_running(false)
{
}

CDesktopClient::~CDesktopClient() {
    Shutdown();
}

bool CDesktopClient::Initialize(HINSTANCE hInstance) {
    m_hInstance = hInstance;
    
    INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX) };
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);
    
    if (!CreateMainWindow()) {
        return false;
    }
    
    CreateControls();
    m_running = true;
    return true;
}

bool CDesktopClient::CreateMainWindow() {
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"DesktopClientClass";
    
    if (!RegisterClassEx(&wc)) {
        return false;
    }
    
    m_hwnd = CreateWindowEx(
        0,
        L"DesktopClientClass",
        L"MIT Method Desktop Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 500,
        nullptr, nullptr, m_hInstance, this
    );
    
    if (!m_hwnd) {
        return false;
    }
    
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    return true;
}

void CDesktopClient::CreateControls() {
    m_statusLabel = CreateWindowEx(
        0, L"STATIC", L"Status: Disconnected",
        WS_CHILD | WS_VISIBLE,
        20, 20, 200, 20,
        m_hwnd, nullptr, m_hInstance, nullptr
    );
    
    m_connectButton = CreateWindowEx(
        0, L"BUTTON", L"Connect to Driver",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 50, 150, 30,
        m_hwnd, (HMENU)1, m_hInstance, nullptr
    );
    
    m_disconnectButton = CreateWindowEx(
        0, L"BUTTON", L"Disconnect",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        180, 50, 150, 30,
        m_hwnd, (HMENU)2, m_hInstance, nullptr
    );
    EnableWindow(m_disconnectButton, FALSE);
    
    m_espCheckbox = CreateWindowEx(
        0, L"BUTTON", L"Enable ESP",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 100, 150, 20,
        m_hwnd, (HMENU)3, m_hInstance, nullptr
    );
    EnableWindow(m_espCheckbox, FALSE);
    
    m_aimbotCheckbox = CreateWindowEx(
        0, L"BUTTON", L"Enable Aimbot",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 130, 150, 20,
        m_hwnd, (HMENU)4, m_hInstance, nullptr
    );
    EnableWindow(m_aimbotCheckbox, FALSE);
    
    CreateWindowEx(
        0, L"STATIC", L"Log:",
        WS_CHILD | WS_VISIBLE,
        20, 160, 50, 20,
        m_hwnd, nullptr, m_hInstance, nullptr
    );
    
    m_logEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        20, 180, 540, 250,
        m_hwnd, nullptr, m_hInstance, nullptr
    );
}

void CDesktopClient::UpdateDriverStatus() {
    if (m_connected) {
        SetWindowText(m_statusLabel, L"Status: Connected");
        EnableWindow(m_connectButton, FALSE);
        EnableWindow(m_disconnectButton, TRUE);
        EnableWindow(m_espCheckbox, TRUE);
        EnableWindow(m_aimbotCheckbox, TRUE);
    } else {
        SetWindowText(m_statusLabel, L"Status: Disconnected");
        EnableWindow(m_connectButton, TRUE);
        EnableWindow(m_disconnectButton, FALSE);
        EnableWindow(m_espCheckbox, FALSE);
        EnableWindow(m_aimbotCheckbox, FALSE);
    }
}

void CDesktopClient::HandleCommand(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
        case 1: // Connect
            if (m_driver->Connect()) {
                m_connected = true;
                UpdateDriverStatus();
                SetWindowText(m_logEdit, L"Connected to driver successfully.\r\n");
            } else {
                SetWindowText(m_logEdit, L"Failed to connect to driver.\r\n");
            }
            break;
            
        case 2: // Disconnect
            m_driver->Disconnect();
            m_connected = false;
            UpdateDriverStatus();
            SetWindowText(m_logEdit, L"Disconnected from driver.\r\n");
            break;
            
        case 3: // ESP Checkbox
            if (m_connected) {
                bool espEnabled = SendMessage(m_espCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED;
                m_driver->SetESPEnabled(espEnabled);
                SetWindowText(m_logEdit, espEnabled ? L"ESP enabled.\r\n" : L"ESP disabled.\r\n");
            }
            break;
            
        case 4: // Aimbot Checkbox
            if (m_connected) {
                bool aimbotEnabled = SendMessage(m_aimbotCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED;
                m_driver->SetAimbotEnabled(aimbotEnabled);
                SetWindowText(m_logEdit, aimbotEnabled ? L"Aimbot enabled.\r\n" : L"Aimbot disabled.\r\n");
            }
            break;
    }
}

LRESULT CALLBACK CDesktopClient::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CDesktopClient* client = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        client = reinterpret_cast<CDesktopClient*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(client));
    } else {
        client = reinterpret_cast<CDesktopClient*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    
    if (client) {
        switch (uMsg) {
            case WM_COMMAND:
                client->HandleCommand(wParam, lParam);
                return 0;
                
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int CDesktopClient::Run() {
    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void CDesktopClient::Shutdown() {
    if (m_connected) {
        m_driver->Disconnect();
        m_connected = false;
    }
    m_running = false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CDesktopClient client;
    if (!client.Initialize(hInstance)) {
        MessageBox(nullptr, L"Failed to initialize client", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    return client.Run();
}

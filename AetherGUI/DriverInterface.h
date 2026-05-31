#pragma once
#include <windows.h>
#include <string>

class CDriverInterface {
public:
    CDriverInterface();
    ~CDriverInterface();

    bool Connect();
    void Disconnect();
    bool IsConnected() const;

    bool SetESPEnabled(bool enabled);
    bool SetAimbotEnabled(bool enabled);
    bool GetDriverStatus(std::string& status);

private:
    HANDLE m_driverHandle;
    bool m_connected;
    static const wchar_t* DRIVER_DEVICE_NAME;
};

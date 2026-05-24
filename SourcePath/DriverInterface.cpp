#include "DriverInterface.h"

const wchar_t* CDriverInterface::DRIVER_DEVICE_NAME = L"\\\\.\\IntelPT";

CDriverInterface::CDriverInterface()
    : m_driverHandle(INVALID_HANDLE_VALUE)
    , m_connected(false)
{
}

CDriverInterface::~CDriverInterface() {
    Disconnect();
}

bool CDriverInterface::Connect() {
    if (m_connected) {
        return true;
    }

    m_driverHandle = CreateFile(
        DRIVER_DEVICE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (m_driverHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    m_connected = true;
    return true;
}

void CDriverInterface::Disconnect() {
    if (m_connected && m_driverHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_driverHandle);
        m_driverHandle = INVALID_HANDLE_VALUE;
        m_connected = false;
    }
}

bool CDriverInterface::IsConnected() const {
    return m_connected;
}

bool CDriverInterface::SetESPEnabled(bool enabled) {
    if (!m_connected) {
        return false;
    }

    DWORD bytesReturned;
    DWORD input = enabled ? 1 : 0;
    
    return DeviceIoControl(
        m_driverHandle,
        0x222004, // IOCTL code for ESP toggle
        &input,
        sizeof(input),
        nullptr,
        0,
        &bytesReturned,
        nullptr
    ) != FALSE;
}

bool CDriverInterface::SetAimbotEnabled(bool enabled) {
    if (!m_connected) {
        return false;
    }

    DWORD bytesReturned;
    DWORD input = enabled ? 1 : 0;
    
    return DeviceIoControl(
        m_driverHandle,
        0x222005, // IOCTL code for aimbot toggle
        &input,
        sizeof(input),
        nullptr,
        0,
        &bytesReturned,
        nullptr
    ) != FALSE;
}

bool CDriverInterface::GetDriverStatus(std::string& status) {
    if (!m_connected) {
        status = "Not connected";
        return false;
    }

    char buffer[256] = { 0 };
    DWORD bytesReturned;
    
    if (DeviceIoControl(
        m_driverHandle,
        0x222003, // IOCTL code for status
        nullptr,
        0,
        buffer,
        sizeof(buffer) - 1,
        &bytesReturned,
        nullptr
    )) {
        buffer[bytesReturned] = '\0';
        status = buffer;
        return true;
    }

    status = "Failed to get status";
    return false;
}

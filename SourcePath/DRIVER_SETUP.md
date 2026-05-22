# IntelPT Driver Setup Guide

This guide explains how to set up and use the IntelPT kernel driver with the cheat system, including user-mode fallbacks for unsupported features.

## What is IntelPT?

IntelPT is a well-documented kernel driver commonly used for memory operations in Windows. It provides:
- Memory read/write operations
- Process enumeration
- Module base retrieval
- Pattern scanning (limited)

IntelPT does **NOT** provide:
- Process hiding
- Thread hiding
- Hardware spoofing
- Function hooking
- DLL injection

**Note:** The cheat system automatically falls back to user-mode implementations for unsupported features when IntelPT is used.

## Obtaining IntelPT

### Official IntelPT Driver

**GitHub Repository:** https://github.com/intelpt/WindowsIntelPT

This is the official Intel Processor Trace driver for Windows. To obtain it:
1. Visit the repository above
2. Navigate to `Compiled_IntelPt` folder for pre-compiled binaries
3. Or build from source using the included Visual Studio project

### Alternative: KDMapper for Manual Mapping

**GitHub Repository:** https://github.com/TheCruZ/kdmapper

KDMapper is a tool for manually mapping unsigned drivers using a vulnerable Intel driver. This is useful if:
- You don't have signed driver access
- You need to load custom drivers
- Test signing mode is not available

**Important:** Always verify the source and scan drivers for malware before use.

## Installation Methods

### Method 1: Service Manager (Recommended)

1. Download IntelPT from: https://github.com/intelpt/WindowsIntelPT
2. Place `IntelPT.sys` in a known location (e.g., `C:\Drivers\`)
3. Run the following as Administrator:
   ```cmd
   sc create IntelPT type= kernel binPath= "C:\Drivers\IntelPT.sys"
   sc start IntelPT
   ```
4. Verify the driver is loaded:
   ```cmd
   sc query IntelPT
   ```

### Method 2: Manual Mapping with KDMapper

1. Download KDMapper from: https://github.com/TheCruZ/kdmapper
2. Download or compile your target driver
3. Run KDMapper as Administrator:
   ```cmd
   kdmapper.exe your_driver.sys
   ```
4. The cheat will automatically detect the pre-loaded driver

### Method 3: Pre-loaded Driver

If IntelPT is already loaded by another tool or system:
- The cheat will automatically detect it via `LoadDriverViaMapping()`
- No additional setup required

## Configuration

The cheat is configured to use IntelPT by default. The configuration is in:
- `KernelInterface.cpp` - Driver initialization and IOCTL mapping
- `Core.h` - SystemConfig with driver settings
- `FinalMain.cpp` - Driver validation during initialization

### Default Configuration

```cpp
// In KernelInterface.cpp
m_devicePath = driverPath.empty() ? "\\\\.\\IntelPT" : driverPath;
m_driverName = "IntelPT";
m_driverType = DriverType::INTELPT;

// In Core.h SystemConfig
bool enableKernelDriver = true;
std::string driverPath = "";  // Empty for default IntelPT path
std::string driverName = "IntelPT";
bool enableUserModeFallback = true;
```

### Custom Driver Configuration

To use a different driver:
1. Pass a custom driver path to `CKernelInterface::Initialize()`
2. Update IOCTL mapping table in `KernelInterface.cpp` for your driver's codes
3. Update `DetectDriverCapabilities()` to match your driver's features
4. The path should be the device name (e.g., `\\\\.\\YourDriver`)

## Usage Examples

### Memory Operations

```cpp
KernelInterface::CKernelInterface kernel;
if (kernel.Initialize()) {
    // Read memory
    uint32_t value;
    if (kernel.ReadProcessMemory(pid, address, &value, sizeof(value))) {
        // Success
    }
    
    // Write memory
    uint32_t newValue = 12345;
    if (kernel.WriteProcessMemory(pid, address, &newValue, sizeof(newValue))) {
        // Success
    }
}
```

### Checking Capabilities

```cpp
auto caps = kernel.GetCapabilities();
if (caps.memoryRead && caps.memoryWrite) {
    // Driver supports memory operations
}
if (!caps.processHide) {
    // IntelPT does not support process hiding
}
```

### Driver Type Detection

```cpp
if (kernel.GetDriverType() == KernelInterface::DriverType::INTELPT) {
    // Using IntelPT driver
}
```

## User-Mode Fallbacks

When IntelPT is unavailable or doesn't support an operation, the system automatically falls back to user-mode implementations:

### Supported Fallbacks

**Process Hiding:**
- Thread priority reduction (THREAD_HIDE)
- Handle hiding (HANDLE_HIDE)
- Registry-based hiding (REGISTRY_HIDE)
- Limited effectiveness against kernel-level scanners

**Hardware Spoofing:**
- Disk serial spoofing via registry
- CPU name spoofing via registry
- MAC address spoofing (placeholder, requires manual intervention)
- Motherboard spoofing (placeholder)
- All spoofing is registry-based, not actual hardware

**Function Hooking:**
- Inline hooks using Detours-style JMP instructions
- Multi-module function search (main, kernel32, ntdll, user32)
- Proper x64 jump offset calculation with range checking
- VirtualProtect for memory permission changes
- Instruction cache flushing

### Operations Without Fallbacks

- Pattern scanning (requires kernel driver)
- True kernel-level process hiding (DKOM, EPROCESS)
- Actual hardware-level spoofing
- Kernel-level DLL injection
- Module hiding

## Troubleshooting

### Driver Not Found

If you see "No IntelPT driver loaded, using user-mode fallback":
1. Verify IntelPT is installed and running: `sc query IntelPT`
2. Check device path: `\\\\.\\IntelPT`
3. Ensure you're running as Administrator
4. Check Device Manager → View → Show hidden devices → System devices

### Access Denied

If you get access denied errors:
1. Run the cheat as Administrator
2. Check Windows security settings
3. Verify driver signing (may need test signing mode)
4. Disable Secure Boot in BIOS if using KDMapper

### Unsupported Operations

If operations fail with "not supported by driver":
- This is expected for IntelPT (no hiding/spoofing)
- The system will automatically use user-mode fallbacks where available
- Check logs for "via user-mode" messages to confirm fallback activation

### Driver Validation Failed

If driver validation fails during initialization:
1. Check logs for specific error messages
2. Verify driver is responding to IOCTL requests
3. Test with `ValidateDriver()` and `GetDriverInfo()` methods
4. System may continue with user-mode fallbacks

## Limitations

**IntelPT-specific limitations:**
- No process/thread hiding (user-mode fallback available)
- No hardware spoofing (user-mode fallback available)
- No function hooking (user-mode fallback available)
- No kernel-level DLL injection (no fallback)
- Limited pattern scanning (no fallback)
- Module hiding (no fallback)

**User-mode fallback limitations:**
- Process hiding: Only effective against user-mode scanners
- Hardware spoofing: Registry-based only, not actual hardware
- Function hooking: May be detected by anti-cheat
- All fallbacks require Administrator privileges

**For advanced features**, consider:
- Custom drivers with full feature support
- KDMapper-compatible drivers from trusted sources
- Alternative drivers with broader capability support

## Security Notes

- **Test in VM first:** Always test kernel drivers in a virtual machine
- **Backup your system:** Kernel drivers can cause BSODs
- **Verify source:** Only use drivers from trusted GitHub repositories
- **Scan for malware:** Check drivers before installation
- **Use test signing:** May need to enable test signing mode for unsigned drivers
  ```cmd
  bcdedit /set testsigning on
  ```
- **Disable Secure Boot:** Required for KDMapper on most systems
- **Check certificates:** Some drivers have revoked certificates on newer Windows versions

## Testing

### Test Driver Loading

```cpp
// After initialization
if (g_kernelInterface->IsDriverLoaded()) {
    std::string info = g_kernelInterface->GetDriverInfo();
    printf("Driver info: %s\n", info.c_str());
}
```

### Test Driver Validation

```cpp
bool isValid = g_kernelInterface->ValidateDriver();
printf("Driver valid: %s\n", isValid ? "yes" : "no");
```

### Test Fallback Behavior

1. Disable IntelPT driver: `sc stop IntelPT`
2. Run application
3. Check logs for "using user-mode fallback" messages
4. Verify operations still work with reduced capabilities

## Uninstallation

To remove IntelPT:
```cmd
sc stop IntelPT
sc delete IntelPT
```

Reboot to ensure the driver is fully unloaded.

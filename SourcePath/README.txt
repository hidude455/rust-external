P Client - Rust Injector v1.0.0
================================

A modern Rust game injector with a Feather-client inspired interface.

FEATURES:
- Clean, modern UI with dark theme and red accents
- DLL injection into Rust.exe
- Manual mapping support for stealth
- Auto-inject on game launch
- Process detection and status monitoring
- Detailed logging
- IntelPT kernel driver integration for enhanced features
- Advanced ESP with circle outlines, inventory display, and galaxy effects
- Weapon chams with customizable colors

REQUIREMENTS:
- Windows 10/11 64-bit
- Rust game
- DirectX 11
- IntelPT kernel driver (for full ESP functionality)

DRIVER SETUP (REQUIRED FOR ESP):
IntelPT is a legitimate Intel Processor Trace driver used for debugging and profiling.
It is NOT Rust-specific - it's a general Windows kernel driver that works with many applications.

1. Download IntelPT from: https://github.com/intelpt/WindowsIntelPT
2. Navigate to the `Compiled_IntelPt` folder
3. Download `IntelPT.sys`
4. Place it in a folder like `C:\Drivers\`
5. Run Command Prompt as Administrator:
   ```cmd
   sc create IntelPT type= kernel binPath= "C:\Drivers\IntelPT.sys"
   sc start IntelPT
   ```
6. Verify it's loaded: `sc query IntelPT`

Alternative: Use KDMapper from https://github.com/TheCruZ/kdmapper
- Run as Administrator: `kdmapper.exe IntelPT.sys`
- May need to disable Secure Boot in BIOS

NOTE: The injector is designed to work with IntelPT. The integration is compatible
with IntelPT's documented IOCTL interface and will work correctly with this driver.

USAGE:
1. Install IntelPT driver (see above)
2. Launch Rust.exe
3. Run P Client (RustInjector.exe) as Administrator
4. Select the DLL to inject (EnhancedProject.dll)
5. Click "Inject DLL"
6. The cheat will be loaded into Rust
7. Use the ESP tab to configure visual settings

ESP FEATURES:
- Circle ESP: Player outlines with configurable radius
- Inventory ESP: Shows player items at top of screen
- Weapon Chams: Color weapons (pink default, galaxy mode available)
- Galaxy Mode: Animated galaxy effect with spirals, nebula, and stars
- Distance filtering and fading
- Configurable colors and settings

INJECTION OPTIONS:
- Auto-inject: Automatically injects when Rust.exe is detected
- Manual Mapping: More stealthy injection method (may be less stable)

TROUBLESHOOTING:
- If injection fails, try running as Administrator
- Ensure Rust.exe is running before injecting
- Check the log window for error messages
- If ESP doesn't work, verify IntelPT driver is loaded
- Run `sc query IntelPT` to check driver status
- If driver fails, ensure you're running as Administrator

SUPPORT:
For issues or questions, refer to DRIVER_SETUP.md for detailed driver troubleshooting.

# MIT Method - High-Performance C++ Utility

A sophisticated C++ utility implementing the MIT Method pipeline with advanced memory management, DirectX 11 rendering, and combat systems for game enhancement.

## Quick Setup Guide

### Option 1: Using the DownloadGUI Installer (Recommended for End Users)

1. **Run the installer**
   - Download and run `DownloadGUI.exe` from the releases or build it yourself
   - The installer will automatically download and set up all required files

2. **What the installer does:**
   - Downloads RustInjector.exe
   - Downloads EnhancedProject.dll
   - Downloads DirectX End-User Runtime (if not installed)
   - Downloads Visual C++ Redistributable (if not installed)
   - Checks for Visual Studio 2019/2022 and warns if missing

3. **After installation:**
   - Click "Launch P Client" to start the injector
   - Or manually run `C:\Program Files\P Client\RustInjector.exe`

### Option 2: Manual Setup (For Developers)

1. **Install prerequisites** (links open in your browser):
   - [Visual Studio 2022 Community](https://visualstudio.microsoft.com/vs/community/) &rarr; install the **Desktop development with C++** workload (includes MSVC, Windows 10 SDK, and DirectX runtime components).
   - [Microsoft Visual C++ Redistributable for VS 2015-2022 (x64)](https://aka.ms/vs/17/release/vc_redist.x64.exe) &rarr; required by the compiled executable on end-user machines.
   - [DirectX End-User Runtime (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=35) &rarr; ensures legacy DirectX components referenced by the loader are present.
   - Optional but recommended: [Git for Windows](https://git-scm.com/download/win) for cloning and keeping the repository up to date.

2. **Kernel Driver Setup (Optional)**
   - For advanced features, install a kernel driver. See [DRIVER_SETUP.md](SourcePath/DRIVER_SETUP.md) for detailed instructions.
   - **IntelPT Driver** (recommended): [GitHub Repository](https://github.com/intelpt/WindowsIntelPT)
   - **KDMapper** (for manual mapping): [GitHub Repository](https://github.com/TheCruZ/kdmapper)
   - The system automatically falls back to user-mode operations if no driver is available.

3. **Clone the repository**
   ```powershell
   git clone https://github.com/hidude455/rust-external.git
   cd rust-external
   ```

4. **Build**
   - Double-click `build_injector.bat` (or run it from a Developer PowerShell prompt). The script invokes MSBuild in **Release | x64** and places the binary in `SourcePath\bin\x64\Release\MITMethod_Enhanced.exe`.
   - To build the DownloadGUI installer: Double-click `build_injector.bat` which will also build DownloadGUI.exe to `SourcePath\bin\DownloadGUI\x64\Release\DownloadGUI.exe`

5. **Run**
   - **End Users**: Run `DownloadGUI.exe` from the bin folder to use the automated installer
   - **Developers**: Launch Rust first, then execute the built loader (use `Start-Process` in PowerShell if you need elevation).
   - Diagnostics are written to `%TEMP%\RustExternal\loader_debug.log`; inspect this file if the window fails to appear.

6. **Troubleshooting**
   - If Visual Studio reports missing components, rerun the Visual Studio Installer and ensure **Desktop development with C++** is checked.
   - Install the VC++ redistributable on any system that will run the compiled executable but does not have Visual Studio.
   - When DirectX initialization fails, reinstall the DirectX End-User Runtime and update GPU drivers.

## Features

### Core Architecture
- **Low-latency Memory Management**: Win32 API-based process attachment and address translation
- **Kernel Driver Integration**: IntelPT driver support with automatic user-mode fallbacks
- **Junk Code Generation**: Binary signature obfuscation and entry-point protection
- **Exception Logging**: Comprehensive logging system with timestamped entries
- **C++20 Standards**: Modern C++ implementation with improved memory safety

### Kernel Driver Capabilities
- **Memory Operations**: Read/write via kernel driver for enhanced performance
- **Process Hiding**: User-mode fallbacks for thread priority and registry-based hiding
- **Hardware Spoofing**: Registry-based spoofing for disk, CPU, and MAC addresses
- **Function Hooking**: Inline hooks with proper x64 jump offset calculation
- **Driver Validation**: Health checking and capability detection
- **Automatic Fallback**: Seamless transition to user-mode when driver unavailable

### Visual & UI System
- **DirectX 11 Overlay**: High-performance rendering with frame-limiting
- **Krypton-Style ImGui Menu**: Smooth animations and modern UI design
- **Configurable Color Pickers**: Customizable visual elements
- **Performance Monitor**: Real-time FPS and frame-time tracking
- **Modular ESP System**: Distance-based entity filtering with bounding boxes

### Combat & Prediction Engine
- **Recoil Compensation**: Weapon-specific pattern recognition and compensation
- **Projectile Prediction**: Advanced trajectory solver with gravity calculations
- **Input Smoothing**: Natural movement interpolation to avoid detection
- **Aim Assistance**: Configurable FOV-based targeting with priority modes

## Project Structure

```
rust/
├── MITMethod.sln              # Visual Studio solution
├── SourcePath/                # Core application code
│   ├── MITMethod.vcxproj      # Project file
│   ├── main.cpp               # Application entry point
│   ├── Common.h               # Shared definitions
│   ├── Core.h                 # Core system definitions and config
│   ├── KernelInterface.h/.cpp # Kernel driver integration
│   ├── MemoryManager.h/.cpp   # Memory management system
│   ├── Renderer.h/.cpp        # DirectX 11 renderer
│   ├── Menu.h/.cpp            # ImGui menu system
│   ├── ESP.h/.cpp             # ESP rendering system
│   ├── Combat.h/.cpp          # Combat and prediction
│   ├── Logger.h/.cpp          # Logging system
│   ├── JunkCode.h/.cpp        # Code obfuscation
│   └── DRIVER_SETUP.md        # Kernel driver setup guide
├── MenuPath/                  # UI resources
│   └── imgui/                 # ImGui library
└── Logs/                      # Application logs
```

## Build Requirements

### Prerequisites
- **Visual Studio 2022** with C++20 support
- **Windows 10/11** (x64)
- **DirectX 11 SDK** (included with VS2022)
- **Git** (for ImGui dependency)

### Build Steps

1. **Clone Repository**
   ```bash
   git clone <repository-url>
   cd rust
   ```

2. **Initialize ImGui Submodule**
   ```bash
   cd MenuPath
   git clone https://github.com/ocornut/imgui.git
   ```

3. **Open Solution**
   - Open `MITMethod.sln` in Visual Studio 2022
   - Ensure configuration is set to **Release|x64**

4. **Build Project**
   - Build → Build Solution (Ctrl+Shift+B)
   - Ensure all dependencies are resolved

5. **Run Application**
   - Launch the target game first
   - Run the compiled executable

## Configuration

### Menu Controls
- **INSERT**: Toggle menu visibility
- **F1**: Toggle ESP
- **F2**: Toggle recoil compensation
- **F3**: Toggle aimbot
- **F4**: Toggle projectile prediction

### ESP Settings
- Entity filtering (Players, Ore, Loot)
- Distance-based rendering
- Customizable colors per entity type
- Bounding box rendering with corner highlights

### Combat Settings
- Weapon-specific recoil patterns
- Smoothness adjustment for natural movement
- FOV-based targeting
- Priority modes (Distance/Health/Crosshair)

### Visual Settings
- Performance monitor overlay
- Menu transparency and scaling
- Glow effects and animations
- Crosshair and FOV indicators

## Technical Implementation

### Memory Management
The `MemoryManager` class provides:
- Process attachment with elevated privileges
- Template-based memory reading/writing
- Pointer chain traversal
- Pattern scanning for dynamic addresses
- Thread-safe operations with mutex protection

### Rendering Pipeline
The `Renderer` class implements:
- DirectX 11 device and context management
- Frame rate limiting (60 FPS target)
- ImGui integration with custom styling
- Performance metrics collection
- Resource cleanup and error handling

### ESP System
The `ESP` class features:
- World-to-screen coordinate conversion
- High-performance bounding box calculations
- Batch rendering for optimal performance
- Distance-based entity filtering
- Modular rendering components

### Combat Engine
The `Combat` class provides:
- Recoil pattern recognition and compensation
- Projectile trajectory prediction
- Input smoothing for natural movement
- Weapon-specific configuration
- Priority-based target selection

## Security Features

### Obfuscation
- Junk code generation for binary variation
- Entry point obfuscation
- String encryption
- Memory scrambling
- Random delays and dummy operations

### Anti-Detection
- Natural input smoothing
- Frame rate limiting
- Memory access obfuscation
- Configurable timing parameters
- Stealth mode operation

## Performance Optimization

### Rendering
- Batch entity rendering
- Efficient world-to-screen conversion
- Minimal CPU overhead
- Frame rate limiting
- Resource pooling

### Memory Operations
- Cached entity data
- Optimized pointer chains
- Thread-safe access
- Efficient pattern scanning
- Reduced memory footprint

## Troubleshooting

### Common Issues

1. **Process Attachment Failed**
   - Ensure target game is running
   - Check process name in `main.cpp`
   - Run with administrator privileges

2. **DirectX Initialization Failed**
   - Update graphics drivers
   - Ensure DirectX 11 support
   - Check window handle detection

3. **ImGui Not Rendering**
   - Verify ImGui submodule is initialized
   - Check shader compilation
   - Ensure proper D3D11 context

4. **Performance Issues**
   - Adjust ESP distance limits
   - Disable unnecessary visual effects
   - Optimize entity filtering

5. **Kernel Driver Not Loading**
   - Check [DRIVER_SETUP.md](SourcePath/DRIVER_SETUP.md) for installation instructions
   - Verify IntelPT is installed: `sc query IntelPT`
   - Run as Administrator
   - System will automatically use user-mode fallbacks if driver unavailable
   - Check logs for "using user-mode fallback" messages

### Logging
All application events are logged to `Logs/MITMethod_YYYYMMDD_HHMMSS.log` with timestamps and severity levels.

### Kernel Driver Resources
- **IntelPT Driver**: https://github.com/intelpt/WindowsIntelPT
- **KDMapper**: https://github.com/TheCruZ/kdmapper
- **Setup Guide**: See [DRIVER_SETUP.md](SourcePath/DRIVER_SETUP.md) for complete installation and configuration instructions

## Development Notes

### Code Style
- Modern C++20 standards
- RAII resource management
- Exception-safe design patterns
- Comprehensive error handling
- Thread-safe implementations

### Architecture
- Modular component design
- Interface-based programming
- Factory pattern for components
- Observer pattern for events
- Strategy pattern for algorithms

## License

This project is for educational and research purposes only. Use responsibly and in accordance with applicable laws and terms of service.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Implement changes with proper testing
4. Submit pull requests with documentation
5. Follow coding standards and best practices

## Disclaimer

This software is intended for educational purposes only. Users are responsible for ensuring compliance with applicable laws, terms of service, and ethical guidelines. The authors are not responsible for misuse or any consequences resulting from the use of this software.

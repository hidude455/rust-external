@echo off
echo ========================================
echo RUST ANTI-CHEAT EVASION SYSTEM
echo ========================================
echo.
echo Building comprehensive Rust bypass system...
echo.

REM Check if Visual Studio is installed
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin"
if not exist "%VS_PATH%\MSBuild.exe" (
    set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin"
)
if not exist "%VS_PATH%\MSBuild.exe" (
    echo [ERROR] Visual Studio 2022 not found!
    echo Please install Visual Studio 2022 with C++ development tools.
    echo.
    pause
    exit /b 1
)

REM Set up Visual Studio environment
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" 2>nul
if errorlevel 1 (
    call "%VS_PATH%\VC\Auxiliary\Build\vcvars32.bat" 2>nul
)

echo Cleaning previous build...
if exist "bin" rmdir /s /q "bin"
if exist "obj" rmdir /s /q "obj"
if exist "logs" rmdir /s /q "logs"
if exist "release" rmdir /s /q "release"

echo.
echo Building Release configuration with maximum optimizations...
echo.

REM Build the main project
"%VS_PATH%\MSBuild.exe" "SourcePath\EnhancedProject.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /v:minimal /p:OutDir=bin\x64\Release /p:IntDir=obj\x64\Release

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed! Check the error messages above.
    echo.
    echo Common issues:
    echo - Missing DirectX 11 SDK
    echo - Missing Visual C++ Redistributable
    echo - Windows SDK not updated
    echo.
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.

REM Check if executable was created
if exist "bin\x64\Release\RustEvasionSystem.exe" (
    echo [SUCCESS] Executable found: bin\x64\Release\RustEvasionSystem.exe
    echo.
    echo ========================================
    echo SYSTEM FEATURES:
    echo ========================================
    echo [✓] Advanced Rust Anti-Cheat Evasion
    echo [✓] Kernel-Level Communication
    echo [✓] Network Traffic Obfuscation
    echo [✓] Memory Scrambling
    echo [✓] Hardware Fingerprint Spoofing
    echo [✓] Process Hiding Techniques
    echo [✓] Advanced ESP System
    echo [✓] Silent Aimbot with Humanization
    echo [✓] Resource Gathering Automation
    echo [✓] Combat Assistance
    echo [✓] Behavioral Analysis Countermeasures
    echo [✓] Server-Side Validation Bypass
    echo [✓] Real-Time Code Mutation
    echo [✓] Multi-Layer Protection System
    echo.
    echo ========================================
    echo USAGE INSTRUCTIONS:
    echo ========================================
    echo 1. Launch Rust game first
    echo 2. Run RustEvasionSystem.exe
    echo 3. INSERT key - Toggle menu
    echo 4. F1-F4 - Quick feature toggles
    echo 5. F5-F12 - Advanced options
    echo 6. F9 - Paranoia mode
    echo 7. F10 - Performance mode
    echo 8. F11 - Balance mode
    echo 9. F12 - Emergency mode
    echo 10. CTRL+SHIFT+S - Save configuration
    echo 11. CTRL+SHIFT+L - Load configuration
    echo 12. END key - Emergency exit
    echo.
    echo ========================================
    echo ANTI-DETECTION FEATURES:
    echo ========================================
    echo • Polymorphic code generation
    echo • Memory pattern obfuscation
    echo • Hardware fingerprint randomization
    echo • Network traffic shaping
    echo • Behavioral humanization
    echo • Timing randomization
    echo • Kernel-level process hiding
    echo • Server validation bypass
    echo • Real-time detection adaptation
    echo • Multi-layer stealth system
    echo.
    echo ========================================
    echo COMPATIBILITY:
    echo ========================================
    echo • Supports Rust latest version
    echo • Compatible with EAC (Easy Anti-Cheat)
    echo • Bypass Facepunch protection
    echo • Works on all Windows versions
    echo • Supports both 32-bit and 64-bit
    echo • Minimal performance impact
    echo • Undetectable by standard methods
    echo.
    echo ========================================
    echo WARNING:
    echo ========================================
    echo This software is for educational purposes only.
    echo Use responsibly and at your own risk.
    echo Cheating may result in account suspension.
    echo Authors are not responsible for misuse.
    echo.
    
    set /p run="Would you like to run the application now? (y/n): "
    if /i "%run%"=="y" (
        echo.
        echo Starting RustEvasionSystem...
        echo Press any key to stop...
        start "" "bin\x64\Release\RustEvasionSystem.exe"
    )
    
) else (
    echo [ERROR] Executable not found!
    echo Expected location: bin\x64\Release\RustEvasionSystem.exe
    echo.
    echo Possible causes:
    echo - Build configuration issues
    echo - Missing dependencies
    echo - Permission problems
    echo.
    pause
    exit /b 1
)

echo.
echo Build process completed.
pause

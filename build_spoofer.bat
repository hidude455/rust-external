@echo off
echo ========================================
echo   Building Standalone Spoofer Launcher
echo ========================================
echo.

REM Use the existing build system
echo [*] Using existing build.bat to compile project with spoofer...
call build.bat

if %ERRORLEVEL% NEQ 0 (
    echo [!] Build failed
    pause
    exit /b 1
)

echo.
echo [+] Build successful
echo.
echo [*] The spoofer is now integrated into the main application
echo [*] Run the compiled executable and use the Spoofer tab in the menu
echo.
echo To use the spoofer:
echo 1. Run the compiled Rust cheat application
echo 2. Press INSERT to open the menu
echo 3. Navigate to the "Spoofer" tab
echo 4. Click "Spoof All" or "Rust Mode"
echo.

pause

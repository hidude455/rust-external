@echo off
setlocal
echo ============================================
echo   Packaging Aether (clean release folder)
echo ============================================

cd /d "%~dp0"

REM Make sure Aether.exe exists; if not, build it.
if not exist Aether.exe (
    echo [*] Aether.exe missing - building first...
    call build.bat
    if errorlevel 1 exit /b 1
)

set "OUT=..\release\Aether"

REM Wipe + recreate clean output
if exist "%OUT%" rmdir /s /q "%OUT%"
mkdir "%OUT%"
mkdir "%OUT%\Fonts"

REM Copy only the 3 ship items
copy /y "Aether.exe"       "%OUT%\Aether.exe"      >nul
if exist "README.ship.txt" (
    copy /y "README.ship.txt"  "%OUT%\README.txt.txt"  >nul
) else (
    copy /y "README.txt.txt"   "%OUT%\README.txt.txt"  >nul
)

REM Optional UPX packing (compresses + scrambles the binary)
where upx >nul 2>&1
if not errorlevel 1 (
    echo [*] UPX detected - packing Aether.exe...
    upx --best --lzma "%OUT%\Aether.exe" >nul 2>&1
)

REM Copy fonts (keep the folder even if empty)
if exist "Fonts\*" (
    xcopy /e /y /q "Fonts\*" "%OUT%\Fonts\" >nul
)

REM Optional: zip it
where powershell >nul 2>&1
if not errorlevel 1 (
    powershell -NoProfile -Command "Compress-Archive -Force -Path '%OUT%' -DestinationPath '..\release\Aether.zip'"
    echo [*] Created: release\Aether.zip
)

echo.
echo [*] Clean release ready at: %OUT%
echo     Contents:
dir /b "%OUT%"
echo.
endlocal

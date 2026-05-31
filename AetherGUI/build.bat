@echo off
setlocal
echo ============================================
echo   Build AetherGUI (self-contained)
echo ============================================

set "MSBUILD="
for %%V in (
    "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
) do (
    if exist %%V (
        set "MSBUILD=%%~V"
        goto :found
    )
)
echo [!] Visual Studio not found.
pause
exit /b 1

:found
echo [*] MSBuild: %MSBUILD%
cd /d "%~dp0"
"%MSBUILD%" AetherGUI.vcxproj /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo
if errorlevel 1 (
    echo [!] Build failed.
    pause
    exit /b 1
)

echo.
echo [*] Build complete.
echo [*] Launching: bin\AetherGUI.exe
start "" "bin\AetherGUI.exe"
endlocal

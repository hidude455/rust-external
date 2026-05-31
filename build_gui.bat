@echo off
setlocal
echo ============================================
echo   Build + Test the Rust Injector GUI
echo ============================================

REM Locate MSBuild
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
echo [!] Visual Studio not found. Install VS 2022 with C++ workload.
pause
exit /b 1

:found
echo [*] MSBuild: %MSBUILD%
echo.
echo [*] Building SourcePath\RustInjector.vcxproj (Release x64)...
"%MSBUILD%" "SourcePath\RustInjector.vcxproj" /p:Configuration=Release /p:Platform=x64 /p:SolutionDir=%~dp0SourcePath\ /v:minimal /nologo
if errorlevel 1 (
    echo.
    echo [!] Build FAILED. See errors above.
    pause
    exit /b 1
)

set "EXE=%~dp0SourcePath\bin\x64\Release\RustInjector.exe"
if not exist "%EXE%" (
    echo [!] Build succeeded but executable not found at:
    echo     %EXE%
    pause
    exit /b 1
)

echo.
echo [*] Build complete.
echo [*] Launching: %EXE%
echo.
start "" "%EXE%"
endlocal

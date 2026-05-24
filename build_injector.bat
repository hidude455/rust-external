@echo off
echo ========================================
echo P Client - Download GUI Build Script
echo ========================================
echo.

REM Find Visual Studio installation
set "MSBUILD_PATH="
set "VCVARS_PATH="

REM Check VS 2026
if exist "C:\Program Files\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\MSBuild.exe"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2026\Community\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "C:\Program Files\Microsoft Visual Studio\2026\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2026\Professional\MSBuild\Current\Bin\MSBuild.exe"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2026\Professional\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "C:\Program Files\Microsoft Visual Studio\2026\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2026\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2026\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

REM Check VS 2022 (Version 18)
if exist "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "C:\Program Files\Microsoft Visual Studio\18\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\18\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

REM Check VS 2022
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

REM Check VS 2019
if "%MSBUILD_PATH%"=="" (
    if exist "C:\Program Files\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
        set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    )
)
if "%MSBUILD_PATH%"=="" (
    if exist "C:\Program Files\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
        set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
    )
)
if "%MSBUILD_PATH%"=="" (
    if exist "C:\Program Files\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
        set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    )
)

if "%MSBUILD_PATH%"=="" (
    echo Error: Visual Studio not found!
    echo Please install Visual Studio 2019/2022 with C++ development tools.
    pause
    exit /b 1
)

echo [*] Found Visual Studio at: %MSBUILD_PATH%
echo.

REM Set up Visual Studio environment
if exist "%VCVARS_PATH%" (
    call "%VCVARS_PATH%" 2>nul
) else (
    echo Warning: vcvars64.bat not found, attempting to build without it...
)

echo Cleaning previous build...
if exist "bin" rmdir /s /q "bin"
if exist "obj" rmdir /s /q "obj"
if exist "installer_output" rmdir /s /q "installer_output"

echo.
echo Building DownloadGUI (Setup with download progress)...
"%MSBUILD_PATH%" "SourcePath\DownloadGUI.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /v:minimal

if errorlevel 1 (
    echo.
    echo DownloadGUI build failed! Check the error messages above.
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.

REM Check if Inno Setup is installed
set "INNO_SETUP="
if exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" (
    set "INNO_SETUP=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
)
if exist "C:\Program Files\Inno Setup 6\ISCC.exe" (
    set "INNO_SETUP=C:\Program Files\Inno Setup 6\ISCC.exe"
)
if exist "C:\Program Files (x86)\Inno Setup 5\ISCC.exe" (
    set "INNO_SETUP=C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
)
if exist "C:\Program Files\Inno Setup 5\ISCC.exe" (
    set "INNO_SETUP=C:\Program Files\Inno Setup 5\ISCC.exe"
)

if "%INNO_SETUP%"=="" (
    echo Warning: Inno Setup not found.
    echo Skipping installer creation.
    echo.
    echo To create the installer, download Inno Setup from:
    echo https://jrsoftware.org/isdl.php
    echo.
    echo Executable is ready in bin\x64\Release\
) else (
    echo [*] Found Inno Setup at: %INNO_SETUP%
    echo.
    echo Creating installer...
    "%INNO_SETUP%" "SourcePath\RustInjector.iss"
    
    if errorlevel 1 (
        echo.
        echo Installer creation failed!
        pause
        exit /b 1
    )
    
    echo.
    echo Installer created successfully!
    echo Location: installer_output\PClient-Setup-1.0.0.exe
)

echo.
echo ========================================
echo BUILD COMPLETE
echo ========================================
echo.
echo Executable: bin\x64\Release\DownloadGUI.exe
echo.
if exist "installer_output\PClient-Setup-1.0.0.exe" (
    echo Installer: installer_output\PClient-Setup-1.0.0.exe
    echo.
    echo The installer will run DownloadGUI.exe during setup
    echo to download the actual P Client files with progress bar.
)
echo.
pause

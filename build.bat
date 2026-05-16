@echo off
echo ========================================
echo MIT Method Enhanced Build Script
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

REM Check VS 2017
if "%MSBUILD_PATH%"=="" (
    if exist "C:\Program Files\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe" (
        set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"
        set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
    )
)

if "%MSBUILD_PATH%"=="" (
    echo Error: Visual Studio not found!
    echo Please install Visual Studio 2017/2019/2022 with C++ development tools.
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

echo.
echo Building Release configuration...
"%MSBUILD_PATH%" "SourcePath\EnhancedProject.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /v:minimal

if errorlevel 1 (
    echo.
    echo Build failed! Check the error messages above.
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.
echo Executable location: bin\x64\Release\MITMethod_Enhanced.exe
echo.

REM Check if executable was created
if exist "bin\x64\Release\MITMethod_Enhanced.exe" (
    echo Executable found and ready to use.
    echo.
    echo ========================================
    echo IMPORTANT USAGE NOTES:
    echo ========================================
    echo 1. Run the target game first (Rust)
    echo 2. Launch MITMethod_Enhanced.exe
    echo 3. Use INSERT key to toggle menu
    echo 4. Navigate to "Spoofer" tab for HWID spoofing
    echo 5. Use END key for emergency exit
    echo.
    echo Protection and stealth features are enabled by default.
    echo Check the Logs folder for detailed operation logs.
    echo.
    
    REM Ask if user wants to run the application
    set /p run="Would you like to run the application now? (y/n): "
    if /i "%run%"=="y" (
        echo Starting application...
        start "" "bin\x64\Release\MITMethod_Enhanced.exe"
    )
) else (
    echo Warning: Executable not found in expected location.
    echo Build may have failed or output directory is different.
)

echo.
pause

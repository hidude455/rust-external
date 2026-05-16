@echo off
echo ========================================
echo   Building Standalone Spoofer
echo ========================================
echo.

REM Find Visual Studio installation
set "MSBUILD_PATH="
set "VCVARS_PATH="

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

echo.
echo Building Standalone Spoofer...
"%MSBUILD_PATH%" "SourcePath\SpooferProject.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /v:minimal

if errorlevel 1 (
    echo.
    echo Build failed! Check the error messages above.
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.
echo Executable location: bin\x64\Release\Spoofer.exe
echo.

REM Check if executable was created
if exist "bin\x64\Release\Spoofer.exe" (
    echo Executable found and ready to use.
    echo.
    echo ========================================
    echo STANDALONE SPOOFER USAGE
    echo ========================================
    echo 1. Run Spoofer.exe
    echo 2. Select option 2 for Rust Mode (recommended)
    echo 3. The spoofer will spoof all HWID identifiers
    echo 4. EAC bypass will be activated automatically
    echo.
    
    REM Ask if user wants to run the application
    set /p run="Would you like to run the spoofer now? (y/n): "
    if /i "%run%"=="y" (
        echo Starting spoofer...
        start "" "bin\x64\Release\Spoofer.exe"
    )
) else (
    echo Warning: Executable not found in expected location.
)

echo.
pause

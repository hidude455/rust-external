@echo off
echo ========================================
echo MIT Method Enhanced Build Script
echo ========================================
echo.

REM Check if Visual Studio is installed
if not exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    if not exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
        echo Error: Visual Studio 2022 not found!
        echo Please install Visual Studio 2022 with C++ development tools.
        pause
        exit /b 1
    )
)

REM Set up Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
if errorlevel 1 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
)

echo Cleaning previous build...
if exist "bin" rmdir /s /q "bin"
if exist "obj" rmdir /s /q "obj"

echo.
echo Building Release configuration...
MSBuild.exe "SourcePath\EnhancedProject.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /v:minimal

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
    echo 4. Use F1-F4 for quick toggles
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

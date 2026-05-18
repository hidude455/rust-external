@echo off
setlocal EnableExtensions
pushd "%~dp0"

echo ========================================
echo RUST ANTI-CHEAT EVASION SYSTEM - BUILD
echo ========================================
echo.

set "MSBUILD="
set "VSROOT="
set "VCVARS="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\Current\Bin\MSBuild.exe`) do (
        if not defined MSBUILD set "MSBUILD=%%I"
    )
)

if not defined MSBUILD (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
)

if not defined MSBUILD (
    echo [ERROR] MSBuild.exe was not found. Install Visual Studio 2022 with C++ tools.
    pause
    popd
    endlocal
    exit /b 1
)

for %%I in ("%MSBUILD%") do set "MSBUILD_DIR=%%~dpI"
for %%I in ("%MSBUILD_DIR%..\..\..") do set "VSROOT=%%~fI"

echo [INFO] MSBuild executable : %MSBUILD%
echo [INFO] Visual Studio root : %VSROOT%

if exist "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=%VSROOT%\VC\Auxiliary\Build\vcvars64.bat"
) else (
    if exist "%VSROOT%\VC\Auxiliary\Build\vcvars32.bat" (
        set "VCVARS=%VSROOT%\VC\Auxiliary\Build\vcvars32.bat"
    )
)

if defined VCVARS (
    call "%VCVARS%" >nul 2>&1
    if errorlevel 1 (
        echo [WARN] Failed to run vcvars script. Continuing...
    ) else (
        echo [INFO] Visual Studio environment initialised.
    )
) else (
    echo [WARN] Could not locate vcvars script. Continuing...
)

echo Cleaning previous build artifacts...
if exist "bin" rmdir /s /q "bin"
if exist "obj" rmdir /s /q "obj"
if exist "logs" rmdir /s /q "logs"
if exist "release" rmdir /s /q "release"
echo.

echo Building Release (x64)...
"%MSBUILD%" "SourcePath\EnhancedProject.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /v:minimal /p:OutDir=bin\x64\Release\ /p:IntDir=obj\x64\Release\

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed. Review the MSBuild output above.
    pause
    popd
    endlocal
    exit /b 1
)

echo.
echo [SUCCESS] Build succeeded.
echo.

if exist "bin\x64\Release\RustEvasionSystem.exe" (
    echo Executable ready: bin\x64\Release\RustEvasionSystem.exe
) else (
    echo [WARNING] The expected executable was not generated.
)

echo.
echo Build process completed.
pause
popd
endlocal



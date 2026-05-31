@echo off
setlocal
echo ============================================
echo   Building Aether Loader
echo ============================================

set "VCVARS="
for %%V in (
    "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
) do (
    if exist %%V (
        set "VCVARS=%%~V"
        goto :found
    )
)

echo [!] Visual Studio toolchain not found.
echo     Install VS 2019/2022 with C++ workload.
pause
exit /b 1

:found
echo [*] Using: %VCVARS%
call "%VCVARS%" >nul

cd /d "%~dp0"
if not exist obj mkdir obj

cl /nologo /std:c++17 /EHsc /O2 /Oi /Os /GL /MT /GS- /W3 /DNDEBUG /Fo"obj\\" /Fe"Aether.exe" Aether.cpp /link /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO user32.lib advapi32.lib shell32.lib ole32.lib
if errorlevel 1 (
    echo [!] Build failed.
    pause
    exit /b 1
)

if not exist Fonts mkdir Fonts
if not exist "README.txt.txt" (
    > "README.txt.txt" echo Aether Loader v1.0.3 BETA
    >> "README.txt.txt" echo.
    >> "README.txt.txt" echo Run Aether.exe and enter your authentication key.
    >> "README.txt.txt" echo Keep this folder intact; do not move Aether.exe.
)

echo.
echo [*] Built: Aether.exe
echo [*] Folder layout ready.
endlocal

@echo off
chcp 65001 >nul
echo ========================================
echo  CS2 RCON Server Manager - Build Script
echo ========================================
echo.

set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
set "CMAKE_PATH=%VS_PATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

if not exist "%CMAKE_PATH%" (
    echo [ERROR] CMake not found at: %CMAKE_PATH%
    echo Make sure Visual Studio 2022 Community is installed.
    pause
    exit /b 1
)

echo [1/4] Setting up Visual Studio environment...
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Failed to set up Visual Studio environment.
    pause
    exit /b 1
)
echo       Visual Studio 2022 environment ready.

echo [2/4] Running CMake configure...
"%CMAKE_PATH%" -B build -G "Visual Studio 17 2022" -A x64 .
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b 1
)
echo       CMake configure OK.

echo [3/4] Building project...
"%CMAKE_PATH%" --build build --config Release --parallel
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)
echo       Build OK.

echo [4/4] Copying executable...
if exist "build\Release\cs2_rcon_panel.exe" (
    copy /Y "build\Release\cs2_rcon_panel.exe" "cs2_rcon_panel.exe" >nul
    echo       Executable: cs2_rcon_panel.exe
)

echo.
echo ========================================
echo  Build successful!
echo  Run: cs2_rcon_panel.exe
echo ========================================
pause

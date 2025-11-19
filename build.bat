@echo off
REM Dither's Boyfriend - Windows Build Script
REM
REM Prerequisites:
REM - Visual Studio 2019 or later (with C++ tools)
REM - CMake 3.15+
REM - vcpkg for dependency management
REM
REM Install dependencies with vcpkg:
REM   vcpkg install opencv4:x64-windows glfw3:x64-windows

setlocal enabledelayedexpansion

echo =========================================
echo Dither's Boyfriend - Windows Build
echo =========================================
echo.

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake not found! Please install CMake 3.15 or later.
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)

REM Check for vcpkg (optional but recommended)
if defined VCPKG_ROOT (
    echo [INFO] Using vcpkg from: %VCPKG_ROOT%
    set CMAKE_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
) else (
    echo [WARNING] VCPKG_ROOT not set. Make sure OpenCV and GLFW are installed.
    echo.
    echo To use vcpkg:
    echo 1. Clone vcpkg: git clone https://github.com/Microsoft/vcpkg.git
    echo 2. Bootstrap: cd vcpkg ^&^& bootstrap-vcpkg.bat
    echo 3. Install deps: vcpkg install opencv4:x64-windows glfw3:x64-windows
    echo 4. Set VCPKG_ROOT environment variable to vcpkg directory
    echo.
    set CMAKE_TOOLCHAIN=
)

REM Create build directory
if not exist "build_windows" mkdir build_windows
cd build_windows

REM Download Dear ImGui if needed
if not exist "..\external\imgui" (
    echo [*] Downloading Dear ImGui...
    cd ..
    git clone --depth 1 https://github.com/ocornut/imgui.git external\imgui
    cd build_windows
)

REM Configure with CMake
echo [*] Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 %CMAKE_TOOLCHAIN%
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

REM Build Release version
echo [*] Building Release version...
cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo =========================================
echo Build successful!
echo =========================================
echo Executables are in: build_windows\Release\
echo.
echo   - dithers-boyfriend.exe     (GUI version)
echo   - dithers-boyfriend-cli.exe (CLI version)
echo.
echo To create a distribution package:
echo   cmake --build . --config Release --target package
echo =========================================
echo.

cd ..
pause

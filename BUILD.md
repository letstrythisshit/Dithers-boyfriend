# Building Dither's Boyfriend

This document explains how to build Dither's Boyfriend on different platforms for distribution.

## Table of Contents

- [Windows](#windows)
- [Linux](#linux)
- [macOS](#macos)
- [Cross-Platform Build (CMake)](#cross-platform-build-cmake)
- [Creating Distribution Packages](#creating-distribution-packages)

---

## Prerequisites by Platform

### All Platforms
- C++17 compatible compiler
- CMake 3.15 or later
- Git

### Windows
- **Visual Studio 2019 or later** (with C++ desktop development)
- **vcpkg** (recommended for dependencies)
  ```bash
  git clone https://github.com/Microsoft/vcpkg.git
  cd vcpkg
  .\bootstrap-vcpkg.bat
  .\vcpkg install opencv4:x64-windows glfw3:x64-windows
  ```

### Linux
- GCC 7+ or Clang 6+
- Development packages:
  ```bash
  # Ubuntu/Debian
  sudo apt-get install build-essential cmake git pkg-config \
      libopencv-dev libglfw3-dev libgl1-mesa-dev zenity

  # Fedora
  sudo dnf install gcc-c++ cmake git opencv-devel glfw-devel \
      mesa-libGL-devel zenity

  # Arch Linux
  sudo pacman -S base-devel cmake git opencv glfw-x11 mesa zenity
  ```

### macOS
- **Xcode Command Line Tools**
- **Homebrew** (for dependencies)
  ```bash
  brew install cmake opencv glfw pkg-config
  ```

---

## Quick Build (Linux/macOS)

```bash
# Automated build script
chmod +x build.sh
./build.sh
```

Or using Makefile:
```bash
make setup  # Install deps + download ImGui
make        # Build both GUI and CLI
```

---

## Windows Build

### Option 1: Using build.bat (Recommended)

```batch
REM Make sure vcpkg is set up first
set VCPKG_ROOT=C:\path\to\vcpkg

REM Run build script
build.bat
```

### Option 2: Manual CMake Build

```batch
REM Create build directory
mkdir build_windows
cd build_windows

REM Download ImGui (if needed)
git clone --depth 1 https://github.com/ocornut/imgui.git ..\external\imgui

REM Configure
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

REM Build
cmake --build . --config Release

REM Executables will be in build_windows\Release\
```

---

## Cross-Platform Build (CMake)

The CMake build system works on all platforms:

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Install (optional)
sudo cmake --install .
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_GUI` | ON | Build GUI version |
| `BUILD_CLI` | ON | Build CLI version |
| `CMAKE_BUILD_TYPE` | - | Debug or Release |

Example:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON -DBUILD_CLI=ON
```

---

## Creating Distribution Packages

### Windows

```batch
cd build_windows
cmake --build . --config Release --target package
```

This creates:
- `DithersBoyfriend-1.0.0-win64.zip` - Portable ZIP
- `DithersBoyfriend-1.0.0-win64.exe` - NSIS installer (if NSIS installed)

### Linux

```bash
cd build
cmake --build . --target package
```

This creates:
- `DithersBoyfriend-1.0.0-Linux.tar.gz` - Tarball
- `DithersBoyfriend-1.0.0-Linux.deb` - Debian package (on Debian-based systems)

### macOS

```bash
cd build
cmake --build . --target package
```

This creates:
- `DithersBoyfriend-1.0.0-Darwin.dmg` - Disk image
- `DithersBoyfriend-1.0.0-Darwin.zip` - ZIP archive

---

## Building for Multiple Platforms

### From Linux (Cross-Compilation)

Build Windows binaries using MinGW:
```bash
sudo apt-get install mingw-w64

mkdir build_mingw
cd build_mingw
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw.cmake
cmake --build .
```

### Using Docker for Reproducible Builds

Create multi-platform builds using Docker:

```dockerfile
# Dockerfile.linux
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    build-essential cmake git libopencv-dev libglfw3-dev libgl1-mesa-dev
COPY . /app
WORKDIR /app
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build
```

```bash
# Build Linux binary
docker build -f Dockerfile.linux -t dithers-linux .
docker run --rm -v $(pwd)/dist:/dist dithers-linux \
    cp build/dithers-boyfriend build/dithers-boyfriend-cli /dist/
```

---

## Troubleshooting

### Windows: OpenCV DLLs not found

If the application fails to start due to missing DLLs:
1. The build script should auto-copy OpenCV DLLs to the output directory
2. If not, manually copy them from `vcpkg\installed\x64-windows\bin\*.dll`
3. Or use `windeployqt` equivalent for dependencies

### Linux: GLFW linking errors

```bash
# Install GLFW development package
sudo apt-get install libglfw3-dev
```

### macOS: Code signing required

For macOS 10.15+:
```bash
# Ad-hoc code signing
codesign -s - dithers-boyfriend
```

### All Platforms: ImGui not found

If ImGui isn't auto-downloaded:
```bash
git clone --depth 1 https://github.com/ocornut/imgui.git external/imgui
```

---

## Build Artifacts

After building, you'll have:

### GUI Version
- **Windows**: `dithers-boyfriend.exe` (with OpenCV DLLs)
- **Linux**: `dithers-boyfriend`
- **macOS**: `dithers-boyfriend` (or `dithers-boyfriend.app` bundle)

### CLI Version
- **Windows**: `dithers-boyfriend-cli.exe`
- **Linux**: `dithers-boyfriend-cli`
- **macOS**: `dithers-boyfriend-cli`

---

## Continuous Integration

Example GitHub Actions workflow:

```yaml
name: Build All Platforms

on: [push, pull_request]

jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - uses: microsoft/setup-msbuild@v1
      - name: Setup vcpkg
        run: |
          git clone https://github.com/Microsoft/vcpkg.git
          .\vcpkg\bootstrap-vcpkg.bat
          .\vcpkg\vcpkg install opencv4:x64-windows glfw3:x64-windows
      - name: Build
        run: .\build.bat

  build-linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y libopencv-dev libglfw3-dev libgl1-mesa-dev
      - name: Build
        run: |
          chmod +x build.sh
          ./build.sh

  build-macos:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Dependencies
        run: brew install opencv glfw cmake
      - name: Build
        run: |
          mkdir build && cd build
          cmake .. -DCMAKE_BUILD_TYPE=Release
          cmake --build .
```

---

## Distribution Checklist

Before distributing:

- [ ] Test on clean system (no dev tools)
- [ ] Include all required DLLs/shared libraries
- [ ] Add README and LICENSE files
- [ ] Create user documentation
- [ ] Test drag-and-drop functionality
- [ ] Test file dialogs on target platform
- [ ] Verify all 24 algorithms work
- [ ] Test with various image formats
- [ ] Package with installer (optional)

---

## Support

For build issues, please check:
1. Dependencies are correctly installed
2. CMake version is 3.15+
3. Compiler supports C++17
4. ImGui was downloaded successfully

For platform-specific issues, refer to the README.md troubleshooting section.

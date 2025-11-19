# Building Dither's Boyfriend on Windows

This guide will help you build Dither's Boyfriend on Windows for distribution.

## Prerequisites

### Required Software

1. **Visual Studio 2019 or later** (with C++ desktop development workload)
   - Download from: https://visualstudio.microsoft.com/downloads/
   - During installation, select "Desktop development with C++"

2. **CMake 3.15+**
   - Download from: https://cmake.org/download/
   - Make sure to add CMake to system PATH during installation

3. **Git**
   - Download from: https://git-scm.com/download/win

4. **vcpkg** (for dependency management)
   ```powershell
   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

### Install Dependencies

```powershell
cd C:\vcpkg
.\vcpkg install opencv4:x64-windows glfw3:x64-windows
```

This will take 15-30 minutes as it compiles OpenCV from source.

## Quick Build

### Option 1: Using build.bat (Easiest)

```batch
REM Set vcpkg path
set VCPKG_ROOT=C:\vcpkg

REM Clone the repository
git clone <repository-url>
cd Dithers-boyfriend

REM Run the build script
build.bat
```

The script will:
- Download Dear ImGui automatically
- Configure with CMake
- Build Release binaries
- Copy required DLLs

Executables will be in: `build_windows\Release\`

### Option 2: Manual CMake Build

```batch
REM Clone repository
git clone <repository-url>
cd Dithers-boyfriend

REM Download Dear ImGui
git clone --depth 1 https://github.com/ocornut/imgui.git external\imgui

REM Create build directory
mkdir build_windows
cd build_windows

REM Configure
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

REM Build
cmake --build . --config Release

REM Executables are in: build_windows\Release\
```

## Building Different Configurations

### Debug Build

```batch
cmake --build . --config Debug
```

### Release with Debug Info

```batch
cmake --build . --config RelWithDebInfo
```

### CLI Only (no GUI dependencies needed)

```batch
cmake .. -DBUILD_GUI=OFF -DBUILD_CLI=ON
cmake --build . --config Release
```

## Creating Distribution Package

### ZIP Archive

```batch
cd build_windows
cmake --build . --config Release --target package
```

This creates: `DithersBoyfriend-1.0.0-win64.zip`

### Installer (requires NSIS)

1. Install NSIS from: https://nsis.sourceforge.io/Download
2. Run:
   ```batch
   cmake --build . --config Release --target package
   ```

This creates: `DithersBoyfriend-1.0.0-win64.exe` installer

## Troubleshooting

### vcpkg not found

Make sure VCPKG_ROOT environment variable is set:
```batch
set VCPKG_ROOT=C:\vcpkg
```

Or specify the toolchain file manually:
```batch
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### OpenCV DLLs not found at runtime

The build script should automatically copy DLLs to the output directory. If they're missing:

1. Find OpenCV DLLs in:
   ```
   C:\vcpkg\installed\x64-windows\bin\opencv_*.dll
   ```

2. Copy them to the same directory as `dithers-boyfriend.exe`

### Build fails with "cannot open file 'opencv_world470.lib'"

Make sure OpenCV was installed correctly:
```batch
vcpkg list | findstr opencv
```

If not listed, reinstall:
```batch
vcpkg install opencv4:x64-windows
```

### "GL/gl.h not found"

Windows OpenGL headers are included with Visual Studio. Make sure you have the "Desktop development with C++" workload installed.

## Portable Build

To create a truly portable build (no installer):

1. Build in Release mode
2. Copy the following to a distribution folder:
   ```
   dithers-boyfriend.exe
   dithers-boyfriend-cli.exe
   opencv_world4XX.dll
   opencv_videoio_ffmpeg4XX_64.dll (if using video features)
   README.md
   LICENSE
   ```

3. Test on a clean Windows machine without development tools

## Cross-Compiling from Linux

You can cross-compile Windows binaries from Linux using MinGW-w64:

```bash
# Install MinGW-w64
sudo apt-get install mingw-w64

# Install dependencies for cross-compilation
# (This is more complex and requires cross-compiled OpenCV and GLFW)

# Configure for Windows
mkdir build_mingw64
cd build_mingw64
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake
cmake --build .
```

Note: Cross-compilation is advanced and requires properly cross-compiled dependencies.

## Performance Optimization

For maximum performance:

```batch
cmake .. -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_CXX_FLAGS="/O2 /GL" ^
  -DCMAKE_EXE_LINKER_FLAGS="/LTCG"
```

This enables:
- `/O2` - Maximum optimization
- `/GL` - Whole program optimization
- `/LTCG` - Link-time code generation

## 32-bit Build

For 32-bit Windows:

```batch
# Install 32-bit dependencies
vcpkg install opencv4:x86-windows glfw3:x86-windows

# Configure for x86
cmake .. -G "Visual Studio 17 2022" -A Win32 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Continuous Integration

Example GitHub Actions workflow for Windows:

```yaml
name: Windows Build

on: [push, pull_request]

jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3

      - name: Setup vcpkg
        run: |
          git clone https://github.com/Microsoft/vcpkg.git
          cd vcpkg
          .\bootstrap-vcpkg.bat
          .\vcpkg integrate install
          .\vcpkg install opencv4:x64-windows glfw3:x64-windows

      - name: Build
        run: |
          mkdir build
          cd build
          cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
          cmake --build . --config Release

      - name: Package
        run: |
          cd build
          cmake --build . --config Release --target package

      - name: Upload Artifact
        uses: actions/upload-artifact@v3
        with:
          name: windows-binaries
          path: build/*.zip
```

## Testing

Before distributing, test on:
- Windows 10 (minimum supported version)
- Windows 11
- Fresh Windows install without development tools
- Different screen resolutions
- Systems with integrated graphics (Intel HD)
- Systems with dedicated graphics (NVIDIA/AMD)

## Distribution Checklist

- [ ] Build in Release mode
- [ ] Test all 24 dithering algorithms
- [ ] Test drag-and-drop functionality
- [ ] Test file dialogs
- [ ] Test with various image formats
- [ ] Include all required DLLs
- [ ] Add README.txt for users
- [ ] Test on clean Windows system
- [ ] Create installer or ZIP archive
- [ ] Code sign executable (optional, recommended)

## Code Signing (Optional)

For professional distribution:

1. Obtain a code signing certificate
2. Sign the executable:
   ```batch
   signtool sign /f mycert.pfx /p password /t http://timestamp.digicert.com dithers-boyfriend.exe
   ```

This removes the "Unknown Publisher" warning when users run the application.

## Support

For build issues specific to Windows:
- Check Visual Studio version (2019+ required)
- Verify CMake version (3.15+ required)
- Ensure vcpkg installed dependencies correctly
- Check Windows SDK is installed

See BUILD.md for more general cross-platform build information.

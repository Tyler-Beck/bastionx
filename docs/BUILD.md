# Bastionx Build Guide

This document provides comprehensive build instructions for Bastionx on Windows.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Environment Setup](#environment-setup)
3. [Installing Dependencies](#installing-dependencies)
4. [Building the Project](#building-the-project)
5. [Running Tests](#running-tests)
6. [Troubleshooting](#troubleshooting)
7. [Advanced Build Options](#advanced-build-options)

---

## Prerequisites

### Required Software

1. **Windows 10/11** (64-bit)
   - Windows 7+ supported but not recommended

2. **Visual Studio 2022** (Community Edition or better)
   - Download from: https://visualstudio.microsoft.com/
   - Workload: "Desktop development with C++"
   - Individual components:
     - C++20 compiler
     - CMake tools for Windows
     - Windows 10 SDK

3. **CMake 3.21+**
   - Included with Visual Studio
   - Or download standalone: https://cmake.org/download/

4. **Git**
   - Download from: https://git-scm.com/
   - Required for cloning vcpkg

### Recommended Software

- **Windows Terminal** (for better PowerShell experience)
- **Visual Studio Code** (for editing code outside of Visual Studio)

---

## Environment Setup

### 1. Install Visual Studio 2022

1. Download the Visual Studio installer
2. Select "Desktop development with C++" workload
3. In the "Individual components" tab, ensure these are selected:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools (latest)
   - C++ CMake tools for Windows
   - Windows 10 SDK (10.0.19041.0 or later)
   - C++ AddressSanitizer (optional, for memory debugging)

4. Complete installation (may take 30-60 minutes)

### 2. Install vcpkg (C++ Package Manager)

vcpkg is Microsoft's cross-platform C++ package manager. We use it to manage all dependencies.

#### Installation Steps

Open PowerShell as Administrator and run:

```powershell
# Create a directory for development tools (if not exists)
New-Item -ItemType Directory -Force -Path C:\dev

# Clone vcpkg repository
cd C:\dev
git clone https://github.com/Microsoft/vcpkg.git

# Navigate to vcpkg directory
cd vcpkg

# Bootstrap vcpkg (builds the executable)
.\bootstrap-vcpkg.bat

# Integrate with Visual Studio (one-time setup)
.\vcpkg integrate install
```

#### Set VCPKG_ROOT Environment Variable (Optional but Recommended)

This allows CMake to find vcpkg automatically:

1. Open "Edit the system environment variables"
2. Click "Environment Variables"
3. Under "System variables", click "New"
4. Variable name: `VCPKG_ROOT`
5. Variable value: `C:\dev\vcpkg`
6. Click OK and restart any open terminals

---

## Installing Dependencies

From the vcpkg directory, install all required libraries:

```powershell
cd C:\dev\vcpkg

# Install all dependencies at once (recommended)
.\vcpkg install qt6-base:x64-windows libsodium:x64-windows sqlite3:x64-windows gtest:x64-windows
```

Or install individually:

```powershell
# Qt 6 (largest dependency, takes 30-60 minutes)
.\vcpkg install qt6-base:x64-windows

# libsodium (cryptography library)
.\vcpkg install libsodium:x64-windows

# SQLite3 (database)
.\vcpkg install sqlite3:x64-windows

# Google Test (unit testing framework)
.\vcpkg install gtest:x64-windows
```

### Dependency Installation Notes

- **Qt 6**: Largest dependency (~2-3 GB), takes significant time to build
- **libsodium**: Fast to install (~2-5 minutes)
- **SQLite3**: Very fast (~1 minute)
- **GTest**: Fast (~2-3 minutes)

**Total installation time**: 30-90 minutes depending on hardware
**Total disk space required**: ~5-6 GB

### Verifying Installation

Check installed packages:

```powershell
.\vcpkg list
```

You should see:
```
gtest:x64-windows
libsodium:x64-windows
qt6-base:x64-windows
sqlite3:x64-windows
```

---

## Building the Project

### Step 1: Clone the Repository (if not already done)

```powershell
cd C:\Users\17326\spring2026
git clone <repository-url> bastionx
cd bastionx
```

### Step 2: Configure with CMake

Create a build directory and configure:

```powershell
# From the bastionx directory
mkdir build
cd build

# Configure (pointing to vcpkg toolchain)
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
```

**Alternative**: If you set the VCPKG_ROOT environment variable:

```powershell
cmake ..
```

CMake will automatically find the toolchain file.

### Step 3: Build

#### Debug Build (default)

```powershell
cmake --build . --config Debug
```

#### Release Build (optimized)

```powershell
cmake --build . --config Release
```

#### Build Both Configurations

```powershell
cmake --build . --config Debug
cmake --build . --config Release
```

### Build Output

After a successful build, you should see:

```
build/
├── Debug/
│   ├── bastionx_core.lib      # Core library
│   └── bastionx_tests.exe     # Test executable
└── Release/
    ├── bastionx_core.lib
    └── bastionx_tests.exe
```

---

## Running Tests

### Run All Tests

```powershell
# From build directory
.\Debug\bastionx_tests.exe

# Or Release build
.\Release\bastionx_tests.exe
```

### Run Specific Test Suite

```powershell
# Run only CryptoService tests
.\Debug\bastionx_tests.exe --gtest_filter=CryptoServiceTest.*

# Run only SecureMemory tests
.\Debug\bastionx_tests.exe --gtest_filter=SecureMemoryTest.*
```

### Run Specific Test

```powershell
# Run a single test
.\Debug\bastionx_tests.exe --gtest_filter=CryptoServiceTest.EncryptDecryptRoundTrip
```

### Verbose Test Output

```powershell
# Show detailed output
.\Debug\bastionx_tests.exe --gtest_verbose
```

### Expected Test Output

All tests should pass:

```
[==========] Running 23 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 11 tests from CryptoServiceTest
[ RUN      ] CryptoServiceTest.KeyDerivationDeterministic
[       OK ] CryptoServiceTest.KeyDerivationDeterministic (250 ms)
...
[==========] 23 tests from 2 test suites ran. (5000 ms total)
[  PASSED  ] 23 tests.
```

**Note**: Key derivation tests may take 100-500ms each due to Argon2id's intentional slowness.

---

## Troubleshooting

### Issue: "Could not find vcpkg toolchain file"

**Solution**: Explicitly specify the toolchain file:

```powershell
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Issue: "Could not find Qt6"

**Cause**: Qt 6 not installed via vcpkg

**Solution**:

```powershell
cd C:\dev\vcpkg
.\vcpkg install qt6-base:x64-windows
```

### Issue: "Could not find unofficial-sodium"

**Cause**: libsodium not installed or wrong package name

**Solution**:

```powershell
cd C:\dev\vcpkg
.\vcpkg install libsodium:x64-windows
```

**Note**: In CMake, libsodium is found as `unofficial-sodium` (vcpkg convention).

### Issue: "LNK1104: cannot open file 'Qt6Core.lib'"

**Cause**: Mismatch between build configuration and library configuration

**Solution**: Ensure you're building the same configuration (Debug vs Release). Rebuild dependencies if needed:

```powershell
cd C:\dev\vcpkg
.\vcpkg remove qt6-base:x64-windows
.\vcpkg install qt6-base:x64-windows
```

### Issue: Tests fail with "sodium_init() failed"

**Cause**: libsodium not properly linked or DLL not found

**Solution**:

1. Ensure libsodium is installed:
   ```powershell
   .\vcpkg install libsodium:x64-windows
   ```

2. Verify CMake found libsodium during configuration:
   ```
   -- Found unofficial-sodium: ...
   ```

3. Rebuild clean:
   ```powershell
   cd build
   cmake --build . --config Debug --clean-first
   ```

### Issue: Build is very slow

**Cause**: Qt 6 is large and takes time to build

**Solutions**:
- Use prebuilt Qt binaries (not available via vcpkg)
- Build Release configuration only: `cmake --build . --config Release`
- Use parallel builds: `cmake --build . --config Debug -j8` (use number of CPU cores)

### Issue: "Out of memory" during dependency installation

**Cause**: Building Qt 6 requires significant RAM

**Solutions**:
- Close other applications
- Increase virtual memory (pagefile)
- Install dependencies one at a time instead of all at once

---

## Advanced Build Options

### Static Linking

To statically link dependencies (larger executable, no DLL dependencies):

```powershell
# Install static libraries
cd C:\dev\vcpkg
.\vcpkg install qt6-base:x64-windows-static
.\vcpkg install libsodium:x64-windows-static
.\vcpkg install sqlite3:x64-windows-static
.\vcpkg install gtest:x64-windows-static

# Configure with static triplet
cd C:\Users\17326\spring2026\bastionx\build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
```

### Enable Address Sanitizer (Memory Debugging)

Address Sanitizer helps detect memory leaks and buffer overflows:

```powershell
# Configure with ASAN enabled
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake -DENABLE_ASAN=ON

# Build and run tests
cmake --build . --config Debug
.\Debug\bastionx_tests.exe
```

**Note**: ASAN requires Visual Studio 2022 with the "C++ AddressSanitizer" component.

### Compiler Warnings as Errors

To enable strict compilation (warnings as errors):

Edit `CMakeLists.txt` and uncomment:

```cmake
if(MSVC)
    add_compile_options(/W4 /WX)  # Warnings as errors
```

### Custom vcpkg Location

If vcpkg is installed in a different location:

```powershell
cmake .. -DCMAKE_TOOLCHAIN_FILE=D:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Building from Visual Studio IDE

1. Open Visual Studio 2022
2. File → Open → CMake → Select `CMakeLists.txt` in bastionx directory
3. Visual Studio will configure the project automatically
4. Select build configuration (Debug/Release) from toolbar
5. Build → Build All (or press Ctrl+Shift+B)

### Generating Visual Studio Solution

If you prefer .sln files:

```powershell
cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
```

This generates `Bastionx.sln` which you can open in Visual Studio.

---

## Build Performance Tips

1. **Use SSD**: Building Qt 6 benefits greatly from fast disk I/O
2. **Parallel builds**: Use `-j` flag: `cmake --build . -j8`
3. **Incremental builds**: After first build, rebuilds are much faster
4. **ccache**: Consider using ccache for faster rebuilds (advanced)

---

## Next Steps

After successfully building and testing:

1. Read [CRYPTO_SPEC.md](CRYPTO_SPEC.md) to understand cryptographic design
2. Read [THREAT_MODEL.md](THREAT_MODEL.md) to understand security boundaries
3. Review the implementation plan in [../CLAUDE.md](../CLAUDE.md)
4. Start Phase 2 implementation (VaultService, NotesRepository)

---

## Getting Help

If you encounter issues not covered here:

1. Check the [GitHub Issues](https://github.com/your-repo/bastionx/issues)
2. Review vcpkg documentation: https://vcpkg.io/
3. Check libsodium documentation: https://libsodium.gitbook.io/
4. Qt 6 documentation: https://doc.qt.io/qt-6/

---

**Last Updated**: 2026-02-05
**Bastionx Version**: 0.1.0 (Phase 0 & 1)

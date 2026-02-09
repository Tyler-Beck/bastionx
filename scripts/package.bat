@echo off
REM ============================================================
REM  Bastionx Portable Packaging Script
REM  Creates a self-contained dist/ folder with all dependencies.
REM ============================================================

setlocal

set PROJECT_ROOT=%~dp0..
set BUILD_DIR=%PROJECT_ROOT%\build
set DIST_DIR=%PROJECT_ROOT%\dist\Bastionx
set VCPKG_BIN=%PROJECT_ROOT%\vcpkg_installed\x64-windows\bin
set VCPKG_DEBUG_BIN=%PROJECT_ROOT%\vcpkg_installed\x64-windows\debug\bin
set WINDEPLOYQT=%PROJECT_ROOT%\vcpkg_installed\x64-windows\tools\Qt6\bin\windeployqt.exe
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"
set CONFIG=Release

echo === Building Release configuration ===
%CMAKE% --build "%BUILD_DIR%" --config %CONFIG%
if errorlevel 1 (
    echo ERROR: Build failed. Run cmake configure first.
    echo   %CMAKE% -B build -G "Visual Studio 18 2026" -A x64 ^
    echo     -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" ^
    echo     -DVCPKG_MANIFEST_MODE=OFF ^
    echo     -DVCPKG_INSTALLED_DIR="%PROJECT_ROOT%/vcpkg_installed"
    exit /b 1
)

echo === Preparing dist folder ===
if exist "%DIST_DIR%" rmdir /s /q "%DIST_DIR%"
mkdir "%DIST_DIR%"

REM Copy main executable
copy "%BUILD_DIR%\%CONFIG%\bastionx.exe" "%DIST_DIR%\" >nul

REM Copy vcpkg runtime DLLs (release)
copy "%VCPKG_BIN%\libsodium.dll" "%DIST_DIR%\" >nul 2>&1
copy "%VCPKG_BIN%\sqlcipher.dll" "%DIST_DIR%\" >nul 2>&1

REM Run windeployqt to gather Qt DLLs and plugins
echo === Running windeployqt ===
"%WINDEPLOYQT%" --release --no-translations --no-opengl-sw --no-system-d3d-compiler "%DIST_DIR%\bastionx.exe"
if errorlevel 1 (
    echo WARNING: windeployqt failed, copying Qt DLLs manually...
    copy "%VCPKG_BIN%\Qt6Core.dll" "%DIST_DIR%\" >nul 2>&1
    copy "%VCPKG_BIN%\Qt6Gui.dll" "%DIST_DIR%\" >nul 2>&1
    copy "%VCPKG_BIN%\Qt6Widgets.dll" "%DIST_DIR%\" >nul 2>&1
    mkdir "%DIST_DIR%\platforms" 2>nul
    copy "%PROJECT_ROOT%\vcpkg_installed\x64-windows\Qt6\plugins\platforms\qwindows.dll" "%DIST_DIR%\platforms\" >nul 2>&1
    mkdir "%DIST_DIR%\styles" 2>nul
    copy "%PROJECT_ROOT%\vcpkg_installed\x64-windows\Qt6\plugins\styles\qmodernwindowsstyle.dll" "%DIST_DIR%\styles\" >nul 2>&1
)

echo.
echo === Package complete ===
echo   Output: %DIST_DIR%
echo.
dir /b "%DIST_DIR%"
echo.
echo To run: %DIST_DIR%\bastionx.exe

endlocal

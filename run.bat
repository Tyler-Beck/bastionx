@echo off
set PATH=%~dp0build\vcpkg_installed\x64-windows\debug\bin;%~dp0build\vcpkg_installed\x64-windows\bin;%PATH%
set QT_PLUGIN_PATH=%~dp0build\vcpkg_installed\x64-windows\debug\Qt6\plugins
start "" "%~dp0build\Debug\bastionx.exe"

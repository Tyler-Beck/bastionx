@echo off
set PATH=%~dp0vcpkg_installed\x64-windows\debug\bin;%~dp0vcpkg_installed\x64-windows\bin;%PATH%
start "" "%~dp0build\Debug\bastionx.exe"

@echo off
setlocal
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File native/package.ps1 -Rebuild
if errorlevel 1 pause
exit /b %errorlevel%

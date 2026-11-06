@echo off
setlocal
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File native/balance.ps1
if errorlevel 1 pause
exit /b %errorlevel%

@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"
title RDZEN - Kompilacja C++
powershell.exe -NoProfile -ExecutionPolicy Bypass -File native\build.ps1
if errorlevel 1 goto :failed
echo Gotowe. Uruchom START.bat.
pause
exit /b 0
:failed
echo Kompilacja nie powiodla sie. Sprawdz komunikat powyzej.
pause
exit /b 1

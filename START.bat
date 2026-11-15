@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"
title RDZEN - Centrum dowodzenia
powershell.exe -NoProfile -ExecutionPolicy Bypass -File native\start.ps1 %*
if errorlevel 1 goto :failed
exit /b 0
:failed
echo.
echo Nie udalo sie uruchomic gry. Sprawdz komunikat powyzej.
pause
exit /b 1

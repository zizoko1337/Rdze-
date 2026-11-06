@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"
title RDZEN - Centrum dowodzenia
if not exist "native\build\rdzen-game.exe" (
  echo Przygotowywanie natywnej gry...
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File native\build.ps1 -GameOnly
  if errorlevel 1 goto :failed
)
"native\build\rdzen-game.exe" %*
if errorlevel 1 goto :failed
exit /b 0
:failed
echo.
echo Nie udalo sie uruchomic gry. Sprawdz komunikat powyzej.
pause
exit /b 1

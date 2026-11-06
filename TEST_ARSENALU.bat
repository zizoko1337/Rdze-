@echo off
setlocal
cd /d "%~dp0"
title RDZEN - Test nowego arsenalu
if not exist "native\build\rdzen-game.exe" (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File native\build.ps1 -GameOnly
  if errorlevel 1 goto :failed
)
native\build\rdzen-game.exe --smoke-test
if errorlevel 1 goto :failed
native\build\rdzen-game.exe --stress-test
if errorlevel 1 goto :failed
echo Wyniki i zrzuty: artifacts\arsenal
pause
exit /b 0
:failed
echo Test nie powiodl sie. Sprawdz komunikat powyzej.
pause
exit /b 1

@echo off
setlocal
cd /d "%~dp0"
title RDZEN - Natywny test wydajnosci
if not exist "native\build\rdzen-native.exe" (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File native\build.ps1
  if errorlevel 1 goto :failed
)
native\build\rdzen-native.exe
if errorlevel 1 if not errorlevel 2 goto :failed
exit /b 0
:failed
echo Test nie mogl zostac uruchomiony. Sprawdz komunikat powyzej.
pause

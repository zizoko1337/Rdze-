@echo off
setlocal
cd /d "%~dp0"
title RDZEN - Automatyczny benchmark
if not exist "native\build\rdzen-native.exe" (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File native\build.ps1
  if errorlevel 1 goto :failed
)
echo Test trwa okolo 105 sekund. Pozostaw okno testu aktywne.
echo Wyniki zostana zapisane w artifacts\native\benchmark.
native\build\rdzen-native.exe --benchmark
if errorlevel 1 goto :failed
echo Gotowe. Wyniki: artifacts\native\benchmark\summary.csv
pause
exit /b 0
:failed
echo Test nie zakonczyl sie poprawnie. Sprawdz komunikat powyzej.
pause

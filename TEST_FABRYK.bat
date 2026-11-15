@echo off
setlocal
cd /d "%~dp0"
call START.bat --factory-test
if errorlevel 1 goto :failed
echo Test dostawy i produkcji zakonczony. Zrzuty: artifacts\command\factories
pause
exit /b 0
:failed
echo Test fabryk nie powiodl sie. Sprawdz komunikat powyzej.
pause
exit /b 1

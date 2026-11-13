@echo off
setlocal
cd /d "%~dp0"
call START.bat --coffee-test
if errorlevel 1 goto :failed
echo Test kawy zakonczony. Raport i zrzuty: artifacts\command\coffee
pause
exit /b 0
:failed
echo Test kawy nie powiodl sie. Sprawdz komunikat powyzej.
pause
exit /b 1

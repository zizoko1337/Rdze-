@echo off
setlocal
cd /d "%~dp0"
call START.bat --command-test
if errorlevel 1 goto :failed
call START.bat --command-test --minigun
if errorlevel 1 goto :failed
echo Test obu dzial zakonczony. Raporty i zrzuty: artifacts\command
pause
exit /b 0
:failed
echo Test nie powiodl sie.
pause
exit /b 1

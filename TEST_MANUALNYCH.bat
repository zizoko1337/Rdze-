@echo off
setlocal
cd /d "%~dp0"
call START.bat --manual-test
if errorlevel 1 goto :failed
echo Test dziewieciu dzial zakonczony. Raporty: artifacts\command\manual
pause
exit /b 0
:failed
echo Test nie powiodl sie.
pause
exit /b 1

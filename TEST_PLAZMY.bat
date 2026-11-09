@echo off
setlocal
cd /d "%~dp0"
call START.bat --plasma-test
if errorlevel 1 goto :failed
echo Test plazmy i laboratorium zakonczony. Raporty: artifacts\command\plasma
pause
exit /b 0
:failed
echo Test plazmy nie powiodl sie.
pause
exit /b 1

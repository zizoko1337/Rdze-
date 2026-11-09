@echo off
setlocal
cd /d "%~dp0"
call START.bat --radio-test
if errorlevel 1 goto :failed
echo Test radia zakonczony. Raporty: artifacts\command\radio
pause
exit /b 0
:failed
echo Test radia nie powiodl sie.
pause
exit /b 1

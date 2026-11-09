@echo off
setlocal
cd /d "%~dp0"
call START.bat --cooling-test
if errorlevel 1 goto :failed
echo Test chlodzenia zakonczony. Raporty: artifacts\command\cooling
pause
exit /b 0
:failed
echo Test chlodzenia nie powiodl sie.
pause
exit /b 1

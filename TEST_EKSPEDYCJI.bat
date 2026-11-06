@echo off
setlocal
cd /d "%~dp0"
call START.bat --expedition-test
if errorlevel 1 goto :failed
echo Wyniki: artifacts\command\laser\expedition-result.json
pause
exit /b 0
:failed
echo Test ekspedycji nie powiodl sie.
pause
exit /b 1

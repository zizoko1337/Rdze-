@echo off
setlocal
cd /d "%~dp0"
call START.bat --delivery-test
if errorlevel 1 goto :failed
echo Raport i zrzuty: artifacts\command\laser\delivery-result.json
pause
exit /b 0
:failed
echo Test dostawy nie powiodl sie.
pause
exit /b 1

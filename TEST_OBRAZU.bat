@echo off
setlocal
cd /d "%~dp0"
if not exist "native\build\render-tests.exe" (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File native\build.ps1 -GameOnly
  if errorlevel 1 goto :failed
)
native\build\render-tests.exe
if errorlevel 1 goto :failed
echo Raport i zrzuty: artifacts\render
pause
exit /b 0
:failed
echo Test obrazu nie powiodl sie.
pause
exit /b 1

@echo off
setlocal
cd /d "%~dp0"
call START.bat --upgrade-test
exit /b %errorlevel%

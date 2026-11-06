@echo off
setlocal
cd /d "%~dp0"
call START.bat --support-test
exit /b %errorlevel%

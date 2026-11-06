@echo off
setlocal
cd /d "%~dp0"
call START.bat --services-test
exit /b %errorlevel%

@echo off
setlocal
cd /d "%~dp0"
call START.bat --services-lab
exit /b %errorlevel%

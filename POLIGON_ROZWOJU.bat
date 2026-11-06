@echo off
setlocal
cd /d "%~dp0"
call START.bat --progression-lab
exit /b %errorlevel%

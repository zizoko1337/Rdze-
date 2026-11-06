@echo off
setlocal
cd /d "%~dp0"
call START.bat --frontier-lab
exit /b %errorlevel%

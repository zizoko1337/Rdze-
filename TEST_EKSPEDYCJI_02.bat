@echo off
setlocal
cd /d "%~dp0"
call START.bat --frontier-test
exit /b %errorlevel%

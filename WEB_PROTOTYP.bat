@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"
title RDZEN - Ostatni przyczolek
where node >nul 2>nul
if errorlevel 1 (
  echo Brak Node.js. Zainstaluj wersje LTS z https://nodejs.org/
  echo Nastepnie uruchom ten plik ponownie.
  pause
  exit /b 1
)
if not exist "dist\index.html" (
  if not exist "node_modules\vite\bin\vite.js" (
    echo Przygotowywanie bibliotek gry. Pierwsze uruchomienie wymaga internetu.
    call npm.cmd ci
    if errorlevel 1 goto :failed
  )
  echo Budowanie gry...
  call npm.cmd run build
  if errorlevel 1 goto :failed
)
node scripts\launch.mjs
if errorlevel 1 goto :failed
exit /b 0
:failed
echo.
echo Nie udalo sie uruchomic gry. Sprawdz komunikat powyzej.
pause
exit /b 1

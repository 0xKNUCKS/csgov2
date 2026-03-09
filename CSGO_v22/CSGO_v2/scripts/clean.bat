@echo off
REM Clean build artifacts
setlocal

set ROOT=%~dp0..

echo [*] Cleaning build directory...
if exist "%ROOT%\build" (
    rmdir /s /q "%ROOT%\build"
    echo [+] Build directory removed.
) else (
    echo [*] Nothing to clean.
)

pause

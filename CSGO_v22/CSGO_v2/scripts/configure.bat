@echo off
REM Configure the CMake project (run once or after CMakeLists.txt changes)
setlocal

set CMAKE="C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set ROOT=%~dp0..

echo [*] Configuring CMake project...
%CMAKE% -S "%ROOT%" -B "%ROOT%\build" -G "Visual Studio 18 2026" -A Win32

if %ERRORLEVEL% NEQ 0 (
    echo [!] Configuration FAILED
    pause
    exit /b 1
)

echo [+] Configuration complete.
pause

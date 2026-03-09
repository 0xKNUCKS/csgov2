@echo off
REM Build the project in Debug configuration
setlocal

set CMAKE="C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set ROOT=%~dp0..

REM Auto-configure if build dir doesn't exist
if not exist "%ROOT%\build\CMakeCache.txt" (
    echo [*] Build directory not found, running configure first...
    call "%~dp0configure.bat"
)

echo [*] Building Debug...
%CMAKE% --build "%ROOT%\build" --config Debug

if %ERRORLEVEL% NEQ 0 (
    echo [!] Build FAILED
    pause
    exit /b 1
)

echo [+] Build succeeded.
echo     DLL:    build\Debug\CSGO_v2.dll
echo     Loader: build\Debug\CSGO_Loader.exe
pause

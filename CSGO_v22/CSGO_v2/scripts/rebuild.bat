@echo off
REM Clean + reconfigure + build Debug
setlocal

echo [*] Full rebuild starting...
call "%~dp0clean.bat"
call "%~dp0configure.bat"
call "%~dp0build-debug.bat"

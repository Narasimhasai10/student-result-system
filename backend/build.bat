@echo off
setlocal

echo ========================================
echo Student Result System - C++ Backend
echo ========================================
echo.

where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: CMake is not installed or not in PATH
    pause
    exit /b 1
)

if not exist "build" (
    mkdir build
)

cd build

echo.
echo Generating CMake build files for Visual Studio 2026...
cmake .. -G "Visual Studio 18 2026" -A x64

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo.
echo Building project...
cmake --build . --config Release

if errorlevel 1 (
    echo.
    echo ERROR: Build failed
    pause
    exit /b 1
)

cd ..

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo 1. Run Terminal Application
echo    .\build\bin\Release\server.exe
echo.
echo 2. Run Website Backend
echo    .\build\bin\Release\web_server.exe
echo.

set /p run="Do you want to run the website backend now? (y/n): "

if /i "%run%"=="y" (
    echo.
    echo Starting Student Result System Web Server...
    echo.
    .\build\bin\Release\web_server.exe
)

endlocal
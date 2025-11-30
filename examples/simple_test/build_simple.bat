@echo off
REM Simple build test - minimal HEIDIC program

setlocal enabledelayedexpansion

REM Get project root
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%..\.."
cd /d "%PROJECT_ROOT%"

echo === Simple HEIDIC Build Test ===
echo.

REM Step 1: Compile HEIDIC to C++
echo Compiling HEIDIC: examples\simple_test\simple_test.hd
cargo run -- compile examples\simple_test\simple_test.hd
if errorlevel 1 (
    echo HEIDIC compilation failed!
    exit /b 1
)

REM Check if C++ file was generated
if not exist "examples\simple_test\simple_test.cpp" (
    echo Error: Generated C++ file not found!
    exit /b 1
)

echo.
echo Compiling C++...
cd /d "%SCRIPT_DIR%"

REM Simple C++ compilation - no external dependencies
g++ -std=c++17 -O3 simple_test.cpp -o simple_test.exe
if errorlevel 1 (
    echo C++ compilation failed!
    exit /b 1
)

echo.
echo === Build successful! ===
echo Run with: .\simple_test.exe
echo.

endlocal


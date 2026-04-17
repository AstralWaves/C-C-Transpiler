@echo off
REM Quick Start Guide - Multi-Language Source-to-Source Compiler (Windows)

setlocal enabledelayedexpansion

echo.
echo === Multi-Language Source-to-Source Compiler ===
echo.
echo Step 1: Build the C++ Backend
echo ================================

cd "Multi-Language Source-to-Source Compiler\backend-cpp"

g++ -Iinclude -std=c++11 ^
    src/main.cpp src/lexer.cpp src/parser.cpp src/semantic.cpp ^
    src/intermediate_code.cpp src/optimizer.cpp src/symbol_table.cpp ^
    src/codegen/code_generator.cpp ^
    -o build/compiler.exe

if %errorlevel% equ 0 (
    echo ✓ Backend compiled successfully!
) else (
    echo ✗ Backend compilation failed!
    pause
    exit /b 1
)

echo.
echo Step 2: Test Backend
echo ====================

(
    echo int main^(^) { printf^("Hello"^); return 0; }
) > test_input.c

.\build\compiler.exe test_input.c python test_output.py
echo ✓ Backend test completed

echo.
echo Step 3: API Server Ready
echo ========================
echo PHP API Path: ..\php-backend-laravel\public\index.php
echo Start with:   php artisan serve --port=8000
echo.

echo Step 4: Frontend Ready
echo ====================
echo Frontend Path: ..\frontend\compiler.html
echo Start with:    python -m http.server 8001
echo.

echo === BUILD COMPLETE ===
echo.
echo To run the system:
echo   1. Terminal 1: cd "Multi-Language Source-to-Source Compiler\php-backend-laravel" ^&^& php artisan serve --port=8000
echo   2. Terminal 2: cd "Multi-Language Source-to-Source Compiler\frontend" ^&^& python -m http.server 8001
echo   3. Open browser: http://localhost:8001/compiler.html
echo.

pause

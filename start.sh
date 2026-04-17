#!/bin/bash
# Quick Start Guide - Multi-Language Source-to-Source Compiler

echo "=== Multi-Language Source-to-Source Compiler ==="
echo ""
echo "Step 1: Build the C++ Backend"
echo "================================"
cd backend-cpp
g++ -Iinclude -std=c++11 \
    src/main.cpp src/lexer.cpp src/parser.cpp src/semantic.cpp \
    src/intermediate_code.cpp src/optimizer.cpp src/symbol_table.cpp \
    src/codegen/code_generator.cpp \
    -o build/compiler.exe

if [ $? -eq 0 ]; then
    echo "✓ Backend compiled successfully!"
else
    echo "✗ Backend compilation failed!"
    exit 1
fi

echo ""
echo "Step 2: Test Backend"
echo "==================="
echo "int main() { printf(\"Hello\"); return 0; }" > test_input.c
./build/compiler.exe test_input.c python test_output.py
echo "✓ Backend test completed"

echo ""
echo "Step 3: Start PHP API Server"
echo "============================"
cd ../php-backend
echo "Starting PHP server on http://localhost:8000"
php -S localhost:8000 &
PHP_PID=$!

echo ""
echo "Step 4: Start Frontend Server"
echo "============================="
cd ../frontend
echo "Starting frontend on http://localhost:8001"
python -m http.server 8001 &
FRONTEND_PID=$!

echo ""
echo "=== SYSTEM READY ==="
echo ""
echo "Frontend:  http://localhost:8001/compiler.html"
echo "API:       http://localhost:8000/api/compile"
echo ""
echo "Press Ctrl+C to stop"
wait

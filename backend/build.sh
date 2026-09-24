#!/bin/bash

# Quick start script for building and running the C++ backend (Linux/macOS)

echo "========================================"
echo "Student Result System - C++ Backend"
echo "========================================"
echo ""

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed"
    echo "Please install CMake from https://cmake.org/"
    exit 1
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Generate build files
echo ""
echo "Generating CMake build files..."
cd build
cmake ..

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

# Build the project
echo ""
echo "Building project..."
cmake --build . --config Release

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

cd ..

# Ask user if they want to run the terminal application
echo ""
echo "Build completed successfully!"
echo ""
read -p "Do you want to run the terminal application now? (y/n): " run
if [ "$run" = "y" ] || [ "$run" = "Y" ]; then
    echo ""
    echo "Starting Student Result Management System"
    echo "Press Ctrl+C to stop the server"
    echo ""
    ./build/bin/server
else
    echo ""
    echo "To run the terminal application, execute: ./build/bin/server"
    echo "To run the website API, execute: ./build/bin/web_server"
fi

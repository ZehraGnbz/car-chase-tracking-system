#!/bin/bash

echo "🚗🚁 Car Chase Tracking System - Web UI"
echo "======================================"
echo

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: Please run this script from the project root directory"
    exit 1
fi

# Check if Python 3 is available
if ! command -v python3 &> /dev/null; then
    echo "❌ Error: Python 3 is required but not installed"
    exit 1
fi

# Check if pip is available
if ! command -v pip3 &> /dev/null; then
    echo "❌ Error: pip3 is required but not installed"
    exit 1
fi

echo "📦 Installing Python dependencies..."
cd backend
pip3 install -r requirements.txt
if [ $? -ne 0 ]; then
    echo "❌ Error: Failed to install Python dependencies"
    exit 1
fi
cd ..

echo "🔨 Building C++ tracking system..."
./build.sh
if [ $? -ne 0 ]; then
    echo "❌ Error: Failed to build C++ system"
    exit 1
fi

echo "✅ Build completed successfully!"
echo

# Check if advanced_car_tracker exists
if [ ! -f "build/advanced_car_tracker" ]; then
    echo "❌ Error: advanced_car_tracker executable not found"
    echo "Please ensure the build completed successfully"
    exit 1
fi

echo "🌐 Starting web interface..."
echo "Frontend will be available at: http://localhost:8080"
echo "Press Ctrl+C to stop the server"
echo

# Start the Flask backend
cd backend
python3 app.py 
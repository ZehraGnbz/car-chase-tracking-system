#!/bin/bash

# Advanced Car Chase Tracking System Build Script

echo "=== Advanced Car Chase Tracking System Build Script ==="

# Check if OpenCV is installed
if ! pkg-config --exists opencv4; then
    echo "OpenCV not found. Please install OpenCV first."
    echo "On macOS: brew install opencv"
    echo "On Ubuntu: sudo apt-get install libopencv-dev"
    exit 1
fi

# Get OpenCV version
OPENCV_VERSION=$(pkg-config --modversion opencv4)
echo "OpenCV found: $OPENCV_VERSION"

# Create build directory
mkdir -p build
cd build

echo "Configuring with CMake..."
cmake ..

if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed!"
    exit 1
fi

echo "Building project..."
make -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -ne 0 ]; then
    echo "Error: Build failed!"
    exit 1
fi

echo "=== Build completed successfully! ==="
echo "Executables:"
echo "  build/car_tracker              # Basic car tracker"
echo "  build/advanced_car_tracker     # Advanced car tracker with occlusion handling"
echo "  build/tracking_controller      # Interactive GUI controller"
echo ""

echo "Basic Tracker Usage:"
echo "  ./car_tracker                                    # Run with default video"
echo "  ./car_tracker -i input.mp4 -o output.mp4        # Process custom video"
echo "  ./car_tracker --help                            # Show help"
echo ""

echo "Advanced Tracker Usage:"
echo "  ./advanced_car_tracker                          # Run with default video"
echo "  ./advanced_car_tracker -i input.mp4 -o output.mp4"
echo "  ./advanced_car_tracker --occlusion-threshold 0.4"
echo "  ./advanced_car_tracker --help                   # Show advanced help"
echo ""

echo "Interactive Controller Usage:"
echo "  ./tracking_controller -i input.mp4              # Start with video"
echo "  ./tracking_controller -i input.mp4 -o output.mp4 # With recording"
echo "  ./tracking_controller --help                    # Show controller help"
echo ""

echo "Advanced Features:"
echo "  ✓ Partial occlusion handling"
echo "  ✓ Re-identification for lost targets"
echo "  ✓ Camera motion compensation"
echo "  ✓ Interactive target selection (mouse click)"
echo "  ✓ Advanced motion prediction"
echo "  ✓ Similar vehicle detection"
echo "  ✓ Real-time parameter adjustment"
echo "  ✓ Visual control panel"
echo "  ✓ Live statistics monitoring"
echo ""

echo "Interactive Controls (Controller):"
echo "  SPACE: Play/Pause"
echo "  T: Toggle Basic/Advanced mode"
echo "  O: Toggle occlusion handling"
echo "  R: Toggle re-identification"
echo "  C: Toggle camera compensation"
echo "  D: Toggle debug info"
echo "  V: Toggle recording"
echo "  Mouse: Select target vehicle"
echo "  ESC: Exit" 
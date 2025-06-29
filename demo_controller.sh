#!/bin/bash

echo "🚗🚁 Car Chase Tracking Controller Demo"
echo "======================================"
echo ""

# Check if we're in the right directory
if [ ! -f "build/tracking_controller" ]; then
    echo "❌ Error: tracking_controller not found!"
    echo "Please run ./build.sh first to build the project."
    exit 1
fi

# Check if default video exists
DEFAULT_VIDEO="FULL_ Aerial view of WILD police chase in Chicago.mp4"
if [ ! -f "$DEFAULT_VIDEO" ]; then
    echo "⚠️  Warning: Default video not found: $DEFAULT_VIDEO"
    echo "You can use any MP4 video file with the controller."
    echo ""
    echo "Demo scenarios:"
    echo "1. Basic tracking demo"
    echo "2. Advanced features demo"
    echo "3. Interactive tutorial"
    echo ""
    read -p "Choose a demo (1-3) or press Enter to exit: " choice
    
    case $choice in
        1)
            echo "📹 Basic Tracking Demo"
            echo "Launching controller with basic settings..."
            echo "Controls: SPACE=Play/Pause, T=Toggle mode, ESC=Exit"
            cd build && ./tracking_controller --help
            ;;
        2)
            echo "🎯 Advanced Features Demo"
            echo "Launching controller with advanced features..."
            echo "Features: Occlusion handling, Re-ID, Camera compensation"
            cd build && ./tracking_controller --help
            ;;
        3)
            echo "📚 Interactive Tutorial"
            echo "Launching tutorial mode..."
            echo "Follow the on-screen instructions"
            cd build && ./tracking_controller --help
            ;;
        *)
            echo "Demo cancelled."
            exit 0
            ;;
    esac
    exit 0
fi

echo "✅ Default video found: $DEFAULT_VIDEO"
echo ""

echo "🎬 Available Demo Scenarios:"
echo "1. Basic Vehicle Tracking"
echo "2. Advanced Occlusion Handling"
echo "3. Target Re-identification"
echo "4. Camera Motion Compensation"
echo "5. Full Feature Demo"
echo "6. Interactive Tutorial"
echo ""

read -p "Choose a demo scenario (1-6): " scenario

case $scenario in
    1)
        echo ""
        echo "🚗 Demo 1: Basic Vehicle Tracking"
        echo "================================"
        echo "This demo shows basic vehicle detection and tracking."
        echo ""
        echo "Controls:"
        echo "  SPACE: Play/Pause"
        echo "  Mouse: Click on vehicles to select"
        echo "  ESC: Exit"
        echo ""
        echo "Starting basic tracking demo..."
        cd build && ./tracking_controller -i "../$DEFAULT_VIDEO"
        ;;
    2)
        echo ""
        echo "🌳 Demo 2: Advanced Occlusion Handling"
        echo "====================================="
        echo "This demo shows how the system handles partial occlusions."
        echo ""
        echo "Features to test:"
        echo "  - Press O to enable occlusion handling"
        echo "  - Adjust occlusion threshold slider"
        echo "  - Watch vehicles being tracked through occlusions"
        echo ""
        echo "Starting occlusion handling demo..."
        cd build && ./tracking_controller -i "../$DEFAULT_VIDEO"
        ;;
    3)
        echo ""
        echo "🔄 Demo 3: Target Re-identification"
        echo "=================================="
        echo "This demo shows re-acquisition of lost targets."
        echo ""
        echo "Features to test:"
        echo "  - Press R to enable re-identification"
        echo "  - Adjust Re-ID threshold slider"
        echo "  - Select a target and watch it being re-acquired"
        echo ""
        echo "Starting re-identification demo..."
        cd build && ./tracking_controller -i "../$DEFAULT_VIDEO"
        ;;
    4)
        echo ""
        echo "🚁 Demo 4: Camera Motion Compensation"
        echo "===================================="
        echo "This demo shows handling of helicopter camera movement."
        echo ""
        echo "Features to test:"
        echo "  - Press C to enable camera compensation"
        echo "  - Adjust camera sensitivity slider"
        echo "  - Watch tracking stability during camera motion"
        echo ""
        echo "Starting camera compensation demo..."
        cd build && ./tracking_controller -i "../$DEFAULT_VIDEO"
        ;;
    5)
        echo ""
        echo "🎯 Demo 5: Full Feature Demo"
        echo "==========================="
        echo "This demo shows all advanced features working together."
        echo ""
        echo "All features enabled:"
        echo "  ✓ Occlusion handling"
        echo "  ✓ Re-identification"
        echo "  ✓ Camera compensation"
        echo "  ✓ Target selection"
        echo "  ✓ Real-time parameter adjustment"
        echo ""
        echo "Starting full feature demo..."
        cd build && ./tracking_controller -i "../$DEFAULT_VIDEO" -o "../demo_output.mp4"
        ;;
    6)
        echo ""
        echo "📚 Demo 6: Interactive Tutorial"
        echo "=============================="
        echo "This tutorial guides you through all features step by step."
        echo ""
        echo "Tutorial steps:"
        echo "1. Basic playback controls"
        echo "2. Vehicle detection and tracking"
        echo "3. Target selection"
        echo "4. Parameter adjustment"
        echo "5. Advanced features"
        echo "6. Recording and saving"
        echo ""
        echo "Starting interactive tutorial..."
        cd build && ./tracking_controller -i "../$DEFAULT_VIDEO"
        ;;
    *)
        echo "❌ Invalid choice. Please run the script again."
        exit 1
        ;;
esac

echo ""
echo "🎉 Demo completed!"
echo ""
echo "💡 Tips for best results:"
echo "  - Start with default settings"
echo "  - Adjust one parameter at a time"
echo "  - Use mouse to select interesting vehicles"
echo "  - Try different combinations of features"
echo ""
echo "📖 For more information, see CONTROLLER_GUIDE.md" 
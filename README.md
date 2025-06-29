# 🚗🚁 Car Chase Tracking System

Advanced vehicle tracking system designed for helicopter police chase videos with AI-powered detection and multi-object tracking capabilities.

## ✨ Features

### Core Tracking Features
- **YOLO-based Vehicle Detection**: Real-time vehicle detection using YOLO models
- **Multi-Object Tracking**: Kalman filter-based tracking with unique ID assignment
- **Advanced Occlusion Handling**: Maintains tracking during partial vehicle occlusion
- **Re-identification**: Re-identifies vehicles after temporary loss
- **Camera Motion Compensation**: Handles helicopter camera movements
- **Similar Vehicle Discrimination**: Distinguishes between similar-looking vehicles

### Web Interface
- **Modern Web UI**: Beautiful, responsive interface with drag-and-drop file upload
- **Real-time Progress**: Live progress tracking with statistics
- **Parameter Tuning**: Interactive sliders for all tracking parameters
- **Video Preview**: Built-in video preview before processing
- **Result Download**: Easy download of processed videos
- **Cross-platform**: Works on any device with a web browser

## 🚀 Quick Start

### Option 1: Web Interface (Recommended)
```bash
# Start the web interface
./start_webui.sh
```
Then open http://localhost:5000 in your browser.

### Option 2: Command Line
```bash
# Build the system
./build.sh

# Run basic tracker
./build/car_tracker -i input_video.mp4 -o output_video.mp4

# Run advanced tracker
./build/advanced_car_tracker -i input_video.mp4 -o output_video.mp4
```

## 📋 Requirements

### System Requirements
- **macOS/Linux/Windows** (with C++17 support)
- **Python 3.7+** (for web interface)
- **OpenCV 4.5+**
- **CMake 3.16+**

### Dependencies
- OpenCV (with contrib modules)
- YOLO model files (included)
- Python packages (for web interface):
  - Flask
  - Flask-CORS
  - opencv-python
  - numpy
  - Pillow

## 🛠️ Installation

### 1. Clone the Repository
```bash
git clone <repository-url>
cd track
```

### 2. Install Dependencies
```bash
# Install OpenCV (macOS)
brew install opencv

# Install OpenCV (Ubuntu/Debian)
sudo apt-get install libopencv-dev

# Install Python dependencies (for web interface)
cd backend
pip3 install -r requirements.txt
cd ..
```

### 3. Build the System
```bash
./build.sh
```

## 🌐 Web Interface Usage

### Starting the Web Interface
```bash
./start_webui.sh
```

### Using the Interface
1. **Upload Video**: Drag and drop or click to select a video file
2. **Adjust Parameters**: Use sliders to fine-tune detection and tracking
3. **Enable Features**: Toggle advanced features as needed
4. **Start Processing**: Click "Start Tracking" to begin
5. **Monitor Progress**: Watch real-time progress and statistics
6. **Download Result**: Download the processed video when complete

### Supported Video Formats
- MP4
- AVI
- MOV
- MKV
- MK4

## 🎛️ Command Line Usage

### Basic Tracker
```bash
./build/car_tracker -i input.mp4 -o output.mp4
```

### Advanced Tracker
```bash
./build/advanced_car_tracker \
  -i input.mp4 \
  -o output.mp4 \
  --detection-threshold 0.5 \
  --occlusion-threshold 0.3 \
  --reid-threshold 0.7 \
  --camera-sensitivity 0.1
```

### Controller Interface
```bash
./build/tracking_controller
```

## ⚙️ Parameters

### Detection Parameters
- **Detection Threshold** (0.0-1.0): Confidence threshold for vehicle detection
- **Occlusion Threshold** (0.0-1.0): Threshold for occlusion handling
- **Re-ID Threshold** (0.0-1.0): Threshold for re-identification
- **Camera Sensitivity** (0.0-1.0): Sensitivity to camera motion

### Advanced Features
- **Partial Occlusion Handling**: Maintains tracking during occlusion
- **Re-identification**: Re-identifies vehicles after loss
- **Camera Motion Compensation**: Compensates for camera movement

## 📊 Output

### Video Output
- **Bounding Boxes**: Vehicles are highlighted with colored rectangles
- **Tracking IDs**: Each vehicle has a unique, persistent ID
- **Trajectory Lines**: Shows vehicle movement paths
- **Statistics Overlay**: Displays real-time tracking statistics

### Statistics
- **Processing Time**: Total time to process the video
- **Vehicles Detected**: Number of unique vehicles tracked
- **FPS**: Average frames per second processed
- **Accuracy**: Tracking accuracy percentage

## 🔧 Advanced Configuration

### YOLO Model Configuration
```cpp
// In VehicleDetector.cpp
const float CONFIDENCE_THRESHOLD = 0.5;
const float NMS_THRESHOLD = 0.4;
```

### Tracking Parameters
```cpp
// In AdvancedCarTracker.cpp
const int MAX_AGE = 30;           // Maximum frames to keep lost track
const float IOU_THRESHOLD = 0.3;  // IoU threshold for association
```

## 🐛 Troubleshooting

### Common Issues

**Build Errors**
```bash
# Clean and rebuild
rm -rf build
./build.sh
```

**OpenCV Not Found**
```bash
# macOS
export OPENCV_DIR=/usr/local/opt/opencv

# Linux
export OPENCV_DIR=/usr/local
```

**Web Interface Not Starting**
```bash
# Check Python dependencies
cd backend
pip3 install -r requirements.txt

# Check if port 5000 is available
lsof -i :5000
```

**Video Processing Fails**
- Ensure video file is not corrupted
- Check video format is supported
- Verify sufficient disk space for output

### Performance Tips
- Use SSD storage for faster video I/O
- Close other applications during processing
- Use lower resolution videos for faster processing
- Adjust detection threshold based on video quality

## 📁 Project Structure

```
track/
├── src/                    # C++ source files
│   ├── main.cpp           # Basic tracker main
│   ├── advanced_main.cpp  # Advanced tracker main
│   ├── controller_main.cpp # Controller main
│   ├── CarTracker.cpp     # Basic tracking system
│   ├── AdvancedCarTracker.cpp # Advanced tracking
│   ├── TrackingController.cpp # GUI controller
│   └── VehicleDetector.cpp # YOLO detection
├── webui/                  # Web interface
│   └── index.html         # Frontend HTML
├── backend/               # Flask backend
│   ├── app.py            # Flask application
│   ├── requirements.txt  # Python dependencies
│   └── templates/        # HTML templates
├── models/               # YOLO model files
├── build/               # Build output
├── build.sh            # Build script
├── start_webui.sh      # Web interface startup
└── README.md           # This file
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- OpenCV team for computer vision library
- YOLO authors for object detection models
- Bootstrap team for UI components
- Flask team for web framework

---

**Note**: This system is designed for research and educational purposes. Always ensure compliance with local laws and regulations when processing video content. 
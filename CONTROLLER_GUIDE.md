# Car Chase Tracking Controller - User Guide

## Overview

The **Tracking Controller** is an interactive GUI application that provides easy-to-use controls for the car chase tracking system. It combines the power of advanced tracking algorithms with an intuitive user interface.

## Quick Start

### 1. Launch the Controller
```bash
# Start with a video file
./tracking_controller -i your_video.mp4

# Start with video and save output
./tracking_controller -i your_video.mp4 -o output.mp4
```

### 2. Basic Controls
- **SPACE**: Play/Pause video
- **S**: Stop video
- **ESC**: Exit application
- **Mouse Click**: Select target vehicle

## GUI Windows

### 1. Main Video Window
- **Name**: "Car Chase Tracker"
- **Content**: Live video with tracking overlays
- **Features**: 
  - Real-time vehicle detection and tracking
  - Bounding boxes around detected vehicles
  - Target selection with mouse clicks
  - Color-coded tracking information

### 2. Control Panel
- **Name**: "Controls"
- **Content**: Status information and controls
- **Features**:
  - Playback status (Playing/Paused/Stopped)
  - Tracking mode (Basic/Advanced)
  - Feature status indicators
  - Live statistics
  - Control instructions

### 3. Parameter Sliders
- **Name**: "Parameters"
- **Content**: Real-time adjustable parameters
- **Sliders**:
  - Detection Threshold (0-100%)
  - Occlusion Threshold (0-100%)
  - Re-ID Threshold (0-100%)
  - Camera Sensitivity (0-100%)

## Interactive Controls

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **SPACE** | Play/Pause video |
| **S** | Stop video |
| **T** | Toggle Basic/Advanced tracking mode |
| **O** | Toggle partial occlusion handling |
| **R** | Toggle re-identification |
| **C** | Toggle camera motion compensation |
| **D** | Toggle debug information display |
| **V** | Toggle video recording |
| **ESC** | Exit application |

### Mouse Controls

| Action | Function |
|--------|----------|
| **Left Click** | Select target vehicle |
| **Click on vehicle** | Make it the primary target |
| **Click on empty area** | Clear target selection |

## Advanced Features

### 1. Partial Occlusion Handling
- **Purpose**: Track vehicles even when partially hidden
- **When to use**: Videos with bridges, trees, or other occlusions
- **Toggle**: Press **O** key
- **Adjust**: Use "Occlusion Threshold" slider

### 2. Re-identification
- **Purpose**: Re-acquire lost targets using appearance features
- **When to use**: When vehicles disappear and reappear
- **Toggle**: Press **R** key
- **Adjust**: Use "Re-ID Threshold" slider

### 3. Camera Motion Compensation
- **Purpose**: Handle helicopter camera movement
- **When to use**: Aerial videos with camera motion
- **Toggle**: Press **C** key
- **Adjust**: Use "Camera Sensitivity" slider

### 4. Target Selection
- **Purpose**: Focus tracking on specific vehicles
- **How to use**: Click on any vehicle in the video
- **Visual feedback**: Selected target highlighted in yellow
- **Clear**: Click on empty area or press **C**

## Parameter Adjustment

### Real-time Sliders

#### Detection Threshold (0-100%)
- **Low values (0-30%)**: More sensitive, detects more vehicles
- **High values (70-100%)**: Less sensitive, fewer false positives
- **Recommended**: 50-70% for most videos

#### Occlusion Threshold (0-100%)
- **Low values (0-30%)**: Handles severe occlusions
- **High values (70-100%)**: Only handles minor occlusions
- **Recommended**: 30-50% for helicopter videos

#### Re-ID Threshold (0-100%)
- **Low values (0-30%)**: Easy re-identification
- **High values (70-100%)**: Strict re-identification
- **Recommended**: 60-80% for accurate re-acquisition

#### Camera Sensitivity (0-100%)
- **Low values (0-30%)**: Minimal camera motion compensation
- **High values (70-100%)**: Aggressive camera motion compensation
- **Recommended**: 10-30% for most aerial videos

## Workflow Examples

### Example 1: Basic Vehicle Tracking
1. Launch: `./tracking_controller -i video.mp4`
2. Press **SPACE** to start playback
3. Watch vehicles being detected and tracked
4. Use mouse to select interesting vehicles

### Example 2: Advanced Occlusion Handling
1. Launch with helicopter video
2. Press **O** to enable occlusion handling
3. Adjust "Occlusion Threshold" to 40%
4. Press **C** to enable camera compensation
5. Adjust "Camera Sensitivity" to 20%

### Example 3: Target Re-acquisition
1. Launch with challenging video
2. Press **R** to enable re-identification
3. Adjust "Re-ID Threshold" to 70%
4. Select a target vehicle with mouse
5. Watch it being re-acquired when lost

### Example 4: Recording Session
1. Launch: `./tracking_controller -i input.mp4 -o output.mp4`
2. Configure parameters as needed
3. Press **V** to start recording
4. Process the video with your settings
5. Press **V** again to stop recording

## Troubleshooting

### Common Issues

#### Video Not Loading
- **Problem**: "Could not open video file"
- **Solution**: Check file path and format (MP4, AVI, MOV supported)

#### Poor Detection
- **Problem**: Few or no vehicles detected
- **Solution**: Lower "Detection Threshold" to 30-40%

#### Too Many False Positives
- **Problem**: Non-vehicles being detected
- **Solution**: Increase "Detection Threshold" to 70-80%

#### Tracking Loss During Occlusions
- **Problem**: Vehicles lost when partially hidden
- **Solution**: Enable occlusion handling (press **O**) and adjust threshold

#### Camera Motion Issues
- **Problem**: Tracking drifts due to camera movement
- **Solution**: Enable camera compensation (press **C**) and adjust sensitivity

#### Performance Issues
- **Problem**: Low FPS or lag
- **Solution**: 
  - Disable some features (O, R, C keys)
  - Lower detection threshold
  - Use smaller video resolution

### Performance Tips

1. **For Real-time Processing**:
   - Use Basic mode (press **T**)
   - Disable advanced features
   - Lower detection threshold

2. **For High Accuracy**:
   - Use Advanced mode
   - Enable all features
   - Adjust thresholds carefully

3. **For Helicopter Videos**:
   - Enable camera compensation
   - Enable occlusion handling
   - Use moderate sensitivity settings

## Statistics and Monitoring

### Live Statistics Display
- **FPS**: Frames per second (performance indicator)
- **Frame Count**: Current frame / total frames
- **Vehicles Detected**: Number of tracked vehicles
- **Processing Time**: Time per frame in milliseconds

### Status Indicators
- **Playback Status**: Playing/Paused/Stopped
- **Tracking Mode**: Basic/Advanced
- **Feature Status**: ON/OFF for each feature
- **Target Selection**: Active/None

## File Formats

### Supported Input Formats
- MP4 (recommended)
- AVI
- MOV
- Other formats supported by OpenCV

### Output Recording
- **Format**: MP4
- **Codec**: H.264
- **Quality**: Same as input video
- **Features**: Includes all tracking overlays

## Advanced Usage

### Batch Processing
```bash
# Process multiple videos
for video in *.mp4; do
    ./tracking_controller -i "$video" -o "tracked_$video"
done
```

### Parameter Optimization
1. Start with default settings
2. Adjust one parameter at a time
3. Test with challenging parts of video
4. Save settings for future use

### Integration with Other Tools
- Output videos can be processed with video editing software
- Tracking data can be exported for analysis
- Screenshots can be captured for documentation

## Support and Feedback

For issues or questions:
1. Check this guide first
2. Try different parameter settings
3. Test with different video types
4. Report bugs with video examples

---

**Happy Tracking!** 🚗🚁 
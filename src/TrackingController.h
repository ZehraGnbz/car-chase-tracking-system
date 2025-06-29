#pragma once

#include "AdvancedCarTracker.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>
#include <functional>

class TrackingController {
public:
    TrackingController();
    ~TrackingController();
    
    // Main interface
    void run();
    bool initialize();
    
    // File operations
    void loadVideo(const std::string& videoPath);
    void saveVideo(const std::string& outputPath);
    void selectVideoFile();
    void selectOutputFile();
    std::string openFileDialog(const std::string& title, const std::string& filter = "Video files (*.mp4 *.avi *.mov *.mkv);;All files (*.*)");
    std::string saveFileDialog(const std::string& title, const std::string& defaultName = "output.mp4");
    
    // Playback controls
    void play();
    void pause();
    void stop();
    void stepForward();
    void stepBackward();
    void setPlaybackSpeed(double speed);
    
    // Tracking controls
    void enableBasicTracking();
    void enableAdvancedTracking();
    void toggleTracking();
    
    // Advanced features
    void setOcclusionThreshold(float threshold);
    void setReIdThreshold(float threshold);
    void setCameraSensitivity(float sensitivity);
    void togglePartialOcclusion();
    void toggleReIdentification();
    void toggleCameraCompensation();
    
    // Target selection
    void selectTarget(int x, int y);
    void clearTarget();
    void enableTargetSelection(bool enable);
    
    // Display options
    void toggleDebugInfo();
    void toggleRecording();
    void setDetectionThreshold(float threshold);
    
    // Statistics
    double getFPS() const;
    int getFrameCount() const;
    int getTotalFrames() const;
    double getProgress() const;
    
    // Callbacks
    void setVideoLoadedCallback(std::function<void(const std::string&)> callback);
    void setTrackingUpdateCallback(std::function<void(const std::vector<cv::Rect>&)> callback);
    void setStatusUpdateCallback(std::function<void(const std::string&)> callback);

private:
    // GUI elements
    void createControlWindow();
    void createTrackbarWindow();
    void updateGUI();
    void handleMouseEvents(int event, int x, int y, int flags, void* userdata);
    void drawControlPanel();
    void drawStatusPanel();
    
    // Video processing
    void processFrame();
    void updateDisplay();
    void updateStatistics();
    void handleKeyPress(char key);
    
    // State management
    void updateTrackingState();
    void saveSettings();
    void loadSettings();
    
    // Member variables
    std::unique_ptr<AdvancedCarTracker> tracker_;
    cv::VideoCapture videoCapture_;
    cv::VideoWriter videoWriter_;
    
    // GUI windows
    cv::Mat controlPanel_;
    cv::Mat statusPanel_;
    std::string mainWindowName_;
    std::string controlWindowName_;
    
    // State variables
    bool isPlaying_;
    bool isPaused_;
    bool isAdvancedMode_;
    bool showDebugInfo_;
    bool enableRecording_;
    bool targetSelectionMode_;
    bool partialOcclusionEnabled_;
    bool reIdEnabled_;
    bool cameraCompensationEnabled_;
    
    // Parameters
    float detectionThreshold_;
    float occlusionThreshold_;
    float reIdThreshold_;
    float cameraSensitivity_;
    double playbackSpeed_;
    
    // Statistics
    int currentFrame_;
    int totalFrames_;
    double fps_;
    double processingTime_;
    int vehiclesDetected_;
    
    // Callbacks
    std::function<void(const std::string&)> videoLoadedCallback_;
    std::function<void(const std::vector<cv::Rect>&)> trackingUpdateCallback_;
    std::function<void(const std::string&)> statusUpdateCallback_;
    
    // Mouse callback wrapper
    static void onMouse(int event, int x, int y, int flags, void* userdata);
}; 
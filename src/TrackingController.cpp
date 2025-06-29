#include "TrackingController.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdlib>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <gtk/gtk.h>
#endif

TrackingController::TrackingController() 
    : isPlaying_(false), isPaused_(false), isAdvancedMode_(true),
      showDebugInfo_(true), enableRecording_(false), targetSelectionMode_(false),
      partialOcclusionEnabled_(true), reIdEnabled_(true), cameraCompensationEnabled_(true),
      detectionThreshold_(0.5f), occlusionThreshold_(0.3f), reIdThreshold_(0.7f),
      cameraSensitivity_(0.1f), playbackSpeed_(1.0), currentFrame_(0), totalFrames_(0),
      fps_(0.0), processingTime_(0.0), vehiclesDetected_(0),
      mainWindowName_("Car Chase Tracker"), controlWindowName_("Controls") {
    
    tracker_ = std::make_unique<AdvancedCarTracker>();
}

TrackingController::~TrackingController() {
    cv::destroyAllWindows();
}

bool TrackingController::initialize() {
    std::cout << "Initializing Tracking Controller..." << std::endl;
    
    // Create GUI windows
    createControlWindow();
    createTrackbarWindow();
    
    // Set up mouse callback
    cv::setMouseCallback(mainWindowName_, onMouse, this);
    
    // Load default settings
    loadSettings();
    
    std::cout << "Tracking Controller initialized successfully!" << std::endl;
    return true;
}

void TrackingController::run() {
    if (!isPlaying_) return;
    
    cv::Mat frame;
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (isPlaying_) {
        if (!isPaused_) {
            videoCapture_ >> frame;
            if (frame.empty()) {
                std::cout << "End of video reached." << std::endl;
                stop();
                break;
            }
            
            currentFrame_++;
            
            // Process frame
            auto startTime = std::chrono::high_resolution_clock::now();
            processFrame();
            auto endTime = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            processingTime_ = duration.count();
            
            // Update display
            updateDisplay();
            updateStatistics();
        }
        
        // Update GUI
        updateGUI();
        
        // Handle key events
        char key = cv::waitKey(static_cast<int>(30 / playbackSpeed_));
        handleKeyPress(key);
        
        // Check for window close
        if (cv::getWindowProperty(mainWindowName_, cv::WND_PROP_VISIBLE) < 1) {
            break;
        }
    }
}

void TrackingController::createControlWindow() {
    cv::namedWindow(mainWindowName_, cv::WINDOW_AUTOSIZE);
    cv::namedWindow(controlWindowName_, cv::WINDOW_AUTOSIZE);
    
    // Create control panel
    controlPanel_ = cv::Mat::zeros(400, 300, CV_8UC3);
    drawControlPanel();
    
    cv::imshow(controlWindowName_, controlPanel_);
}

void TrackingController::createTrackbarWindow() {
    cv::namedWindow("Parameters", cv::WINDOW_AUTOSIZE);
    
    // Create trackbars
    cv::createTrackbar("Detection Threshold", "Parameters", nullptr, 100, 
        [](int value, void* userdata) {
            TrackingController* controller = static_cast<TrackingController*>(userdata);
            controller->setDetectionThreshold(value / 100.0f);
        }, this);
    
    cv::createTrackbar("Occlusion Threshold", "Parameters", nullptr, 100,
        [](int value, void* userdata) {
            TrackingController* controller = static_cast<TrackingController*>(userdata);
            controller->setOcclusionThreshold(value / 100.0f);
        }, this);
    
    cv::createTrackbar("Re-ID Threshold", "Parameters", nullptr, 100,
        [](int value, void* userdata) {
            TrackingController* controller = static_cast<TrackingController*>(userdata);
            controller->setReIdThreshold(value / 100.0f);
        }, this);
    
    cv::createTrackbar("Camera Sensitivity", "Parameters", nullptr, 100,
        [](int value, void* userdata) {
            TrackingController* controller = static_cast<TrackingController*>(userdata);
            controller->setCameraSensitivity(value / 100.0f);
        }, this);
    
    // Set initial values
    cv::setTrackbarPos("Detection Threshold", "Parameters", static_cast<int>(detectionThreshold_ * 100));
    cv::setTrackbarPos("Occlusion Threshold", "Parameters", static_cast<int>(occlusionThreshold_ * 100));
    cv::setTrackbarPos("Re-ID Threshold", "Parameters", static_cast<int>(reIdThreshold_ * 100));
    cv::setTrackbarPos("Camera Sensitivity", "Parameters", static_cast<int>(cameraSensitivity_ * 100));
}

void TrackingController::drawControlPanel() {
    controlPanel_ = cv::Scalar(50, 50, 50);
    
    // Title
    cv::putText(controlPanel_, "Car Chase Tracker", cv::Point(10, 30), 
               cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 2);
    
    // Playback controls
    cv::putText(controlPanel_, "Playback Controls:", cv::Point(10, 70), 
               cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 1);
    
    std::string playStatus = isPlaying_ ? (isPaused_ ? "PAUSED" : "PLAYING") : "STOPPED";
    cv::putText(controlPanel_, "Status: " + playStatus, cv::Point(10, 95), 
               cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    
    // Tracking mode
    std::string modeText = isAdvancedMode_ ? "Advanced Mode" : "Basic Mode";
    cv::putText(controlPanel_, "Mode: " + modeText, cv::Point(10, 120), 
               cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
    
    // Features status
    cv::putText(controlPanel_, "Features:", cv::Point(10, 150), 
               cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 1);
    
    cv::putText(controlPanel_, "Occlusion: " + std::string(partialOcclusionEnabled_ ? "ON" : "OFF"), 
               cv::Point(10, 175), cv::FONT_HERSHEY_SIMPLEX, 0.4, 
               partialOcclusionEnabled_ ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255), 1);
    
    cv::putText(controlPanel_, "Re-ID: " + std::string(reIdEnabled_ ? "ON" : "OFF"), 
               cv::Point(10, 195), cv::FONT_HERSHEY_SIMPLEX, 0.4, 
               reIdEnabled_ ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255), 1);
    
    cv::putText(controlPanel_, "Camera Comp: " + std::string(cameraCompensationEnabled_ ? "ON" : "OFF"), 
               cv::Point(10, 215), cv::FONT_HERSHEY_SIMPLEX, 0.4, 
               cameraCompensationEnabled_ ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255), 1);
    
    // Statistics
    cv::putText(controlPanel_, "Statistics:", cv::Point(10, 250), 
               cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 1);
    
    std::stringstream ss;
    ss << "Frame: " << currentFrame_ << "/" << totalFrames_;
    cv::putText(controlPanel_, ss.str(), cv::Point(10, 275), 
               cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255), 1);
    
    ss.str("");
    ss << "FPS: " << std::fixed << std::setprecision(1) << fps_;
    cv::putText(controlPanel_, ss.str(), cv::Point(10, 295), 
               cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255), 1);
    
    ss.str("");
    ss << "Vehicles: " << vehiclesDetected_;
    cv::putText(controlPanel_, ss.str(), cv::Point(10, 315), 
               cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255), 1);
    
    // Instructions
    cv::putText(controlPanel_, "Controls:", cv::Point(10, 350), 
               cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1);
    cv::putText(controlPanel_, "SPACE: Play/Pause", cv::Point(10, 370), 
               cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(200, 200, 200), 1);
    cv::putText(controlPanel_, "T: Toggle Mode", cv::Point(10, 385), 
               cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(200, 200, 200), 1);
}

void TrackingController::updateGUI() {
    drawControlPanel();
    cv::imshow(controlWindowName_, controlPanel_);
}

void TrackingController::processFrame() {
    if (!tracker_) return;
    
    // Get current frame from video capture
    cv::Mat frame;
    videoCapture_.set(cv::CAP_PROP_POS_FRAMES, currentFrame_ - 1);
    videoCapture_ >> frame;
    
    if (!frame.empty()) {
        // Process with tracker
        tracker_->processFrame(frame);
        
        // Update display
        cv::imshow(mainWindowName_, frame);
        
        // Save if recording
        if (enableRecording_ && videoWriter_.isOpened()) {
            videoWriter_.write(frame);
        }
    }
}

void TrackingController::updateDisplay() {
    // Display is handled in processFrame
}

void TrackingController::updateStatistics() {
    // Calculate FPS
    static auto lastFpsTime = std::chrono::high_resolution_clock::now();
    static int frameCount = 0;
    
    frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFpsTime);
    
    if (duration.count() >= 1000) {
        fps_ = frameCount * 1000.0 / duration.count();
        frameCount = 0;
        lastFpsTime = currentTime;
    }
}

void TrackingController::handleKeyPress(char key) {
    switch (key) {
        case ' ': // Space
            if (isPlaying_) {
                if (isPaused_) {
                    play();
                } else {
                    pause();
                }
            }
            break;
        case 's':
        case 'S':
            stop();
            break;
        case 't':
        case 'T':
            toggleTracking();
            break;
        case 'o':
        case 'O':
            togglePartialOcclusion();
            break;
        case 'r':
        case 'R':
            toggleReIdentification();
            break;
        case 'c':
        case 'C':
            toggleCameraCompensation();
            break;
        case 'd':
        case 'D':
            toggleDebugInfo();
            break;
        case 'v':
        case 'V':
            toggleRecording();
            break;
        case 27: // ESC
            stop();
            break;
    }
}

// File operations
void TrackingController::loadVideo(const std::string& videoPath) {
    videoCapture_.open(videoPath);
    if (!videoCapture_.isOpened()) {
        std::cerr << "Error: Could not open video file: " << videoPath << std::endl;
        return;
    }
    
    totalFrames_ = static_cast<int>(videoCapture_.get(cv::CAP_PROP_FRAME_COUNT));
    currentFrame_ = 0;
    
    // Initialize tracker with video
    tracker_->initialize(videoPath);
    
    if (videoLoadedCallback_) {
        videoLoadedCallback_(videoPath);
    }
    
    std::cout << "Video loaded: " << videoPath << std::endl;
    std::cout << "Total frames: " << totalFrames_ << std::endl;
}

void TrackingController::saveVideo(const std::string& outputPath) {
    if (!videoCapture_.isOpened()) return;
    
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    double fps = videoCapture_.get(cv::CAP_PROP_FPS);
    cv::Size frameSize(static_cast<int>(videoCapture_.get(cv::CAP_PROP_FRAME_WIDTH)),
                      static_cast<int>(videoCapture_.get(cv::CAP_PROP_FRAME_HEIGHT)));
    
    videoWriter_.open(outputPath, fourcc, fps, frameSize);
    if (!videoWriter_.isOpened()) {
        std::cerr << "Error: Could not open output video file: " << outputPath << std::endl;
        return;
    }
    
    enableRecording_ = true;
    std::cout << "Recording enabled: " << outputPath << std::endl;
}

// Playback controls
void TrackingController::play() {
    if (!videoCapture_.isOpened()) return;
    
    isPlaying_ = true;
    isPaused_ = false;
    
    if (statusUpdateCallback_) {
        statusUpdateCallback_("Playing");
    }
}

void TrackingController::pause() {
    isPaused_ = true;
    
    if (statusUpdateCallback_) {
        statusUpdateCallback_("Paused");
    }
}

void TrackingController::stop() {
    isPlaying_ = false;
    isPaused_ = false;
    currentFrame_ = 0;
    
    if (videoWriter_.isOpened()) {
        videoWriter_.release();
        enableRecording_ = false;
    }
    
    if (statusUpdateCallback_) {
        statusUpdateCallback_("Stopped");
    }
}

void TrackingController::stepForward() {
    if (!videoCapture_.isOpened()) return;
    
    currentFrame_ = std::min(currentFrame_ + 1, totalFrames_);
    processFrame();
}

void TrackingController::stepBackward() {
    if (!videoCapture_.isOpened()) return;
    
    currentFrame_ = std::max(currentFrame_ - 1, 0);
    processFrame();
}

// Tracking controls
void TrackingController::enableBasicTracking() {
    isAdvancedMode_ = false;
    updateTrackingState();
}

void TrackingController::enableAdvancedTracking() {
    isAdvancedMode_ = true;
    updateTrackingState();
}

void TrackingController::toggleTracking() {
    isAdvancedMode_ = !isAdvancedMode_;
    updateTrackingState();
    
    std::cout << "Switched to " << (isAdvancedMode_ ? "Advanced" : "Basic") << " mode" << std::endl;
}

// Advanced features
void TrackingController::setOcclusionThreshold(float threshold) {
    occlusionThreshold_ = threshold;
    if (tracker_) {
        tracker_->setOcclusionThreshold(threshold);
    }
}

void TrackingController::setReIdThreshold(float threshold) {
    reIdThreshold_ = threshold;
    if (tracker_) {
        tracker_->setReIdThreshold(threshold);
    }
}

void TrackingController::setCameraSensitivity(float sensitivity) {
    cameraSensitivity_ = sensitivity;
    if (tracker_) {
        tracker_->setCameraMotionSensitivity(sensitivity);
    }
}

void TrackingController::togglePartialOcclusion() {
    partialOcclusionEnabled_ = !partialOcclusionEnabled_;
    if (tracker_) {
        tracker_->enablePartialOcclusionHandling(partialOcclusionEnabled_);
    }
    std::cout << "Partial occlusion: " << (partialOcclusionEnabled_ ? "ON" : "OFF") << std::endl;
}

void TrackingController::toggleReIdentification() {
    reIdEnabled_ = !reIdEnabled_;
    if (tracker_) {
        tracker_->enableReIdentification(reIdEnabled_);
    }
    std::cout << "Re-identification: " << (reIdEnabled_ ? "ON" : "OFF") << std::endl;
}

void TrackingController::toggleCameraCompensation() {
    cameraCompensationEnabled_ = !cameraCompensationEnabled_;
    if (tracker_) {
        tracker_->enableCameraMotionCompensation(cameraCompensationEnabled_);
    }
    std::cout << "Camera compensation: " << (cameraCompensationEnabled_ ? "ON" : "OFF") << std::endl;
}

// Display options
void TrackingController::toggleDebugInfo() {
    showDebugInfo_ = !showDebugInfo_;
    if (tracker_) {
        tracker_->setDebugMode(showDebugInfo_);
    }
    std::cout << "Debug info: " << (showDebugInfo_ ? "ON" : "OFF") << std::endl;
}

void TrackingController::toggleRecording() {
    enableRecording_ = !enableRecording_;
    if (!enableRecording_ && videoWriter_.isOpened()) {
        videoWriter_.release();
    }
    std::cout << "Recording: " << (enableRecording_ ? "ON" : "OFF") << std::endl;
}

void TrackingController::setDetectionThreshold(float threshold) {
    detectionThreshold_ = threshold;
    // Note: This would need to be implemented in the tracker
}

// Statistics
double TrackingController::getFPS() const { return fps_; }
int TrackingController::getFrameCount() const { return currentFrame_; }
int TrackingController::getTotalFrames() const { return totalFrames_; }
double TrackingController::getProgress() const { 
    return totalFrames_ > 0 ? static_cast<double>(currentFrame_) / totalFrames_ : 0.0; 
}

// Callbacks
void TrackingController::setVideoLoadedCallback(std::function<void(const std::string&)> callback) {
    videoLoadedCallback_ = callback;
}

void TrackingController::setTrackingUpdateCallback(std::function<void(const std::vector<cv::Rect>&)> callback) {
    trackingUpdateCallback_ = callback;
}

void TrackingController::setStatusUpdateCallback(std::function<void(const std::string&)> callback) {
    statusUpdateCallback_ = callback;
}

// Private methods
void TrackingController::updateTrackingState() {
    if (!tracker_) return;
    
    if (isAdvancedMode_) {
        tracker_->enablePartialOcclusionHandling(partialOcclusionEnabled_);
        tracker_->enableReIdentification(reIdEnabled_);
        tracker_->enableCameraMotionCompensation(cameraCompensationEnabled_);
    }
}

void TrackingController::saveSettings() {
    // Save settings to file (implementation needed)
}

void TrackingController::loadSettings() {
    // Load settings from file (implementation needed)
}

void TrackingController::handleMouseEvents(int event, int x, int y, int flags, void* userdata) {
    TrackingController* controller = static_cast<TrackingController*>(userdata);
    if (controller && event == cv::EVENT_LBUTTONDOWN) {
        controller->selectTarget(x, y);
    }
}

void TrackingController::selectTarget(int x, int y) {
    if (tracker_) {
        tracker_->handleMouseClick(x, y);
    }
}

void TrackingController::clearTarget() {
    if (tracker_) {
        tracker_->clearTargetSelection();
    }
}

void TrackingController::enableTargetSelection(bool enable) {
    targetSelectionMode_ = enable;
    if (tracker_) {
        tracker_->setTargetSelectionMode(enable);
    }
}

void TrackingController::setPlaybackSpeed(double speed) {
    playbackSpeed_ = speed;
}

// Static mouse callback
void TrackingController::onMouse(int event, int x, int y, int flags, void* userdata) {
    TrackingController* controller = static_cast<TrackingController*>(userdata);
    if (controller) {
        controller->handleMouseEvents(event, x, y, flags, userdata);
    }
}

std::string TrackingController::openFileDialog(const std::string& title, const std::string& filter) {
#ifdef __APPLE__
    // Use AppleScript to show file dialog
    std::string script = "osascript -e 'set theFile to choose file with prompt \"" + title + "\"' -e 'POSIX path of theFile'";
    FILE* pipe = popen(script.c_str(), "r");
    if (!pipe) return "";
    char buffer[512];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    // Remove trailing newline
    if (!result.empty() && result[result.size()-1] == '\n') result.pop_back();
    return result;
#else
    // Fallback: ask user in terminal
    std::string path;
    std::cout << "Enter video file path: ";
    std::getline(std::cin, path);
    return path;
#endif
}

std::string TrackingController::saveFileDialog(const std::string& title, const std::string& defaultName) {
#ifdef __APPLE__
    // Use AppleScript to show save dialog
    std::string script = "osascript -e 'set theFile to choose file name with prompt \"" + title + "\" default name \"" + defaultName + "\"' -e 'POSIX path of theFile'";
    FILE* pipe = popen(script.c_str(), "r");
    if (!pipe) return "";
    char buffer[512];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    if (!result.empty() && result[result.size()-1] == '\n') result.pop_back();
    return result;
#else
    std::string path;
    std::cout << "Enter output file path: ";
    std::getline(std::cin, path);
    return path;
#endif
}

void TrackingController::selectVideoFile() {
    std::string path = openFileDialog("Select a video file");
    if (!path.empty()) {
        loadVideo(path);
    }
}

void TrackingController::selectOutputFile() {
    std::string path = saveFileDialog("Select output file", "output.mp4");
    if (!path.empty()) {
        saveVideo(path);
    }
} 
#pragma once

#include "AdvancedTrackingSystem.h"
#include "VehicleDetector.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>

class AdvancedCarTracker {
private:
    std::unique_ptr<AdvancedTrackingSystem> trackingSystem_;
    std::unique_ptr<VehicleDetector> vehicleDetector_;
    cv::VideoCapture videoCapture_;
    cv::VideoWriter videoWriter_;
    
    bool isRunning_;
    bool showDebugInfo_;
    bool enableRecording_;
    std::string outputVideoPath_;
    
    // Interactive target selection
    bool targetSelectionMode_;
    cv::Point lastClickPoint_;
    bool targetSelected_;
    int selectedTargetId_;
    
    // Performance metrics
    int frameCount_;
    double totalProcessingTime_;
    double averageFPS_;
    
    int frameSkip;
    int frameCounter;
    bool realtimeMode;
    float resolutionScale;

public:
    AdvancedCarTracker();
    ~AdvancedCarTracker();
    
    bool initialize(const std::string& videoPath, const std::string& modelPath = "");
    bool initializeCamera(int cameraIndex = 0);
    void run();
    void stop();
    
    // Configuration
    void setDebugMode(bool enable);
    void setRecordingMode(bool enable, const std::string& outputPath = "");
    void setTargetSelectionMode(bool enable);
    
    // Interactive features
    void handleMouseClick(int x, int y);
    void selectTarget(const cv::Point& point);
    void clearTargetSelection();
    
    // Performance monitoring
    double getAverageFPS() const;
    int getFrameCount() const;
    double getTotalProcessingTime() const;
    
    // Advanced features
    void enablePartialOcclusionHandling(bool enable);
    void enableReIdentification(bool enable);
    void enableCameraMotionCompensation(bool enable);
    void setOcclusionThreshold(float threshold);
    void setReIdThreshold(float threshold);
    void setCameraMotionSensitivity(float sensitivity);
    
    // Frame processing (public for controller access)
    void processFrame(const cv::Mat& frame);
    
    bool processVideo();
    
    // Parameter setters
    void setFrameSkip(int skip);
    void setRealtimeMode(bool mode);
    void setResolutionScale(float scale);

private:
    void drawUI(cv::Mat& frame);
    void drawPerformanceInfo(cv::Mat& frame);
    void drawTargetInfo(cv::Mat& frame);
    void handleKeyPress(int key);
    void saveFrame(const cv::Mat& frame);
    void updatePerformanceMetrics(double processingTime);
    
    // Mouse callback wrapper
    static void onMouse(int event, int x, int y, int flags, void* userdata);
}; 
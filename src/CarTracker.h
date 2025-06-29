#pragma once

#include "VehicleDetector.h"
#include "TrackingSystem.h"
#include <opencv2/opencv.hpp>
#include <string>

class CarTracker {
public:
    CarTracker();
    ~CarTracker();
    
    bool initialize();
    bool processVideo(const std::string& videoPath, const std::string& outputPath = "");
    bool processFrame(const cv::Mat& frame, cv::Mat& outputFrame);
    
    void setDetectionThreshold(float threshold);
    void setTrackingParameters(int maxAge, int minHits, float iouThreshold);
    void enableDisplay(bool enable);
    void setOutputPath(const std::string& path);
    
    // Statistics
    int getTotalVehiclesDetected() const;
    int getCurrentActiveTracks() const;
    double getAverageProcessingTime() const;
    
private:
    VehicleDetector detector_;
    TrackingSystem tracker_;
    bool displayEnabled_;
    std::string outputPath_;
    
    // Statistics
    int totalVehiclesDetected_;
    std::vector<double> processingTimes_;
    
    void drawStatistics(cv::Mat& frame);
    void saveFrame(const cv::Mat& frame, int frameNumber);
    void updateStatistics(double processingTime);
}; 
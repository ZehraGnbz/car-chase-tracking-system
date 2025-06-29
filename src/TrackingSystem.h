#pragma once

#include "VehicleDetector.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <unordered_map>

struct TrackedVehicle {
    int id;
    cv::Rect boundingBox;
    cv::Point2f velocity;
    float confidence;
    int age;
    int totalHits;
    int consecutiveHits;
    int consecutiveMisses;
    cv::KalmanFilter kalmanFilter;
    std::string label;
    bool isActive;
    
    TrackedVehicle() : id(-1), confidence(0.0f), age(0), totalHits(0), 
                      consecutiveHits(0), consecutiveMisses(0), isActive(false) {}
};

class TrackingSystem {
public:
    TrackingSystem();
    ~TrackingSystem();
    
    void initialize();
    std::vector<TrackedVehicle> update(const std::vector<Detection>& detections, 
                                      const cv::Mat& frame);
    void drawTracks(cv::Mat& frame, const std::vector<TrackedVehicle>& tracks);
    void reset();
    
    void setMaxAge(int maxAge);
    void setMinHits(int minHits);
    void setIoUThreshold(float threshold);
    
private:
    std::vector<TrackedVehicle> tracks_;
    int nextId_;
    int maxAge_;
    int minHits_;
    float iouThreshold_;
    
    cv::KalmanFilter createKalmanFilter();
    void predictTracks();
    void updateTracks(const std::vector<Detection>& detections);
    void createNewTracks(const std::vector<Detection>& detections);
    void removeStaleTracks();
    float calculateIoU(const cv::Rect& rect1, const cv::Rect& rect2);
    cv::Point2f calculateVelocity(const cv::Rect& prevBox, const cv::Rect& currBox);
    void updateKalmanFilter(TrackedVehicle& track, const cv::Rect& detection);
    cv::Rect predictKalmanPosition(const TrackedVehicle& track);
}; 
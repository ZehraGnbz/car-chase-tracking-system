#pragma once

#include "TrackingSystem.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <deque>

struct AdvancedTrackedVehicle : public TrackedVehicle {
    // Appearance features for re-identification
    cv::Mat appearanceFeatures;
    std::deque<cv::Mat> featureHistory;
    
    // Occlusion handling
    float visibilityRatio;  // 0.0 = fully occluded, 1.0 = fully visible
    cv::Rect estimatedFullBox;  // Estimated full bounding box when partially visible
    bool isPartiallyOccluded;
    
    // Motion prediction
    cv::Point2f predictedPosition;
    cv::Point2f velocityHistory[5];  // Last 5 velocity measurements
    float motionConfidence;
    
    // Re-identification
    int reIdScore;
    std::string uniqueSignature;
    
    // Camera motion compensation
    cv::Point2f cameraMotionOffset;
    
    AdvancedTrackedVehicle() : visibilityRatio(1.0f), isPartiallyOccluded(false), 
                              motionConfidence(0.0f), reIdScore(0) {}
};

class AdvancedTrackingSystem : public TrackingSystem {
public:
    AdvancedTrackingSystem();
    ~AdvancedTrackingSystem();
    
    void initialize();
    std::vector<AdvancedTrackedVehicle> updateAdvanced(const std::vector<Detection>& detections, 
                                                       const cv::Mat& frame);
    
    // Target selection and management
    void setPrimaryTarget(int targetId);
    void clearPrimaryTarget();
    int getPrimaryTargetId() const;
    bool isPrimaryTarget(int trackId) const;
    
    // Occlusion handling
    void setOcclusionThreshold(float threshold);
    void enablePartialTracking(bool enable);
    
    // Re-identification
    void setReIdThreshold(float threshold);
    void enableReIdentification(bool enable);
    
    // Camera motion compensation
    void enableCameraMotionCompensation(bool enable);
    void setCameraMotionSensitivity(float sensitivity);
    
    // Advanced visualization
    void drawAdvancedTracks(cv::Mat& frame, const std::vector<AdvancedTrackedVehicle>& tracks);
    void drawTargetSelection(cv::Mat& frame);

protected:
    std::vector<AdvancedTrackedVehicle> advancedTracks_;
    int primaryTargetId_;
    bool partialTrackingEnabled_;
    bool reIdEnabled_;
    bool cameraMotionCompensationEnabled_;
    
    float occlusionThreshold_;
    float reIdThreshold_;
    float cameraMotionSensitivity_;
    
    cv::Mat previousFrame_;
    cv::Point2f globalCameraMotion_;
    
    // Advanced tracking methods
    void updateAdvancedTracks(const std::vector<Detection>& detections, const cv::Mat& frame);
    void handlePartialOcclusion(AdvancedTrackedVehicle& track, const cv::Mat& frame);
    void estimateFullBoundingBox(AdvancedTrackedVehicle& track);
    cv::Mat extractAppearanceFeatures(const cv::Mat& frame, const cv::Rect& roi);
    float calculateReIdScore(const cv::Mat& features1, const cv::Mat& features2);
    void updateCameraMotion(const cv::Mat& currentFrame, const cv::Mat& previousFrame);
    void compensateCameraMotion(std::vector<Detection>& detections);
    void predictMotion(AdvancedTrackedVehicle& track);
    void updateVelocityHistory(AdvancedTrackedVehicle& track);
    cv::Point2f calculatePredictedPosition(const AdvancedTrackedVehicle& track);
    void handleTargetReacquisition(AdvancedTrackedVehicle& track, const std::vector<Detection>& detections);
    bool isSimilarVehicle(const AdvancedTrackedVehicle& track1, const AdvancedTrackedVehicle& track2);
    void mergeSimilarTracks();
    void generateUniqueSignature(AdvancedTrackedVehicle& track);
}; 
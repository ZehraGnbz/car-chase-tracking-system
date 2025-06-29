#include "AdvancedTrackingSystem.h"
#include <iostream>
#include <algorithm>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

AdvancedTrackingSystem::AdvancedTrackingSystem() 
    : primaryTargetId_(-1), partialTrackingEnabled_(true), reIdEnabled_(true),
      cameraMotionCompensationEnabled_(true), occlusionThreshold_(0.3f),
      reIdThreshold_(0.7f), cameraMotionSensitivity_(0.1f) {
}

AdvancedTrackingSystem::~AdvancedTrackingSystem() {
}

void AdvancedTrackingSystem::initialize() {
    TrackingSystem::initialize();
    advancedTracks_.clear();
    primaryTargetId_ = -1;
    globalCameraMotion_ = cv::Point2f(0, 0);
}

std::vector<AdvancedTrackedVehicle> AdvancedTrackingSystem::updateAdvanced(
    const std::vector<Detection>& detections, const cv::Mat& frame) {
    
    // Update camera motion if enabled
    if (cameraMotionCompensationEnabled_ && !previousFrame_.empty()) {
        updateCameraMotion(frame, previousFrame_);
    }
    
    // Compensate for camera motion in detections
    std::vector<Detection> compensatedDetections = detections;
    if (cameraMotionCompensationEnabled_) {
        compensateCameraMotion(compensatedDetections);
    }
    
    // Update advanced tracks
    updateAdvancedTracks(compensatedDetections, frame);
    
    // Handle partial occlusion for all tracks
    for (auto& track : advancedTracks_) {
        if (track.isActive) {
            handlePartialOcclusion(track, frame);
            predictMotion(track);
        }
    }
    
    // Handle target reacquisition for primary target
    if (primaryTargetId_ >= 0) {
        auto primaryTrack = std::find_if(advancedTracks_.begin(), advancedTracks_.end(),
            [this](const AdvancedTrackedVehicle& track) {
                return track.id == primaryTargetId_;
            });
        
        if (primaryTrack != advancedTracks_.end() && primaryTrack->isActive) {
            handleTargetReacquisition(*primaryTrack, compensatedDetections);
        }
    }
    
    // Merge similar tracks
    mergeSimilarTracks();
    
    // Store current frame for next iteration
    frame.copyTo(previousFrame_);
    
    // Return active tracks
    std::vector<AdvancedTrackedVehicle> activeTracks;
    for (const auto& track : advancedTracks_) {
        if (track.isActive) {
            activeTracks.push_back(track);
        }
    }
    
    return activeTracks;
}

void AdvancedTrackingSystem::updateAdvancedTracks(const std::vector<Detection>& detections, 
                                                  const cv::Mat& frame) {
    // First, update basic tracking
    std::vector<TrackedVehicle> basicTracks = TrackingSystem::update(detections, frame);
    
    // Update advanced tracks with basic tracking results
    for (const auto& basicTrack : basicTracks) {
        auto advancedTrack = std::find_if(advancedTracks_.begin(), advancedTracks_.end(),
            [&basicTrack](const AdvancedTrackedVehicle& track) {
                return track.id == basicTrack.id;
            });
        
        if (advancedTrack != advancedTracks_.end()) {
            // Update basic properties
            advancedTrack->boundingBox = basicTrack.boundingBox;
            advancedTrack->confidence = basicTrack.confidence;
            advancedTrack->label = basicTrack.label;
            advancedTrack->velocity = basicTrack.velocity;
            advancedTrack->isActive = basicTrack.isActive;
            
            // Update appearance features
            if (!frame.empty() && !basicTrack.boundingBox.empty()) {
                advancedTrack->appearanceFeatures = extractAppearanceFeatures(frame, basicTrack.boundingBox);
                advancedTrack->featureHistory.push_back(advancedTrack->appearanceFeatures.clone());
                
                // Keep only last 10 features
                if (advancedTrack->featureHistory.size() > 10) {
                    advancedTrack->featureHistory.pop_front();
                }
            }
            
            // Update velocity history
            updateVelocityHistory(*advancedTrack);
            
            // Generate unique signature
            generateUniqueSignature(*advancedTrack);
        } else {
            // Create new advanced track
            AdvancedTrackedVehicle newTrack;
            newTrack.id = basicTrack.id;
            newTrack.boundingBox = basicTrack.boundingBox;
            newTrack.confidence = basicTrack.confidence;
            newTrack.label = basicTrack.label;
            newTrack.velocity = basicTrack.velocity;
            newTrack.isActive = basicTrack.isActive;
            
            if (!frame.empty() && !basicTrack.boundingBox.empty()) {
                newTrack.appearanceFeatures = extractAppearanceFeatures(frame, basicTrack.boundingBox);
                newTrack.featureHistory.push_back(newTrack.appearanceFeatures.clone());
            }
            
            generateUniqueSignature(newTrack);
            advancedTracks_.push_back(newTrack);
        }
    }
}

void AdvancedTrackingSystem::handlePartialOcclusion(AdvancedTrackedVehicle& track, const cv::Mat& frame) {
    if (!partialTrackingEnabled_) return;
    
    // Calculate visibility ratio based on detection confidence and bounding box size
    float expectedArea = track.estimatedFullBox.area();
    float actualArea = track.boundingBox.area();
    
    if (expectedArea > 0) {
        track.visibilityRatio = actualArea / expectedArea;
    } else {
        track.visibilityRatio = 1.0f;
    }
    
    // Check if track is partially occluded
    track.isPartiallyOccluded = (track.visibilityRatio < occlusionThreshold_);
    
    if (track.isPartiallyOccluded) {
        // Estimate full bounding box
        estimateFullBoundingBox(track);
        
        // Use motion prediction to maintain tracking
        track.predictedPosition = calculatePredictedPosition(track);
        
        // Adjust confidence based on occlusion
        track.confidence *= (0.5f + 0.5f * track.visibilityRatio);
    } else {
        // Update estimated full box when fully visible
        track.estimatedFullBox = track.boundingBox;
    }
}

void AdvancedTrackingSystem::estimateFullBoundingBox(AdvancedTrackedVehicle& track) {
    // Use velocity and previous full box to estimate current full box
    if (track.estimatedFullBox.area() > 0) {
        cv::Point2f center(track.estimatedFullBox.x + track.estimatedFullBox.width / 2.0f,
                          track.estimatedFullBox.y + track.estimatedFullBox.height / 2.0f);
        
        // Predict center position
        cv::Point2f predictedCenter = center + track.velocity;
        
        // Create estimated full box
        track.estimatedFullBox.x = predictedCenter.x - track.estimatedFullBox.width / 2;
        track.estimatedFullBox.y = predictedCenter.y - track.estimatedFullBox.height / 2;
    } else {
        // If no previous full box, estimate based on current partial box
        float scaleFactor = 1.0f / track.visibilityRatio;
        cv::Point2f center(track.boundingBox.x + track.boundingBox.width / 2.0f,
                          track.boundingBox.y + track.boundingBox.height / 2.0f);
        
        int estimatedWidth = static_cast<int>(track.boundingBox.width * scaleFactor);
        int estimatedHeight = static_cast<int>(track.boundingBox.height * scaleFactor);
        
        track.estimatedFullBox = cv::Rect(
            center.x - estimatedWidth / 2,
            center.y - estimatedHeight / 2,
            estimatedWidth,
            estimatedHeight
        );
    }
}

cv::Mat AdvancedTrackingSystem::extractAppearanceFeatures(const cv::Mat& frame, const cv::Rect& roi) {
    if (roi.empty() || roi.x < 0 || roi.y < 0 || 
        roi.x + roi.width > frame.cols || roi.y + roi.height > frame.rows) {
        return cv::Mat();
    }
    
    cv::Mat roiMat = frame(roi);
    
    // Resize to standard size for feature extraction
    cv::Mat resizedRoi;
    cv::resize(roiMat, resizedRoi, cv::Size(64, 64));
    
    // Convert to grayscale
    cv::Mat grayRoi;
    if (resizedRoi.channels() == 3) {
        cv::cvtColor(resizedRoi, grayRoi, cv::COLOR_BGR2GRAY);
    } else {
        grayRoi = resizedRoi.clone();
    }
    
    // Extract HOG features
    cv::HOGDescriptor hog(cv::Size(64, 64), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9);
    std::vector<float> descriptors;
    hog.compute(grayRoi, descriptors);
    
    // Convert to Mat
    cv::Mat features(descriptors);
    return features.reshape(1, 1);
}

float AdvancedTrackingSystem::calculateReIdScore(const cv::Mat& features1, const cv::Mat& features2) {
    if (features1.empty() || features2.empty()) return 0.0f;
    
    // Calculate cosine similarity
    double dotProduct = features1.dot(features2);
    double norm1 = cv::norm(features1);
    double norm2 = cv::norm(features2);
    
    if (norm1 > 0 && norm2 > 0) {
        return static_cast<float>(dotProduct / (norm1 * norm2));
    }
    
    return 0.0f;
}

void AdvancedTrackingSystem::updateCameraMotion(const cv::Mat& currentFrame, const cv::Mat& previousFrame) {
    // Use optical flow to estimate camera motion
    cv::Mat grayCurrent, grayPrevious;
    cv::cvtColor(currentFrame, grayCurrent, cv::COLOR_BGR2GRAY);
    cv::cvtColor(previousFrame, grayPrevious, cv::COLOR_BGR2GRAY);
    
    // Detect good features to track
    std::vector<cv::Point2f> prevPts, nextPts;
    cv::goodFeaturesToTrack(grayPrevious, prevPts, 100, 0.01, 10);
    
    if (prevPts.empty()) return;
    
    // Calculate optical flow
    std::vector<uchar> status;
    std::vector<float> err;
    cv::calcOpticalFlowPyrLK(grayPrevious, grayCurrent, prevPts, nextPts, status, err);
    
    // Calculate average motion
    cv::Point2f totalMotion(0, 0);
    int validPoints = 0;
    
    for (size_t i = 0; i < prevPts.size(); ++i) {
        if (status[i]) {
            totalMotion += (nextPts[i] - prevPts[i]);
            validPoints++;
        }
    }
    
    if (validPoints > 0) {
        globalCameraMotion_ = totalMotion / validPoints * cameraMotionSensitivity_;
    }
}

void AdvancedTrackingSystem::compensateCameraMotion(std::vector<Detection>& detections) {
    for (auto& detection : detections) {
        detection.boundingBox.x += static_cast<int>(globalCameraMotion_.x);
        detection.boundingBox.y += static_cast<int>(globalCameraMotion_.y);
    }
}

void AdvancedTrackingSystem::predictMotion(AdvancedTrackedVehicle& track) {
    track.predictedPosition = calculatePredictedPosition(track);
    track.motionConfidence = 0.8f; // Base confidence
}

void AdvancedTrackingSystem::updateVelocityHistory(AdvancedTrackedVehicle& track) {
    // Shift velocity history
    for (int i = 4; i > 0; --i) {
        track.velocityHistory[i] = track.velocityHistory[i-1];
    }
    track.velocityHistory[0] = track.velocity;
}

cv::Point2f AdvancedTrackingSystem::calculatePredictedPosition(const AdvancedTrackedVehicle& track) {
    cv::Point2f currentCenter(track.boundingBox.x + track.boundingBox.width / 2.0f,
                             track.boundingBox.y + track.boundingBox.height / 2.0f);
    
    // Calculate average velocity from history
    cv::Point2f avgVelocity(0, 0);
    int validVelocities = 0;
    
    for (int i = 0; i < 5; ++i) {
        if (track.velocityHistory[i].x != 0 || track.velocityHistory[i].y != 0) {
            avgVelocity += track.velocityHistory[i];
            validVelocities++;
        }
    }
    
    if (validVelocities > 0) {
        avgVelocity /= validVelocities;
    } else {
        avgVelocity = track.velocity;
    }
    
    return currentCenter + avgVelocity;
}

void AdvancedTrackingSystem::handleTargetReacquisition(AdvancedTrackedVehicle& track, 
                                                       const std::vector<Detection>& detections) {
    if (!reIdEnabled_) return;
    
    // If primary target is lost, try to re-identify it
    if (!track.isActive || track.isPartiallyOccluded) {
        float bestScore = 0.0f;
        int bestDetectionIdx = -1;
        
        for (size_t i = 0; i < detections.size(); ++i) {
            // Calculate re-identification score
            cv::Mat detectionFeatures = extractAppearanceFeatures(cv::Mat(), detections[i].boundingBox);
            float score = calculateReIdScore(track.appearanceFeatures, detectionFeatures);
            
            if (score > bestScore && score > reIdThreshold_) {
                bestScore = score;
                bestDetectionIdx = i;
            }
        }
        
        if (bestDetectionIdx >= 0) {
            // Re-acquire target
            const auto& detection = detections[bestDetectionIdx];
            track.boundingBox = detection.boundingBox;
            track.confidence = detection.confidence;
            track.isActive = true;
            track.isPartiallyOccluded = false;
            track.visibilityRatio = 1.0f;
            track.reIdScore = static_cast<int>(bestScore * 100);
            
            std::cout << "Re-acquired primary target " << track.id 
                      << " with score: " << bestScore << std::endl;
        }
    }
}

bool AdvancedTrackingSystem::isSimilarVehicle(const AdvancedTrackedVehicle& track1, 
                                              const AdvancedTrackedVehicle& track2) {
    // Check if vehicles are similar based on appearance and size
    if (track1.label != track2.label) return false;
    
    // Compare sizes
    float sizeRatio = static_cast<float>(track1.boundingBox.area()) / track2.boundingBox.area();
    if (sizeRatio < 0.5f || sizeRatio > 2.0f) return false;
    
    // Compare appearance features
    float appearanceScore = calculateReIdScore(track1.appearanceFeatures, track2.appearanceFeatures);
    
    return appearanceScore > 0.8f;
}

void AdvancedTrackingSystem::mergeSimilarTracks() {
    for (auto it1 = advancedTracks_.begin(); it1 != advancedTracks_.end(); ++it1) {
        for (auto it2 = it1 + 1; it2 != advancedTracks_.end(); ++it2) {
            if (it1->isActive && it2->isActive && isSimilarVehicle(*it1, *it2)) {
                // Merge tracks (keep the one with higher confidence)
                if (it1->confidence > it2->confidence) {
                    it2->isActive = false;
                } else {
                    it1->isActive = false;
                    break;
                }
            }
        }
    }
}

void AdvancedTrackingSystem::generateUniqueSignature(AdvancedTrackedVehicle& track) {
    // Generate a unique signature based on appearance and motion
    std::string signature = track.label + "_";
    signature += std::to_string(track.boundingBox.width) + "x" + std::to_string(track.boundingBox.height) + "_";
    signature += std::to_string(static_cast<int>(track.velocity.x)) + "_" + std::to_string(static_cast<int>(track.velocity.y));
    
    track.uniqueSignature = signature;
}

// Target management methods
void AdvancedTrackingSystem::setPrimaryTarget(int targetId) {
    primaryTargetId_ = targetId;
    std::cout << "Primary target set to: " << targetId << std::endl;
}

void AdvancedTrackingSystem::clearPrimaryTarget() {
    primaryTargetId_ = -1;
}

int AdvancedTrackingSystem::getPrimaryTargetId() const {
    return primaryTargetId_;
}

bool AdvancedTrackingSystem::isPrimaryTarget(int trackId) const {
    return trackId == primaryTargetId_;
}

// Configuration methods
void AdvancedTrackingSystem::setOcclusionThreshold(float threshold) {
    occlusionThreshold_ = threshold;
}

void AdvancedTrackingSystem::enablePartialTracking(bool enable) {
    partialTrackingEnabled_ = enable;
}

void AdvancedTrackingSystem::setReIdThreshold(float threshold) {
    reIdThreshold_ = threshold;
}

void AdvancedTrackingSystem::enableReIdentification(bool enable) {
    reIdEnabled_ = enable;
}

void AdvancedTrackingSystem::enableCameraMotionCompensation(bool enable) {
    cameraMotionCompensationEnabled_ = enable;
}

void AdvancedTrackingSystem::setCameraMotionSensitivity(float sensitivity) {
    cameraMotionSensitivity_ = sensitivity;
}

// Visualization methods
void AdvancedTrackingSystem::drawAdvancedTracks(cv::Mat& frame, 
                                                const std::vector<AdvancedTrackedVehicle>& tracks) {
    for (const auto& track : tracks) {
        if (!track.isActive) continue;
        
        // Choose color based on track status
        cv::Scalar color;
        if (track.id == primaryTargetId_) {
            color = cv::Scalar(0, 255, 255); // Yellow for primary target
        } else if (track.isPartiallyOccluded) {
            color = cv::Scalar(0, 165, 255); // Orange for occluded
        } else if (track.label == "car") {
            color = cv::Scalar(0, 255, 0); // Green for cars
        } else if (track.label == "truck") {
            color = cv::Scalar(0, 0, 255); // Red for trucks
        } else {
            color = cv::Scalar(255, 255, 0); // Cyan for others
        }
        
        // Draw bounding box
        cv::rectangle(frame, track.boundingBox, color, 2);
        
        // Draw estimated full box if partially occluded
        if (track.isPartiallyOccluded) {
            cv::rectangle(frame, track.estimatedFullBox, color, 1, cv::LINE_8);
        }
        
        // Draw ID and label
        std::string label = track.label + " #" + std::to_string(track.id);
        if (track.id == primaryTargetId_) {
            label += " [PRIMARY]";
        }
        if (track.isPartiallyOccluded) {
            label += " [OCCLUDED]";
        }
        
        cv::putText(frame, label, cv::Point(track.boundingBox.x, track.boundingBox.y - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, color, 2);
        
        // Draw visibility ratio
        std::string visText = "Vis: " + std::to_string(static_cast<int>(track.visibilityRatio * 100)) + "%";
        cv::putText(frame, visText, cv::Point(track.boundingBox.x, track.boundingBox.y + track.boundingBox.height + 20),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
        
        // Draw predicted position
        if (track.motionConfidence > 0.5f) {
            cv::circle(frame, track.predictedPosition, 5, color, -1);
            cv::line(frame, cv::Point(track.boundingBox.x + track.boundingBox.width / 2,
                                    track.boundingBox.y + track.boundingBox.height / 2),
                    track.predictedPosition, color, 2);
        }
    }
}

void AdvancedTrackingSystem::drawTargetSelection(cv::Mat& frame) {
    if (primaryTargetId_ >= 0) {
        std::string text = "Primary Target: " + std::to_string(primaryTargetId_);
        cv::putText(frame, text, cv::Point(10, frame.rows - 60), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2);
    }
} 
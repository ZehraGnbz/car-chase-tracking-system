#include "TrackingSystem.h"
#include <iostream>
#include <algorithm>

TrackingSystem::TrackingSystem() 
    : nextId_(0), maxAge_(30), minHits_(3), iouThreshold_(0.3f) {
}

TrackingSystem::~TrackingSystem() {
}

void TrackingSystem::initialize() {
    tracks_.clear();
    nextId_ = 0;
}

std::vector<TrackedVehicle> TrackingSystem::update(const std::vector<Detection>& detections, 
                                                  const cv::Mat& frame) {
    // Predict new locations of existing tracks
    predictTracks();
    
    // Update tracks with detections
    updateTracks(detections);
    
    // Create new tracks for unmatched detections
    createNewTracks(detections);
    
    // Remove stale tracks
    removeStaleTracks();
    
    // Return active tracks
    std::vector<TrackedVehicle> activeTracks;
    for (const auto& track : tracks_) {
        if (track.isActive) {
            activeTracks.push_back(track);
        }
    }
    
    return activeTracks;
}

void TrackingSystem::drawTracks(cv::Mat& frame, const std::vector<TrackedVehicle>& tracks) {
    for (const auto& track : tracks) {
        if (!track.isActive) continue;
        
        // Draw bounding box
        cv::Scalar color;
        if (track.label == "car") {
            color = cv::Scalar(0, 255, 0); // Green for cars
        } else if (track.label == "truck") {
            color = cv::Scalar(0, 0, 255); // Red for trucks
        } else if (track.label == "bus") {
            color = cv::Scalar(255, 0, 0); // Blue for buses
        } else {
            color = cv::Scalar(255, 255, 0); // Cyan for others
        }
        
        cv::rectangle(frame, track.boundingBox, color, 2);
        
        // Draw ID and label
        std::string label = track.label + " #" + std::to_string(track.id);
        cv::putText(frame, label, cv::Point(track.boundingBox.x, track.boundingBox.y - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, color, 2);
        
        // Draw velocity vector
        if (track.velocity.x != 0 || track.velocity.y != 0) {
            cv::Point2f center(track.boundingBox.x + track.boundingBox.width / 2,
                             track.boundingBox.y + track.boundingBox.height / 2);
            cv::Point2f endPoint = center + track.velocity * 10;
            cv::arrowedLine(frame, center, endPoint, color, 2, 8, 0, 0.3);
        }
        
        // Draw confidence
        std::string confText = "Conf: " + std::to_string(static_cast<int>(track.confidence * 100)) + "%";
        cv::putText(frame, confText, cv::Point(track.boundingBox.x, track.boundingBox.y + track.boundingBox.height + 20),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
    }
}

void TrackingSystem::reset() {
    tracks_.clear();
    nextId_ = 0;
}

void TrackingSystem::setMaxAge(int maxAge) {
    maxAge_ = maxAge;
}

void TrackingSystem::setMinHits(int minHits) {
    minHits_ = minHits;
}

void TrackingSystem::setIoUThreshold(float threshold) {
    iouThreshold_ = threshold;
}

cv::KalmanFilter TrackingSystem::createKalmanFilter() {
    // State: [x, y, width, height, vx, vy, vw, vh]
    // Measurement: [x, y, width, height]
    cv::KalmanFilter kf(8, 4, 0);
    
    // State transition matrix
    kf.transitionMatrix = cv::Mat::eye(8, 8, CV_32F);
    kf.transitionMatrix.at<float>(0, 4) = 1.0f; // x += vx
    kf.transitionMatrix.at<float>(1, 5) = 1.0f; // y += vy
    kf.transitionMatrix.at<float>(2, 6) = 1.0f; // width += vw
    kf.transitionMatrix.at<float>(3, 7) = 1.0f; // height += vh
    
    // Measurement matrix
    kf.measurementMatrix = cv::Mat::zeros(4, 8, CV_32F);
    kf.measurementMatrix.at<float>(0, 0) = 1.0f;
    kf.measurementMatrix.at<float>(1, 1) = 1.0f;
    kf.measurementMatrix.at<float>(2, 2) = 1.0f;
    kf.measurementMatrix.at<float>(3, 3) = 1.0f;
    
    // Process noise covariance
    kf.processNoiseCov = cv::Mat::eye(8, 8, CV_32F) * 1e-2;
    kf.processNoiseCov.at<float>(4, 4) = 1e-1; // Velocity noise
    kf.processNoiseCov.at<float>(5, 5) = 1e-1;
    kf.processNoiseCov.at<float>(6, 6) = 1e-1;
    kf.processNoiseCov.at<float>(7, 7) = 1e-1;
    
    // Measurement noise covariance
    kf.measurementNoiseCov = cv::Mat::eye(4, 4, CV_32F) * 1e-1;
    
    // Initial state covariance
    kf.errorCovPost = cv::Mat::eye(8, 8, CV_32F) * 1e-1;
    
    return kf;
}

void TrackingSystem::predictTracks() {
    for (auto& track : tracks_) {
        if (track.isActive) {
            // Predict using Kalman filter
            cv::KalmanFilter tempKF = track.kalmanFilter;
            cv::Mat prediction = tempKF.predict();
            
            // Update bounding box from prediction
            track.boundingBox.x = static_cast<int>(prediction.at<float>(0));
            track.boundingBox.y = static_cast<int>(prediction.at<float>(1));
            track.boundingBox.width = static_cast<int>(prediction.at<float>(2));
            track.boundingBox.height = static_cast<int>(prediction.at<float>(3));
            
            // Update velocity
            track.velocity.x = prediction.at<float>(4);
            track.velocity.y = prediction.at<float>(5);
            
            track.age++;
            track.consecutiveMisses++;
        }
    }
}

void TrackingSystem::updateTracks(const std::vector<Detection>& detections) {
    std::vector<bool> detectionMatched(detections.size(), false);
    
    for (auto& track : tracks_) {
        if (!track.isActive) continue;
        
        float bestIoU = 0.0f;
        int bestDetectionIdx = -1;
        
        // Find best matching detection
        for (size_t i = 0; i < detections.size(); ++i) {
            if (detectionMatched[i]) continue;
            
            float iou = calculateIoU(track.boundingBox, detections[i].boundingBox);
            if (iou > bestIoU && iou > iouThreshold_) {
                bestIoU = iou;
                bestDetectionIdx = i;
            }
        }
        
        if (bestDetectionIdx >= 0) {
            // Update track with detection
            const auto& detection = detections[bestDetectionIdx];
            updateKalmanFilter(track, detection.boundingBox);
            
            track.boundingBox = detection.boundingBox;
            track.confidence = detection.confidence;
            track.label = detection.label;
            track.totalHits++;
            track.consecutiveHits++;
            track.consecutiveMisses = 0;
            track.isActive = true;
            
            detectionMatched[bestDetectionIdx] = true;
        } else {
            // No detection matched
            if (track.consecutiveMisses > maxAge_) {
                track.isActive = false;
            }
        }
    }
}

void TrackingSystem::createNewTracks(const std::vector<Detection>& detections) {
    for (size_t i = 0; i < detections.size(); ++i) {
        bool matched = false;
        
        // Check if detection is already matched to existing track
        for (const auto& track : tracks_) {
            if (track.isActive) {
                float iou = calculateIoU(track.boundingBox, detections[i].boundingBox);
                if (iou > iouThreshold_) {
                    matched = true;
                    break;
                }
            }
        }
        
        if (!matched) {
            // Create new track
            TrackedVehicle newTrack;
            newTrack.id = nextId_++;
            newTrack.boundingBox = detections[i].boundingBox;
            newTrack.confidence = detections[i].confidence;
            newTrack.label = detections[i].label;
            newTrack.kalmanFilter = createKalmanFilter();
            
            // Initialize Kalman filter state
            cv::Mat state = cv::Mat::zeros(8, 1, CV_32F);
            state.at<float>(0) = static_cast<float>(detections[i].boundingBox.x);
            state.at<float>(1) = static_cast<float>(detections[i].boundingBox.y);
            state.at<float>(2) = static_cast<float>(detections[i].boundingBox.width);
            state.at<float>(3) = static_cast<float>(detections[i].boundingBox.height);
            newTrack.kalmanFilter.statePre = state.clone();
            newTrack.kalmanFilter.statePost = state.clone();
            
            newTrack.isActive = true;
            tracks_.push_back(newTrack);
        }
    }
}

void TrackingSystem::removeStaleTracks() {
    tracks_.erase(
        std::remove_if(tracks_.begin(), tracks_.end(),
                      [this](const TrackedVehicle& track) {
                          return !track.isActive || 
                                 (track.consecutiveMisses > maxAge_ && track.totalHits < minHits_);
                      }),
        tracks_.end()
    );
}

float TrackingSystem::calculateIoU(const cv::Rect& rect1, const cv::Rect& rect2) {
    int x1 = std::max(rect1.x, rect2.x);
    int y1 = std::max(rect1.y, rect2.y);
    int x2 = std::min(rect1.x + rect1.width, rect2.x + rect2.width);
    int y2 = std::min(rect1.y + rect1.height, rect2.y + rect2.height);
    
    if (x2 < x1 || y2 < y1) return 0.0f;
    
    int intersection = (x2 - x1) * (y2 - y1);
    int area1 = rect1.width * rect1.height;
    int area2 = rect2.width * rect2.height;
    int union_area = area1 + area2 - intersection;
    
    return static_cast<float>(intersection) / static_cast<float>(union_area);
}

cv::Point2f TrackingSystem::calculateVelocity(const cv::Rect& prevBox, const cv::Rect& currBox) {
    cv::Point2f prevCenter(prevBox.x + prevBox.width / 2.0f, prevBox.y + prevBox.height / 2.0f);
    cv::Point2f currCenter(currBox.x + currBox.width / 2.0f, currBox.y + currBox.height / 2.0f);
    
    return currCenter - prevCenter;
}

void TrackingSystem::updateKalmanFilter(TrackedVehicle& track, const cv::Rect& detection) {
    cv::Mat measurement = cv::Mat::zeros(4, 1, CV_32F);
    measurement.at<float>(0) = static_cast<float>(detection.x);
    measurement.at<float>(1) = static_cast<float>(detection.y);
    measurement.at<float>(2) = static_cast<float>(detection.width);
    measurement.at<float>(3) = static_cast<float>(detection.height);
    
    track.kalmanFilter.correct(measurement);
}

cv::Rect TrackingSystem::predictKalmanPosition(const TrackedVehicle& track) {
    cv::KalmanFilter tempKF = track.kalmanFilter;
    cv::Mat prediction = tempKF.predict();
    
    cv::Rect predictedBox;
    predictedBox.x = static_cast<int>(prediction.at<float>(0));
    predictedBox.y = static_cast<int>(prediction.at<float>(1));
    predictedBox.width = static_cast<int>(prediction.at<float>(2));
    predictedBox.height = static_cast<int>(prediction.at<float>(3));
    
    return predictedBox;
} 
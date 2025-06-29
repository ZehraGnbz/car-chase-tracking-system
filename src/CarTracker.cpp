#include "CarTracker.h"
#include <iostream>
#include <chrono>
#include <iomanip>

CarTracker::CarTracker() 
    : displayEnabled_(true), totalVehiclesDetected_(0) {
}

CarTracker::~CarTracker() {
}

bool CarTracker::initialize() {
    std::cout << "Initializing Car Chase Tracking System..." << std::endl;
    
    if (!detector_.initialize()) {
        std::cerr << "Failed to initialize vehicle detector!" << std::endl;
        return false;
    }
    
    tracker_.initialize();
    
    std::cout << "Car Chase Tracking System initialized successfully!" << std::endl;
    return true;
}

bool CarTracker::processVideo(const std::string& videoPath, const std::string& outputPath) {
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video file: " << videoPath << std::endl;
        return false;
    }
    
    // Get video properties
    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(cv::CAP_PROP_FPS);
    int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    
    std::cout << "Video properties:" << std::endl;
    std::cout << "  Resolution: " << frameWidth << "x" << frameHeight << std::endl;
    std::cout << "  FPS: " << fps << std::endl;
    std::cout << "  Total frames: " << totalFrames << std::endl;
    
    // Setup video writer if output path is specified
    cv::VideoWriter videoWriter;
    if (!outputPath.empty()) {
        int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
        videoWriter.open(outputPath, fourcc, fps, cv::Size(frameWidth, frameHeight));
        if (!videoWriter.isOpened()) {
            std::cerr << "Error: Could not open output video file: " << outputPath << std::endl;
            return false;
        }
    }
    
    cv::Mat frame, outputFrame;
    int frameCount = 0;
    
    std::cout << "Starting video processing..." << std::endl;
    
    while (true) {
        cap >> frame;
        if (frame.empty()) break;
        
        frameCount++;
        
        // Process frame
        auto startTime = std::chrono::high_resolution_clock::now();
        bool success = processFrame(frame, outputFrame);
        auto endTime = std::chrono::high_resolution_clock::now();
        
        if (!success) {
            std::cerr << "Error processing frame " << frameCount << std::endl;
            continue;
        }
        
        // Calculate processing time
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        updateStatistics(duration.count());
        
        // Display progress
        if (frameCount % 30 == 0) {
            double progress = (double)frameCount / totalFrames * 100.0;
            std::cout << "Progress: " << std::fixed << std::setprecision(1) 
                     << progress << "% (" << frameCount << "/" << totalFrames 
                     << " frames)" << std::endl;
        }
        
        // Write to output video
        if (videoWriter.isOpened()) {
            videoWriter.write(outputFrame);
        }
        
        // Display frame
        if (displayEnabled_) {
            cv::imshow("Car Chase Tracker", outputFrame);
            
            char key = cv::waitKey(1);
            if (key == 27) { // ESC key
                std::cout << "Processing stopped by user." << std::endl;
                break;
            } else if (key == 'p') { // Pause
                cv::waitKey(0);
            }
        }
    }
    
    cap.release();
    if (videoWriter.isOpened()) {
        videoWriter.release();
    }
    
    if (displayEnabled_) {
        cv::destroyAllWindows();
    }
    
    // Print final statistics
    std::cout << "\n=== Processing Complete ===" << std::endl;
    std::cout << "Total frames processed: " << frameCount << std::endl;
    std::cout << "Total vehicles detected: " << totalVehiclesDetected_ << std::endl;
    std::cout << "Average processing time: " << getAverageProcessingTime() << " ms" << std::endl;
    
    if (!outputPath.empty()) {
        std::cout << "Output saved to: " << outputPath << std::endl;
    }
    
    return true;
}

bool CarTracker::processFrame(const cv::Mat& frame, cv::Mat& outputFrame) {
    try {
        // Copy input frame to output
        frame.copyTo(outputFrame);
        
        // Detect vehicles
        std::vector<Detection> detections = detector_.detectVehicles(frame);
        
        // Update tracking system
        std::vector<TrackedVehicle> tracks = tracker_.update(detections, frame);
        
        // Update statistics
        totalVehiclesDetected_ += detections.size();
        
        // Draw tracks on output frame
        tracker_.drawTracks(outputFrame, tracks);
        
        // Draw statistics
        drawStatistics(outputFrame);
        
        return true;
    }
    catch (const cv::Exception& e) {
        std::cerr << "OpenCV error in processFrame: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error in processFrame: " << e.what() << std::endl;
        return false;
    }
}

void CarTracker::setDetectionThreshold(float threshold) {
    detector_.setConfidenceThreshold(threshold);
}

void CarTracker::setTrackingParameters(int maxAge, int minHits, float iouThreshold) {
    tracker_.setMaxAge(maxAge);
    tracker_.setMinHits(minHits);
    tracker_.setIoUThreshold(iouThreshold);
}

void CarTracker::enableDisplay(bool enable) {
    displayEnabled_ = enable;
}

void CarTracker::setOutputPath(const std::string& path) {
    outputPath_ = path;
}

int CarTracker::getTotalVehiclesDetected() const {
    return totalVehiclesDetected_;
}

int CarTracker::getCurrentActiveTracks() const {
    // This would need to be implemented in TrackingSystem
    return 0; // Placeholder
}

double CarTracker::getAverageProcessingTime() const {
    if (processingTimes_.empty()) return 0.0;
    
    double sum = 0.0;
    for (double time : processingTimes_) {
        sum += time;
    }
    return sum / processingTimes_.size();
}

void CarTracker::drawStatistics(cv::Mat& frame) {
    // Draw statistics overlay
    std::string stats = "Vehicles Detected: " + std::to_string(totalVehiclesDetected_);
    cv::putText(frame, stats, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 
                0.7, cv::Scalar(255, 255, 255), 2);
    
    std::string avgTime = "Avg Time: " + std::to_string(static_cast<int>(getAverageProcessingTime())) + "ms";
    cv::putText(frame, avgTime, cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 
                0.7, cv::Scalar(255, 255, 255), 2);
    
    // Draw instructions
    std::string instructions = "ESC: Exit | P: Pause | SPACE: Step";
    cv::putText(frame, instructions, cv::Point(10, frame.rows - 20), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
}

void CarTracker::saveFrame(const cv::Mat& frame, int frameNumber) {
    if (!outputPath_.empty()) {
        std::string filename = outputPath_ + "/frame_" + std::to_string(frameNumber) + ".jpg";
        cv::imwrite(filename, frame);
    }
}

void CarTracker::updateStatistics(double processingTime) {
    processingTimes_.push_back(processingTime);
    
    // Keep only last 100 measurements to avoid memory issues
    if (processingTimes_.size() > 100) {
        processingTimes_.erase(processingTimes_.begin());
    }
} 
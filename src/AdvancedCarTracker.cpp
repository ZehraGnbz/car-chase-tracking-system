#include "AdvancedCarTracker.h"
#include <iostream>
#include <chrono>
#include <iomanip>

AdvancedCarTracker::AdvancedCarTracker() 
    : isRunning_(false), showDebugInfo_(true), enableRecording_(false),
      targetSelectionMode_(false), targetSelected_(false), selectedTargetId_(-1),
      frameCount_(0), totalProcessingTime_(0.0), averageFPS_(0.0),
      frameSkip(1), frameCounter(0), realtimeMode(false), resolutionScale(1.0f) {
}

AdvancedCarTracker::~AdvancedCarTracker() {
    stop();
}

bool AdvancedCarTracker::initialize(const std::string& videoPath, const std::string& modelPath) {
    std::cout << "Initializing Advanced Car Chase Tracking System..." << std::endl;
    
    // Initialize vehicle detector
    vehicleDetector_ = std::make_unique<VehicleDetector>();
    if (!vehicleDetector_->initialize()) {
        std::cerr << "Failed to initialize vehicle detector!" << std::endl;
        return false;
    }
    
    // Initialize tracking system
    trackingSystem_ = std::make_unique<AdvancedTrackingSystem>();
    trackingSystem_->initialize();
    
    // Open video capture
    if (!videoPath.empty()) {
        videoCapture_.open(videoPath);
        if (!videoCapture_.isOpened()) {
            std::cerr << "Error: Could not open video file: " << videoPath << std::endl;
            return false;
        }
    }
    
    // Set up mouse callback for interactive target selection
    cv::namedWindow("Advanced Car Chase Tracker", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("Advanced Car Chase Tracker", onMouse, this);
    
    std::cout << "Advanced Car Chase Tracking System initialized successfully!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Mouse Click: Select target vehicle" << std::endl;
    std::cout << "  'C': Clear primary target" << std::endl;
    std::cout << "  'T': Toggle target selection mode" << std::endl;
    std::cout << "  'I': Show track information" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    
    return true;
}

bool AdvancedCarTracker::initializeCamera(int cameraIndex) {
    std::cout << "Initializing camera capture..." << std::endl;
    
    // Initialize vehicle detector
    vehicleDetector_ = std::make_unique<VehicleDetector>();
    if (!vehicleDetector_->initialize()) {
        std::cerr << "Failed to initialize vehicle detector!" << std::endl;
        return false;
    }
    
    // Initialize tracking system
    trackingSystem_ = std::make_unique<AdvancedTrackingSystem>();
    trackingSystem_->initialize();
    
    // Open camera capture
    videoCapture_.open(cameraIndex);
    if (!videoCapture_.isOpened()) {
        std::cerr << "Error: Could not open camera " << cameraIndex << std::endl;
        return false;
    }
    
    // Set up mouse callback for interactive target selection
    cv::namedWindow("Advanced Car Chase Tracker", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("Advanced Car Chase Tracker", onMouse, this);
    
    std::cout << "Camera initialized successfully!" << std::endl;
    return true;
}

void AdvancedCarTracker::run() {
    if (!videoCapture_.isOpened()) {
        std::cerr << "Error: No video source available!" << std::endl;
        return;
    }
    
    isRunning_ = true;
    cv::Mat frame;
    
    std::cout << "Starting advanced tracking..." << std::endl;
    
    while (isRunning_) {
        videoCapture_ >> frame;
        if (frame.empty()) {
            std::cout << "End of video stream." << std::endl;
            break;
        }
        
        auto startTime = std::chrono::high_resolution_clock::now();
        processFrame(frame);
        auto endTime = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        updatePerformanceMetrics(duration.count());
        
        // Display frame
        cv::imshow("Advanced Car Chase Tracker", frame);
        
        char key = cv::waitKey(1);
        if (key == 27) { // ESC key
            std::cout << "Stopping tracking..." << std::endl;
            break;
        } else {
            handleKeyPress(key);
        }
    }
    
    stop();
}

void AdvancedCarTracker::stop() {
    isRunning_ = false;
    if (videoCapture_.isOpened()) {
        videoCapture_.release();
    }
    if (videoWriter_.isOpened()) {
        videoWriter_.release();
    }
    cv::destroyAllWindows();
}

void AdvancedCarTracker::processFrame(const cv::Mat& frame) {
    try {
        // Detect vehicles
        std::vector<Detection> detections = vehicleDetector_->detectVehicles(frame);
        
        // Update tracking
        std::vector<AdvancedTrackedVehicle> tracks = trackingSystem_->updateAdvanced(detections, frame);
        
        // Draw results
        cv::Mat outputFrame = frame.clone();
        trackingSystem_->drawAdvancedTracks(outputFrame, tracks);
        trackingSystem_->drawTargetSelection(outputFrame);
        
        // Draw UI
        drawUI(outputFrame);
        
        // Save frame if recording
        if (enableRecording_) {
            saveFrame(outputFrame);
        }
        
        // Update display
        outputFrame.copyTo(frame);
        
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV error in processFrame: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in processFrame: " << e.what() << std::endl;
    }
}

void AdvancedCarTracker::drawUI(cv::Mat& frame) {
    drawPerformanceInfo(frame);
    drawTargetInfo(frame);
}

void AdvancedCarTracker::drawPerformanceInfo(cv::Mat& frame) {
    if (!showDebugInfo_) return;
    
    std::string fpsText = "FPS: " + std::to_string(static_cast<int>(averageFPS_));
    std::string frameText = "Frame: " + std::to_string(frameCount_);
    std::string timeText = "Time: " + std::to_string(static_cast<int>(totalProcessingTime_)) + "ms";
    
    cv::putText(frame, fpsText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, frameText, cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, timeText, cv::Point(10, 90), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
}

void AdvancedCarTracker::drawTargetInfo(cv::Mat& frame) {
    if (targetSelected_) {
        std::string targetText = "Target: " + std::to_string(selectedTargetId_);
        cv::putText(frame, targetText, cv::Point(10, frame.rows - 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2);
    }
}

void AdvancedCarTracker::handleKeyPress(int key) {
    switch (key) {
        case 'c':
        case 'C':
            clearTargetSelection();
            break;
        case 't':
        case 'T':
            setTargetSelectionMode(!targetSelectionMode_);
            break;
        case 'i':
        case 'I':
            showDebugInfo_ = !showDebugInfo_;
            break;
        case 'r':
        case 'R':
            setRecordingMode(!enableRecording_);
            break;
    }
}

void AdvancedCarTracker::saveFrame(const cv::Mat& frame) {
    if (!videoWriter_.isOpened()) {
        int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
        videoWriter_.open(outputVideoPath_, fourcc, 30.0, frame.size());
    }
    
    if (videoWriter_.isOpened()) {
        videoWriter_.write(frame);
    }
}

void AdvancedCarTracker::updatePerformanceMetrics(double processingTime) {
    frameCount_++;
    totalProcessingTime_ += processingTime;
    averageFPS_ = 1000.0 / (totalProcessingTime_ / frameCount_);
}

// Configuration methods
void AdvancedCarTracker::setDebugMode(bool enable) {
    showDebugInfo_ = enable;
}

void AdvancedCarTracker::setRecordingMode(bool enable, const std::string& outputPath) {
    enableRecording_ = enable;
    if (!outputPath.empty()) {
        outputVideoPath_ = outputPath;
    }
}

void AdvancedCarTracker::setTargetSelectionMode(bool enable) {
    targetSelectionMode_ = enable;
}

// Interactive features
void AdvancedCarTracker::handleMouseClick(int x, int y) {
    if (targetSelectionMode_) {
        selectTarget(cv::Point(x, y));
    }
}

void AdvancedCarTracker::selectTarget(const cv::Point& point) {
    // This would need to be implemented based on current tracks
    // For now, just set a dummy target
    selectedTargetId_ = 1; // Placeholder
    targetSelected_ = true;
    trackingSystem_->setPrimaryTarget(selectedTargetId_);
    std::cout << "Target selected: " << selectedTargetId_ << std::endl;
}

void AdvancedCarTracker::clearTargetSelection() {
    targetSelected_ = false;
    selectedTargetId_ = -1;
    trackingSystem_->clearPrimaryTarget();
    std::cout << "Target selection cleared." << std::endl;
}

// Performance monitoring
double AdvancedCarTracker::getAverageFPS() const {
    return averageFPS_;
}

int AdvancedCarTracker::getFrameCount() const {
    return frameCount_;
}

double AdvancedCarTracker::getTotalProcessingTime() const {
    return totalProcessingTime_;
}

// Advanced features
void AdvancedCarTracker::enablePartialOcclusionHandling(bool enable) {
    trackingSystem_->enablePartialTracking(enable);
}

void AdvancedCarTracker::enableReIdentification(bool enable) {
    trackingSystem_->enableReIdentification(enable);
}

void AdvancedCarTracker::enableCameraMotionCompensation(bool enable) {
    trackingSystem_->enableCameraMotionCompensation(enable);
}

void AdvancedCarTracker::setOcclusionThreshold(float threshold) {
    trackingSystem_->setOcclusionThreshold(threshold);
}

void AdvancedCarTracker::setReIdThreshold(float threshold) {
    trackingSystem_->setReIdThreshold(threshold);
}

void AdvancedCarTracker::setCameraMotionSensitivity(float sensitivity) {
    trackingSystem_->setCameraMotionSensitivity(sensitivity);
}

// Mouse callback wrapper
void AdvancedCarTracker::onMouse(int event, int x, int y, int flags, void* userdata) {
    AdvancedCarTracker* tracker = static_cast<AdvancedCarTracker*>(userdata);
    if (tracker && event == cv::EVENT_LBUTTONDOWN) {
        tracker->handleMouseClick(x, y);
    }
}

void AdvancedCarTracker::setFrameSkip(int skip) {
    frameSkip = std::max(1, skip);
    std::cout << "Frame skip set to: " << frameSkip << std::endl;
}

void AdvancedCarTracker::setRealtimeMode(bool mode) {
    realtimeMode = mode;
    std::cout << "Real-time mode: " << (realtimeMode ? "Enabled" : "Disabled") << std::endl;
}

void AdvancedCarTracker::setResolutionScale(float scale) {
    resolutionScale = std::max(0.1f, std::min(1.0f, scale));
    std::cout << "Resolution scale set to: " << resolutionScale << std::endl;
}

bool AdvancedCarTracker::processVideo() {
    if (!videoCapture_.isOpened()) {
        std::cerr << "Error: No video source available!" << std::endl;
        return false;
    }
    
    cv::Mat frame;
    int processedFrames = 0;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "Starting video processing with optimizations:" << std::endl;
    std::cout << "  Frame skip: " << frameSkip << std::endl;
    std::cout << "  Real-time mode: " << (realtimeMode ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Resolution scale: " << resolutionScale << std::endl;
    
    while (true) {
        videoCapture_ >> frame;
        if (frame.empty()) break;
        
        frameCounter++;
        
        // Only process every Nth frame based on frameSkip
        if (frameCounter % frameSkip != 0) {
            // For skipped frames, just write the original frame
            if (videoWriter_.isOpened()) {
                videoWriter_.write(frame);
            }
            continue;
        }
        
        processedFrames++;
        
        // Process frame
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        // Scale frame for faster processing
        cv::Mat processedFrame = frame;
        if (resolutionScale != 1.0f) {
            cv::Size newSize(frame.cols * resolutionScale, frame.rows * resolutionScale);
            cv::resize(frame, processedFrame, newSize);
        }
        
        // Detect vehicles
        std::vector<Detection> detections = vehicleDetector_->detectVehicles(processedFrame);
        
        // Scale detections back to original size if needed
        if (resolutionScale != 1.0f) {
            for (auto& detection : detections) {
                detection.boundingBox.x /= resolutionScale;
                detection.boundingBox.y /= resolutionScale;
                detection.boundingBox.width /= resolutionScale;
                detection.boundingBox.height /= resolutionScale;
            }
        }
        
        // Update tracking
        std::vector<AdvancedTrackedVehicle> tracks = trackingSystem_->updateAdvanced(detections, frame);
        
        // Draw results
        cv::Mat resultFrame = frame.clone();
        trackingSystem_->drawAdvancedTracks(resultFrame, tracks);
        trackingSystem_->drawTargetSelection(resultFrame);
        
        // Add real-time info overlay
        if (realtimeMode) {
            std::string info = "Real-time Mode | Frame: " + std::to_string(frameCounter) + 
                              " | FPS: " + std::to_string(static_cast<int>(averageFPS_));
            cv::putText(resultFrame, info, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        }
        
        // Write processed frame
        if (videoWriter_.isOpened()) {
            videoWriter_.write(resultFrame);
        }
        
        auto frameEnd = std::chrono::high_resolution_clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
        totalProcessingTime_ += frameDuration.count();
        
        // Calculate current FPS
        if (processedFrames > 0) {
            averageFPS_ = (processedFrames * 1000.0) / totalProcessingTime_;
        }
        
        // Progress update every 50 processed frames for real-time feel
        if (processedFrames % 50 == 0) {
            double progress = (double)frameCounter / frameCount_ * 100.0;
            std::cout << "Progress: " << std::fixed << std::setprecision(1) << progress << "% ";
            std::cout << "(Frame " << frameCounter << "/" << frameCount_ << ", Processed: " << processedFrames << ") ";
            std::cout << "FPS: " << std::fixed << std::setprecision(1) << averageFPS_ << std::endl;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Calculate final statistics
    frameCount_ = frameCounter;
    averageFPS_ = processedFrames > 0 ? (processedFrames * 1000.0) / totalProcessingTime_ : 0.0;
    
    std::cout << std::endl;
    std::cout << "Processing completed!" << std::endl;
    std::cout << "Total frames: " << frameCount_ << std::endl;
    std::cout << "Processed frames: " << processedFrames << std::endl;
    std::cout << "Frame skip: " << frameSkip << std::endl;
    std::cout << "Resolution scale: " << resolutionScale << std::endl;
    std::cout << "Average processing time per frame: " << (totalProcessingTime_ / processedFrames) << " ms" << std::endl;
    std::cout << "Average FPS: " << std::fixed << std::setprecision(2) << averageFPS_ << std::endl;
    std::cout << "Total processing time: " << totalDuration.count() << " ms" << std::endl;
    
    return true;
} 
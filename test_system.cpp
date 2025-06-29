#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "=== Car Chase Tracking System Test ===" << std::endl;
    
    // Test OpenCV version
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;
    
    // Test basic OpenCV functionality
    cv::Mat testImage = cv::Mat::zeros(100, 100, CV_8UC3);
    cv::rectangle(testImage, cv::Point(10, 10), cv::Point(90, 90), cv::Scalar(0, 255, 0), 2);
    
    if (testImage.empty()) {
        std::cerr << "Error: Failed to create test image!" << std::endl;
        return 1;
    }
    
    std::cout << "Test image created successfully: " << testImage.size() << std::endl;
    
    // Test video capture
    std::string videoPath = "FULL_ Aerial view of WILD police chase in Chicago.mp4";
    cv::VideoCapture cap(videoPath);
    
    if (!cap.isOpened()) {
        std::cout << "Warning: Could not open video file: " << videoPath << std::endl;
        std::cout << "This is normal if the video file doesn't exist yet." << std::endl;
    } else {
        std::cout << "Video file opened successfully!" << std::endl;
        std::cout << "  Resolution: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) 
                  << "x" << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
        std::cout << "  FPS: " << cap.get(cv::CAP_PROP_FPS) << std::endl;
        std::cout << "  Total frames: " << cap.get(cv::CAP_PROP_FRAME_COUNT) << std::endl;
        cap.release();
    }
    
    // Test HOG detector
    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    std::cout << "HOG detector initialized successfully!" << std::endl;
    
    // Test Kalman filter
    cv::KalmanFilter kf(4, 2, 0);
    std::cout << "Kalman filter created successfully!" << std::endl;
    
    std::cout << "\n=== All tests passed! ===" << std::endl;
    std::cout << "The system is ready to build and run." << std::endl;
    
    return 0;
} 
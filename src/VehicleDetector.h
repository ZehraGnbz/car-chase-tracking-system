#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>

struct Detection {
    cv::Rect boundingBox;
    float confidence;
    int classId;
    std::string label;
};

class VehicleDetector {
public:
    VehicleDetector();
    ~VehicleDetector();
    
    bool initialize();
    std::vector<Detection> detectVehicles(const cv::Mat& frame);
    void setConfidenceThreshold(float threshold);
    void setNMSThreshold(float threshold);
    
private:
    cv::dnn::Net net_;
    std::vector<std::string> classNames_;
    float confidenceThreshold_;
    float nmsThreshold_;
    cv::Size inputSize_;
    
    std::vector<cv::String> getOutputsNames();
    void preprocessFrame(const cv::Mat& frame, cv::Mat& blob);
    std::vector<Detection> postprocessDetections(const cv::Mat& frame, 
                                                const std::vector<cv::Mat>& outputs);
    void drawDetections(cv::Mat& frame, const std::vector<Detection>& detections);
    std::vector<Detection> detectVehiclesHOG(const cv::Mat& frame);
}; 
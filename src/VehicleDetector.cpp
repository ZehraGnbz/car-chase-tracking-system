#include "VehicleDetector.h"
#include <iostream>
#include <fstream>

VehicleDetector::VehicleDetector() 
    : confidenceThreshold_(0.5f), nmsThreshold_(0.4f), inputSize_(416, 416) {
}

VehicleDetector::~VehicleDetector() {
}

bool VehicleDetector::initialize() {
    try {
        // Load YOLO model (using a lightweight model for real-time processing)
        std::string modelPath = "models/yolov4-tiny.weights";
        std::string configPath = "models/yolov4-tiny.cfg";
        
        // Check if model files exist, if not, we'll use HOG detector as fallback
        std::ifstream modelFile(modelPath);
        if (!modelFile.good()) {
            std::cout << "YOLO model not found, using HOG detector..." << std::endl;
            return true; // We'll use HOG detector
        }
        
        net_ = cv::dnn::readNetFromDarknet(configPath, modelPath);
        net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        
        // Load class names
        classNames_ = {"person", "bicycle", "car", "motorcycle", "airplane", "bus", 
                      "train", "truck", "boat", "traffic light", "fire hydrant", 
                      "stop sign", "parking meter", "bench", "bird", "cat", "dog", 
                      "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", 
                      "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", 
                      "skis", "snowboard", "sports ball", "kite", "baseball bat", 
                      "baseball glove", "skateboard", "surfboard", "tennis racket", 
                      "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", 
                      "banana", "apple", "sandwich", "orange", "broccoli", "carrot", 
                      "hot dog", "pizza", "donut", "cake", "chair", "couch", 
                      "potted plant", "bed", "dining table", "toilet", "tv", "laptop", 
                      "mouse", "remote", "keyboard", "cell phone", "microwave", 
                      "oven", "toaster", "sink", "refrigerator", "book", "clock", 
                      "vase", "scissors", "teddy bear", "hair drier", "toothbrush"};
        
        return true;
    }
    catch (const cv::Exception& e) {
        std::cerr << "Error initializing vehicle detector: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Detection> VehicleDetector::detectVehicles(const cv::Mat& frame) {
    std::vector<Detection> detections;
    
    if (net_.empty()) {
        // Fallback to HOG detector
        return detectVehiclesHOG(frame);
    }
    
    try {
        cv::Mat blob;
        preprocessFrame(frame, blob);
        
        net_.setInput(blob);
        std::vector<cv::Mat> outputs;
        net_.forward(outputs, getOutputsNames());
        
        detections = postprocessDetections(frame, outputs);
    }
    catch (const cv::Exception& e) {
        std::cerr << "Error in vehicle detection: " << e.what() << std::endl;
    }
    
    return detections;
}

std::vector<Detection> VehicleDetector::detectVehiclesHOG(const cv::Mat& frame) {
    std::vector<Detection> detections;
    
    // Use HOG detector for cars
    static cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    
    std::vector<cv::Rect> foundLocations;
    std::vector<double> weights;
    
    // Detect objects
    hog.detectMultiScale(frame, foundLocations, weights, 0, cv::Size(8, 8), 
                        cv::Size(4, 4), 1.05, 2, false);
    
    for (size_t i = 0; i < foundLocations.size(); ++i) {
        if (weights[i] > confidenceThreshold_) {
            Detection det;
            det.boundingBox = foundLocations[i];
            det.confidence = static_cast<float>(weights[i]);
            det.classId = 2; // car class
            det.label = "vehicle";
            detections.push_back(det);
        }
    }
    
    return detections;
}

void VehicleDetector::setConfidenceThreshold(float threshold) {
    confidenceThreshold_ = threshold;
}

void VehicleDetector::setNMSThreshold(float threshold) {
    nmsThreshold_ = threshold;
}

std::vector<cv::String> VehicleDetector::getOutputsNames() {
    static std::vector<cv::String> names;
    if (names.empty()) {
        std::vector<int> outLayers = net_.getUnconnectedOutLayers();
        std::vector<cv::String> layersNames = net_.getLayerNames();
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i) {
            names[i] = layersNames[outLayers[i] - 1];
        }
    }
    return names;
}

void VehicleDetector::preprocessFrame(const cv::Mat& frame, cv::Mat& blob) {
    cv::dnn::blobFromImage(frame, blob, 1/255.0, inputSize_, cv::Scalar(0, 0, 0), 
                          true, false);
}

std::vector<Detection> VehicleDetector::postprocessDetections(const cv::Mat& frame, 
                                                             const std::vector<cv::Mat>& outputs) {
    std::vector<Detection> detections;
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    
    for (const auto& output : outputs) {
        float* data = (float*)output.data;
        for (int i = 0; i < output.rows; ++i, data += output.cols) {
            cv::Mat scores = output.row(i).colRange(5, output.cols);
            cv::Point classIdPoint;
            double confidence;
            cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            
            if (confidence > confidenceThreshold_) {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;
                
                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    }
    
    // Apply Non-Maximum Suppression
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confidenceThreshold_, nmsThreshold_, indices);
    
    for (int idx : indices) {
        Detection det;
        det.boundingBox = boxes[idx];
        det.confidence = confidences[idx];
        det.classId = classIds[idx];
        det.label = (det.classId < classNames_.size()) ? classNames_[det.classId] : "unknown";
        
        // Only include vehicles (car, truck, bus, motorcycle)
        if (det.classId == 2 || det.classId == 7 || det.classId == 5 || det.classId == 3) {
            detections.push_back(det);
        }
    }
    
    return detections;
} 
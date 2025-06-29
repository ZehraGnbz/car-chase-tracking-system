#include "CarTracker.h"
#include <iostream>
#include <string>

void printUsage(const std::string& programName) {
    std::cout << "Car Chase Tracking System" << std::endl;
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -i, --input <video_path>     Input video file (default: FULL_ Aerial view of WILD police chase in Chicago.mp4)" << std::endl;
    std::cout << "  -o, --output <output_path>   Output video file" << std::endl;
    std::cout << "  -t, --threshold <value>      Detection confidence threshold (0.0-1.0, default: 0.5)" << std::endl;
    std::cout << "  -m, --max-age <frames>       Maximum age for tracks (default: 30)" << std::endl;
    std::cout << "  -h, --min-hits <count>       Minimum hits for track confirmation (default: 3)" << std::endl;
    std::cout << "  -u, --iou-threshold <value>  IoU threshold for tracking (0.0-1.0, default: 0.3)" << std::endl;
    std::cout << "  -n, --no-display            Disable real-time display" << std::endl;
    std::cout << "  --help                      Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " -i input.mp4 -o output.mp4 -t 0.6" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string inputVideo = "FULL_ Aerial view of WILD police chase in Chicago.mp4";
    std::string outputVideo = "";
    float detectionThreshold = 0.5f;
    int maxAge = 30;
    int minHits = 3;
    float iouThreshold = 0.3f;
    bool displayEnabled = true;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-i" || arg == "--input") {
            if (i + 1 < argc) {
                inputVideo = argv[++i];
            } else {
                std::cerr << "Error: Missing argument for " << arg << std::endl;
                return 1;
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputVideo = argv[++i];
            } else {
                std::cerr << "Error: Missing argument for " << arg << std::endl;
                return 1;
            }
        } else if (arg == "-t" || arg == "--threshold") {
            if (i + 1 < argc) {
                detectionThreshold = std::stof(argv[++i]);
                if (detectionThreshold < 0.0f || detectionThreshold > 1.0f) {
                    std::cerr << "Error: Detection threshold must be between 0.0 and 1.0" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: Missing argument for " << arg << std::endl;
                return 1;
            }
        } else if (arg == "-m" || arg == "--max-age") {
            if (i + 1 < argc) {
                maxAge = std::stoi(argv[++i]);
                if (maxAge < 1) {
                    std::cerr << "Error: Max age must be positive" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: Missing argument for " << arg << std::endl;
                return 1;
            }
        } else if (arg == "-h" || arg == "--min-hits") {
            if (i + 1 < argc) {
                minHits = std::stoi(argv[++i]);
                if (minHits < 1) {
                    std::cerr << "Error: Min hits must be positive" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: Missing argument for " << arg << std::endl;
                return 1;
            }
        } else if (arg == "-u" || arg == "--iou-threshold") {
            if (i + 1 < argc) {
                iouThreshold = std::stof(argv[++i]);
                if (iouThreshold < 0.0f || iouThreshold > 1.0f) {
                    std::cerr << "Error: IoU threshold must be between 0.0 and 1.0" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: Missing argument for " << arg << std::endl;
                return 1;
            }
        } else if (arg == "-n" || arg == "--no-display") {
            displayEnabled = false;
        } else {
            std::cerr << "Error: Unknown argument " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    std::cout << "=== Car Chase Tracking System ===" << std::endl;
    std::cout << "Input video: " << inputVideo << std::endl;
    if (!outputVideo.empty()) {
        std::cout << "Output video: " << outputVideo << std::endl;
    }
    std::cout << "Detection threshold: " << detectionThreshold << std::endl;
    std::cout << "Max age: " << maxAge << " frames" << std::endl;
    std::cout << "Min hits: " << minHits << std::endl;
    std::cout << "IoU threshold: " << iouThreshold << std::endl;
    std::cout << "Display enabled: " << (displayEnabled ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
    
    // Create and initialize car tracker
    CarTracker tracker;
    
    if (!tracker.initialize()) {
        std::cerr << "Failed to initialize car tracker!" << std::endl;
        return 1;
    }
    
    // Configure tracker parameters
    tracker.setDetectionThreshold(detectionThreshold);
    tracker.setTrackingParameters(maxAge, minHits, iouThreshold);
    tracker.enableDisplay(displayEnabled);
    
    // Process video
    if (!tracker.processVideo(inputVideo, outputVideo)) {
        std::cerr << "Failed to process video!" << std::endl;
        return 1;
    }
    
    std::cout << "Processing completed successfully!" << std::endl;
    return 0;
} 
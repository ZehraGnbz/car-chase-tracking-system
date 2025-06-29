#include "AdvancedCarTracker.h"
#include <iostream>
#include <string>

void printAdvancedUsage(const std::string& programName) {
    std::cout << "Advanced Car Chase Tracking System" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Basic Options:" << std::endl;
    std::cout << "  -i, --input <video_path>     Input video file (default: FULL_ Aerial view of WILD police chase in Chicago.mp4)" << std::endl;
    std::cout << "  -o, --output <output_path>   Output video file" << std::endl;
    std::cout << "  -t, --threshold <value>      Detection confidence threshold (0.0-1.0, default: 0.5)" << std::endl;
    std::cout << std::endl;
    std::cout << "Advanced Tracking Options:" << std::endl;
    std::cout << "  --occlusion-threshold <value>    Occlusion detection threshold (0.0-1.0, default: 0.3)" << std::endl;
    std::cout << "  --reid-threshold <value>         Re-identification threshold (0.0-1.0, default: 0.7)" << std::endl;
    std::cout << "  --camera-sensitivity <value>     Camera motion sensitivity (0.0-1.0, default: 0.1)" << std::endl;
    std::cout << "  --disable-partial-tracking       Disable partial occlusion tracking" << std::endl;
    std::cout << "  --disable-reidentification       Disable re-identification" << std::endl;
    std::cout << "  --disable-camera-compensation    Disable camera motion compensation" << std::endl;
    std::cout << "  --frame-skip <value>             Process every Nth frame (default: 1)" << std::endl;
    std::cout << "  --realtime-mode                  Enable real-time processing mode" << std::endl;
    std::cout << "  --resolution-scale <value>         Scale resolution (0.1-1.0, default: 1.0)" << std::endl;
    std::cout << std::endl;
    std::cout << "Interactive Controls:" << std::endl;
    std::cout << "  Mouse Click: Select target vehicle" << std::endl;
    std::cout << "  C: Clear primary target" << std::endl;
    std::cout << "  T: Toggle target selection mode" << std::endl;
    std::cout << "  I: Show track information" << std::endl;
    std::cout << "  A: Toggle advanced features" << std::endl;
    std::cout << "  P: Pause/Resume" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " -i input.mp4 -o output.mp4 --occlusion-threshold 0.4" << std::endl;
    std::cout << "  " << programName << " --reid-threshold 0.8 --camera-sensitivity 0.2" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string inputVideo = "FULL_ Aerial view of WILD police chase in Chicago.mp4";
    std::string outputVideo = "output_tracked.mp4";
    float detectionThreshold = 0.5f;
    float occlusionThreshold = 0.3f;
    float reidThreshold = 0.7f;
    float cameraSensitivity = 0.1f;
    int frameSkip = 1;  // Process every frame by default
    bool realtimeMode = false;
    float resolutionScale = 1.0f;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-i" || arg == "--input") {
            if (i + 1 < argc) inputVideo = argv[++i];
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) outputVideo = argv[++i];
        } else if (arg == "-t" || arg == "--threshold") {
            if (i + 1 < argc) detectionThreshold = std::stof(argv[++i]);
        } else if (arg == "--occlusion-threshold") {
            if (i + 1 < argc) occlusionThreshold = std::stof(argv[++i]);
        } else if (arg == "--reid-threshold") {
            if (i + 1 < argc) reidThreshold = std::stof(argv[++i]);
        } else if (arg == "--camera-sensitivity") {
            if (i + 1 < argc) cameraSensitivity = std::stof(argv[++i]);
        } else if (arg == "--frame-skip") {
            if (i + 1 < argc) frameSkip = std::stoi(argv[++i]);
        } else if (arg == "--realtime-mode") {
            realtimeMode = true;
        } else if (arg == "--resolution-scale") {
            if (i + 1 < argc) resolutionScale = std::stof(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "Advanced Car Chase Tracking System\n";
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -i, --input <file>           Input video file\n";
            std::cout << "  -o, --output <file>          Output video file\n";
            std::cout << "  -t, --threshold <value>      Detection threshold (0.0-1.0)\n";
            std::cout << "  --occlusion-threshold <value> Occlusion threshold (0.0-1.0)\n";
            std::cout << "  --reid-threshold <value>     Re-identification threshold (0.0-1.0)\n";
            std::cout << "  --camera-sensitivity <value> Camera motion sensitivity (0.0-1.0)\n";
            std::cout << "  --frame-skip <value>         Process every Nth frame (default: 1)\n";
            std::cout << "  --realtime-mode              Enable real-time processing mode\n";
            std::cout << "  --resolution-scale <value>   Scale resolution (0.1-1.0, default: 1.0)\n";
            std::cout << "  --help                       Show this help\n";
            return 0;
        }
    }
    
    std::cout << "🚗🚁 Advanced Car Chase Tracking System\n";
    std::cout << "=====================================\n";
    std::cout << "Input: " << inputVideo << std::endl;
    std::cout << "Output: " << outputVideo << std::endl;
    std::cout << "Detection Threshold: " << detectionThreshold << std::endl;
    std::cout << "Occlusion Threshold: " << occlusionThreshold << std::endl;
    std::cout << "Re-ID Threshold: " << reidThreshold << std::endl;
    std::cout << "Camera Sensitivity: " << cameraSensitivity << std::endl;
    std::cout << "Frame Skip: " << frameSkip << std::endl;
    std::cout << "Real-time Mode: " << (realtimeMode ? "Enabled" : "Disabled") << std::endl;
    std::cout << "Resolution Scale: " << resolutionScale << std::endl;
    std::cout << std::endl;
    
    // Initialize advanced tracking system
    AdvancedCarTracker tracker;
    
    if (!tracker.initialize(inputVideo)) {
        std::cerr << "Failed to initialize advanced car tracker!" << std::endl;
        return -1;
    }
    
    // Set advanced parameters
    tracker.setOcclusionThreshold(occlusionThreshold);
    tracker.setReIdThreshold(reidThreshold);
    tracker.setCameraMotionSensitivity(cameraSensitivity);
    tracker.setFrameSkip(frameSkip);
    tracker.setRealtimeMode(realtimeMode);
    tracker.setResolutionScale(resolutionScale);
    
    std::cout << "Starting advanced tracking with real-time optimizations..." << std::endl;
    
    // Process video
    if (!tracker.processVideo()) {
        std::cerr << "Failed to process video!" << std::endl;
        return -1;
    }
    
    std::cout << "Advanced tracking completed successfully!" << std::endl;
    return 0;
} 
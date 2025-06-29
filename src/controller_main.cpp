#include "TrackingController.h"
#include <iostream>
#include <string>

void printControllerUsage(const std::string& programName) {
    std::cout << "Car Chase Tracking Controller" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -i, --input <video_path>     Input video file" << std::endl;
    std::cout << "  -o, --output <output_path>   Output video file" << std::endl;
    std::cout << "  --help                       Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Interactive Controls:" << std::endl;
    std::cout << "  SPACE: Play/Pause video" << std::endl;
    std::cout << "  S: Stop video" << std::endl;
    std::cout << "  T: Toggle between Basic/Advanced tracking" << std::endl;
    std::cout << "  O: Toggle partial occlusion handling" << std::endl;
    std::cout << "  R: Toggle re-identification" << std::endl;
    std::cout << "  C: Toggle camera motion compensation" << std::endl;
    std::cout << "  D: Toggle debug information" << std::endl;
    std::cout << "  V: Toggle video recording" << std::endl;
    std::cout << "  Mouse Click: Select target vehicle" << std::endl;
    std::cout << "  ESC: Exit application" << std::endl;
    std::cout << std::endl;
    std::cout << "GUI Features:" << std::endl;
    std::cout << "  ✓ Real-time parameter adjustment with sliders" << std::endl;
    std::cout << "  ✓ Live statistics and performance monitoring" << std::endl;
    std::cout << "  ✓ Visual control panel with status indicators" << std::endl;
    std::cout << "  ✓ Interactive target selection" << std::endl;
    std::cout << "  ✓ Frame-by-frame playback control" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " -i police_chase.mp4 -o tracked_output.mp4" << std::endl;
}

void onVideoLoaded(const std::string& videoPath) {
    std::cout << "✓ Video loaded successfully: " << videoPath << std::endl;
}

void onStatusUpdate(const std::string& status) {
    std::cout << "Status: " << status << std::endl;
}

int main(int argc, char* argv[]) {
    std::string inputVideo = "";
    std::string outputVideo = "";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printControllerUsage(argv[0]);
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
        } else {
            std::cerr << "Error: Unknown argument " << arg << std::endl;
            printControllerUsage(argv[0]);
            return 1;
        }
    }
    
    std::cout << "=== Car Chase Tracking Controller ===" << std::endl;
    std::cout << "Starting interactive tracking interface..." << std::endl;
    std::cout << std::endl;
    
    // Create and initialize controller
    TrackingController controller;
    
    if (!controller.initialize()) {
        std::cerr << "Failed to initialize tracking controller!" << std::endl;
        return 1;
    }
    
    // Set up callbacks
    controller.setVideoLoadedCallback(onVideoLoaded);
    controller.setStatusUpdateCallback(onStatusUpdate);
    
    // Load video if specified
    if (!inputVideo.empty()) {
        controller.loadVideo(inputVideo);
        
        // Set up recording if output path is specified
        if (!outputVideo.empty()) {
            controller.saveVideo(outputVideo);
        }
        
        // Start playback
        controller.play();
        
        // Run the controller
        controller.run();
    } else {
        std::cout << "No video file specified. Use -i option to load a video." << std::endl;
        std::cout << "Example: " << argv[0] << " -i police_chase.mp4" << std::endl;
        std::cout << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        cv::waitKey(0);
    }
    
    std::cout << "Tracking session completed." << std::endl;
    return 0;
} 
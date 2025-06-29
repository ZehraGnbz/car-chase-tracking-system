cmake_minimum_required(VERSION 3.10)
project(CarChaseTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})

# Print OpenCV version
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

# Source files for basic car tracker
set(CAR_TRACKER_SOURCES
    src/main.cpp
    src/CarTracker.cpp
    src/VehicleDetector.cpp
    src/TrackingSystem.cpp
)

# Source files for advanced car tracker
set(ADVANCED_CAR_TRACKER_SOURCES
    src/advanced_main.cpp
    src/AdvancedCarTracker.cpp
    src/AdvancedTrackingSystem.cpp
    src/VehicleDetector.cpp
    src/TrackingSystem.cpp
)

# Source files for tracking controller
set(TRACKING_CONTROLLER_SOURCES
    src/controller_main.cpp
    src/TrackingController.cpp
    src/AdvancedCarTracker.cpp
    src/AdvancedTrackingSystem.cpp
    src/VehicleDetector.cpp
    src/TrackingSystem.cpp
)

# Create executables
add_executable(car_tracker ${CAR_TRACKER_SOURCES})
add_executable(advanced_car_tracker ${ADVANCED_CAR_TRACKER_SOURCES})
add_executable(tracking_controller ${TRACKING_CONTROLLER_SOURCES})

# Link OpenCV libraries
target_link_libraries(car_tracker ${OpenCV_LIBS})
target_link_libraries(advanced_car_tracker ${OpenCV_LIBS})
target_link_libraries(tracking_controller ${OpenCV_LIBS})

# Set output directory
set_target_properties(car_tracker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_target_properties(advanced_car_tracker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_target_properties(tracking_controller PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Set compiler flags
if(MSVC)
    target_compile_options(car_tracker PRIVATE /W4)
    target_compile_options(advanced_car_tracker PRIVATE /W4)
else()
    target_compile_options(car_tracker PRIVATE -Wall -Wextra -O3)
    target_compile_options(advanced_car_tracker PRIVATE -Wall -Wextra -O3)
endif() 
# Audio Node
cmake_minimum_required(VERSION 3.10)

# Audio Node executable
add_executable(audio_node
    audio_main.cpp
)

# Include directories
target_include_directories(audio_node PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Find the required Paho libraries
find_package(PahoMqttCpp REQUIRED)

# Link the executable against the libraries
target_link_libraries(audio_node PRIVATE PahoMqttCpp::mqttpp)

# Audio Node

# Audio Node executable
add_executable(audio_node
    audio_main.cpp
)

# Include directories
target_include_directories(audio_node PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Find the required Paho MQTT libraries
find_package(PahoMqttCpp REQUIRED)

# Find PortAudio
find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# Include PortAudio directories
target_include_directories(audio_node PRIVATE ${PORTAUDIO_INCLUDE_DIRS})

# Link the executable against the libraries
target_link_libraries(audio_node PRIVATE 
    PahoMqttCpp::paho-mqttpp3
    ${PORTAUDIO_LIBRARIES}
)

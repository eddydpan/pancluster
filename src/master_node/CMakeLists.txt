# Master Node

# Master Node executable
add_executable(master_node
    master_main.cpp
)

# Include directories
target_include_directories(master_node PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Find the required Paho MQTT libraries
find_package(PahoMqttCpp REQUIRED)

# Link the executable against the libraries
target_link_libraries(master_node PRIVATE 
    PahoMqttCpp::paho-mqttpp3
)

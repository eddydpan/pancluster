cmake_minimum_required(VERSION 3.10)
project(pancluster)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build discovery library
add_library(discovery STATIC
    common/discovery.cpp
    common/discovery.h
)

target_include_directories(discovery PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add subdirectories
add_subdirectory(master_node)

# Only build follower nodes if requested
if(BUILD_FOLLOWER_NODES)
    add_subdirectory(follower_nodes)
endif()

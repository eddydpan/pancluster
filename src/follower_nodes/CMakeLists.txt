# Follower Nodes
cmake_minimum_required(VERSION 3.10)

# Follower Node executable
add_executable(follower_node
    # Add your source files here
    # main.cpp
)

# Include directories
target_include_directories(follower_node PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
# target_link_libraries(follower_node PRIVATE
#     # Add your libraries here
# )

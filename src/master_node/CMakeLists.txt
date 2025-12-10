# Master Node
cmake_minimum_required(VERSION 3.10)

# Master Node executable
add_executable(master_node
    # Add your source files here
    # main.cpp
)

# Include directories
target_include_directories(master_node PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
# target_link_libraries(master_node PRIVATE
#     # Add your libraries here
# )

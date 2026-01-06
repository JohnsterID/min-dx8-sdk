cmake_minimum_required(VERSION 3.10)
project(D3D8Test)

# Include the min-dx8-sdk
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/d3d8lib)

# Create a simple test executable
add_executable(test_d3d8 main.cpp)
target_link_libraries(test_d3d8 PRIVATE d3d8lib)

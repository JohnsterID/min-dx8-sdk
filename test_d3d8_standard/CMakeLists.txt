cmake_minimum_required(VERSION 3.10)
project(D3D8StandardTests C)

# This builds tests for the STANDARD Direct3D8 API (32-bit)
# Compare with test_d3d8thk (64-bit) to assess d3d8thk viability

# Use the min-dx8-sdk for headers and libraries
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Test 1: Basic D3D8 functionality
add_executable(test_basic test_basic.c)
target_link_libraries(test_basic 
    d3d8
    dinput8
    dxguid
    gdi32
    user32
)

# Test 2: Device creation and management
add_executable(test_device test_device.c)
target_link_libraries(test_device
    d3d8
    dinput8
    dxguid
    gdi32
    user32
)

# Test 3: Rendering operations
add_executable(test_rendering test_rendering.c)
target_link_libraries(test_rendering
    d3d8
    dinput8
    dxguid
    gdi32
    user32
)

# Set output directory
set_target_properties(
    test_basic test_device test_rendering
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

message(STATUS "D3D8 Standard Test Suite configured")
message(STATUS "  Platform: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Tests: test_basic, test_device, test_rendering")

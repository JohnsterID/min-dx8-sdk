cmake_minimum_required(VERSION 3.10)
project(d3d8thk_tests C)

# These tests are for Windows 64-bit MinGW only
if(NOT MINGW)
    message(FATAL_ERROR "d3d8thk tests require MinGW-w64 toolchain")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "d3d8thk tests require 64-bit target (x86_64)")
endif()

# Test executables
add_executable(test_basic test_basic.c)
add_executable(test_surface test_surface.c)
add_executable(test_context test_context.c)
add_executable(test_draw test_draw.c)

# Link all tests against d3d8thk
target_link_libraries(test_basic PRIVATE d3d8thk gdi32)
target_link_libraries(test_surface PRIVATE d3d8thk gdi32)
target_link_libraries(test_context PRIVATE d3d8thk gdi32)
target_link_libraries(test_draw PRIVATE d3d8thk gdi32)

# Console applications
set_target_properties(test_basic test_surface test_context test_draw
    PROPERTIES
    WIN32_EXECUTABLE FALSE
)

# Build all tests by default
add_custom_target(all_tests
    DEPENDS test_basic test_surface test_context test_draw
)

# Install tests to a test directory
install(TARGETS test_basic test_surface test_context test_draw
    RUNTIME DESTINATION bin/tests
)

# Print build information
message(STATUS "")
message(STATUS "=== d3d8thk Test Suite Configuration ===")
message(STATUS "Target Architecture: x86_64 (64-bit)")
message(STATUS "Toolchain: MinGW-w64")
message(STATUS "Tests to build:")
message(STATUS "  - test_basic.exe    : DirectDraw object management")
message(STATUS "  - test_surface.exe  : Surface creation and management")
message(STATUS "  - test_context.exe  : D3D rendering context")
message(STATUS "  - test_draw.exe     : Drawing primitives")
message(STATUS "")
message(STATUS "NOTE: These tests must be run on Windows.")
message(STATUS "      They will fail on Linux/Wine (d3d8thk needs real Windows kernel).")
message(STATUS "")

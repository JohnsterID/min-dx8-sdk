add_library(d3d8lib INTERFACE)

# Common libraries for all compilers
target_link_libraries(d3d8lib INTERFACE dinput8 dxguid)

# MSVC-specific configuration
if(MSVC)
    # Use bundled MSVC-compiled .lib files
    target_link_libraries(d3d8lib INTERFACE d3d8 d3dx8)
    target_link_directories(d3d8lib BEFORE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_options(d3d8lib INTERFACE /NODEFAULTLIB:libci.lib)
    
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "12.0.8804")
        target_link_libraries(d3d8lib INTERFACE legacy_stdio_definitions)
        target_link_options(d3d8lib INTERFACE /SAFESEH:NO)
    endif()
endif()

# MinGW-specific configuration
if(MINGW)
    # MinGW-w64 DirectX 8 support varies by architecture:
    # i686 (32-bit): libd3d8.a + libd3dx8d.a (debug only, no release version)
    # x86_64 (64-bit): libd3d8thk.a only (no libd3dx8 libraries at all)
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64-bit: Use d3d8thk (thunk library) - no d3dx8 available
        target_link_libraries(d3d8lib INTERFACE d3d8thk)
        message(STATUS "MinGW 64-bit: Using d3d8thk (no d3dx8 support available)")
    else()
        # 32-bit: Use standard d3d8 + d3dx8d (debug version)
        target_link_libraries(d3d8lib INTERFACE d3d8 d3dx8d)
        message(STATUS "MinGW 32-bit: Using d3d8 + d3dx8d")
    endif()
    
    # ddraw.h header is part of current windows SDK, so can't include it in modern builds
    target_include_directories(d3d8lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/extra)
endif()

target_compile_definitions(d3d8lib INTERFACE -DBUILD_WITH_D3D8)
target_include_directories(d3d8lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
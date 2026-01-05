add_library(d3d8lib INTERFACE)

if(MSVC)
    # Use bundled MSVC-compiled .lib files
    target_link_libraries(d3d8lib INTERFACE d3d8 d3dx8 dinput8 dxguid)
    target_link_directories(d3d8lib BEFORE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_options(d3d8lib INTERFACE /NODEFAULTLIB:libci.lib)
    
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "12.0.8804")
        target_link_libraries(d3d8lib INTERFACE legacy_stdio_definitions)
        target_link_options(d3d8lib INTERFACE /SAFESEH:NO)
    endif()
elseif(MINGW)
    # MinGW-w64 has DirectX 8 libraries in the system
    # Use d3dx8d (debug library works for both debug and release)
    target_link_libraries(d3d8lib INTERFACE d3d8 d3dx8d dinput8 dxguid)
    # ddraw.h header is part of current windows SDK, so can't include it in modern builds
    target_include_directories(d3d8lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/extra)
else()
    # Other compilers - use system libraries
    target_link_libraries(d3d8lib INTERFACE d3d8 d3dx8 dinput8 dxguid)
    target_include_directories(d3d8lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/extra)
endif()

target_compile_definitions(d3d8lib INTERFACE -DBUILD_WITH_D3D8)
target_include_directories(d3d8lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
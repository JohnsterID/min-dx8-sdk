# min-dx8-sdk

Minimal DirectX 8 SDK headers and libraries for MinGW-w64 cross-compilation.

Originally created for building Command & Conquer games (Renegade, Generals, Zero Hour).
Now enhanced with 64-bit MinGW support and comprehensive documentation.

## Features

- [OK] **32-bit MinGW (i686)**: Full D3D8 + D3DX8d support
- [OK] **64-bit MinGW (x86_64)**: D3D8thk kernel layer (wrapper needed for full support)
- [OK] **Architecture detection**: Automatic selection of appropriate libraries
- [OK] **CMake integration**: Easy to use in your projects

## Repository Structure

```
min-dx8-sdk/
├── CMakeLists.txt          # Main build configuration
├── README.md               # This file
│
├── docs/                   # Documentation
│   ├── D3D8_STATUS.md     # D3D8 analysis and wrapper plan
│   ├── D3DX8_STATUS.md    # D3DX8 status and alternatives
│   └── TESTING.md         # Test results (32-bit & 64-bit)
│
├── min-dx8-sdk/           # SDK files (original name kept)
│   ├── d3d8.h, d3dx8*.h   # Headers
│   ├── *.lib              # MSVC import libraries
│   └── extra/             # Additional headers
│
├── test_project/          # Test programs
│   ├── main.cpp
│   └── toolchain-*.cmake
│
└── extracted/             # Extracted data
    └── d3dx8_functions.txt
```

## Quick Start

### Build Test Application

**32-bit MinGW:**
```bash
cd test_project
mkdir build-mingw32 && cd build-mingw32
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw-w64-i686.cmake ..
cmake --build .
```

**64-bit MinGW:**
```bash
cd test_project
mkdir build-mingw64 && cd build-mingw64
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw-w64-x86_64.cmake ..
cmake --build .
```

## Current Status

### 32-bit (i686)
[OK] **Working**: Full D3D8 + D3DX8d support  
[OK] Libraries: `d3d8`, `d3dx8d`, `dinput8`, `dxguid`  

### 64-bit (x86_64)
[WARN] **Limited**: d3d8thk kernel layer only  
[OK] Libraries: `d3d8thk`, `dinput8`, `dxguid`  
[NO] Missing: `Direct3DCreate8()`, COM interfaces  
[BUILD] **Solution**: See [docs/D3D8_STATUS.md](docs/D3D8_STATUS.md)

## Documentation

- **[D3D8 Status](docs/D3D8_STATUS.md)**: Analysis and wrapper plan
- **[D3DX8 Status](docs/D3DX8_STATUS.md)**: Missing features and alternatives
- **[Testing Results](docs/TESTING.md)**: Build test results
- **[D3D8THK Reference](docs/D3D8THK_REFERENCE.md)**: Thunk layer documentation
- **[Style Guide](STYLE.md)**: Code style and conventions

## License

Headers and documentation provided for compatibility purposes.
Not endorsed or approved by Microsoft or EA. Use at your own risk.

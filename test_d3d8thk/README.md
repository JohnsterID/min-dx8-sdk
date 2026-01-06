# d3d8thk Test Suite

## Purpose

This test suite validates that MinGW-w64's `d3d8thk` library provides working DirectDraw and Direct3D kernel-mode functions on Windows 64-bit.

**Critical:** These tests must run on **real Windows 64-bit**. Wine does not implement `OsThunk*` functions.

## Test Files

### test_basic.c
**What it tests:**
- `OsThunkDdCreateDirectDrawObject()` - DirectDraw object creation
- `OsThunkDdQueryDirectDrawObject()` - Capability querying
- `OsThunkDdDeleteDirectDrawObject()` - Object cleanup
- Invalid handle rejection

**Expected result:** All operations succeed, capabilities returned

**Why it matters:** Foundation for all D3D8 operations

### test_surface.c
**What it tests:**
- `OsThunkDdCreateSurface()` - Surface creation (back buffer, texture)
- `OsThunkDdDestroySurface()` - Surface cleanup
- Different surface types (RGB, texture, render target)

**Expected result:** Surfaces can be created and destroyed

**Why it matters:** Surfaces are used for rendering targets, textures, depth buffers

### test_context.c
**What it tests:**
- `OsThunkD3dContextCreate()` - D3D rendering context creation
- `OsThunkD3dContextDestroy()` - Context cleanup
- Context validation

**Expected result:** Context creation succeeds (may fail on modern GPUs without D3D8 driver support)

**Why it matters:** D3D context is required for all rendering operations

### test_draw.c
**What it tests:**
- `OsThunkD3dDrawPrimitives2()` - Main rendering function
- Command buffer structure
- Vertex buffer operations

**Expected result:** Function is callable (actual rendering may fail without proper setup)

**Why it matters:** This is the core rendering function that handles ALL D3D8 drawing

## Building

### Prerequisites
- MinGW-w64 x86_64 toolchain
- CMake 3.10+
- Windows 64-bit for running tests

### Cross-compile from Linux

```bash
mkdir build-test-d3d8thk
cd build-test-d3d8thk

cmake ../test_d3d8thk \
    -DCMAKE_TOOLCHAIN_FILE=../test_project/toolchain-mingw-w64-x86_64.cmake

make
```

### Native build on Windows (MinGW-w64)

```bash
mkdir build-test-d3d8thk
cd build-test-d3d8thk
cmake ../test_d3d8thk -G "MinGW Makefiles"
mingw32-make
```

## Running Tests

### On Windows 64-bit

```cmd
cd build-test-d3d8thk

REM Run all tests
test_basic.exe
test_surface.exe
test_context.exe
test_draw.exe

REM Or run individually with output redirection
test_basic.exe > test_basic_output.txt
```

### Expected Output Example

```
==========================================================
  d3d8thk Basic Functionality Test Suite
==========================================================

=== DirectDraw Object Lifecycle ===
[1] Getting device context...
[OK] GetDC: SUCCESS (handle=0x00000001)

[2] Creating DirectDraw object...
[OK] OsThunkDdCreateDirectDrawObject: SUCCESS (handle=0x12345678)

[3] Querying DirectDraw capabilities...
[OK] OsThunkDdQueryDirectDrawObject: SUCCESS
      - Callback flags: 0x00000001 0x00000002 0x00000004
      - Video memory heaps: 1
      - FourCC codes: 4

[4] Destroying DirectDraw object...
[OK] OsThunkDdDeleteDirectDrawObject: SUCCESS

[5] Releasing device context...
[OK] ReleaseDC: SUCCESS

==========================================================
  TEST SUMMARY
==========================================================
[SUCCESS] All tests passed!

Conclusion: d3d8thk DirectDraw object management works correctly.
```

## Interpreting Results

### All Tests Pass
[OK] **d3d8thk works correctly on your system**
- Proceed with D3D8 wrapper implementation
- All necessary kernel functions are available
- GPU driver supports D3D8 operations

### test_basic or test_surface Fail
[NO] **d3d8thk has fundamental issues**
- Do NOT proceed with wrapper
- Investigate MinGW-w64 library issues
- Check Windows version compatibility

### test_context or test_draw Fail
[WARN] **Driver may not support D3D8**
- Common on modern GPUs (DirectX 12+ era)
- May need driver updates or older GPU
- Consider alternative approach (WineD3D translation)

## Known Limitations

### 1. Modern GPU Drivers
Many modern GPU drivers dropped DirectX 8 support:
- NVIDIA: D3D8 support removed in newer drivers
- AMD: D3D8 support limited or removed
- Intel: D3D8 support limited

**Workaround:** Test on older hardware or use compatibility mode

### 2. Wine Cannot Run These Tests
Wine does not implement `OsThunk*` functions:
- Tests will fail immediately
- No GPU driver to talk to
- Completely different architecture

**Solution:** Must test on real Windows

### 3. Virtual Machines
VMs may have limited D3D8 support:
- VirtualBox: Basic DirectDraw only
- VMware: Better but still limited
- QEMU: Depends on GPU passthrough

**Recommendation:** Test on bare metal if possible

## Test Results Template

Copy this template and fill in your results:

```
=== Test Environment ===
OS: Windows 10/11 64-bit (version: _______)
CPU: _______________________
GPU: _______________________
Driver: ___________________
MinGW-w64: GCC version _____

=== Test Results ===
test_basic:     [PASS/FAIL] - Notes: __________
test_surface:   [PASS/FAIL] - Notes: __________
test_context:   [PASS/FAIL] - Notes: __________
test_draw:      [PASS/FAIL] - Notes: __________

=== Conclusion ===
Can proceed with wrapper: [YES/NO]
Issues found: ___________________________
Recommendations: ________________________
```

## Next Steps

### If All Tests Pass
1. Document test results
2. Begin D3D8 wrapper implementation
3. Start with Phase 1 (COM infrastructure)
4. Reference [docs/D3D8_STATUS.md](../docs/D3D8_STATUS.md) for implementation plan

### If Tests Fail
1. Document which tests failed and error codes
2. Check Windows version compatibility
3. Update GPU drivers
4. Consider testing on different hardware
5. Review [docs/D3D8THK_REFERENCE.md](../docs/D3D8THK_REFERENCE.md) for function details

## Troubleshooting

### Test crashes immediately
- Check Windows version (needs Windows 7+)
- Verify MinGW-w64 runtime DLLs are available
- Run from command prompt to see error messages

### "Function not found" errors
- d3d8thk.dll may be missing
- MinGW-w64 library mismatch
- Check linking with `x86_64-w64-mingw32-nm`

### "Access denied" errors
- May need administrator privileges
- GPU drivers need proper access
- Check Windows security settings

### Context creation fails
- Normal on modern GPUs
- Not a blocker - wrapper can still be built
- Rendering might work through compatibility layers

## Contributing Test Results

If you run these tests, please share results:
1. Fill out the test results template above
2. Note any unusual behavior
3. Document your hardware/software configuration
4. Open an issue or PR with findings

This helps understand d3d8thk compatibility across different systems.

## License

These test files are licensed under GPL3, same as the rest of the repository.
See [LICENSE-GPL3.txt](../LICENSE-GPL3.txt) for details.

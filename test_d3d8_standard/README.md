# Standard D3D8 API Test Suite (32-bit)

This test suite validates the **standard Direct3D8 API** on 32-bit Windows. It provides a comparison baseline for evaluating the **d3d8thk** wrapper (64-bit).

## Purpose

**Compare behavior between:**
- **Standard D3D8 API** (32-bit, this suite) - The "gold standard" reference
- **d3d8thk wrapper** (64-bit, `../test_d3d8thk/`) - The thunk layer implementation

## Test Coverage

| Test Suite | What It Tests | Comparison With |
|------------|---------------|-----------------|
| **test_basic** | Direct3DCreate8, adapter enumeration, capabilities | d3d8thk: test_basic.c |
| **test_device** | Device creation, validation, state queries | d3d8thk: test_context.c |
| **test_rendering** | BeginScene/EndScene, Clear, Present, render states | d3d8thk: test_draw.c |

## Building

### 32-bit MinGW (Windows or Linux cross-compile)

```bash
mkdir build-32bit
cd build-32bit
cmake .. -DCMAKE_TOOLCHAIN_FILE=../test_project/toolchain-mingw-w64-i686.cmake
make
```

### Native Windows (MSVC or MinGW-w64)

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A Win32
cmake --build .
```

## Running Tests

Execute each test individually:

```cmd
test_basic.exe
test_device.exe
test_rendering.exe
```

**Expected results:**
- On D3D8-capable systems: Tests pass
- On modern systems without D3D8: Tests fail with driver errors (expected)
- In VMs/CI: Tests fail due to lack of GPU support (expected)

## Comparison Guide

### 1. Basic Functionality Comparison

**Standard D3D8 (this):**
```c
IDirect3D8* pD3D = Direct3DCreate8(D3D_SDK_VERSION);
UINT count = IDirect3D8_GetAdapterCount(pD3D);
```

**d3d8thk (64-bit):**
```c
HANDLE hDD = OsThunkDdCreateDirectDrawObject(hdc);
BOOL result = OsThunkDdQueryDirectDrawObject(hDD, ...);
```

**Compare:**
- Do both succeed in creating objects?
- Do both fail gracefully with invalid parameters?
- Are error codes consistent?

### 2. Device/Context Creation

**Standard D3D8:**
```c
hr = IDirect3D8_CreateDevice(pD3D, D3DADAPTER_DEFAULT, 
                              D3DDEVTYPE_HAL, hwnd, 
                              flags, &params, &pDevice);
```

**d3d8thk:**
```c
HANDLE hContext = OsThunkD3dContextCreate(hDD, contextData);
```

**Compare:**
- Do both create contexts successfully?
- Do both handle missing GPU drivers similarly?
- Are capabilities reported consistently?

### 3. Rendering Operations

**Standard D3D8:**
```c
IDirect3DDevice8_BeginScene(pDevice);
IDirect3DDevice8_Clear(...);
IDirect3DDevice8_EndScene(pDevice);
IDirect3DDevice8_Present(...);
```

**d3d8thk:**
```c
OsThunkD3dDrawPrimitives2(hContext, drawData);
```

**Compare:**
- Do both support the same primitive types?
- Are draw calls structured similarly?
- Do both handle errors consistently?

## Analysis Workflow

1. **Run both test suites on same hardware**
   ```bash
   # 32-bit standard tests
   cd test_d3d8_standard/build-32bit
   ./test_basic.exe > basic_32bit.txt
   ./test_device.exe > device_32bit.txt
   ./test_rendering.exe > rendering_32bit.txt
   
   # 64-bit d3d8thk tests
   cd ../test_d3d8thk/build-64bit
   ./test_basic.exe > basic_64bit.txt
   ./test_context.exe > context_64bit.txt
   ./test_draw.exe > draw_64bit.txt
   ```

2. **Compare outputs side-by-side**
   ```bash
   diff basic_32bit.txt basic_64bit.txt
   diff device_32bit.txt context_64bit.txt
   diff rendering_32bit.txt draw_64bit.txt
   ```

3. **Document differences**
   - What works on 32-bit but fails on 64-bit?
   - Are error codes different?
   - Are capabilities missing in d3d8thk?
   - Are there performance differences?

4. **Assess wrapper viability**
   - If results are similar → d3d8thk is viable
   - If major differences → wrapper needs special handling
   - If d3d8thk fails completely → may not be usable

## Key Differences to Expect

| Aspect | Standard D3D8 (32-bit) | d3d8thk (64-bit) |
|--------|------------------------|------------------|
| **API** | High-level COM objects | Low-level thunk functions |
| **Object Model** | IDirect3D8, IDirect3DDevice8 | HAL handles, DDraw objects |
| **Error Handling** | HRESULT codes | BOOL return + GetLastError() |
| **Abstraction** | Device-centric | Driver-centric |
| **Documentation** | Well documented | Minimal documentation |

## Success Criteria

✅ **d3d8thk is viable if:**
- Object creation succeeds on both
- Capabilities are reported (even if different)
- Basic rendering operations complete
- Error handling is predictable

❌ **d3d8thk has limitations if:**
- Functions return success but do nothing
- Crashes occur with valid parameters
- Missing critical functionality
- Inconsistent error reporting

## Next Steps

After comparison:

1. **If d3d8thk works similarly** → Proceed with wrapper implementation
2. **If minor differences** → Document workarounds in wrapper
3. **If major issues** → Investigate alternatives or limit 64-bit support

## See Also

- `../test_d3d8thk/` - 64-bit d3d8thk test suite
- `../test_project/` - Simple link test
- `../CMakeLists.txt` - Main SDK configuration
- `../docs/D3D8THK_REFERENCE.md` - d3d8thk API documentation (if exists)

## Platform Support

| Architecture | Support | Notes |
|--------------|---------|-------|
| **i686 (32-bit)** | ✅ Full | Standard D3D8 available in MinGW-w64 |
| **x86_64 (64-bit)** | ❌ N/A | This suite is 32-bit only |

For 64-bit testing, use `../test_d3d8thk/` instead.

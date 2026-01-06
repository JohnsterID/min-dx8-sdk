# Investigating d3d8thk (64-bit MinGW)

## What is d3d8thk?

The "thk" suffix means **"thunk"** - a compatibility/translation layer.

On 64-bit Windows, `d3d8thk.dll` is a thunk that translates 64-bit D3D8 calls to the underlying system.

---

## What's Available in MinGW-w64?

Let me check what we have:

## Investigation Results

### 64-bit MinGW (x86_64)
```bash
$ ls -lh /usr/x86_64-w64-mingw32/lib/ | grep d3d8
-rw-r--r-- 1 root root  46K libd3d8thk.a

$ x86_64-w64-mingw32-nm /usr/x86_64-w64-mingw32/lib/libd3d8thk.a | grep " T " | wc -l
56 functions
```

**All functions are OsThunk* (kernel-level DirectDraw/D3D thunks):**
```
OsThunkDdBlt
OsThunkDdLock
OsThunkDdUnlock
OsThunkD3dContextCreate
OsThunkD3dContextDestroy
OsThunkD3dDrawPrimitives2
OsThunkD3dValidateTextureStageState
... (49 more)
```

**[NO] NO Direct3DCreate8** - The main D3D8 entry point is missing!
**[NO] NO COM interface imports** - No IDirect3D8, IDirect3DDevice8, etc.

### 32-bit MinGW (i686) for Comparison
```bash
$ i686-w64-mingw32-nm /usr/i686-w64-mingw32/lib/libd3d8.a | grep " T "
_Direct3DCreate8@4            [OK] Main entry point
_ValidatePixelShader@16       [OK] Shader validation
_ValidateVertexShader@20      [OK] Shader validation
```

### 64-bit D3D9 for Comparison
```bash
$ x86_64-w64-mingw32-nm /usr/x86_64-w64-mingw32/lib/libd3d9.a | grep " T "
Direct3DCreate9              [OK] Main entry point
Direct3DCreate9Ex
D3DPERF_BeginEvent
D3DPERF_EndEvent
... (more)
```

---

## Analysis: What is d3d8thk?

### The "thk" Problem

**d3d8thk = DirectDraw/Direct3D kernel thunk layer**

These `OsThunk*` functions are:
- **Low-level kernel mode** DirectDraw operations
- Used by **ddraw.dll** and the DirectDraw portion of d3d8.dll
- **NOT the Direct3D8 COM API** that applications use

### What's Missing from d3d8thk:

| Function | Status | Impact |
|----------|--------|--------|
| `Direct3DCreate8` | [NO] Missing | Can't create IDirect3D8 interface |
| `IDirect3D8` methods | [NO] Missing | Can't enumerate adapters, check caps |
| `IDirect3DDevice8` methods | [NO] Missing | Can't render anything |
| Shader validators | [NO] Missing | Can't validate shaders |

**Bottom line:** `d3d8thk.a` alone is **not sufficient** for D3D8 applications!

---

## What Does This Mean?

### For 64-bit D3D8 Applications:

1. **Applications call:** `Direct3DCreate8()` → [NO] Not in d3d8thk
2. **Then call:** `IDirect3D8->CreateDevice()` → [NO] Not in d3d8thk  
3. **Then call:** `IDirect3DDevice8->BeginScene()` → [NO] Not in d3d8thk

**Conclusion:** You **cannot** build a working D3D8 app with only d3d8thk!

### What d3d8thk Actually Provides:

These are **kernel-level functions** that a **full d3d8.dll implementation** would call internally:
```
Your App
    ↓ calls
[d3d8.dll] ← We need to create this!
    ↓ calls internally
[d3d8thk.a] ← What MinGW provides
    ↓ calls
[Windows Kernel]
```

---

## The Solution: Create Full d3d8.dll

### Option 1: Wine's d3d8 + d3d8thk Backend
```
Your App
    ↓
[Wine's d3d8.dll] ← Provides COM interfaces (Direct3DCreate8, IDirect3D8, etc.)
    ↓
[wined3d.dll] ← Wine's translation layer
    ↓
[OpenGL/Vulkan] ← Rendering backend
```

**Pros:**
[OK] Complete D3D8 COM interface
[OK] Works on Linux/Windows
[OK] Battle-tested
[NO] Large dependency (wined3d)

### Option 2: Thin Wrapper on d3d8thk (More Work)
```
Your App
    ↓
[Our d3d8.dll] ← Implement COM interfaces
    ↓
[d3d8thk.a] ← Use for kernel operations
    ↓
[Windows Kernel]
```

**Pros:**
[OK] Smaller, direct to Windows kernel
[OK] No wined3d dependency
[NO] Have to implement COM interfaces ourselves
[NO] Only works on Windows

### Option 3: Hybrid Approach [*] RECOMMENDED
```
Your App
    ↓
[Our d3d8.dll] ← COM interfaces adapted from Wine d3d8
    ↓ 
[d3d8thk.a] ← Use for low-level ops
    ↓
[Windows Kernel]
```

**Combine Wine's COM interface code with d3d8thk backend!**

---

## Next Steps: What Should We Do?

### Investigate Wine's d3d8 Structure

1. **Download Wine's d3d8 source**
   ```bash
   git clone --depth 1 https://gitlab.winehq.org/wine/wine.git wine-d3d8
   cd wine-d3d8/dlls/d3d8
   ```

2. **See how Wine implements:**
   - `Direct3DCreate8()` entry point
   - `IDirect3D8` COM interface
   - `IDirect3DDevice8` COM interface
   - Connection to wined3d

3. **Adapt to use d3d8thk instead of wined3d:**
   - Keep COM interface layer
   - Replace wined3d calls with d3d8thk calls
   - Much less work than full implementation!

### Create Minimal Test

Before full implementation, test that d3d8thk works:
```c
// test_d3d8thk.c
#include <windows.h>
#include <stdio.h>

// Declare OsThunk functions
BOOL WINAPI OsThunkDdQueryDirectDrawObject(void*, void*, void*, void*, void*);

int main() {
    printf("Testing d3d8thk functions...\n");
    // Try calling a thunk function
    return 0;
}
```

Want me to:
1. **Download and analyze Wine's d3d8 source?** 📥
2. **Create a test program for d3d8thk?** 🧪
3. **Design the hybrid d3d8 architecture?** [PLAN]

Let me know which direction to take!
# Minimal D3D8 Wrapper on Top of d3d8thk

## Strategy: Use What MinGW Provides, Add Only What's Missing

### What We Have (d3d8thk)
[OK] 56 kernel-level functions (`OsThunk*`)
[OK] Low-level DirectDraw operations
[OK] Low-level D3D rendering primitives

### What We Need to Add
[NO] `Direct3DCreate8()` - Entry point
[NO] `IDirect3D8` COM interface
[NO] `IDirect3DDevice8` COM interface  
[NO] Other D3D8 COM interfaces (textures, surfaces, etc.)
[NO] `ValidatePixelShader()` and `ValidateVertexShader()`

---

## Step 1: Understand What d3d8thk Provides

Let me extract the complete function list and categorize:

### Complete d3d8thk Function List (56 functions)

```
OsThunkD3dContextCreate
OsThunkD3dContextDestroy
OsThunkD3dContextDestroyAll
OsThunkD3dDrawPrimitives2
OsThunkD3dValidateTextureStageState
OsThunkDdAlphaBlt
OsThunkDdAttachSurface
OsThunkDdBeginMoCompFrame
OsThunkDdBlt
OsThunkDdCanCreateD3DBuffer
OsThunkDdCanCreateSurface
OsThunkDdColorControl
OsThunkDdCreateD3DBuffer
OsThunkDdCreateDirectDrawObject
OsThunkDdCreateMoComp
OsThunkDdCreateSurface
OsThunkDdCreateSurfaceEx
OsThunkDdCreateSurfaceObject
OsThunkDdDeleteDirectDrawObject
OsThunkDdDeleteSurfaceObject
OsThunkDdDestroyD3DBuffer
OsThunkDdDestroyMoComp
OsThunkDdDestroySurface
OsThunkDdEndMoCompFrame
OsThunkDdFlip
OsThunkDdFlipToGDISurface
OsThunkDdGetAvailDriverMemory
OsThunkDdGetBltStatus
OsThunkDdGetDC
OsThunkDdGetDriverInfo
OsThunkDdGetDriverState
OsThunkDdGetDxHandle
OsThunkDdGetFlipStatus
OsThunkDdGetInternalMoCompInfo
OsThunkDdGetMoCompBuffInfo
OsThunkDdGetMoCompFormats
OsThunkDdGetMoCompGuids
OsThunkDdGetScanLine
OsThunkDdLock
OsThunkDdLockD3D
OsThunkDdQueryDirectDrawObject
OsThunkDdQueryMoCompStatus
OsThunkDdReenableDirectDrawObject
OsThunkDdReleaseDC
OsThunkDdRenderMoComp
OsThunkDdResetVisrgn
OsThunkDdSetColorKey
OsThunkDdSetExclusiveMode
OsThunkDdSetGammaRamp
OsThunkDdSetOverlayPosition
OsThunkDdUnattachSurface
OsThunkDdUnlock
OsThunkDdUnlockD3D
OsThunkDdUpdateOverlay
OsThunkDdWaitForVerticalBlank
```

### Categorization

**D3D-specific (5 functions):**
- `OsThunkD3dContextCreate` - Create D3D rendering context
- `OsThunkD3dContextDestroy` - Destroy D3D context
- `OsThunkD3dContextDestroyAll` - Cleanup all contexts
- `OsThunkD3dDrawPrimitives2` - **Main rendering function!**
- `OsThunkD3dValidateTextureStageState` - Validate texture states

**DirectDraw Surface Management (23 functions):**
- Create/destroy surfaces
- Lock/unlock for CPU access
- Blit operations
- DC (device context) access

**DirectDraw Object Management (5 functions):**
- Create/query/delete DirectDraw objects
- Driver queries

**Display/Present (8 functions):**
- Flip, VBlank wait
- Gamma, color control
- Exclusive mode

**Motion Compensation (MoComp) (10 functions):**
- Video acceleration support
- Rarely used by D3D8 apps

---

## Step 2: Analyze What's Missing

### Public D3D8 API (What Apps Call)

From `d3d8.h`:
```c
// Main entry point
IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion);

// Shader validators  
HRESULT WINAPI ValidatePixelShader(...);
HRESULT WINAPI ValidateVertexShader(...);
```

### COM Interfaces (What Apps Use)

```c
// IDirect3D8 - Adapter enumeration, device creation
interface IDirect3D8 : IUnknown {
    RegisterSoftwareDevice(void* pInitializeFunction);
    GetAdapterCount();
    GetAdapterIdentifier(UINT Adapter, ...);
    GetAdapterModeCount(UINT Adapter);
    EnumAdapterModes(UINT Adapter, ...);
    GetAdapterDisplayMode(UINT Adapter, ...);
    CheckDeviceType(...);
    CheckDeviceFormat(...);
    CheckDeviceMultiSampleType(...);
    CheckDepthStencilMatch(...);
    GetDeviceCaps(UINT Adapter, ...);
    CreateDevice(UINT Adapter, ..., IDirect3DDevice8** ppReturnedDeviceInterface);
}

// IDirect3DDevice8 - Main rendering interface (~120 methods!)
interface IDirect3DDevice8 : IUnknown {
    // Device management
    TestCooperativeLevel();
    GetDeviceCaps(...);
    Present(...);
    Reset(...);
    
    // Rendering state
    BeginScene();
    EndScene();
    Clear(...);
    SetRenderState(...);
    GetRenderState(...);
    
    // Drawing
    DrawPrimitive(...);
    DrawIndexedPrimitive(...);
    DrawPrimitiveUP(...);
    DrawIndexedPrimitiveUP(...);
    
    // Resource creation
    CreateTexture(...);
    CreateVertexBuffer(...);
    CreateIndexBuffer(...);
    CreateRenderTarget(...);
    CreateDepthStencilSurface(...);
    
    // Shaders
    CreateVertexShader(...);
    SetVertexShader(...);
    CreatePixelShader(...);
    SetPixelShader(...);
    
    // Textures
    SetTexture(...);
    GetTexture(...);
    SetTextureStageState(...);
    
    // ... ~90 more methods
}

// Other interfaces
IDirect3DTexture8
IDirect3DCubeTexture8
IDirect3DVolumeTexture8
IDirect3DSurface8
IDirect3DVolume8
IDirect3DVertexBuffer8
IDirect3DIndexBuffer8
IDirect3DSwapChain8
```

---

## Step 3: Architecture Design

### Minimal Wrapper Structure

```
d3d8-wrapper/
├── d3d8.c              - Entry points (Direct3DCreate8, validators)
├── d3d8_device.c       - IDirect3DDevice8 implementation
├── d3d8_interface.c    - IDirect3D8 implementation  
├── d3d8_texture.c      - Texture interfaces
├── d3d8_surface.c      - Surface interfaces
├── d3d8_buffer.c       - Vertex/index buffer interfaces
├── d3d8_private.h      - Internal structures
└── d3d8.spec           - Export definitions
```

### Key Implementation Strategy

**IDirect3DDevice8::DrawPrimitive() → OsThunkD3dDrawPrimitives2()**

This is the critical path! Most D3D8 device methods eventually call `OsThunkD3dDrawPrimitives2()`.

```c
// Our wrapper
HRESULT STDMETHODCALLTYPE d3d8_device_DrawPrimitive(
    IDirect3DDevice8 *iface,
    D3DPRIMITIVETYPE PrimitiveType,
    UINT StartVertex,
    UINT PrimitiveCount)
{
    struct d3d8_device *device = impl_from_IDirect3DDevice8(iface);
    
    // Build command buffer for kernel
    D3DHAL_DRAWPRIMITIVES2DATA data;
    data.dwVertexType = device->vertex_shader;
    data.dwVertexSize = device->vertex_size;
    data.dwFirstVertex = StartVertex;
    data.dwPrimitiveCount = PrimitiveCount;
    
    // Call the d3d8thk function
    return OsThunkD3dDrawPrimitives2(device->context, &data);
}
```

---

## Step 4: Minimal Implementation Plan

### Phase 1: Core Infrastructure (Week 1)
1. [OK] COM infrastructure (IUnknown, refcounting)
2. [OK] `Direct3DCreate8()` entry point
3. [OK] `IDirect3D8` stub (adapter enum, caps)
4. [OK] `IDirect3D8::CreateDevice()` - creates device object
5. [OK] Basic `IDirect3DDevice8` structure

**Test:** App can call `Direct3DCreate8()` and create a device

### Phase 2: Device Context (Week 2)
1. [OK] `IDirect3DDevice8::BeginScene()/EndScene()`
2. [OK] `IDirect3DDevice8::Clear()`
3. [OK] `IDirect3DDevice8::Present()`
4. [OK] Connect to `OsThunkD3dContextCreate()`
5. [OK] Connect to `OsThunkDdCreateSurface()` for back buffer

**Test:** App can clear screen to a color

### Phase 3: Basic Rendering (Week 3)
1. [OK] Vertex buffer creation (use `OsThunkDdCreateSurface()` with D3D flag)
2. [OK] `SetVertexShader()` / `SetStreamSource()`
3. [OK] `DrawPrimitive()` → `OsThunkD3dDrawPrimitives2()`
4. [OK] Render state management

**Test:** App can draw a colored triangle

### Phase 4: Textures (Week 4)
1. [OK] Texture creation (`OsThunkDdCreateSurface()` for texture surfaces)
2. [OK] Lock/Unlock for texture upload
3. [OK] `SetTexture()` / `SetTextureStageState()`
4. [OK] Basic sampling

**Test:** App can draw a textured quad

### Phase 5: Complete API (Weeks 5-8)
1. Index buffers
2. Pixel/vertex shaders
3. Render targets
4. Depth/stencil
5. All render states
6. All texture stage states

---

## Step 5: Build System

```cmake
# d3d8-wrapper/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(d3d8-wrapper)

set(D3D8_SOURCES
    d3d8.c
    d3d8_interface.c
    d3d8_device.c
    d3d8_texture.c
    d3d8_surface.c
    d3d8_buffer.c
)

# Create d3d8.dll
add_library(d3d8 SHARED ${D3D8_SOURCES} d3d8.spec)

# Link against d3d8thk (what MinGW provides)
target_link_libraries(d3d8 PRIVATE
    d3d8thk      # ← Use MinGW's thunk layer!
    dxguid
    uuid
)

# Include our headers
target_include_directories(d3d8 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../min-dx8-sdk
)

# For 64-bit only
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Additional 64-bit specific settings
    target_compile_definitions(d3d8 PRIVATE _WIN64)
endif()
```

---

## Comparison: Minimal Wrapper vs Wine Full Stack

| Aspect | Minimal Wrapper | Wine d3d8 + wined3d |
|--------|----------------|---------------------|
| **Size** | ~5,000 LOC | ~50,000+ LOC |
| **Dependencies** | d3d8thk only | wined3d, OpenGL/Vulkan |
| **Platform** | Windows only | Cross-platform |
| **Performance** | Direct to kernel | Translation overhead |
| **Effort** | 6-8 weeks | 3-4 weeks (adaptation) |
| **Maintenance** | We maintain | Wine maintains |

---

## Next Steps

Want me to:
1. [OK] **Create the basic project structure** (d3d8-wrapper directory)
2. [OK] **Implement Phase 1** (COM infrastructure + Direct3DCreate8)
3. [OK] **Create a test program** that validates it works
4. [OK] **Document the d3d8thk calling conventions** (how to use OsThunk functions)

Which should we start with? 

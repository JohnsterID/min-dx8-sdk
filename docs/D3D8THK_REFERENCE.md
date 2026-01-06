# d3d8thk Reference - 64-bit D3D8 Kernel Thunk Layer

## Overview

`d3d8thk.dll` is the Windows 64-bit D3D8 kernel thunk layer. It provides **56 low-level functions** (OsThunk*) that implement DirectDraw and Direct3D operations at the kernel level.

**Provided by:** MinGW-w64 as `libd3d8thk.a` (import library)

**Location:** `/usr/x86_64-w64-mingw32/lib/libd3d8thk.a`

---

## Function Categories

### D3D Context Management (3 functions)

| Function | Purpose |
|----------|---------|
| **OsThunkD3dContextCreate** | Create D3D rendering context |
| **OsThunkD3dContextDestroy** | Destroy D3D context |
| **OsThunkD3dContextDestroyAll** | Cleanup all contexts |

**Usage:** Call `OsThunkD3dContextCreate` after creating surfaces. This is what `IDirect3D8::CreateDevice()` would call internally.

### D3D Rendering (2 functions) [*] CRITICAL

| Function | Purpose |
|----------|---------|
| **OsThunkD3dDrawPrimitives2** | **Main rendering function** - executes all drawing commands |
| **OsThunkD3dValidateTextureStageState** | Validate texture stage states |

**OsThunkD3dDrawPrimitives2** is the most important function! All `Draw*()` methods eventually call this.

```c
// How IDirect3DDevice8::DrawPrimitive would use it:
HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
    D3DNTHAL_DRAWPRIMITIVES2DATA data;
    data.dwVertexType = currentVertexShader;
    data.dwFirstVertex = StartVertex;
    data.dwPrimitiveCount = PrimitiveCount;
    
    return OsThunkD3dDrawPrimitives2(cmdBuffer, vertexBuffer, &data, ...);
}
```

### DirectDraw Surface Management (23 functions)

These handle surfaces, textures, render targets:

**Creation:**
- `OsThunkDdCreateSurface` - Create surface/texture
- `OsThunkDdCreateSurfaceEx` - Extended surface creation
- `OsThunkDdCreateSurfaceObject` - Create surface object

**Destruction:**
- `OsThunkDdDestroySurface` - Destroy surface
- `OsThunkDdDeleteSurfaceObject` - Delete surface object

**Lock/Unlock** (CPU access):
- `OsThunkDdLock` / `OsThunkDdUnlock` - General lock/unlock
- `OsThunkDdLockD3D` / `OsThunkDdUnlockD3D` - D3D-specific

**Blit:**
- `OsThunkDdBlt` - Standard blit
- `OsThunkDdAlphaBlt` - Alpha blending blit

**Attachment:**
- `OsThunkDdAttachSurface` / `OsThunkDdUnattachSurface` - Surface attachment

**Device Context:**
- `OsThunkDdGetDC` / `OsThunkDdReleaseDC` - GDI DC access

**Status:**
- `OsThunkDdGetBltStatus` / `OsThunkDdGetFlipStatus` - Query operation status

### DirectDraw Object Management (5 functions)

| Function | Purpose |
|----------|---------|
| `OsThunkDdCreateDirectDrawObject` | Create DirectDraw object from HDC |
| `OsThunkDdQueryDirectDrawObject` | Query caps and callbacks |
| `OsThunkDdDeleteDirectDrawObject` | Delete DirectDraw object |
| `OsThunkDdReenableDirectDrawObject` | Re-enable after mode change |
| `OsThunkDdGetDriverState` | Get driver state |

**Usage:** `Direct3DCreate8()` would use these to enumerate adapters and get capabilities.

### Display Management (8 functions)

**Flip/Present:**
- `OsThunkDdFlip` - Flip surfaces (present)
- `OsThunkDdFlipToGDISurface` - Flip to GDI surface
- `OsThunkDdWaitForVerticalBlank` - VSync

**Mode:**
- `OsThunkDdSetExclusiveMode` - Exclusive/windowed mode
- `OsThunkDdSetGammaRamp` - Gamma correction

**Info:**
- `OsThunkDdGetScanLine` - Current scanline
- `OsThunkDdCanCreateSurface` - Check if surface can be created
- `OsThunkDdGetAvailDriverMemory` - Available video memory

### D3D Buffer Management (3 functions)

For vertex/index buffers:

| Function | Purpose |
|----------|---------|
| `OsThunkDdCanCreateD3DBuffer` | Check if buffer can be created |
| `OsThunkDdCreateD3DBuffer` | Create vertex/index buffer |
| `OsThunkDdDestroyD3DBuffer` | Destroy buffer |

### Motion Compensation (10 functions)

Video acceleration (rarely used for D3D8):
- `OsThunkDdCreateMoComp` / `OsThunkDdDestroyMoComp`
- `OsThunkDdBeginMoCompFrame` / `OsThunkDdEndMoCompFrame`
- `OsThunkDdRenderMoComp`
- `OsThunkDdQueryMoCompStatus`
- `OsThunkDdGetMoCompGuids` / `OsThunkDdGetMoCompFormats`
- `OsThunkDdGetMoCompBuffInfo`
- `OsThunkDdGetInternalMoCompInfo`

### Miscellaneous (6 functions)

- `OsThunkDdGetDxHandle` - Get DX handle
- `OsThunkDdSetOverlayPosition` / `OsThunkDdUpdateOverlay` - Overlay support
- `OsThunkDdColorControl` - Color controls
- `OsThunkDdResetVisrgn` - Reset visible region
- `OsThunkDdGetDriverInfo` - Driver information

---

## Calling Conventions

### Windows x64 Calling Convention

All functions use **`WINAPI`** (`__stdcall` on 32-bit, `__fastcall` on 64-bit):

**64-bit (x86_64):**
- First 4 integer args: RCX, RDX, R8, R9
- Floating point args: XMM0-XMM3
- Additional args on stack
- Caller cleans up stack
- Return value in RAX

### Function Signatures

See `research/d3d8thk_functions.h` for complete declarations.

**Example:**
```c
BOOL WINAPI OsThunkD3dContextCreate(
    HANDLE hDirectDrawLocal,              // RCX
    HANDLE hSurfColor,                     // RDX  
    HANDLE hSurfZ,                         // R8
    D3DNTHAL_CONTEXTCREATEI *pContextData  // R9
);
```

---

## Data Structures

Most data structures are defined in Windows DDK headers:

- `D3DNTHAL_*` - D3D HAL structures
- `DD_*` - DirectDraw structures  
- `PDD_*` - Pointers to DD structures
- `FLATPTR` - Flat pointer (DWORD_PTR)
- `VIDEOMEMORY` - Video memory descriptor

**Note:** Many of these are not in standard DirectX SDK headers. They're internal kernel structures.

---

## Example Usage Pattern

### Creating a Device (Simplified)

```c
// 1. Create DirectDraw object
HDC hdc = GetDC(hwnd);
HANDLE hDD = OsThunkDdCreateDirectDrawObject(hdc);

// 2. Query capabilities
DD_HALINFO halInfo;
D3DNTHAL_CALLBACKS d3dCallbacks;
OsThunkDdQueryDirectDrawObject(hDD, &halInfo, NULL, 
                                 &d3dCallbacks, NULL, NULL, 
                                 NULL, NULL, NULL, NULL, NULL);

// 3. Create back buffer surface
HANDLE hSurface;
DDSURFACEDESC surfDesc = {0};
surfDesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
surfDesc.dwWidth = 640;
surfDesc.dwHeight = 480;
surfDesc.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;

DD_SURFACE_GLOBAL surfGlobal;
DD_SURFACE_LOCAL surfLocal;
DD_SURFACE_MORE surfMore;
DD_CREATESURFACEDATA createData;

OsThunkDdCreateSurface(hDD, &hSurface, &surfDesc, 
                        &surfGlobal, &surfLocal, &surfMore,
                        &createData, NULL);

// 4. Create D3D rendering context
D3DNTHAL_CONTEXTCREATEI contextData = {0};
OsThunkD3dContextCreate(hDD, hSurface, NULL, &contextData);

// 5. Now ready to render using OsThunkD3dDrawPrimitives2
```

### Drawing Primitives (Simplified)

```c
// Fill command buffer with render states and vertex data
D3DNTHAL_DRAWPRIMITIVES2DATA drawData = {0};
drawData.dwVertexType = D3DFVF_XYZ | D3DFVF_DIFFUSE;
drawData.dwFirstVertex = 0;
drawData.dwPrimitiveCount = 2; // 2 triangles = 1 quad

FLATPTR cmdBuffer, vtxBuffer;
DWORD cmdSize, vtxSize;

OsThunkD3dDrawPrimitives2(hCmdBuf, hVtxBuf, &drawData,
                           &cmdBuffer, &cmdSize,
                           &vtxBuffer, &vtxSize);
```

---

## Key Insights

### 1. **These are NOT the Public API**

Applications never call `OsThunk*` functions directly. They're internal functions that `d3d8.dll` uses.

**Application → d3d8.dll → d3d8thk.dll → Kernel**

### 2. **No COM Interfaces**

d3d8thk provides raw kernel functions, not COM interfaces like `IDirect3D8` or `IDirect3DDevice8`.

### 3. **Complex Data Structures**

Many structures (`D3DNTHAL_*`, `DD_*`) are from Windows DDK (Driver Development Kit), not DirectX SDK.

### 4. **OsThunkD3dDrawPrimitives2 is Critical**

This one function handles ALL rendering. It takes:
- Command buffer (render states, textures, shaders)
- Vertex buffer
- Draw parameters

### 5. **Surface Creation Multipurpose**

`OsThunkDdCreateSurface` creates:
- Back buffers
- Textures
- Render targets
- Depth/stencil buffers

Differentiated by `DDSCAPS_*` flags.

---

## References

- **Header:** `research/d3d8thk_functions.h` - Function declarations
- **Library:** `/usr/x86_64-w64-mingw32/lib/libd3d8thk.a`
- **Symbol Count:** 56 functions
- **Architecture:** x86_64 (64-bit) only

---

## Next Steps

To build a complete D3D8 wrapper:

1. [OK] **Documented OsThunk functions** (this file)
2. [TODO] **Implement Direct3DCreate8()** entry point
3. [TODO] **Implement IDirect3D8 COM interface**
4. [TODO] **Implement IDirect3DDevice8 COM interface**
5. [TODO] **Map IDirect3DDevice8 methods → OsThunk calls**

See [D3D8_STATUS.md](D3D8_STATUS.md) for implementation plan.

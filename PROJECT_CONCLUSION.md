# MinGW-w64 64-bit Support - Project Conclusion

**Date:** 2026-01-07 -
**Branch:** mingw-64bit-support -
**Status:** CONCLUDED - d3d8thk not viable -

---

## Summary

This project investigated whether d3d8thk (64-bit DirectDraw kernel functions) could enable native 64-bit D3D8 support.

**Result:** d3d8thk does not work on modern GPU drivers and cannot be made to work.

---

## What We Learned

### 32-bit D3D8 Works

**Intel Iris Xe Graphics supports standard D3D8:**
- Direct3DCreate8 succeeds
- Device creation works with Software Vertex Processing
- Hardware VP not supported, but Software VP sufficient
- Games work because they have fallback logic

**Fixes made:**
- Updated test_device.c to try Hardware VP → Software VP → REF
- Updated test_rendering.c with same fallback logic
- Tests now pass on modern hardware

### d3d8thk Does Not Work

**All d3d8thk tests failed at DirectDraw object creation:**
```
OsThunkDdCreateDirectDrawObject(hdc) → returns NULL
```

**Why:**
- Modern GPU drivers (Intel, NVIDIA, AMD) use WDDM architecture
- DirectDraw kernel-mode DDI was deprecated after Windows Vista/7
- Drivers no longer implement legacy DirectDraw kernel interfaces
- This is a driver/OS limitation, not a code issue

**Cannot be fixed:**
- Cannot implement missing kernel drivers
- Cannot force modern drivers to support deprecated interfaces
- d3d8thk functions exist but return errors

### Solution Already Exists

**d3d8to9 wrapper:**
- Mature, production-ready project
- Translates D3D8 API calls to D3D9
- Works on all modern GPUs
- Open source (BSD license)
- No point reinventing this

---

## Project Findings

| Component | Status | Result |
|-----------|--------|--------|
| 32-bit D3D8 | Works | Software VP on Intel Iris Xe |
| test_device.c fix | Fixed | Added Hardware/Software VP fallback |
| test_rendering.c fix | Fixed | Added Hardware/Software VP fallback |
| d3d8thk kernel API | Doesn't work | Driver limitation |
| 64-bit native D3D8 | Not viable via d3d8thk | Use d3d8to9 instead |

---

## Recommendations

### For Users Needing 64-bit D3D8:

**Option 1: Use 32-bit with WOW64 - Recommended
- 32-bit D3D8 works perfectly on 64-bit Windows
- No additional dependencies
- Games already use this

**Option 2: Use d3d8to9 wrapper**
- Get it from: https://github.com/crosire/d3d8to9
- Drop-in replacement d3d8.dll (64-bit)
- Translates to D3D9 (fully supported)

**Option 3: Do nothing**
- 32-bit D3D8 games work fine on modern Windows
- WOW64 handles everything automatically

### For This Project:

**Recommended actions:**
1. Commit the test fixes (Hardware/Software VP fallback)
2. Document findings (this file)
3. Archive the d3d8thk investigation
4. Don't pursue custom 64-bit wrapper (d3d8to9 exists)

---

## Technical Details

### Two Different APIs

**1. Standard D3D8 (d3d8.dll)**
```
User App → d3d8.dll → d3d9.dll (translation layer) → GPU Driver
```
 Works on modern hardware

**2. d3d8thk (kernel DDI)**
```
User App → gdi32.dll → win32k.sys → DirectDraw kernel driver → GPU
 -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  ↑
 -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  BROKEN
```
 Modern drivers don't implement DirectDraw kernel DDI

### Why Modern Drivers Don't Support d3d8thk

**Windows Display Driver Model (WDDM)** introduced in Vista:
- Replaced legacy XPDM model
- Removed DirectDraw kernel-mode DDI
- GPU drivers only implement WDDM interfaces
- High-level APIs (D3D8, D3D9) translated by Microsoft runtime
- Low-level kernel DDI no longer exposed

**Intel Iris Xe (2020+) driver:**
- - Supports: D3D9, D3D10, D3D11, D3D12, Vulkan
- - Supports: D3D8 via d3d8.dll → d3d9.dll translation
- - Doesn't support: DirectDraw kernel DDI (removed)

---

## What Got Fixed

### test_d3d8_standard/test_device.c

**Before:**
```c
CreateDevice(..., D3DCREATE_SOFTWARE_VERTEXPROCESSING, ...);
if (FAILED) → Try REF device
```

**After:**
```c
CreateDevice(..., D3DCREATE_HARDWARE_VERTEXPROCESSING, ...);
if (FAILED) {
 -  - CreateDevice(..., D3DCREATE_SOFTWARE_VERTEXPROCESSING, ...);
 -  - if (FAILED) {
 -  -  -  - CreateDevice(..., D3DDEVTYPE_REF, ...);
 -  - }
}
```

**Result:** Tests now pass on Intel Iris Xe (uses Software VP)

### test_d3d8_standard/test_rendering.c

**Same fix applied** - Hardware VP → Software VP → REF fallback

---

## Conclusion

**Original goal:** Use d3d8thk for native 64-bit D3D8 -
**Finding:** d3d8thk doesn't work on modern drivers -
**Alternative:** d3d8to9 already solves this problem -
**Outcome:** Project goal not achievable via d3d8thk -

**Value delivered:**
- - Documented that d3d8thk is not viable
- - Fixed 32-bit D3D8 tests to work on modern hardware
- - Confirmed 32-bit D3D8 works on Intel Iris Xe
- - Identified existing solution (d3d8to9)

---

## Files Modified

```
test_d3d8_standard/test_device.c -  - - Added Hardware/Software VP fallback
test_d3d8_standard/test_rendering.c - Added Hardware/Software VP fallback
PROJECT_CONCLUSION.md -  -  -  -  -  -  -  - This file
```

**Ready to commit.**

---

## Final Recommendation

**Stop work on d3d8thk approach.**

If 64-bit D3D8 is needed:
- Use existing d3d8to9 wrapper: https://github.com/crosire/d3d8to9
- It's mature, tested, and works on all modern GPUs
- No reason to duplicate this effort

**Project can be archived as "d3d8thk investigation - not viable on modern hardware"**

---

**End of investigation. **

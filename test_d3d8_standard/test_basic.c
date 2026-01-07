/*
 * test_basic.c - Basic Direct3D8 API validation (32-bit)
 *
 * Tests fundamental D3D8 operations:
 * - Direct3DCreate8
 * - Adapter enumeration
 * - Capability querying
 * - Object lifecycle
 *
 * This tests the STANDARD D3D8 API (32-bit) for comparison with d3d8thk (64-bit).
 */

#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>

static void print_test_header(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
}

static void print_result(const char* operation, BOOL success, void* value) {
    if (success) {
        printf("[OK] %s: SUCCESS", operation);
        if (value) {
            printf(" (ptr=%p)", value);
        }
        printf("\n");
    } else {
        printf("[FAIL] %s: FAILED (error=%lu)\n", operation, GetLastError());
    }
}

int test_d3d8_creation(void) {
    print_test_header("Direct3D8 Object Creation");
    
    IDirect3D8* pD3D = NULL;
    int failures = 0;
    
    /* Step 1: Create Direct3D8 object */
    printf("[1] Calling Direct3DCreate8(D3D_SDK_VERSION)...\n");
    pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    
    if (pD3D == NULL) {
        printf("[FAIL] Direct3DCreate8 returned NULL\n");
        printf("       This usually means:\n");
        printf("       - d3d8.dll is not available\n");
        printf("       - D3D SDK version mismatch\n");
        printf("       - System doesn't support D3D8\n");
        return 1;
    }
    print_result("Direct3DCreate8", TRUE, pD3D);
    
    /* Step 2: Release the object */
    printf("\n[2] Releasing Direct3D8 object...\n");
    ULONG refcount = IDirect3D8_Release(pD3D);
    printf("[OK] IDirect3D8::Release returned refcount=%lu\n", refcount);
    
    if (refcount != 0) {
        printf("[WARN] Reference count is not zero after release\n");
        failures++;
    }
    
    return failures;
}

int test_adapter_enumeration(void) {
    print_test_header("Adapter Enumeration");
    
    IDirect3D8* pD3D = NULL;
    UINT adapterCount;
    int failures = 0;
    
    /* Create D3D object */
    pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    if (pD3D == NULL) {
        printf("[FAIL] Could not create Direct3D8 object\n");
        return 1;
    }
    
    /* Step 1: Get adapter count */
    printf("[1] Getting adapter count...\n");
    adapterCount = IDirect3D8_GetAdapterCount(pD3D);
    printf("[OK] IDirect3D8::GetAdapterCount returned %u adapter(s)\n", adapterCount);
    
    if (adapterCount == 0) {
        printf("[WARN] No adapters found - this is unusual\n");
        failures++;
    }
    
    /* Step 2: Query each adapter */
    for (UINT i = 0; i < adapterCount; i++) {
        D3DADAPTER_IDENTIFIER8 identifier;
        HRESULT hr;
        
        printf("\n[Adapter %u]\n", i);
        
        hr = IDirect3D8_GetAdapterIdentifier(pD3D, i, 0, &identifier);
        if (FAILED(hr)) {
            printf("[FAIL] GetAdapterIdentifier failed: hr=0x%08lX\n", hr);
            failures++;
            continue;
        }
        
        printf("  Driver: %s\n", identifier.Driver);
        printf("  Description: %s\n", identifier.Description);
        printf("  VendorId: 0x%04X\n", identifier.VendorId);
        printf("  DeviceId: 0x%04X\n", identifier.DeviceId);
        printf("  Revision: 0x%08lX\n", identifier.Revision);
        printf("  GUID: {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
               identifier.DeviceIdentifier.Data1,
               identifier.DeviceIdentifier.Data2,
               identifier.DeviceIdentifier.Data3,
               identifier.DeviceIdentifier.Data4[0],
               identifier.DeviceIdentifier.Data4[1],
               identifier.DeviceIdentifier.Data4[2],
               identifier.DeviceIdentifier.Data4[3],
               identifier.DeviceIdentifier.Data4[4],
               identifier.DeviceIdentifier.Data4[5],
               identifier.DeviceIdentifier.Data4[6],
               identifier.DeviceIdentifier.Data4[7]);
    }
    
    /* Cleanup */
    IDirect3D8_Release(pD3D);
    
    return failures;
}

int test_display_modes(void) {
    print_test_header("Display Mode Enumeration");
    
    IDirect3D8* pD3D = NULL;
    UINT modeCount;
    int failures = 0;
    
    /* Create D3D object */
    pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    if (pD3D == NULL) {
        printf("[FAIL] Could not create Direct3D8 object\n");
        return 1;
    }
    
    /* Query available display modes for default adapter */
    printf("[1] Querying display modes for default adapter (D3DFMT_X8R8G8B8)...\n");
    modeCount = IDirect3D8_GetAdapterModeCount(pD3D, D3DADAPTER_DEFAULT);
    printf("[OK] Found %u display mode(s)\n", modeCount);
    
    if (modeCount == 0) {
        printf("[WARN] No display modes found\n");
        failures++;
    } else {
        /* Show first few modes as samples */
        UINT samplesToShow = (modeCount < 5) ? modeCount : 5;
        printf("\n[Sample modes - showing %u of %u]:\n", samplesToShow, modeCount);
        
        for (UINT i = 0; i < samplesToShow; i++) {
            D3DDISPLAYMODE mode;
            HRESULT hr = IDirect3D8_EnumAdapterModes(pD3D, D3DADAPTER_DEFAULT, i, &mode);
            
            if (SUCCEEDED(hr)) {
                printf("  Mode %u: %ux%u @ %uHz, Format=%d\n",
                       i, mode.Width, mode.Height, mode.RefreshRate, mode.Format);
            }
        }
    }
    
    /* Cleanup */
    IDirect3D8_Release(pD3D);
    
    return failures;
}

int test_device_caps(void) {
    print_test_header("Device Capabilities");
    
    IDirect3D8* pD3D = NULL;
    D3DCAPS8 caps;
    HRESULT hr;
    int failures = 0;
    
    /* Create D3D object */
    pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    if (pD3D == NULL) {
        printf("[FAIL] Could not create Direct3D8 object\n");
        return 1;
    }
    
    /* Query device capabilities */
    printf("[1] Querying device capabilities for default adapter...\n");
    hr = IDirect3D8_GetDeviceCaps(pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
    
    if (FAILED(hr)) {
        printf("[FAIL] GetDeviceCaps failed: hr=0x%08lX\n", hr);
        printf("       Common causes:\n");
        printf("       - No hardware acceleration available\n");
        printf("       - GPU driver doesn't support D3D8\n");
        IDirect3D8_Release(pD3D);
        return 1;
    }
    
    printf("[OK] GetDeviceCaps succeeded\n");
    printf("\n[Key Capabilities]:\n");
    printf("  Device Type: %lu\n", caps.DeviceType);
    printf("  Max Texture Width: %lu\n", caps.MaxTextureWidth);
    printf("  Max Texture Height: %lu\n", caps.MaxTextureHeight);
    printf("  Max Active Lights: %lu\n", caps.MaxActiveLights);
    printf("  Max Simultaneous Textures: %lu\n", caps.MaxSimultaneousTextures);
    printf("  Max Vertex Blend Matrices: %lu\n", caps.MaxVertexBlendMatrices);
    printf("  Max User Clip Planes: %lu\n", caps.MaxUserClipPlanes);
    printf("  Max Primitive Count: %lu\n", caps.MaxPrimitiveCount);
    printf("  Max Vertex Index: %lu\n", caps.MaxVertexIndex);
    printf("  Max Streams: %lu\n", caps.MaxStreams);
    
    printf("\n[Capability Flags]:\n");
    printf("  Hardware Rasterization: %s\n", 
           (caps.DevCaps & D3DDEVCAPS_HWRASTERIZATION) ? "YES" : "NO");
    printf("  Hardware Transform & Lighting: %s\n",
           (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) ? "YES" : "NO");
    printf("  Pure Device: %s\n",
           (caps.DevCaps & D3DDEVCAPS_PUREDEVICE) ? "YES" : "NO");
    
    /* Cleanup */
    IDirect3D8_Release(pD3D);
    
    return failures;
}

int main(void) {
    printf("==========================================================\n");
    printf("  Standard D3D8 API Test Suite (32-bit)\n");
    printf("==========================================================\n");
    printf("\nThis test validates the STANDARD Direct3D8 API on 32-bit.\n");
    printf("Compare these results with d3d8thk tests (64-bit) to identify\n");
    printf("differences in behavior, capabilities, and limitations.\n");
    printf("\nNOTE: Tests may fail if GPU driver doesn't support D3D8.\n");
    
    int total_failures = 0;
    
    /* Run test suites */
    total_failures += test_d3d8_creation();
    total_failures += test_adapter_enumeration();
    total_failures += test_display_modes();
    total_failures += test_device_caps();
    
    /* Print summary */
    printf("\n==========================================================\n");
    printf("  TEST SUMMARY\n");
    printf("==========================================================\n");
    
    if (total_failures == 0) {
        printf("[SUCCESS] All tests passed!\n");
        printf("\nConclusion: Standard D3D8 API works correctly on this system.\n");
        printf("Compare with d3d8thk results to assess 64-bit viability.\n");
        return 0;
    } else {
        printf("[FAILED] %d test(s) failed\n", total_failures);
        printf("\nPossible causes:\n");
        printf("- GPU driver doesn't support D3D8 (common on modern hardware)\n");
        printf("- d3d8.dll not available or incompatible\n");
        printf("- Running in incompatible environment (VM, CI, etc.)\n");
        printf("\nNext steps:\n");
        printf("- Test on real hardware with D3D8-capable GPU\n");
        printf("- Compare failure patterns with d3d8thk tests\n");
        return 1;
    }
}

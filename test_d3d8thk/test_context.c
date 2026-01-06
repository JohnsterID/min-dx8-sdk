/*
 * test_context.c - d3d8thk D3D context validation
 *
 * Tests D3D rendering context creation:
 * - D3D context creation
 * - Context destruction
 * - Context validation
 *
 * Expected Result: Can create and destroy D3D rendering contexts
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simplified structures from Windows DDK */
typedef struct _DDSURFACEDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    LONG lPitch;
    DWORD dwBackBufferCount;
    DWORD dwReserved1[9];
    struct {
        DWORD dwCaps;
        DWORD dwCaps2;
        DWORD dwCaps3;
        DWORD dwCaps4;
    } ddsCaps;
    struct {
        DWORD dwSize;
        DWORD dwFlags;
        DWORD dwFourCC;
        DWORD dwRGBBitCount;
        DWORD dwRBitMask;
        DWORD dwGBitMask;
        DWORD dwBBitMask;
        DWORD dwRGBAlphaBitMask;
    } ddpfPixelFormat;
} DDSURFACEDESC;

typedef struct _D3DNTHAL_CONTEXTCREATEI {
    HANDLE hDDGlobal;
    HANDLE hDDLocal;
    void* pD3DDriverData;
    void* pUnused1;
    void* pUnused2;
    void* pUnused3;
    DWORD dwPID;
    DWORD dwFlags;
    void* pUnused4;
    HANDLE hContext;
    DWORD dwUnused1;
    DWORD dwUnused2;
} D3DNTHAL_CONTEXTCREATEI;

/* DDSURFACEDESC flags */
#define DDSD_CAPS           0x00000001
#define DDSD_HEIGHT         0x00000002
#define DDSD_WIDTH          0x00000004
#define DDSD_PIXELFORMAT    0x00001000

/* DDSCAPS flags */
#define DDSCAPS_VIDEOMEMORY 0x00004000
#define DDSCAPS_3DDEVICE    0x00002000

/* d3d8thk function declarations */
HANDLE WINAPI OsThunkDdCreateDirectDrawObject(HDC hdc);
BOOL WINAPI OsThunkDdDeleteDirectDrawObject(HANDLE hDD);
BOOL WINAPI OsThunkDdQueryDirectDrawObject(
    HANDLE hDD, void* p1, DWORD* p2, void* p3, void* p4,
    void* p5, void* p6, DWORD* p7, void* p8, DWORD* p9, DWORD* p10
);

DWORD WINAPI OsThunkDdCreateSurface(
    HANDLE hDD,
    HANDLE* phSurface,
    DDSURFACEDESC* pSurfaceDesc,
    void* pSurfaceGlobal,
    void* pSurfaceLocal,
    void* pSurfaceMore,
    void* pCreateSurfaceData,
    void* pHandle
);

DWORD WINAPI OsThunkDdDestroySurface(HANDLE hSurface, BOOL bRealDestroy);

DWORD WINAPI OsThunkD3dContextCreate(
    HANDLE hDD,
    HANDLE hSurface,
    void* pUnused,
    D3DNTHAL_CONTEXTCREATEI* pContextData
);

DWORD WINAPI OsThunkD3dContextDestroy(
    D3DNTHAL_CONTEXTCREATEI* pContextData
);

static void print_test_header(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
}

static HANDLE create_dd_object(void) {
    HDC hdc = GetDC(NULL);
    if (hdc == NULL) {
        return NULL;
    }
    
    HANDLE hDD = OsThunkDdCreateDirectDrawObject(hdc);
    ReleaseDC(NULL, hdc);
    
    if (hDD == NULL || hDD == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    
    /* Initialize with query */
    DWORD flags[3] = {0};
    DWORD heaps = 0;
    DWORD fourcc = 0;
    OsThunkDdQueryDirectDrawObject(hDD, NULL, flags, NULL, NULL, NULL, NULL,
                                     &heaps, NULL, &fourcc, NULL);
    
    return hDD;
}

static HANDLE create_render_target(HANDLE hDD) {
    HANDLE hSurface = NULL;
    DDSURFACEDESC surfDesc;
    DWORD result;
    
    memset(&surfDesc, 0, sizeof(surfDesc));
    surfDesc.dwSize = sizeof(surfDesc);
    surfDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    surfDesc.dwWidth = 640;
    surfDesc.dwHeight = 480;
    surfDesc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;
    surfDesc.ddpfPixelFormat.dwSize = sizeof(surfDesc.ddpfPixelFormat);
    surfDesc.ddpfPixelFormat.dwFlags = 0x40;  /* DDPF_RGB */
    surfDesc.ddpfPixelFormat.dwRGBBitCount = 32;
    surfDesc.ddpfPixelFormat.dwRBitMask = 0x00FF0000;
    surfDesc.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
    surfDesc.ddpfPixelFormat.dwBBitMask = 0x000000FF;
    
    result = OsThunkDdCreateSurface(
        hDD, &hSurface, &surfDesc, NULL, NULL, NULL, NULL, NULL
    );
    
    if (result != 0) {
        return NULL;
    }
    
    return hSurface;
}

int test_context_lifecycle(void) {
    print_test_header("D3D Context Lifecycle");
    
    HANDLE hDD = NULL;
    HANDLE hSurface = NULL;
    D3DNTHAL_CONTEXTCREATEI contextData;
    DWORD result;
    int failures = 0;
    
    /* Step 1: Create DirectDraw object */
    printf("[1] Creating DirectDraw object...\n");
    hDD = create_dd_object();
    if (hDD == NULL) {
        printf("[FAIL] Could not create DirectDraw object\n");
        return 1;
    }
    printf("[OK] DirectDraw object: %p\n", hDD);
    
    /* Step 2: Create render target surface */
    printf("\n[2] Creating render target surface...\n");
    hSurface = create_render_target(hDD);
    if (hSurface == NULL) {
        printf("[FAIL] Could not create render target\n");
        OsThunkDdDeleteDirectDrawObject(hDD);
        return 1;
    }
    printf("[OK] Render target: %p\n", hSurface);
    
    /* Step 3: Create D3D context */
    printf("\n[3] Creating D3D rendering context...\n");
    memset(&contextData, 0, sizeof(contextData));
    contextData.hDDGlobal = hDD;
    contextData.hDDLocal = hDD;
    contextData.dwPID = GetCurrentProcessId();
    
    result = OsThunkD3dContextCreate(
        hDD,
        hSurface,
        NULL,
        &contextData
    );
    
    if (result != 0) {  /* DD_OK = 0 */
        printf("[FAIL] OsThunkD3dContextCreate failed (result=0x%08lX)\n", result);
        printf("       This is expected if GPU driver doesn't support D3D8\n");
        failures++;
    } else {
        printf("[OK] D3D context created\n");
        printf("     - Context handle: %p\n", contextData.hContext);
        printf("     - Process ID: %lu\n", contextData.dwPID);
        
        /* Step 4: Destroy D3D context */
        printf("\n[4] Destroying D3D context...\n");
        result = OsThunkD3dContextDestroy(&contextData);
        if (result != 0) {
            printf("[FAIL] OsThunkD3dContextDestroy failed (result=0x%08lX)\n", result);
            failures++;
        } else {
            printf("[OK] D3D context destroyed\n");
        }
    }
    
    /* Cleanup */
    printf("\n[5] Cleaning up resources...\n");
    OsThunkDdDestroySurface(hSurface, TRUE);
    OsThunkDdDeleteDirectDrawObject(hDD);
    printf("[OK] Cleanup complete\n");
    
    return failures;
}

int test_context_validation(void) {
    print_test_header("Context Validation Tests");
    
    DWORD result;
    D3DNTHAL_CONTEXTCREATEI contextData;
    int expected_failures = 0;
    
    /* Test 1: Create context with NULL DirectDraw handle */
    printf("[1] Testing context creation with NULL DirectDraw handle...\n");
    memset(&contextData, 0, sizeof(contextData));
    
    result = OsThunkD3dContextCreate(NULL, NULL, NULL, &contextData);
    
    if (result != 0) {
        printf("[OK] Correctly rejected NULL handle (result=0x%08lX)\n", result);
    } else {
        printf("[WARN] Unexpectedly accepted NULL handle\n");
        OsThunkD3dContextDestroy(&contextData);
        expected_failures++;
    }
    
    /* Test 2: Destroy NULL context */
    printf("\n[2] Testing context destruction with NULL data...\n");
    memset(&contextData, 0, sizeof(contextData));
    
    result = OsThunkD3dContextDestroy(&contextData);
    
    if (result != 0) {
        printf("[OK] Correctly rejected NULL context (result=0x%08lX)\n", result);
    } else {
        printf("[WARN] Unexpectedly accepted NULL context\n");
        expected_failures++;
    }
    
    return expected_failures;
}

int main(void) {
    printf("==========================================================\n");
    printf("  d3d8thk D3D Context Test Suite\n");
    printf("==========================================================\n");
    printf("\nThis test validates D3D rendering context creation through\n");
    printf("d3d8thk on Windows 64-bit.\n");
    printf("\nNOTE: Some tests may fail if GPU driver doesn't support D3D8.\n");
    
    int total_failures = 0;
    
    /* Run test suites */
    total_failures += test_context_lifecycle();
    total_failures += test_context_validation();
    
    /* Print summary */
    printf("\n==========================================================\n");
    printf("  TEST SUMMARY\n");
    printf("==========================================================\n");
    
    if (total_failures == 0) {
        printf("[SUCCESS] All tests passed!\n");
        printf("\nConclusion: d3d8thk D3D context management works correctly.\n");
        printf("You can proceed with building the D3D8 wrapper.\n");
        return 0;
    } else {
        printf("[FAILED] %d test(s) failed\n", total_failures);
        printf("\nPossible causes:\n");
        printf("- GPU driver doesn't support D3D8 (common on modern hardware)\n");
        printf("- d3d8thk implementation has limitations\n");
        printf("- Test needs adjustment for actual driver behavior\n");
        printf("\nRecommendation: Review failures and adjust wrapper strategy.\n");
        return 1;
    }
}

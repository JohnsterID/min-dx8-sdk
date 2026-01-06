/*
 * test_basic.c - Basic d3d8thk validation
 *
 * Tests fundamental d3d8thk operations:
 * - DirectDraw object creation
 * - Capability querying
 * - Object destruction
 *
 * Expected Result: All operations succeed with valid handles
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

/* d3d8thk function declarations */
HANDLE WINAPI OsThunkDdCreateDirectDrawObject(HDC hdc);
BOOL WINAPI OsThunkDdQueryDirectDrawObject(
    HANDLE hDD,
    void* pHalInfo,
    DWORD* pCallBackFlags,
    void* pD3dCallbacks,
    void* pD3dDriverData,
    void* pD3dBufferCallbacks,
    void* pTextureFormats,
    DWORD* pdwNumHeaps,
    void* pVidMemHeap,
    DWORD* pdwNumFourCC,
    DWORD* pdwFourCC
);
BOOL WINAPI OsThunkDdDeleteDirectDrawObject(HANDLE hDD);

static void print_test_header(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
}

static void print_result(const char* operation, BOOL success, void* value) {
    if (success) {
        printf("[OK] %s: SUCCESS", operation);
        if (value) {
            printf(" (handle=%p)", value);
        }
        printf("\n");
    } else {
        printf("[FAIL] %s: FAILED (error=%lu)\n", operation, GetLastError());
    }
}

int test_directdraw_lifecycle(void) {
    print_test_header("DirectDraw Object Lifecycle");
    
    HDC hdc;
    HANDLE hDD;
    BOOL result;
    int failures = 0;
    
    /* Step 1: Get device context */
    printf("[1] Getting device context...\n");
    hdc = GetDC(NULL);
    if (hdc == NULL) {
        printf("[FAIL] GetDC failed (error=%lu)\n", GetLastError());
        return 1;
    }
    print_result("GetDC", TRUE, hdc);
    
    /* Step 2: Create DirectDraw object */
    printf("\n[2] Creating DirectDraw object...\n");
    hDD = OsThunkDdCreateDirectDrawObject(hdc);
    if (hDD == NULL || hDD == INVALID_HANDLE_VALUE) {
        printf("[FAIL] OsThunkDdCreateDirectDrawObject failed (error=%lu)\n", GetLastError());
        ReleaseDC(NULL, hdc);
        return 1;
    }
    print_result("OsThunkDdCreateDirectDrawObject", TRUE, hDD);
    
    /* Step 3: Query capabilities (basic call) */
    printf("\n[3] Querying DirectDraw capabilities...\n");
    DWORD callBackFlags[3] = {0};
    DWORD numHeaps = 0;
    DWORD numFourCC = 0;
    
    result = OsThunkDdQueryDirectDrawObject(
        hDD,
        NULL,           /* HalInfo - NULL for size query */
        callBackFlags,  /* Callback flags */
        NULL,           /* D3D callbacks */
        NULL,           /* D3D driver data */
        NULL,           /* D3D buffer callbacks */
        NULL,           /* Texture formats */
        &numHeaps,      /* Number of heaps */
        NULL,           /* Video memory heap array */
        &numFourCC,     /* Number of FourCC codes */
        NULL            /* FourCC array */
    );
    
    if (!result) {
        printf("[FAIL] OsThunkDdQueryDirectDrawObject failed (error=%lu)\n", GetLastError());
        failures++;
    } else {
        printf("[OK] OsThunkDdQueryDirectDrawObject: SUCCESS\n");
        printf("      - Callback flags: 0x%08lX 0x%08lX 0x%08lX\n", 
               callBackFlags[0], callBackFlags[1], callBackFlags[2]);
        printf("      - Video memory heaps: %lu\n", numHeaps);
        printf("      - FourCC codes: %lu\n", numFourCC);
    }
    
    /* Step 4: Destroy DirectDraw object */
    printf("\n[4] Destroying DirectDraw object...\n");
    result = OsThunkDdDeleteDirectDrawObject(hDD);
    if (!result) {
        printf("[FAIL] OsThunkDdDeleteDirectDrawObject failed (error=%lu)\n", GetLastError());
        failures++;
    } else {
        print_result("OsThunkDdDeleteDirectDrawObject", TRUE, NULL);
    }
    
    /* Step 5: Release device context */
    printf("\n[5] Releasing device context...\n");
    ReleaseDC(NULL, hdc);
    print_result("ReleaseDC", TRUE, NULL);
    
    return failures;
}

int test_invalid_handles(void) {
    print_test_header("Invalid Handle Handling");
    
    BOOL result;
    int expected_failures = 0;
    
    /* Test 1: Query with NULL handle */
    printf("[1] Testing OsThunkDdQueryDirectDrawObject with NULL handle...\n");
    DWORD callBackFlags[3] = {0};
    DWORD numHeaps = 0;
    DWORD numFourCC = 0;
    
    result = OsThunkDdQueryDirectDrawObject(
        NULL, NULL, callBackFlags, NULL, NULL, NULL, NULL,
        &numHeaps, NULL, &numFourCC, NULL
    );
    
    if (!result) {
        printf("[OK] Correctly rejected NULL handle (error=%lu)\n", GetLastError());
    } else {
        printf("[WARN] Unexpectedly accepted NULL handle\n");
        expected_failures++;
    }
    
    /* Test 2: Delete with NULL handle */
    printf("\n[2] Testing OsThunkDdDeleteDirectDrawObject with NULL handle...\n");
    result = OsThunkDdDeleteDirectDrawObject(NULL);
    
    if (!result) {
        printf("[OK] Correctly rejected NULL handle (error=%lu)\n", GetLastError());
    } else {
        printf("[WARN] Unexpectedly accepted NULL handle\n");
        expected_failures++;
    }
    
    return expected_failures;
}

int main(void) {
    printf("==========================================================\n");
    printf("  d3d8thk Basic Functionality Test Suite\n");
    printf("==========================================================\n");
    printf("\nThis test validates that d3d8thk provides working DirectDraw\n");
    printf("object management functions on Windows 64-bit.\n");
    
    int total_failures = 0;
    
    /* Run test suites */
    total_failures += test_directdraw_lifecycle();
    total_failures += test_invalid_handles();
    
    /* Print summary */
    printf("\n==========================================================\n");
    printf("  TEST SUMMARY\n");
    printf("==========================================================\n");
    
    if (total_failures == 0) {
        printf("[SUCCESS] All tests passed!\n");
        printf("\nConclusion: d3d8thk DirectDraw object management works correctly.\n");
        return 0;
    } else {
        printf("[FAILED] %d test(s) failed\n", total_failures);
        printf("\nConclusion: d3d8thk has issues that need investigation.\n");
        return 1;
    }
}

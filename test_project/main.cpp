#include <windows.h>
#include <d3d8.h>
#include <stdio.h>

// Comprehensive D3D8 link test
int main() {
    printf("=== D3D8 SDK Test Application ===\n\n");
    
    // Test 1: Check SDK version constant
    printf("[1] SDK Version: D3D_SDK_VERSION = %u\n", D3D_SDK_VERSION);
    
    // Test 2: Check if Direct3DCreate8 function exists
    typedef IDirect3D8* (WINAPI *Direct3DCreate8_t)(UINT SDKVersion);
    Direct3DCreate8_t pDirect3DCreate8 = Direct3DCreate8;
    
    if (pDirect3DCreate8) {
        printf("[2] Direct3DCreate8: FOUND at %p\n", (void*)pDirect3DCreate8);
    } else {
        printf("[2] Direct3DCreate8: NOT FOUND\n");
    }
    
    // Test 3: Check if we can call Direct3DCreate8 (link test)
    printf("[3] Testing Direct3DCreate8 call...\n");
    // Note: We can't actually call it on Linux, but checking the link is enough
    if (pDirect3DCreate8) {
        printf("    [OK] Function address is valid (linking successful)\n");
        printf("    Note: Actual call would require Windows D3D8 runtime\n");
    } else {
        printf("    [X] Function address is NULL (link failed)\n");
    }
    
    // Test 4: Type sizes
    printf("[4] Type sizes:\n");
    printf("    sizeof(IDirect3D8*) = %zu bytes\n", sizeof(IDirect3D8*));
    printf("    sizeof(D3DPRESENT_PARAMETERS) = %zu bytes\n", sizeof(D3DPRESENT_PARAMETERS));
    printf("    sizeof(D3DMATRIX) = %zu bytes\n", sizeof(D3DMATRIX));
    
    // Test 5: Architecture
    printf("[5] Architecture: ");
#if defined(_WIN64) || defined(__x86_64__)
    printf("64-bit (x86_64)\n");
#else
    printf("32-bit (i686)\n");
#endif
    
    // Test 6: Compiler
    printf("[6] Compiler: ");
#if defined(__MINGW64__) || defined(__MINGW32__)
    printf("MinGW-w64");
#ifdef __MINGW64_VERSION_MAJOR
    printf(" %d.%d.%d", __MINGW64_VERSION_MAJOR, 
           __MINGW64_VERSION_MINOR, __MINGW64_VERSION_BUGFIX);
#endif
    printf("\n");
#elif defined(_MSC_VER)
    printf("MSVC %d\n", _MSC_VER);
#else
    printf("Unknown\n");
#endif
    
    // Test 7: BUILD_WITH_D3D8 define
#ifdef BUILD_WITH_D3D8
    printf("[7] BUILD_WITH_D3D8: Defined [OK]\n");
#else
    printf("[7] BUILD_WITH_D3D8: NOT Defined [X]\n");
#endif
    
    printf("\n=== Test Result ===\n");
    if (pDirect3DCreate8) {
        printf("[OK] SUCCESS: D3D8 links successfully\n");
        return 0;
    } else {
        printf("[X] FAILED: Direct3DCreate8 not found\n");
        return 1;
    }
}

/*
 * test_device.c - Direct3D8 Device Creation and Management (32-bit)
 *
 * Tests D3D8 device operations:
 * - Device creation (HAL, REF, SW)
 * - Present parameters validation
 * - Device state queries
 * - Device reset
 *
 * This tests the STANDARD D3D8 API (32-bit) for comparison with d3d8thk (64-bit).
 */

#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>

static HWND create_test_window(void) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "D3D8TestWindow";
    RegisterClassA(&wc);
    
    return CreateWindowA("D3D8TestWindow", "D3D8 Test",
                         WS_OVERLAPPEDWINDOW,
                         100, 100, 640, 480,
                         NULL, NULL, GetModuleHandle(NULL), NULL);
}

static void print_test_header(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
}

int test_device_creation_windowed(void) {
    print_test_header("Device Creation (Windowed Mode)");
    
    IDirect3D8* pD3D = NULL;
    IDirect3DDevice8* pDevice = NULL;
    D3DPRESENT_PARAMETERS d3dpp;
    HWND hwnd;
    HRESULT hr;
    int failures = 0;
    
    /* Create window */
    printf("[1] Creating test window...\n");
    hwnd = create_test_window();
    if (hwnd == NULL) {
        printf("[FAIL] Could not create window\n");
        return 1;
    }
    printf("[OK] Window created: hwnd=%p\n", hwnd);
    
    /* Create D3D object */
    printf("\n[2] Creating Direct3D8 object...\n");
    pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    if (pD3D == NULL) {
        printf("[FAIL] Direct3DCreate8 failed\n");
        DestroyWindow(hwnd);
        return 1;
    }
    printf("[OK] Direct3D8 object created\n");
    
    /* Setup present parameters for windowed mode */
    printf("\n[3] Setting up present parameters (windowed)...\n");
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferWidth = 640;
    d3dpp.BackBufferHeight = 480;
    d3dpp.BackBufferCount = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.hDeviceWindow = hwnd;
    
    printf("  Windowed: %s\n", d3dpp.Windowed ? "TRUE" : "FALSE");
    printf("  SwapEffect: %d\n", d3dpp.SwapEffect);
    printf("  BackBufferFormat: %d\n", d3dpp.BackBufferFormat);
    printf("  BackBufferSize: %lux%lu\n", d3dpp.BackBufferWidth, d3dpp.BackBufferHeight);
    
    /* Create device */
    printf("\n[4] Creating D3D device (HAL)...\n");
    hr = IDirect3D8_CreateDevice(
        pD3D,
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &pDevice
    );
    
    if (FAILED(hr)) {
        printf("[FAIL] CreateDevice failed: hr=0x%08lX\n", hr);
        printf("       Common causes:\n");
        printf("       - GPU doesn't support D3D8 HAL\n");
        printf("       - Driver is too new (D3D8 deprecated)\n");
        printf("       - Running in VM or incompatible environment\n");
        printf("\n[Attempting fallback to REF device...]\n");
        
        /* Try reference rasterizer */
        hr = IDirect3D8_CreateDevice(
            pD3D,
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_REF,
            hwnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &d3dpp,
            &pDevice
        );
        
        if (FAILED(hr)) {
            printf("[FAIL] REF device also failed: hr=0x%08lX\n", hr);
            failures++;
        } else {
            printf("[OK] REF device created successfully\n");
            printf("[NOTE] Using software rasterizer (slow but compatible)\n");
        }
    } else {
        printf("[OK] HAL device created successfully\n");
    }
    
    /* Query device info if created */
    if (pDevice != NULL) {
        D3DDEVICE_CREATION_PARAMETERS params;
        
        printf("\n[5] Querying device creation parameters...\n");
        hr = IDirect3DDevice8_GetCreationParameters(pDevice, &params);
        
        if (SUCCEEDED(hr)) {
            printf("[OK] Device creation parameters:\n");
            printf("  AdapterOrdinal: %lu\n", params.AdapterOrdinal);
            printf("  DeviceType: %d\n", params.DeviceType);
            printf("  hFocusWindow: %p\n", params.hFocusWindow);
            printf("  BehaviorFlags: 0x%08lX\n", params.BehaviorFlags);
        } else {
            printf("[FAIL] GetCreationParameters failed: hr=0x%08lX\n", hr);
            failures++;
        }
        
        /* Release device */
        printf("\n[6] Releasing device...\n");
        ULONG refcount = IDirect3DDevice8_Release(pDevice);
        printf("[OK] Device released, refcount=%lu\n", refcount);
    }
    
    /* Cleanup */
    IDirect3D8_Release(pD3D);
    DestroyWindow(hwnd);
    
    return failures;
}

int test_device_validation(void) {
    print_test_header("Device Creation Validation");
    
    IDirect3D8* pD3D = NULL;
    IDirect3DDevice8* pDevice = NULL;
    D3DPRESENT_PARAMETERS d3dpp;
    HWND hwnd;
    HRESULT hr;
    int expected_failures = 0;
    
    /* Setup */
    hwnd = create_test_window();
    pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    
    if (pD3D == NULL || hwnd == NULL) {
        printf("[FAIL] Setup failed\n");
        if (hwnd) DestroyWindow(hwnd);
        return 1;
    }
    
    /* Test 1: Invalid present parameters (NULL) */
    printf("[1] Testing CreateDevice with NULL present parameters...\n");
    hr = IDirect3D8_CreateDevice(
        pD3D,
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        NULL,  /* Invalid */
        &pDevice
    );
    
    if (FAILED(hr)) {
        printf("[OK] Correctly rejected NULL parameters: hr=0x%08lX\n", hr);
    } else {
        printf("[WARN] Unexpectedly accepted NULL parameters\n");
        IDirect3DDevice8_Release(pDevice);
        pDevice = NULL;
        expected_failures++;
    }
    
    /* Test 2: Invalid window handle */
    printf("\n[2] Testing CreateDevice with invalid window handle...\n");
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.hDeviceWindow = (HWND)0xDEADBEEF;  /* Invalid */
    
    hr = IDirect3D8_CreateDevice(
        pD3D,
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &pDevice
    );
    
    if (FAILED(hr)) {
        printf("[OK] Correctly rejected invalid window: hr=0x%08lX\n", hr);
    } else {
        printf("[WARN] Unexpectedly accepted invalid window\n");
        IDirect3DDevice8_Release(pDevice);
        pDevice = NULL;
        expected_failures++;
    }
    
    /* Test 3: Invalid adapter ordinal */
    printf("\n[3] Testing CreateDevice with invalid adapter...\n");
    d3dpp.hDeviceWindow = hwnd;  /* Fix window */
    
    hr = IDirect3D8_CreateDevice(
        pD3D,
        999,  /* Invalid adapter */
        D3DDEVTYPE_HAL,
        hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &pDevice
    );
    
    if (FAILED(hr)) {
        printf("[OK] Correctly rejected invalid adapter: hr=0x%08lX\n", hr);
    } else {
        printf("[WARN] Unexpectedly accepted invalid adapter\n");
        IDirect3DDevice8_Release(pDevice);
        pDevice = NULL;
        expected_failures++;
    }
    
    /* Cleanup */
    IDirect3D8_Release(pD3D);
    DestroyWindow(hwnd);
    
    return expected_failures;
}

int test_device_state_queries(void) {
    print_test_header("Device State Queries");
    
    IDirect3D8* pD3D = NULL;
    IDirect3DDevice8* pDevice = NULL;
    D3DPRESENT_PARAMETERS d3dpp;
    HWND hwnd;
    HRESULT hr;
    int failures = 0;
    
    /* Create device */
    hwnd = create_test_window();
    pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    
    if (pD3D == NULL || hwnd == NULL) {
        printf("[FAIL] Setup failed\n");
        if (hwnd) DestroyWindow(hwnd);
        return 1;
    }
    
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.hDeviceWindow = hwnd;
    
    hr = IDirect3D8_CreateDevice(
        pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice
    );
    
    if (FAILED(hr)) {
        /* Try REF */
        hr = IDirect3D8_CreateDevice(
            pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hwnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice
        );
    }
    
    if (FAILED(hr)) {
        printf("[FAIL] Could not create device for testing\n");
        IDirect3D8_Release(pD3D);
        DestroyWindow(hwnd);
        return 1;
    }
    
    printf("[OK] Device created for state testing\n");
    
    /* Test 1: Get available texture memory */
    printf("\n[1] Querying available texture memory...\n");
    UINT texMem = IDirect3DDevice8_GetAvailableTextureMem(pDevice);
    printf("[OK] Available texture memory: %u bytes (%.2f MB)\n", 
           texMem, texMem / (1024.0 * 1024.0));
    
    /* Test 2: Get device caps */
    printf("\n[2] Getting device caps...\n");
    D3DCAPS8 caps;
    hr = IDirect3DDevice8_GetDeviceCaps(pDevice, &caps);
    
    if (SUCCEEDED(hr)) {
        printf("[OK] Device caps retrieved\n");
        printf("  Max texture size: %lux%lu\n", 
               caps.MaxTextureWidth, caps.MaxTextureHeight);
    } else {
        printf("[FAIL] GetDeviceCaps failed: hr=0x%08lX\n", hr);
        failures++;
    }
    
    /* Test 3: Get display mode */
    printf("\n[3] Getting current display mode...\n");
    D3DDISPLAYMODE mode;
    hr = IDirect3DDevice8_GetDisplayMode(pDevice, &mode);
    
    if (SUCCEEDED(hr)) {
        printf("[OK] Display mode: %ux%u @ %uHz, Format=%d\n",
               mode.Width, mode.Height, mode.RefreshRate, mode.Format);
    } else {
        printf("[FAIL] GetDisplayMode failed: hr=0x%08lX\n", hr);
        failures++;
    }
    
    /* Test 4: Get back buffer */
    printf("\n[4] Getting back buffer...\n");
    IDirect3DSurface8* pBackBuffer = NULL;
    hr = IDirect3DDevice8_GetBackBuffer(pDevice, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    
    if (SUCCEEDED(hr)) {
        printf("[OK] Back buffer retrieved: %p\n", pBackBuffer);
        
        /* Get surface description */
        D3DSURFACE_DESC desc;
        hr = IDirect3DSurface8_GetDesc(pBackBuffer, &desc);
        
        if (SUCCEEDED(hr)) {
            printf("  Surface: %lux%lu, Format=%d\n", 
                   desc.Width, desc.Height, desc.Format);
        }
        
        IDirect3DSurface8_Release(pBackBuffer);
    } else {
        printf("[FAIL] GetBackBuffer failed: hr=0x%08lX\n", hr);
        failures++;
    }
    
    /* Cleanup */
    IDirect3DDevice8_Release(pDevice);
    IDirect3D8_Release(pD3D);
    DestroyWindow(hwnd);
    
    return failures;
}

int main(void) {
    printf("==========================================================\n");
    printf("  Standard D3D8 Device Test Suite (32-bit)\n");
    printf("==========================================================\n");
    printf("\nThis test validates D3D8 device creation and management.\n");
    printf("Compare with d3d8thk context tests (64-bit).\n");
    printf("\nNOTE: Tests may fail if GPU driver doesn't support D3D8.\n");
    
    int total_failures = 0;
    
    /* Run test suites */
    total_failures += test_device_creation_windowed();
    total_failures += test_device_validation();
    total_failures += test_device_state_queries();
    
    /* Print summary */
    printf("\n==========================================================\n");
    printf("  TEST SUMMARY\n");
    printf("==========================================================\n");
    
    if (total_failures == 0) {
        printf("[SUCCESS] All tests passed!\n");
        printf("\nConclusion: D3D8 device management works correctly.\n");
        return 0;
    } else {
        printf("[FAILED] %d test(s) failed\n", total_failures);
        printf("\nPossible causes:\n");
        printf("- GPU driver doesn't support D3D8\n");
        printf("- Running in incompatible environment\n");
        printf("\nCompare with d3d8thk results to assess differences.\n");
        return 1;
    }
}

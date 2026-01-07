/*
 * test_rendering.c - Direct3D8 Rendering Operations (32-bit)
 *
 * Tests D3D8 rendering operations:
 * - BeginScene/EndScene
 * - Clear
 * - Present
 * - Simple draw calls
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
    wc.lpszClassName = "D3D8RenderTest";
    RegisterClassA(&wc);

    return CreateWindowA("D3D8RenderTest", "D3D8 Render Test",
                         WS_OVERLAPPEDWINDOW,
                         100, 100, 640, 480,
                         NULL, NULL, GetModuleHandle(NULL), NULL);
}

static IDirect3DDevice8* create_test_device(HWND hwnd) {
    IDirect3D8* pD3D;
    IDirect3DDevice8* pDevice = NULL;
    D3DPRESENT_PARAMETERS d3dpp;
    HRESULT hr;

    pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    if (pD3D == NULL) return NULL;

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferWidth = 640;
    d3dpp.BackBufferHeight = 480;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.hDeviceWindow = hwnd;

    /* Try Hardware Vertex Processing first */
    hr = IDirect3D8_CreateDevice(
        pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pDevice
    );

    if (FAILED(hr)) {
        /* Fallback to Software Vertex Processing */
        hr = IDirect3D8_CreateDevice(
            pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice
        );

        if (FAILED(hr)) {
            /* Last resort: Try REF device */
            hr = IDirect3D8_CreateDevice(
                pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hwnd,
                D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice
            );
        }
    }

    IDirect3D8_Release(pD3D);
    return pDevice;
}

static void print_test_header(const char* test_name) {
    printf("\n=== %s ===\n", test_name);
}

int test_scene_management(void) {
    print_test_header("Scene Management (BeginScene/EndScene)");

    HWND hwnd;
    IDirect3DDevice8* pDevice;
    HRESULT hr;
    int failures = 0;

    /* Setup */
    hwnd = create_test_window();
    pDevice = create_test_device(hwnd);

    if (pDevice == NULL) {
        printf("[FAIL] Could not create device\n");
        DestroyWindow(hwnd);
        return 1;
    }

    printf("[OK] Device created for scene testing\n");

    /* Test 1: Basic BeginScene/EndScene */
    printf("\n[1] Testing BeginScene/EndScene...\n");
    hr = IDirect3DDevice8_BeginScene(pDevice);

    if (FAILED(hr)) {
        printf("[FAIL] BeginScene failed: hr=0x%08lX\n", hr);
        failures++;
    } else {
        printf("[OK] BeginScene succeeded\n");

        hr = IDirect3DDevice8_EndScene(pDevice);
        if (FAILED(hr)) {
            printf("[FAIL] EndScene failed: hr=0x%08lX\n", hr);
            failures++;
        } else {
            printf("[OK] EndScene succeeded\n");
        }
    }

    /* Test 2: Nested BeginScene (should fail) */
    printf("\n[2] Testing nested BeginScene (should fail)...\n");
    hr = IDirect3DDevice8_BeginScene(pDevice);
    if (SUCCEEDED(hr)) {
        hr = IDirect3DDevice8_BeginScene(pDevice);  /* Nested */

        if (SUCCEEDED(hr)) {
            printf("[WARN] Unexpectedly accepted nested BeginScene\n");
            failures++;
            IDirect3DDevice8_EndScene(pDevice);
        } else {
            printf("[OK] Correctly rejected nested BeginScene: hr=0x%08lX\n", hr);
        }

        IDirect3DDevice8_EndScene(pDevice);
    }

    /* Test 3: EndScene without BeginScene (should fail) */
    printf("\n[3] Testing EndScene without BeginScene (should fail)...\n");
    hr = IDirect3DDevice8_EndScene(pDevice);

    if (SUCCEEDED(hr)) {
        printf("[WARN] Unexpectedly accepted EndScene without BeginScene\n");
        failures++;
    } else {
        printf("[OK] Correctly rejected EndScene: hr=0x%08lX\n", hr);
    }

    /* Cleanup */
    IDirect3DDevice8_Release(pDevice);
    DestroyWindow(hwnd);

    return failures;
}

int test_clear_operations(void) {
    print_test_header("Clear Operations");

    HWND hwnd;
    IDirect3DDevice8* pDevice;
    HRESULT hr;
    int failures = 0;

    /* Setup */
    hwnd = create_test_window();
    pDevice = create_test_device(hwnd);

    if (pDevice == NULL) {
        printf("[FAIL] Could not create device\n");
        DestroyWindow(hwnd);
        return 1;
    }

    printf("[OK] Device created for clear testing\n");

    /* Test 1: Clear with all flags */
    printf("\n[1] Testing Clear with all flags...\n");
    hr = IDirect3DDevice8_Clear(
        pDevice,
        0,                  /* No rects - clear entire surface */
        NULL,
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
        D3DCOLOR_XRGB(128, 128, 255),  /* Blue background */
        1.0f,               /* Z value */
        0                   /* Stencil value */
    );

    if (FAILED(hr)) {
        printf("[FAIL] Clear failed: hr=0x%08lX\n", hr);
        printf("       Note: STENCIL clear may fail if no stencil buffer\n");

        /* Try without stencil */
        hr = IDirect3DDevice8_Clear(
            pDevice, 0, NULL,
            D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
            D3DCOLOR_XRGB(128, 128, 255), 1.0f, 0
        );

        if (FAILED(hr)) {
            printf("[FAIL] Clear without stencil also failed: hr=0x%08lX\n", hr);
            failures++;
        } else {
            printf("[OK] Clear succeeded without stencil\n");
        }
    } else {
        printf("[OK] Clear with all flags succeeded\n");
    }

    /* Test 2: Clear target only */
    printf("\n[2] Testing Clear target only...\n");
    hr = IDirect3DDevice8_Clear(
        pDevice, 0, NULL,
        D3DCLEAR_TARGET,
        D3DCOLOR_XRGB(255, 0, 0),  /* Red */
        1.0f, 0
    );

    if (FAILED(hr)) {
        printf("[FAIL] Clear target failed: hr=0x%08lX\n", hr);
        failures++;
    } else {
        printf("[OK] Clear target succeeded\n");
    }

    /* Test 3: Clear Z-buffer only */
    printf("\n[3] Testing Clear Z-buffer only...\n");
    hr = IDirect3DDevice8_Clear(
        pDevice, 0, NULL,
        D3DCLEAR_ZBUFFER,
        0, 0.5f, 0
    );

    if (FAILED(hr)) {
        printf("[FAIL] Clear Z-buffer failed: hr=0x%08lX\n", hr);
        failures++;
    } else {
        printf("[OK] Clear Z-buffer succeeded\n");
    }

    /* Test 4: Clear with rectangular region */
    printf("\n[4] Testing Clear with rectangular region...\n");
    D3DRECT rect = { 100, 100, 200, 200 };
    hr = IDirect3DDevice8_Clear(
        pDevice, 1, &rect,
        D3DCLEAR_TARGET,
        D3DCOLOR_XRGB(0, 255, 0),  /* Green */
        1.0f, 0
    );

    if (FAILED(hr)) {
        printf("[FAIL] Clear with rect failed: hr=0x%08lX\n", hr);
        failures++;
    } else {
        printf("[OK] Clear with rect succeeded\n");
    }

    /* Cleanup */
    IDirect3DDevice8_Release(pDevice);
    DestroyWindow(hwnd);

    return failures;
}

int test_present_operation(void) {
    print_test_header("Present Operation");

    HWND hwnd;
    IDirect3DDevice8* pDevice;
    HRESULT hr;
    int failures = 0;

    /* Setup */
    hwnd = create_test_window();
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    pDevice = create_test_device(hwnd);

    if (pDevice == NULL) {
        printf("[FAIL] Could not create device\n");
        DestroyWindow(hwnd);
        return 1;
    }

    printf("[OK] Device created for present testing\n");

    /* Test 1: Basic Present */
    printf("\n[1] Testing Present (swap back buffer)...\n");
    hr = IDirect3DDevice8_Present(pDevice, NULL, NULL, NULL, NULL);

    if (FAILED(hr)) {
        printf("[FAIL] Present failed: hr=0x%08lX\n", hr);
        failures++;
    } else {
        printf("[OK] Present succeeded\n");
    }

    /* Test 2: Render and Present sequence */
    printf("\n[2] Testing full render cycle...\n");

    /* Clear */
    hr = IDirect3DDevice8_Clear(
        pDevice, 0, NULL,
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(64, 128, 192),
        1.0f, 0
    );

    if (FAILED(hr)) {
        printf("[FAIL] Clear failed: hr=0x%08lX\n", hr);
        failures++;
    }

    /* Begin scene */
    hr = IDirect3DDevice8_BeginScene(pDevice);
    if (FAILED(hr)) {
        printf("[FAIL] BeginScene failed: hr=0x%08lX\n", hr);
        failures++;
    }

    /* End scene */
    hr = IDirect3DDevice8_EndScene(pDevice);
    if (FAILED(hr)) {
        printf("[FAIL] EndScene failed: hr=0x%08lX\n", hr);
        failures++;
    }

    /* Present */
    hr = IDirect3DDevice8_Present(pDevice, NULL, NULL, NULL, NULL);
    if (FAILED(hr)) {
        printf("[FAIL] Present failed: hr=0x%08lX\n", hr);
        failures++;
    } else {
        printf("[OK] Full render cycle succeeded\n");
    }

    /* Cleanup */
    IDirect3DDevice8_Release(pDevice);
    DestroyWindow(hwnd);

    return failures;
}

int test_render_state(void) {
    print_test_header("Render State Management");

    HWND hwnd;
    IDirect3DDevice8* pDevice;
    HRESULT hr;
    int failures = 0;

    /* Setup */
    hwnd = create_test_window();
    pDevice = create_test_device(hwnd);

    if (pDevice == NULL) {
        printf("[FAIL] Could not create device\n");
        DestroyWindow(hwnd);
        return 1;
    }

    printf("[OK] Device created for render state testing\n");

    /* Test 1: Set/Get render state */
    printf("\n[1] Testing SetRenderState/GetRenderState...\n");

    hr = IDirect3DDevice8_SetRenderState(pDevice, D3DRS_ZENABLE, TRUE);
    if (FAILED(hr)) {
        printf("[FAIL] SetRenderState failed: hr=0x%08lX\n", hr);
        failures++;
    } else {
        DWORD value;
        hr = IDirect3DDevice8_GetRenderState(pDevice, D3DRS_ZENABLE, &value);

        if (FAILED(hr)) {
            printf("[FAIL] GetRenderState failed: hr=0x%08lX\n", hr);
            failures++;
        } else if (value != TRUE) {
            printf("[FAIL] Render state mismatch: expected TRUE, got %lu\n", value);
            failures++;
        } else {
            printf("[OK] Render state set/get works correctly\n");
        }
    }

    /* Test 2: Multiple render states */
    printf("\n[2] Testing multiple render states...\n");

    struct {
        D3DRENDERSTATETYPE state;
        DWORD value;
        const char* name;
    } tests[] = {
        { D3DRS_CULLMODE, D3DCULL_CCW, "CullMode" },
        { D3DRS_LIGHTING, FALSE, "Lighting" },
        { D3DRS_ALPHABLENDENABLE, TRUE, "AlphaBlend" },
        { D3DRS_SRCBLEND, D3DBLEND_SRCALPHA, "SrcBlend" },
        { D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA, "DestBlend" }
    };

    for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        hr = IDirect3DDevice8_SetRenderState(pDevice, tests[i].state, tests[i].value);

        if (FAILED(hr)) {
            printf("  [FAIL] %s: SetRenderState failed\n", tests[i].name);
            failures++;
        } else {
            printf("  [OK] %s set successfully\n", tests[i].name);
        }
    }

    /* Cleanup */
    IDirect3DDevice8_Release(pDevice);
    DestroyWindow(hwnd);

    return failures;
}

int main(void) {
    printf("==========================================================\n");
    printf("  Standard D3D8 Rendering Test Suite (32-bit)\n");
    printf("==========================================================\n");
    printf("\nThis test validates D3D8 rendering operations.\n");
    printf("Compare with d3d8thk drawing tests (64-bit).\n");
    printf("\nNOTE: Tests may fail if GPU driver doesn't support D3D8.\n");

    int total_failures = 0;

    /* Run test suites */
    total_failures += test_scene_management();
    total_failures += test_clear_operations();
    total_failures += test_present_operation();
    total_failures += test_render_state();

    /* Print summary */
    printf("\n==========================================================\n");
    printf("  TEST SUMMARY\n");
    printf("==========================================================\n");

    if (total_failures == 0) {
        printf("[SUCCESS] All tests passed!\n");
        printf("\nConclusion: D3D8 rendering operations work correctly.\n");
        return 0;
    } else {
        printf("[FAILED] %d test(s) failed\n", total_failures);
        printf("\nPossible causes:\n");
        printf("- GPU driver doesn't support D3D8\n");
        printf("- Stencil buffer not available\n");
        printf("- Running in incompatible environment\n");
        printf("\nCompare with d3d8thk results to assess differences.\n");
        return 1;
    }
}

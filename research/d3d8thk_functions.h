/*
 * d3d8thk.h - Windows 64-bit D3D8 Thunk Layer Functions
 * 
 * These are low-level kernel functions exported by d3d8thk.dll (64-bit Windows)
 * Provided by MinGW-w64 in libd3d8thk.a
 * 
 * These functions are NOT the public D3D8 API. They are internal kernel thunks
 * that a full d3d8.dll implementation would call.
 */

#ifndef _D3D8THK_H
#define _D3D8THK_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * D3D Context Management (3 functions)
 * Used to create/destroy Direct3D rendering contexts
 */

// Create a new D3D rendering context
BOOL WINAPI OsThunkD3dContextCreate(HANDLE hDirectDrawLocal, HANDLE hSurfColor, 
                                     HANDLE hSurfZ, D3DNTHAL_CONTEXTCREATEI *pContextCreateData);

// Destroy a D3D rendering context
DWORD WINAPI OsThunkD3dContextDestroy(LPD3DNTHAL_CONTEXTDESTROYDATA pContextDestroyData);

// Destroy all D3D rendering contexts
DWORD WINAPI OsThunkD3dContextDestroyAll(HANDLE hDirectDrawLocal);

/*
 * D3D Rendering (2 functions)
 * Main rendering and validation functions
 */

// Main rendering function - executes drawing commands
// This is the critical function that all Draw* calls eventually use
DWORD WINAPI OsThunkD3dDrawPrimitives2(HANDLE hCmdBuf, HANDLE hVBuf, 
                                        LPD3DNTHAL_DRAWPRIMITIVES2DATA pData, 
                                        FLATPTR *pfpVidMemCmd, DWORD *pdwSizeCmd, 
                                        FLATPTR *pfpVidMemVtx, DWORD *pdwSizeVtx);

// Validate texture stage states
DWORD WINAPI OsThunkD3dValidateTextureStageState(LPD3DNTHAL_VALIDATETEXTURESTAGESTATEDATA pData);

/*
 * DirectDraw Surface Management (23 functions)
 * Used for surfaces, textures, render targets, etc.
 */

BOOL WINAPI OsThunkDdCreateSurface(HANDLE hDirectDrawLocal, HANDLE *hSurface, 
                                    DDSURFACEDESC *puSurfaceDescription, 
                                    DD_SURFACE_GLOBAL *puSurfaceGlobalData,
                                    DD_SURFACE_LOCAL *puSurfaceLocalData, 
                                    DD_SURFACE_MORE *puSurfaceMoreData, 
                                    DD_CREATESURFACEDATA *puCreateSurfaceData,
                                    HANDLE *puhSurface);

BOOL WINAPI OsThunkDdCreateSurfaceEx(HANDLE hDirectDrawLocal, HANDLE hSurface, DWORD dwSurfaceHandle);

BOOL WINAPI OsThunkDdCreateSurfaceObject(HANDLE hDirectDrawLocal, HANDLE hSurface, 
                                          PDD_SURFACE_LOCAL puSurfaceLocal, 
                                          PDD_SURFACE_MORE puSurfaceMore, 
                                          PDD_SURFACE_GLOBAL puSurfaceGlobal, BOOL bComplete);

BOOL WINAPI OsThunkDdDestroySurface(HANDLE hSurface, BOOL bRealDestroy);

BOOL WINAPI OsThunkDdDeleteSurfaceObject(HANDLE hSurfaceObject);

// Lock surface for CPU access
DWORD WINAPI OsThunkDdLock(HANDLE hSurface, PDD_LOCKDATA puLockData, HANDLE hDC);

// Unlock surface after CPU access
DWORD WINAPI OsThunkDdUnlock(HANDLE hSurface, PDD_UNLOCKDATA puUnlockData);

// D3D-specific lock/unlock
DWORD WINAPI OsThunkDdLockD3D(HANDLE hSurface, PDD_LOCKDATA puLockData);
DWORD WINAPI OsThunkDdUnlockD3D(HANDLE hSurface, PDD_UNLOCKDATA puUnlockData);

// Blit operations
DWORD WINAPI OsThunkDdBlt(HANDLE hSurfaceDest, HANDLE hSurfaceSrc, PDD_BLTDATA puBltData);
DWORD WINAPI OsThunkDdAlphaBlt(HANDLE hSurfaceDest, HANDLE hSurfaceSrc, PDD_BLTDATA puBltData);

// Surface attachment
BOOL WINAPI OsThunkDdAttachSurface(HANDLE hSurfaceFrom, HANDLE hSurfaceTo);
BOOL WINAPI OsThunkDdUnattachSurface(HANDLE hSurface, HANDLE hSurfaceAttached);

// Color key
DWORD WINAPI OsThunkDdSetColorKey(HANDLE hSurface, PDD_SETCOLORKEYDATA puSetColorKeyData);

// Device context operations
HDC WINAPI OsThunkDdGetDC(HANDLE hSurface, PALETTEENTRY *puColorTable);
BOOL WINAPI OsThunkDdReleaseDC(HANDLE hSurface);

// Status queries
DWORD WINAPI OsThunkDdGetBltStatus(HANDLE hSurface, PDD_GETBLTSTATUSDATA puGetBltStatusData);
DWORD WINAPI OsThunkDdGetFlipStatus(HANDLE hSurface, PDD_GETFLIPSTATUSDATA puGetFlipStatusData);

/*
 * DirectDraw Object Management (5 functions)
 */

BOOL WINAPI OsThunkDdCreateDirectDrawObject(HDC hdc);
BOOL WINAPI OsThunkDdQueryDirectDrawObject(HANDLE hDirectDrawLocal, 
                                             DD_HALINFO *pHalInfo, 
                                             DWORD *pCallBackFlags, 
                                             LPD3DNTHAL_CALLBACKS puD3dCallbacks, 
                                             LPD3DNTHAL_GLOBALDRIVERDATA puD3dDriverData, 
                                             PDD_D3DBUFCALLBACKS puD3dBufferCallbacks,
                                             LPDDSURFACEDESC puD3dTextureFormats, 
                                             DWORD *puNumHeaps, 
                                             VIDEOMEMORY *puvmList, 
                                             DWORD *puNumFourCC, 
                                             DWORD *puFourCC);

BOOL WINAPI OsThunkDdDeleteDirectDrawObject(HANDLE hDirectDrawLocal);
BOOL WINAPI OsThunkDdReenableDirectDrawObject(HANDLE hDirectDrawLocal, BOOL *pubNewMode);
DWORD WINAPI OsThunkDdGetDriverState(PDD_GETDRIVERSTATEDATA puGetDriverStateData);

/*
 * Display Management (8 functions)
 */

DWORD WINAPI OsThunkDdFlip(HANDLE hSurfaceCurrent, HANDLE hSurfaceTarget, 
                             HANDLE hSurfaceCurrentLeft, HANDLE hSurfaceTargetLeft,
                             PDD_FLIPDATA puFlipData);

DWORD WINAPI OsThunkDdFlipToGDISurface(HANDLE hDirectDraw, PDD_FLIPTOGDISURFACEDATA puFlipToGDISurfaceData);

DWORD WINAPI OsThunkDdSetExclusiveMode(HANDLE hDirectDraw, PDD_SETEXCLUSIVEMODEDATA puSetExclusiveModeData);

DWORD WINAPI OsThunkDdSetGammaRamp(HANDLE hDirectDraw, HDC hdc, LPVOID lpGammaRamp);

DWORD WINAPI OsThunkDdWaitForVerticalBlank(HANDLE hDirectDraw, PDD_WAITFORVERTICALBLANKDATA puWaitForVerticalBlankData);

DWORD WINAPI OsThunkDdGetScanLine(HANDLE hDirectDraw, PDD_GETSCANLINEDATA puGetScanLineData);

BOOL WINAPI OsThunkDdCanCreateSurface(HANDLE hDirectDraw, PDD_CANCREATESURFACEDATA puCanCreateSurfaceData);

DWORD WINAPI OsThunkDdGetAvailDriverMemory(HANDLE hDirectDraw, PDD_GETAVAILDRIVERMEMORYDATA puGetAvailDriverMemoryData);

/*
 * D3D Buffer Management (3 functions)
 * For vertex/index buffers
 */

DWORD WINAPI OsThunkDdCanCreateD3DBuffer(HANDLE hDirectDraw, PDD_CANCREATESURFACEDATA puCanCreateSurfaceData);

DWORD WINAPI OsThunkDdCreateD3DBuffer(HANDLE hDirectDraw, HANDLE *hSurface, 
                                       DDSURFACEDESC *puSurfaceDescription, 
                                       DD_SURFACE_GLOBAL *puSurfaceGlobalData, 
                                       DD_SURFACE_LOCAL *puSurfaceLocalData,
                                       DD_SURFACE_MORE *puSurfaceMoreData, 
                                       DD_CREATESURFACEDATA *puCreateSurfaceData, 
                                       HANDLE *puhSurface);

DWORD WINAPI OsThunkDdDestroyD3DBuffer(HANDLE hSurface);

/*
 * Motion Compensation (10 functions)
 * Video acceleration - rarely used for D3D8 applications
 */

DWORD WINAPI OsThunkDdCreateMoComp(HANDLE hDirectDraw, PDD_CREATEMOCOMPDATA puCreateMoCompData);
DWORD WINAPI OsThunkDdDestroyMoComp(HANDLE hMoComp, PDD_DESTROYMOCOMPDATA puDestroyMoCompData);
DWORD WINAPI OsThunkDdBeginMoCompFrame(HANDLE hMoComp, PDD_BEGINMOCOMPFRAMEDATA puBeginFrameData);
DWORD WINAPI OsThunkDdEndMoCompFrame(HANDLE hMoComp, PDD_ENDMOCOMPFRAMEDATA puEndFrameData);
DWORD WINAPI OsThunkDdRenderMoComp(HANDLE hMoComp, PDD_RENDERMOCOMPDATA puRenderMoCompData);
DWORD WINAPI OsThunkDdQueryMoCompStatus(HANDLE hMoComp, PDD_QUERYMOCOMPSTATUSDATA puQueryMoCompStatusData);
DWORD WINAPI OsThunkDdGetMoCompGuids(HANDLE hDirectDraw, PDD_GETMOCOMPGUIDSDATA puGetMoCompGuidsData);
DWORD WINAPI OsThunkDdGetMoCompFormats(HANDLE hDirectDraw, PDD_GETMOCOMPFORMATSDATA puGetMoCompFormatsData);
DWORD WINAPI OsThunkDdGetMoCompBuffInfo(HANDLE hDirectDraw, PDD_GETMOCOMPCOMPBUFFDATA puGetBuffData);
DWORD WINAPI OsThunkDdGetInternalMoCompInfo(HANDLE hDirectDraw, PDD_GETINTERNALMOCOMPDATA puGetInternalData);

/*
 * Miscellaneous (6 functions)
 */

HANDLE WINAPI OsThunkDdGetDxHandle(HANDLE hDirectDraw, HANDLE hSurface, BOOL bRelease);
DWORD WINAPI OsThunkDdSetOverlayPosition(HANDLE hSurfaceSource, HANDLE hSurfaceDestination, 
                                           PDD_SETOVERLAYPOSITIONDATA puSetOverlayPositionData);
DWORD WINAPI OsThunkDdUpdateOverlay(HANDLE hSurfaceDestination, HANDLE hSurfaceSource, 
                                     PDD_UPDATEOVERLAYDATA puUpdateOverlayData);
DWORD WINAPI OsThunkDdColorControl(HANDLE hSurface, PDD_COLORCONTROLDATA puColorControlData);
BOOL WINAPI OsThunkDdResetVisrgn(HANDLE hSurface, HWND hwnd);
DWORD WINAPI OsThunkDdGetDriverInfo(HANDLE hDirectDrawLocal, PDD_GETDRIVERINFODATA puGetDriverInfoData);

#ifdef __cplusplus
}
#endif

#endif /* _D3D8THK_H */

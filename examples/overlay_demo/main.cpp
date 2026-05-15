/*
 * overlay_demo — draw something every frame via the D3D9 EndScene hook.
 *
 * Proves the render-injection path end-to-end with ZERO external deps:
 * on each presented frame it calls IDirect3DDevice9::Clear (vtable idx
 * 43) on a small screen rect, painting a solid green bar in the
 * top-left. Real mods would instead init ImGui (include <d3d9.h>) here.
 */

#include <nfsmw_sdk/nfsmw_sdk.h>
#include <nfsmw_sdk/d3d9_hooks.h>

/* Minimal D3D9 shapes — avoids pulling <d3d9.h>/d3d9.lib for the demo. */
struct D3DRECT { long x1, y1, x2, y2; };
#define D3DCLEAR_TARGET 0x00000001u
#define NFSMW_D3D9_VT_CLEAR 43   /* engine vt[0xAC] = 43*4 */

typedef long (NFSMW_STDCALL *ClearFn)(void *self, unsigned long Count,
    const D3DRECT *pRects, unsigned long Flags,
    unsigned long Color, float Z, unsigned long Stencil);

static void on_frame(void *device) {
    void **vtbl = *(void***)device;
    ClearFn Clear = (ClearFn)vtbl[NFSMW_D3D9_VT_CLEAR];
    D3DRECT r = { 20, 20, 260, 70 };
    /* ARGB: opaque green — visible proof the hook runs each frame */
    Clear(device, 1, &r, D3DCLEAR_TARGET, 0xFF20C020u, 0.0f, 0);
}

NFSMW_PLUGIN_DECLARE("Overlay Demo", "1.0.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    nfsmw_d3d9_install(on_frame, nullptr);
    OutputDebugStringA("[overlay_demo] D3D9 overlay armed\n");
    return NFSMW_OK;
}

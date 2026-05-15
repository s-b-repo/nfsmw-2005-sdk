/*
 * nfsmw_sdk/d3d9_hooks.h — hook the Direct3D9 render loop (opt-in).
 *
 * NFSMW renders through D3D9. Hooking IDirect3DDevice9::EndScene is the
 * gateway to overlays, ImGui, custom HUD, screenshots, debug draws — the
 * single biggest "mod anything visual" unlock.
 *
 * This header is NOT pulled in by the umbrella. Include it explicitly.
 * It is self-contained: the device is treated as an opaque COM object
 * and hooked by its fixed vtable indices, so it needs NO d3d9.lib and
 * NO DirectX headers (stays kernel32-only like the core SDK). Your
 * callback receives the raw IDirect3DDevice9* — cast it to the real
 * type in your own code if you include <d3d9.h> for drawing/ImGui.
 *
 *   #include <nfsmw_sdk/d3d9_hooks.h>
 *
 *   static void on_frame(void* dev) {  // IDirect3DDevice9*
 *       // draw your overlay here (every presented frame)
 *   }
 *
 *   NFSMW_PLUGIN_MAIN() {
 *       nfsmw_d3d9_install(on_frame, 0, 0);
 *       return NFSMW_OK;
 *   }
 *
 * C++: nfsmw::on_endscene([](void* dev){ ... });
 *
 * Mechanism: spin until the engine's IDirect3DDevice9* global is live,
 * then VMT-hook its EndScene slot (and optionally Reset for window
 * resize). The device vtable is COM-standard; indices are fixed.
 */

#ifndef NFSMW_SDK_D3D9_HOOKS_H
#define NFSMW_SDK_D3D9_HOOKS_H

#include "platform.h"
#include "hooks.h"

/* Engine global: IDirect3DDevice9** (RE DB: idirect3d9device_ptr). */
#ifndef NFSMW_D3D9_DEVICE_PTR
#define NFSMW_D3D9_DEVICE_PTR 0x00982BDCu
#endif

/* Fixed IDirect3DDevice9 vtable slot indices (x86, COM standard).
 * Cross-checked against the engine's own byte offsets in the RE DB:
 *   EndScene = idx 42 (engine vt[0xA8] = 42*4)
 *   Present  = idx 17 (engine vt[0x44] = 17*4)
 *   Reset    = idx 16 */
#define NFSMW_D3D9_VT_RESET     16
#define NFSMW_D3D9_VT_PRESENT   17
#define NFSMW_D3D9_VT_ENDSCENE  42

#ifndef NFSMW_D3D9_WAIT_POLL_MS
#define NFSMW_D3D9_WAIT_POLL_MS 50
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*nfsmw_d3d9_endscene_cb)(void *device /*IDirect3DDevice9* */);
typedef void (*nfsmw_d3d9_reset_cb)(void *device, int post /*0=pre,1=post*/);

/* HRESULT __stdcall EndScene(IDirect3DDevice9*) */
typedef long (NFSMW_STDCALL *nfsmw__endscene_fn)(void *self);
typedef long (NFSMW_STDCALL *nfsmw__reset_fn)(void *self, void *pp);

static nfsmw_d3d9_endscene_cb nfsmw__es_cb     = 0;
static nfsmw_d3d9_reset_cb    nfsmw__rs_cb     = 0;
static nfsmw__endscene_fn     nfsmw__es_orig   = 0;
static nfsmw__reset_fn        nfsmw__rs_orig   = 0;
static volatile LONG          nfsmw__d3d9_done = 0;

static long NFSMW_STDCALL nfsmw__endscene_detour(void *self) {
    if (nfsmw__es_cb) nfsmw__es_cb(self);
    return nfsmw__es_orig ? nfsmw__es_orig(self) : 0;
}

static long NFSMW_STDCALL nfsmw__reset_detour(void *self, void *pp) {
    if (nfsmw__rs_cb) nfsmw__rs_cb(self, 0);            /* pre  */
    long hr = nfsmw__rs_orig ? nfsmw__rs_orig(self, pp) : 0;
    if (nfsmw__rs_cb) nfsmw__rs_cb(self, 1);            /* post */
    return hr;
}

static DWORD WINAPI nfsmw__d3d9_worker(LPVOID lpv) {
    (void)lpv;
    void **dev = 0;
    for (;;) {
        void *d = *(void**)NFSMW_D3D9_DEVICE_PTR;
        if (d) { dev = (void**)d; break; }
        Sleep(NFSMW_D3D9_WAIT_POLL_MS);
    }
    void **vtbl = *(void***)dev;   /* IDirect3DDevice9 vtable */

    nfsmw__es_orig = (nfsmw__endscene_fn)vtbl[NFSMW_D3D9_VT_ENDSCENE];
    nfsmw_vtable_hook((uintptr_t)vtbl, NFSMW_D3D9_VT_ENDSCENE,
                      (void*)&nfsmw__endscene_detour,
                      (void**)&nfsmw__es_orig);

    if (nfsmw__rs_cb) {
        nfsmw__rs_orig = (nfsmw__reset_fn)vtbl[NFSMW_D3D9_VT_RESET];
        nfsmw_vtable_hook((uintptr_t)vtbl, NFSMW_D3D9_VT_RESET,
                          (void*)&nfsmw__reset_detour,
                          (void**)&nfsmw__rs_orig);
    }
    InterlockedExchange(&nfsmw__d3d9_done, 1);
    OutputDebugStringA("[nfsmw_sdk] D3D9 EndScene hooked\n");
    return 0;
}

/* Install the render-loop hooks. endscene runs every presented frame.
 * reset may be NULL if you hold no D3D resources. Idempotent. */
static inline void nfsmw_d3d9_install(nfsmw_d3d9_endscene_cb endscene,
                                      nfsmw_d3d9_reset_cb reset_cb) {
    static volatile LONG started = 0;
    nfsmw__es_cb = endscene;
    nfsmw__rs_cb = reset_cb;
    if (InterlockedCompareExchange(&started, 1, 0) != 0) return;
    HANDLE h = CreateThread(NULL, 0, nfsmw__d3d9_worker, NULL, 0, NULL);
    if (h) CloseHandle(h);
}

#ifdef __cplusplus
} /* extern "C" */

namespace nfsmw {
template <typename F>
inline void on_endscene(F &&fn) {
    static F held = (F&&)fn;
    nfsmw_d3d9_install([](void *d){ held(d); }, nullptr);
}
}  // namespace nfsmw
#endif

#endif /* NFSMW_SDK_D3D9_HOOKS_H */

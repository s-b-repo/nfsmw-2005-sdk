/*
 * nfsmw_sdk/hotkeys.h — edge-triggered keyboard hotkeys for mods.
 *
 * Optional. Including this header pulls in a user32 dependency
 * (GetAsyncKeyState) — the core SDK stays kernel32-only, so only mods
 * that want hotkeys pay for it.
 *
 * Uses background polling rather than the game's own input binding table
 * (PollAllGameActionBindingsPerFrame @ 0x6349B0): independent of game
 * state, works in menus/loading, and survives patched executables.
 *
 *   #include <nfsmw_sdk/hotkeys.h>
 *
 *   static void toggle_nos(void* u) { (void)u;
 *       bool* f = (bool*)NFSMW_GLOBAL_Tweak_InfiniteNOS;
 *       *f = !*f;
 *   }
 *
 *   NFSMW_PLUGIN_MAIN() {
 *       nfsmw_hotkey_register(VK_F5, toggle_nos, 0);
 *       nfsmw_hotkeys_start();          // spawns the poll thread
 *       return NFSMW_OK;
 *   }
 *
 * C++: nfsmw::on_hotkey(VK_F6, []{ ... });
 */

#ifndef NFSMW_SDK_HOTKEYS_H
#define NFSMW_SDK_HOTKEYS_H

#include "platform.h"

#if defined(_MSC_VER)
#  pragma comment(lib, "user32.lib")
#endif

#ifndef NFSMW_HOTKEYS_MAX
#define NFSMW_HOTKEYS_MAX 32
#endif
#ifndef NFSMW_HOTKEYS_POLL_MS
#define NFSMW_HOTKEYS_POLL_MS 30
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*nfsmw_hotkey_cb)(void *user);

typedef struct {
    int             vk;
    nfsmw_hotkey_cb cb;
    void           *user;
    int             was_down;
} nfsmw__hotkey_slot;

static nfsmw__hotkey_slot nfsmw__hk[NFSMW_HOTKEYS_MAX];
static volatile LONG      nfsmw__hk_count   = 0;
static volatile LONG      nfsmw__hk_running = 0;

/* Register a key (VK_* code). Returns 1 on success, 0 if the table is
 * full. Safe to call before nfsmw_hotkeys_start(). */
static inline int nfsmw_hotkey_register(int vk, nfsmw_hotkey_cb cb, void *user) {
    LONG idx = InterlockedIncrement(&nfsmw__hk_count) - 1;
    if (idx >= NFSMW_HOTKEYS_MAX || !cb) {
        InterlockedDecrement(&nfsmw__hk_count);
        return 0;
    }
    nfsmw__hk[idx].vk       = vk;
    nfsmw__hk[idx].cb       = cb;
    nfsmw__hk[idx].user     = user;
    nfsmw__hk[idx].was_down = 0;
    return 1;
}

static DWORD WINAPI nfsmw__hotkey_pump(LPVOID lpv) {
    (void)lpv;
    for (;;) {
        LONG n = nfsmw__hk_count;
        for (LONG i = 0; i < n; ++i) {
            int down = (GetAsyncKeyState(nfsmw__hk[i].vk) & 0x8000) != 0;
            if (down && !nfsmw__hk[i].was_down)
                nfsmw__hk[i].cb(nfsmw__hk[i].user);   /* edge: on press */
            nfsmw__hk[i].was_down = down;
        }
        Sleep(NFSMW_HOTKEYS_POLL_MS);
    }
    return 0;
}

/* Start the poll thread once (idempotent). */
static inline void nfsmw_hotkeys_start(void) {
    if (InterlockedCompareExchange(&nfsmw__hk_running, 1, 0) != 0) return;
    HANDLE h = CreateThread(NULL, 0, nfsmw__hotkey_pump, NULL, 0, NULL);
    if (h) CloseHandle(h);
}

#ifdef __cplusplus
} /* extern "C" */

#include <utility>
namespace nfsmw {

/* Register a no-capture callable (function ptr or stateless lambda). */
template <typename F>
inline bool on_hotkey(int vk, F &&fn) {
    static F held = std::forward<F>(fn);   /* keep it alive */
    bool ok = nfsmw_hotkey_register(vk,
        [](void*) { held(); }, nullptr) != 0;
    nfsmw_hotkeys_start();
    return ok;
}

}  // namespace nfsmw
#endif

#endif /* NFSMW_SDK_HOTKEYS_H */

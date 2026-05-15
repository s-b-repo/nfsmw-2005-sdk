/*
 * nfsmw_sdk/hooks.h — vtable replacement + JMP detour helpers.
 *
 * Two hook patterns supported:
 *   1. nfsmw::vtable_hook(vtbl_addr, slot, new_fn, &orig)  — for vtable slots
 *   2. nfsmw::jmp_detour(target, new_fn, trampoline, sizeof_steal) — for free funcs
 *
 * Both wrap VirtualProtect for write access and restore protection after.
 *
 * For more complex hooking (mid-function detours, signature scanning) use a
 * dedicated library like MinHook (Win32) or polyhook2. This SDK provides
 * the minimum needed for the common 80% of NFSMW mods.
 */

#ifndef NFSMW_SDK_HOOKS_H
#define NFSMW_SDK_HOOKS_H

#include "platform.h"
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Vtable slot replacement ----
 *
 * Replace a single virtual function slot with your own callback.
 * Caller must keep the original pointer around to chain-call.
 *
 *   void (*orig)(void*, float) = NULL;
 *   void my_ondriving(void *this_, float dt) {
 *       // pre-call mod
 *       orig(this_, dt);
 *       // post-call mod
 *   }
 *
 *   nfsmw_vtable_hook(NFSMW_VTBL_AIVehicleHelicopter, 10,
 *                     (void*)my_ondriving, (void**)&orig);
 */
static inline int nfsmw_vtable_hook(uintptr_t vtbl_addr, int slot_index,
                                    void *new_fn, void **orig_out) {
    void **vtbl = (void**)vtbl_addr;
    DWORD old;
    if (!VirtualProtect(&vtbl[slot_index], sizeof(void*), PAGE_EXECUTE_READWRITE, &old))
        return 0;
    if (orig_out) *orig_out = vtbl[slot_index];
    vtbl[slot_index] = new_fn;
    VirtualProtect(&vtbl[slot_index], sizeof(void*), old, &old);
    return 1;
}

/* ---- 5-byte JMP detour ----
 *
 * Patch a 5-byte E9 rel32 jump at `target` to redirect to `new_fn`.
 * Returns a small allocated trampoline that contains the displaced bytes
 * + a JMP back to `target + sizeof_steal`. Caller must free with
 * `nfsmw_free_trampoline(trampoline)` on unload.
 *
 *   void *trampoline = NULL;
 *   nfsmw_jmp_detour((void*)NFSMW_FN_SetSpeedBreakerActive, my_replacement,
 *                    &trampoline, 5);
 *   // Now any call to SetSpeedBreakerActive jumps to my_replacement.
 *   // Call `trampoline()` to invoke the original.
 *
 * NB: sizeof_steal must be ≥ 5 and must NOT split a relative-instruction.
 * For NFSMW most early-prologue bytes are safe but verify with disassembler.
 */
static inline int nfsmw_jmp_detour(void *target, void *new_fn, void **trampoline_out,
                                   size_t sizeof_steal) {
    if (sizeof_steal < 5) return 0;
    uint8_t *t = (uint8_t*)target;

    /* Allocate trampoline: sizeof_steal bytes copy + 5-byte JMP back */
    uint8_t *tramp = (uint8_t*)VirtualAlloc(NULL, sizeof_steal + 5,
                                            MEM_COMMIT | MEM_RESERVE,
                                            PAGE_EXECUTE_READWRITE);
    if (!tramp) return 0;
    memcpy(tramp, t, sizeof_steal);
    tramp[sizeof_steal] = 0xE9;  /* JMP rel32 */
    intptr_t back = (intptr_t)(t + sizeof_steal) - (intptr_t)(tramp + sizeof_steal + 5);
    memcpy(tramp + sizeof_steal + 1, &back, 4);

    /* Patch original to JMP rel32 to new_fn */
    DWORD old;
    if (!VirtualProtect(t, sizeof_steal, PAGE_EXECUTE_READWRITE, &old)) {
        VirtualFree(tramp, 0, MEM_RELEASE);
        return 0;
    }
    t[0] = 0xE9;
    intptr_t fwd = (intptr_t)new_fn - (intptr_t)(t + 5);
    memcpy(t + 1, &fwd, 4);
    /* fill remaining bytes (sizeof_steal - 5) with NOPs */
    for (size_t i = 5; i < sizeof_steal; ++i) t[i] = 0x90;
    VirtualProtect(t, sizeof_steal, old, &old);
    /* Required: another thread may be sitting in this function with the
     * old bytes cached. NB: this is NOT fully thread-safe — if a thread is
     * executing *inside* the stolen prologue at patch time, behaviour is
     * undefined. NFSMW is effectively single-threaded for game code, so
     * this is acceptable here; for general use prefer MinHook. */
    FlushInstructionCache(GetCurrentProcess(), t, sizeof_steal);

    if (trampoline_out) *trampoline_out = tramp;
    return 1;
}

static inline void nfsmw_free_trampoline(void *trampoline) {
    if (trampoline) VirtualFree(trampoline, 0, MEM_RELEASE);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* ===================================================================
 * Robust inline-hook backend (MinHook).
 *
 * The 5-byte E9 nfsmw_jmp_detour above does NOT relocate relative
 * instructions and breaks on short prologues. When the SDK is built
 * with the MinHook backend (default in the CMake build —
 * -DNFSMW_HOOKS_BACKEND=minhook), `nfsmw_inline_hook` uses MinHook's
 * length-disassembler + trampoline engine and hooks ANY function
 * prologue reliably, with safe enable/disable.
 *
 * Building without CMake: define NFSMW_HOOKS_BACKEND_MINHOOK and add
 * the vendored extern/minhook sources + include dir to your build
 * (the nfsmw_add_plugin() CMake helper does this automatically).
 * =================================================================== */
#if defined(NFSMW_HOOKS_BACKEND_MINHOOK)
#include <MinHook.h>

#ifdef __cplusplus
extern "C" {
#endif

static volatile LONG nfsmw__mh_inited = 0;

static inline int nfsmw__mh_ensure_init(void) {
    LONG prev = InterlockedCompareExchange(&nfsmw__mh_inited, 1, 0);
    if (prev == 0) {
        if (MH_Initialize() != MH_OK) { nfsmw__mh_inited = 2; return 0; }
        nfsmw__mh_inited = 3;            /* 3 = ready */
        return 1;
    }
    /* spin until the initializing thread settles (rare) */
    while (nfsmw__mh_inited == 1) { /* yield */ }
    return nfsmw__mh_inited == 3;
}

/* Install an inline hook on any function. `*orig_out` receives a
 * callable trampoline to the original. Returns 1 on success. */
static inline int nfsmw_inline_hook(void *target, void *detour, void **orig_out) {
    if (!nfsmw__mh_ensure_init()) return 0;
    if (MH_CreateHook(target, detour, orig_out) != MH_OK) return 0;
    return MH_EnableHook(target) == MH_OK;
}

static inline int nfsmw_inline_unhook(void *target) {
    if (MH_DisableHook(target) != MH_OK) return 0;
    return MH_RemoveHook(target) == MH_OK;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* NFSMW_HOOKS_BACKEND_MINHOOK */

/* ---- C++ RAII Hook<Sig> wrapper ---- */
#ifdef __cplusplus
namespace nfsmw {

template <typename Sig>
class VTableHook {
public:
    VTableHook(uintptr_t vtbl_addr, int slot, Sig new_fn)
        : vtbl_(vtbl_addr), slot_(slot), orig_(nullptr), installed_(false) {
        installed_ = nfsmw_vtable_hook(vtbl_addr, slot,
                                       reinterpret_cast<void*>(new_fn),
                                       reinterpret_cast<void**>(&orig_));
    }
    ~VTableHook() {
        if (installed_)
            nfsmw_vtable_hook(vtbl_, slot_, reinterpret_cast<void*>(orig_), nullptr);
    }
    Sig orig() const { return orig_; }
    bool installed() const { return installed_; }
private:
    uintptr_t vtbl_;
    int slot_;
    Sig orig_;
    bool installed_;
};

template <typename Sig>
class JmpDetour {
public:
    JmpDetour(uintptr_t target, Sig new_fn, size_t sizeof_steal = 5)
        : trampoline_(nullptr) {
        nfsmw_jmp_detour(reinterpret_cast<void*>(target),
                         reinterpret_cast<void*>(new_fn),
                         &trampoline_, sizeof_steal);
    }
    ~JmpDetour() { nfsmw_free_trampoline(trampoline_); }
    Sig trampoline() const { return reinterpret_cast<Sig>(trampoline_); }
private:
    void *trampoline_;
};

/* InlineHook<Sig> — the recommended way to hook a free function.
 *
 *   using Fn = int (NFSMW_CDECL*)(void*);
 *   Fn orig = nullptr;
 *   int NFSMW_CDECL my(void* a) { return orig(a) + 1; }
 *
 *   static nfsmw::InlineHook<Fn> h(NFSMW_FN_SomeFunc, &my);
 *   orig = h.original();          // call through to the real function
 *
 * With the MinHook backend (default CMake build) this hooks ANY
 * prologue. Without it, it degrades to the 5-byte JmpDetour and the
 * same prologue caveats apply.
 */
template <typename Sig>
class InlineHook {
public:
    InlineHook(uintptr_t target, Sig detour)
        : target_(reinterpret_cast<void*>(target)),
          orig_(nullptr), installed_(false) {
#if defined(NFSMW_HOOKS_BACKEND_MINHOOK)
        installed_ = nfsmw_inline_hook(target_,
            reinterpret_cast<void*>(detour),
            reinterpret_cast<void**>(&orig_)) != 0;
#else
        installed_ = nfsmw_jmp_detour(target_,
            reinterpret_cast<void*>(detour),
            reinterpret_cast<void**>(&orig_), 5) != 0;
#endif
    }
    ~InlineHook() {
#if defined(NFSMW_HOOKS_BACKEND_MINHOOK)
        if (installed_) nfsmw_inline_unhook(target_);
#else
        if (installed_) nfsmw_free_trampoline(reinterpret_cast<void*>(orig_));
#endif
    }
    Sig original() const { return orig_; }
    bool installed() const { return installed_; }
private:
    void *target_;
    Sig orig_;
    bool installed_;
};

}  // namespace nfsmw
#endif

#endif /* NFSMW_SDK_HOOKS_H */

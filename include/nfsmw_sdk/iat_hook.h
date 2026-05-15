/*
 * nfsmw_sdk/iat_hook.h — Import Address Table (IAT) hooking.
 *
 * Redirect calls the game makes into an imported DLL (KERNEL32, USER32,
 * D3D9, DINPUT8, …) by swapping the function pointer in a module's
 * import table. Unlike an inline hook this is non-invasive (no code
 * patching) and trivially reversible — ideal for intercepting API calls
 * (file IO, time, window, D3D creation) without touching engine code.
 *
 * Caveat: only catches calls routed through the IAT. Calls the game
 * makes via GetProcAddress or an already-cached pointer are not caught
 * (use an inline hook on the API itself for those).
 *
 * Self-contained PE walk — kernel32-only, no extra libs.
 *
 *   #include <nfsmw_sdk/iat_hook.h>
 *
 *   static void* (WINAPI* o_LoadLibraryA)(const char*) = 0;
 *   static HMODULE WINAPI my_LL(const char* n) { return o_LoadLibraryA(n); }
 *
 *   nfsmw_iat_hook(0, "KERNEL32.dll", "LoadLibraryA",
 *                  (void*)my_LL, (void**)&o_LoadLibraryA);
 *
 * C++: nfsmw::IatHook raii(0,"KERNEL32.dll","LoadLibraryA",&my_LL,&orig);
 */

#ifndef NFSMW_SDK_IAT_HOOK_H
#define NFSMW_SDK_IAT_HOOK_H

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

static int nfsmw__ieq(const char *a, const char *b) {
    for (; *a && *b; ++a, ++b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');
        if (ca != cb) return 0;
    }
    return *a == *b;
}

/* Swap the IAT thunk for `dll!func` in `module` (NULL = main exe).
 * `*orig_out` receives the previous pointer. Returns 1 on success. */
static inline int nfsmw_iat_hook(HMODULE module, const char *dll,
                                 const char *func, void *detour,
                                 void **orig_out) {
    if (!module) module = GetModuleHandleA(NULL);
    BYTE *base = (BYTE*)module;
    if (!base) return 0;
    DWORD nt_off = *(DWORD*)(base + 0x3C);
    BYTE *nt = base + nt_off;
    if (*(DWORD*)nt != 0x00004550u) return 0;          /* "PE\0\0" */
    /* OptionalHeader.DataDirectory[1] = Import Table (x86: nt+0x80) */
    DWORD imp_rva = *(DWORD*)(nt + 0x80);
    if (!imp_rva) return 0;

    /* IMAGE_IMPORT_DESCRIPTOR: {OFT,TS,FC,Name,FirstThunk} 5 dwords */
    DWORD *desc = (DWORD*)(base + imp_rva);
    for (; desc[0] || desc[4]; desc += 5) {
        const char *modname = (const char*)(base + desc[3]);
        if (!nfsmw__ieq(modname, dll)) continue;

        DWORD oft = desc[0] ? desc[0] : desc[4];        /* names  */
        DWORD ft  = desc[4];                            /* IAT    */
        DWORD *pName = (DWORD*)(base + oft);
        void **pIat  = (void**)(base + ft);
        for (; *pName; ++pName, ++pIat) {
            if (*pName & 0x80000000u) continue;          /* by ordinal */
            /* IMAGE_IMPORT_BY_NAME: WORD hint; char Name[] */
            const char *iname = (const char*)(base + *pName + 2);
            if (nfsmw__ieq(iname, func)) {
                DWORD old;
                if (!VirtualProtect(pIat, sizeof(void*),
                                    PAGE_READWRITE, &old))
                    return 0;
                if (orig_out) *orig_out = *pIat;
                *pIat = detour;
                VirtualProtect(pIat, sizeof(void*), old, &old);
                return 1;
            }
        }
    }
    return 0;
}

/* Restore a previously hooked IAT entry to `orig`. */
static inline int nfsmw_iat_unhook(HMODULE module, const char *dll,
                                   const char *func, void *orig) {
    return nfsmw_iat_hook(module, dll, func, orig, 0);
}

#ifdef __cplusplus
} /* extern "C" */

namespace nfsmw {
class IatHook {
public:
    IatHook(HMODULE mod, const char *dll, const char *fn,
            void *detour, void **orig_out)
        : mod_(mod), dll_(dll), fn_(fn), orig_(nullptr), ok_(false) {
        ok_ = nfsmw_iat_hook(mod, dll, fn, detour, &orig_) != 0;
        if (orig_out) *orig_out = orig_;
    }
    ~IatHook() { if (ok_) nfsmw_iat_unhook(mod_, dll_, fn_, orig_); }
    bool installed() const { return ok_; }
    void *original() const { return orig_; }
private:
    HMODULE mod_; const char *dll_; const char *fn_;
    void *orig_; bool ok_;
};
}  // namespace nfsmw
#endif

#endif /* NFSMW_SDK_IAT_HOOK_H */

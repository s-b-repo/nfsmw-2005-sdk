/*
 * nfsmw_sdk/midhook.h — mid-function (register-context) hook.
 *
 * An inline hook only fires at a function's entry. A *mid* hook fires at
 * an ARBITRARY instruction inside a function, hands you the full CPU
 * register state, lets you read/modify it, then resumes the original
 * code. This is how you patch behaviour at a specific computation site
 * (e.g. clamp a speed right where it's calculated) without rewriting the
 * whole function.
 *
 * Requires the MinHook backend (default CMake build) — it relocates the
 * stolen instructions and builds the resume trampoline. The
 * register-capturing thunk is emitted as raw machine code at runtime
 * (no inline asm), so it is toolchain-independent (MinGW/MSVC/Clang).
 *
 *   #include <nfsmw_sdk/midhook.h>
 *
 *   void on_site(nfsmw_regs* r) {
 *       // e.g. cap a value held in EAX at this instruction
 *       if (r->eax > 0x100) r->eax = 0x100;
 *   }
 *
 *   static nfsmw::MidHook mh(0x006F1234, on_site);   // any instr addr
 *
 * The target address MUST be an instruction boundary with >= 5 bytes of
 * relocatable instructions (MinHook validates / relocates). Modifying
 * r->esp is not supported (stack pivot); everything else is writable.
 */

#ifndef NFSMW_SDK_MIDHOOK_H
#define NFSMW_SDK_MIDHOOK_H

#include "platform.h"
#include "hooks.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Register snapshot, in the exact memory order produced by the thunk
 * (pushfd then pushad). Writes here are applied on resume — except esp_,
 * which popad ignores (no stack pivot). */
typedef struct nfsmw_regs {
    uint32_t eflags;
    uint32_t edi, esi, ebp, esp_, ebx, edx, ecx, eax;  /* PUSHAD order */
} nfsmw_regs;

typedef void (NFSMW_CDECL *nfsmw_mid_cb)(nfsmw_regs *r);

typedef struct {
    void *target;
    void *thunk;       /* runtime RWX stub */
    void *cont;        /* MinHook trampoline (original continuation) */
    int   installed;
} nfsmw_midhook;

/* Emit the register-capturing stub:
 *   60                pushad
 *   9C                pushfd
 *   54                push esp            ; -> &nfsmw_regs
 *   B8 <cb> FF D0     mov eax,cb ; call eax
 *   83 C4 04          add esp,4
 *   9D                popfd
 *   61                popad
 *   68 <cont> C3      push cont ; ret     ; resume w/o clobbering regs
 */
static inline int nfsmw_midhook_install(nfsmw_midhook *mh, uintptr_t target,
                                         nfsmw_mid_cb cb) {
#if !defined(NFSMW_HOOKS_BACKEND_MINHOOK)
    (void)mh; (void)target; (void)cb;
    return 0;   /* needs MinHook backend for relocation/trampoline */
#else
    unsigned char *s = (unsigned char*)VirtualAlloc(
        NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!s) return 0;
    int i = 0;
    s[i++] = 0x60;                                   /* pushad        */
    s[i++] = 0x9C;                                   /* pushfd        */
    s[i++] = 0x54;                                   /* push esp      */
    s[i++] = 0xB8; *(void**)(s+i) = (void*)cb; i+=4; /* mov eax,cb    */
    s[i++] = 0xFF; s[i++] = 0xD0;                    /* call eax      */
    s[i++] = 0x83; s[i++] = 0xC4; s[i++] = 0x04;     /* add esp,4     */
    s[i++] = 0x9D;                                   /* popfd         */
    s[i++] = 0x61;                                   /* popad         */
    /* cont patched after MinHook gives us the trampoline: push imm32  */
    int cont_imm = i + 1;
    s[i++] = 0x68; *(void**)(s+i) = (void*)0; i+=4;  /* push <cont>   */
    s[i++] = 0xC3;                                   /* ret           */

    mh->target = (void*)target;
    mh->thunk  = s;
    mh->cont   = 0;
    mh->installed = 0;

    if (!nfsmw_inline_hook((void*)target, s, &mh->cont)) {
        VirtualFree(s, 0, MEM_RELEASE);
        mh->thunk = 0;
        return 0;
    }
    *(void**)((unsigned char*)s + cont_imm) = mh->cont;  /* patch resume */
    FlushInstructionCache(GetCurrentProcess(), s, 32);
    mh->installed = 1;
    return 1;
#endif
}

static inline void nfsmw_midhook_remove(nfsmw_midhook *mh) {
#if defined(NFSMW_HOOKS_BACKEND_MINHOOK)
    if (mh && mh->installed) {
        nfsmw_inline_unhook(mh->target);
        if (mh->thunk) VirtualFree(mh->thunk, 0, MEM_RELEASE);
        mh->installed = 0;
        mh->thunk = 0;
    }
#else
    (void)mh;
#endif
}

#ifdef __cplusplus
} /* extern "C" */

namespace nfsmw {
class MidHook {
public:
    MidHook(uintptr_t target, nfsmw_mid_cb cb) {
        ok_ = nfsmw_midhook_install(&h_, target, cb) != 0;
    }
    ~MidHook() { nfsmw_midhook_remove(&h_); }
    bool installed() const { return ok_; }
private:
    nfsmw_midhook h_;
    bool ok_;
};
}  // namespace nfsmw
#endif

#endif /* NFSMW_SDK_MIDHOOK_H */

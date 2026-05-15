/*
 * nfsmw_sdk/lua.h — register your own C functions as game script natives.
 *
 * NFSMW's mission/AI/UI scripting is **vanilla Lua 5.0.2** (confirmed by
 * error-string match). ~150 C++ natives are bound in
 * RegisterScriptNativesGameplay @ 0x61e750 via a family of registrar
 * helpers, each `(const char* name, void* fnptr [, flags])`.
 *
 * The robust, version-safe way to add a native:
 *
 *   1. Inline-hook RegisterScriptNativesGameplay.
 *   2. In your detour, call the original (binds all stock natives), then
 *      call the engine's OWN registrar helper for each of yours — so
 *      your native is registered exactly like the built-ins, into the
 *      same script environment, at the right time.
 *
 * Why a helper *address macro* instead of a hardcoded constant: the
 * registrar helper that takes a plain `(name, fnptr)` is one of several
 * arity-specific FUN_0061a / FUN_0061c registrar variants and is not a
 * single stable verified address across builds. Resolve it once for your
 * target
 * (Ghidra or an AOB scan via <nfsmw_sdk/scan.h>) and set
 * NFSMW_LUA_REGISTRAR before including this header. This keeps the SDK
 * honest — it ships the mechanism + verified anchors, not a guessed
 * address that would crash.
 *
 * Your native uses the standard Lua 5.0.2 C ABI:
 *   int my_native(lua_State* L);   // cdecl; return #results
 * Read args with lua_tonumber/lua_tostring(L, idx), push results with
 * lua_pushnumber/lua_pushstring, return the count. Resolve the few Lua
 * C-API entry points you need the same way (typedefs provided below).
 *
 * Opt-in: not in the umbrella.
 */

#ifndef NFSMW_SDK_LUA_H
#define NFSMW_SDK_LUA_H

#include "platform.h"
#include "hooks.h"

/* ---- Verified RE anchors (vanilla Lua 5.0.2) ---- */
#define NFSMW_FN_RegisterScriptNativesGameplay 0x0061E750u
#define NFSMW_FN_luaV_execute                  0x0060E9D0u  /* dispatch loop */
#define NFSMW_FN_luaD_call                     0x006126A0u
#define NFSMW_FN_luaD_precall                  0x0060E7D0u

/* The engine's "register one simple native" helper:
 *   void (cdecl)(const char* name, void* cfunc)
 * NOT a single stable address — set this for your build (Ghidra / AOB).
 * Leave undefined to use the lua_State path (nfsmw_lua_register) instead. */
/* #define NFSMW_LUA_REGISTRAR 0x0061A2F0u */

/* ---- Standard Lua 5.0.2 C API signatures (resolve addresses per build,
 *      e.g. via <nfsmw_sdk/scan.h>; assign into nfsmw_lua_api) ---- */
typedef struct lua_State lua_State;
typedef int (NFSMW_CDECL *lua_CFunction)(lua_State *L);

typedef struct {
    /* minimal set needed to register + write simple natives */
    void   (NFSMW_CDECL *pushcclosure)(lua_State*, lua_CFunction, int);
    void   (NFSMW_CDECL *setglobal)(lua_State*, const char*);   /* 5.0 macro-equiv */
    double (NFSMW_CDECL *tonumber)(lua_State*, int);
    const char* (NFSMW_CDECL *tostring)(lua_State*, int);
    void   (NFSMW_CDECL *pushnumber)(lua_State*, double);
    void   (NFSMW_CDECL *pushstring)(lua_State*, const char*);
    int    (NFSMW_CDECL *gettop)(lua_State*);
} nfsmw_lua_api;

#ifdef __cplusplus
extern "C" {
#endif

typedef void (NFSMW_CDECL *nfsmw_register_natives_fn)(void *ctx);

/* Register `fn` as global script function `name`.
 *
 * Path A (preferred) — NFSMW_LUA_REGISTRAR defined: forwards to the
 * engine's own registrar (identical to a built-in native).
 * Path B — pass a populated nfsmw_lua_api + lua_State to bind via the
 * standard Lua 5.0 sequence (pushcclosure + setglobal). */
static inline int nfsmw_lua_register(const char *name, lua_CFunction fn,
                                     const nfsmw_lua_api *api, lua_State *L) {
#ifdef NFSMW_LUA_REGISTRAR
    (void)api; (void)L;
    ((void (NFSMW_CDECL*)(const char*, void*))NFSMW_LUA_REGISTRAR)(
        name, (void*)fn);
    return 1;
#else
    if (!api || !L || !api->pushcclosure || !api->setglobal) return 0;
    api->pushcclosure(L, fn, 0);
    api->setglobal(L, name);
    return 1;
#endif
}

#ifdef __cplusplus
} /* extern "C" */

namespace nfsmw {
namespace lua {

/* Run `cb(ctx)` right AFTER the engine has registered all stock natives,
 * so the script environment is fully built. Register your natives inside
 * `cb` with nfsmw_lua_register(). Uses the robust inline-hook backend. */
template <typename F>
inline bool on_register_natives(F &&cb) {
    static F held = (F&&)cb;
    static nfsmw_register_natives_fn orig = nullptr;
    struct S {
        static void NFSMW_CDECL detour(void *ctx) {
            if (orig) orig(ctx);     /* bind all built-ins first */
            held(ctx);               /* then add ours */
        }
    };
    static InlineHook<nfsmw_register_natives_fn> hook(
        NFSMW_FN_RegisterScriptNativesGameplay, &S::detour);
    orig = hook.original();
    return hook.installed();
}

}  // namespace lua
}  // namespace nfsmw
#endif

#endif /* NFSMW_SDK_LUA_H */

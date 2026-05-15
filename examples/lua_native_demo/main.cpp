/*
 * lua_native_demo — add a C function callable from game scripts.
 *
 * Hooks RegisterScriptNativesGameplay; after all built-in natives are
 * bound, registers `SdkPing` (a Lua 5.0.2 C function). Scripts can then
 * call SdkPing().
 *
 * NFSMW_LUA_REGISTRAR (the engine's own (name,fnptr) registrar) must be
 * resolved for your build — set it before including <nfsmw_sdk/lua.h>,
 * e.g. found via Ghidra or <nfsmw_sdk/scan.h>. Without it this demo
 * still builds and the hook installs; registration is a no-op until the
 * address (or a populated nfsmw_lua_api + lua_State) is supplied.
 */

#include <nfsmw_sdk/nfsmw_sdk.h>
/* #define NFSMW_LUA_REGISTRAR 0x0061A2F0u   // <- set for your build */
#include <nfsmw_sdk/lua.h>

static int NFSMW_CDECL Sdk_Ping(lua_State *L) {
    (void)L;
    OutputDebugStringA("[lua_native_demo] SdkPing() called from script\n");
    return 0;  /* number of Lua results pushed */
}

NFSMW_PLUGIN_DECLARE("Lua Native Demo", "1.0.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    bool ok = nfsmw::lua::on_register_natives([](void* /*ctx*/) {
        nfsmw_lua_register("SdkPing", &Sdk_Ping, nullptr, nullptr);
    });
    OutputDebugStringA(ok
        ? "[lua_native_demo] hooked native registrar\n"
        : "[lua_native_demo] hook FAILED\n");
    return ok ? NFSMW_OK : NFSMW_FAIL;
}

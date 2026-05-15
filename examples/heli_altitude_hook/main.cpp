/*
 * heli_altitude_hook — vtable-hook the police helicopter.
 *
 * Replaces slot 17 (FilterHeliAltitudeVector @ 0x417A20) of the
 * AIVehicleHelicopter vtable (@ 0x008920D8, 30 slots; wave-16) so we can
 * observe / clamp the helicopter's altitude each tick, then chain-call
 * the original.
 *
 * Demonstrates the SDK's RAII VTableHook<Sig> wrapper.
 */

#include <nfsmw_sdk/nfsmw_sdk.h>

using HeliFilterFn = void (NFSMW_THISCALL*)(void* /*this*/, void* /*vec3*/);

static nfsmw::VTableHook<HeliFilterFn>* g_hook = nullptr;
static HeliFilterFn g_orig = nullptr;

static void NFSMW_THISCALL my_heli_filter(void* self, void* vec) {
    /* pre: could clamp vec->y here for a "low-flying heli" mod */
    if (g_orig) g_orig(self, vec);
    /* post: inspect resulting altitude */
}

NFSMW_PLUGIN_DECLARE("Heli Altitude Hook", "1.0.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    static nfsmw::VTableHook<HeliFilterFn> hook(
        NFSMW_VTBL_AIVehicleHelicopter, 17, &my_heli_filter);
    if (!hook.installed()) {
        OutputDebugStringA("[heli_altitude_hook] vtable hook FAILED\n");
        return NFSMW_FAIL;
    }
    g_hook = &hook;
    g_orig = hook.orig();
    OutputDebugStringA("[heli_altitude_hook] hooked AIVehicleHelicopter vt[17]\n");
    return NFSMW_OK;
}

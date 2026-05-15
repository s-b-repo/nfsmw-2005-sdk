/*
 * struct_offsets_demo — typed field access via verified offsets.
 *
 * Demonstrates reading/writing engine struct fields by the
 * compiler-derived, ABI-invariant offsets in <nfsmw_sdk/structs.h>.
 * Compile-only proof here (no live pointer); a real mod gets the
 * object pointer from a hook/global and applies NFSMW_OFF_*.
 */

#include <nfsmw_sdk/nfsmw_sdk.h>
#include <nfsmw_sdk/structs.h>

NFSMW_PLUGIN_DECLARE("Struct Offsets Demo", "1.0.0", "nfsmw-2005-re")

static void tweak_race(void* race_params /* RaceParameters* */) {
    /* force traffic density byte to 0 on a RaceParameters object */
    NFSMW_FIELD(race_params, NFSMW_OFF_RaceParameters_TrafficDensity,
                unsigned char) = 0;
}

NFSMW_PLUGIN_MAIN() {
    /* sanity: the verified macros exist and are sane at compile time */
    static_assert(NFSMW_SIZEOF_RaceParameters > 0, "size macro present");
    (void)&tweak_race;
    OutputDebugStringA("[struct_offsets_demo] offset macros available\n");
    return NFSMW_OK;
}

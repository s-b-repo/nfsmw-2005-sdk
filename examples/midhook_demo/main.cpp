/*
 * midhook_demo — register-context hook at an arbitrary instruction.
 *
 * Places a mid-function hook at AwardPlayerBounty_Impl entry purely to
 * show the register-capture mechanism (a real mid-hook would target an
 * interior computation site). The callback can read AND modify CPU
 * registers; changes are applied on resume.
 *
 * Requires the MinHook backend (default CMake build).
 */

#include <nfsmw_sdk/nfsmw_sdk.h>
#include <nfsmw_sdk/midhook.h>

static void NFSMW_CDECL at_site(nfsmw_regs *r) {
    /* Example: log, and clamp whatever value sits in ECX. */
    OutputDebugStringA("[midhook_demo] hit; inspecting registers\n");
    if (r->ecx > 0x10000u) r->ecx = 0x10000u;   /* modify -> applied */
}

NFSMW_PLUGIN_DECLARE("MidHook Demo", "1.0.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    static nfsmw::MidHook mh(NFSMW_FN_AwardPlayerBounty_Impl, &at_site);
    OutputDebugStringA(mh.installed()
        ? "[midhook_demo] mid-hook installed\n"
        : "[midhook_demo] FAILED (needs MinHook backend)\n");
    return mh.installed() ? NFSMW_OK : NFSMW_FAIL;
}

/*
 * inline_hook — robust inline detour via the MinHook backend.
 *
 * Hooks AwardPlayerBounty_Impl @ 0x612220 (thiscall) and doubles every
 * bounty award by calling the original with a scaled value. Works on any
 * prologue because the default CMake build uses the vendored MinHook
 * backend (NFSMW_HOOKS_BACKEND_MINHOOK); the simple 5-byte detour could
 * not safely hook an arbitrary prologue.
 *
 * Signature (from the RE DB): void __thiscall (AICopManager* self,
 * Vehicle* v, int amount). We only need the shape to chain-call.
 */

#include <nfsmw_sdk/nfsmw_sdk.h>

using AwardFn = void (NFSMW_THISCALL*)(void* self, void* veh, int amount);

static nfsmw::InlineHook<AwardFn>* g_hook = nullptr;

static void NFSMW_THISCALL award_2x(void* self, void* veh, int amount) {
    AwardFn orig = g_hook ? g_hook->original() : nullptr;
    if (orig) orig(self, veh, amount * 2);   /* double the bounty */
}

NFSMW_PLUGIN_DECLARE("Inline Hook (2x Bounty)", "1.0.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    static nfsmw::InlineHook<AwardFn> hook(
        NFSMW_FN_AwardPlayerBounty_Impl, &award_2x);
    if (!hook.installed()) {
        OutputDebugStringA("[inline_hook] FAILED to install\n");
        return NFSMW_FAIL;
    }
    g_hook = &hook;
    OutputDebugStringA("[inline_hook] AwardPlayerBounty hooked (2x)\n");
    return NFSMW_OK;
}

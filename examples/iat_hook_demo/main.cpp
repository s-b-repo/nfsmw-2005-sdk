/*
 * iat_hook_demo — intercept an imported API via the IAT.
 *
 * Hooks KERNEL32!Sleep in the main module's import table and logs the
 * first few calls, passing through to the real Sleep. Demonstrates
 * non-invasive API interception (no engine code patched).
 */

#include <nfsmw_sdk/nfsmw_sdk.h>
#include <nfsmw_sdk/iat_hook.h>

typedef void (WINAPI *SleepFn)(DWORD);
static SleepFn g_orig_sleep = nullptr;
static long    g_count      = 0;

static void WINAPI my_sleep(DWORD ms) {
    if (g_count < 3) {
        OutputDebugStringA("[iat_hook_demo] Sleep() intercepted\n");
        ++g_count;
    }
    if (g_orig_sleep) g_orig_sleep(ms);
}

NFSMW_PLUGIN_DECLARE("IAT Hook Demo", "1.0.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    if (nfsmw_iat_hook(nullptr, "KERNEL32.dll", "Sleep",
                       (void*)&my_sleep, (void**)&g_orig_sleep))
        OutputDebugStringA("[iat_hook_demo] KERNEL32!Sleep IAT-hooked\n");
    else
        OutputDebugStringA("[iat_hook_demo] Sleep not in main IAT\n");
    return NFSMW_OK;
}

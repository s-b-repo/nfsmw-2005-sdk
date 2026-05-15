/*
 * entry.c — unified loader entry shim.
 *
 * ONE DLL, BOTH loaders. Link exactly this file into every plugin.
 *
 *   - ASI / Ultimate-ASI-Loader: DllMain spawns a worker thread that, after
 *     a short delay, runs the plugin once.
 *   - BepInEx 6 NativeBootstrap: BepInEx calls BepInExNativePlugin_Load,
 *     which runs the plugin once.
 *
 * A one-shot interlocked guard ensures plugin_main runs exactly once even
 * if both paths fire (e.g. ASI loader + BepInEx both present).
 */

#include <nfsmw_sdk/platform.h>

/* entry.c may be compiled by a C++ driver (g++) in some build setups.
 * Force C linkage so BepInEx finds the unmangled export names. */
#ifdef __cplusplus
extern "C" {
#endif

extern int (*nfsmw_plugin_main_ptr)(void);

static volatile LONG g_nfsmw_ran = 0;

static int nfsmw_run_once(const char *via) {
    if (InterlockedCompareExchange(&g_nfsmw_ran, 1, 0) != 0)
        return 0; /* already ran */
    if (!nfsmw_validate_speed_exe()) {
        OutputDebugStringA("[nfsmw_sdk] not inside speed.exe - refusing\n");
        return 1;
    }
    OutputDebugStringA("[nfsmw_sdk] plugin starting via ");
    OutputDebugStringA(via);
    OutputDebugStringA("\n");
    return nfsmw_plugin_main_ptr ? nfsmw_plugin_main_ptr() : -1;
}

/* ---- ASI path ---- */

#ifndef NFSMW_ASI_INIT_DELAY_MS
#define NFSMW_ASI_INIT_DELAY_MS 2000  /* override at compile time if needed */
#endif

static DWORD WINAPI nfsmw_asi_worker(LPVOID lpv) {
    (void)lpv;
    Sleep(NFSMW_ASI_INIT_DELAY_MS); /* let the engine finish early init */
    nfsmw_run_once("ASI");
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
        HANDLE h = CreateThread(NULL, 0, nfsmw_asi_worker, NULL, 0, NULL);
        if (h) CloseHandle(h);
    }
    return TRUE;
}

/* ---- BepInEx 6 Native path ---- */

NFSMW_EXPORT int BepInExNativePlugin_Load(void) {
    return nfsmw_run_once("BepInEx");
}

NFSMW_EXPORT const char *BepInExNativePlugin_GUID    = "nfsmw.sdk.plugin";
NFSMW_EXPORT const char *BepInExNativePlugin_Name    = "NFSMW SDK Plugin";
NFSMW_EXPORT const char *BepInExNativePlugin_Version = "1.0.0";

#ifdef __cplusplus
}  /* extern "C" */
#endif

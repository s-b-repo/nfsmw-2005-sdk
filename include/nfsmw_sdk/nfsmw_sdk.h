/*
 * nfsmw_sdk.h — top-level umbrella header.
 *
 * Include this single file to pull in the full nfsmw SDK.
 *
 *   #include <nfsmw_sdk/nfsmw_sdk.h>
 *
 *   NFSMW_PLUGIN_DECLARE("MyMod", "1.0.0", "Author Name")
 *
 *   NFSMW_PLUGIN_MAIN() {
 *       *nfsmw::Tweak_InfiniteNOS() = true;
 *       return NFSMW_OK;
 *   }
 */

#ifndef NFSMW_SDK_H
#define NFSMW_SDK_H

#include "platform.h"
#include "globals.h"
#include "functions.h"
#include "enums.h"
#include "attributes.h"
#include "hooks.h"
#include "scan.h"

/* ---- Entry-point macros — abstract over ASI vs BepInEx vs Direct ---- */

#define NFSMW_OK     0
#define NFSMW_FAIL  -1

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*nfsmw_plugin_main_fn)(void);

/* The plugin author defines this. Called once, on first speed.exe load. */
extern nfsmw_plugin_main_fn nfsmw_plugin_main_ptr;

/* Plugin metadata — visible to BepInEx and the ASI loader. */
typedef struct {
    const char *name;
    const char *version;
    const char *author;
    const char *target_game;  /* should be "NFSMW" */
} nfsmw_plugin_info_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

/* ---- One-shot plugin-declare macro ----
 *
 * Sets the plugin metadata and emits the loader-specific entry stub.
 *   - For ASI:     DllMain spawns a worker thread that calls plugin_main()
 *   - For BepInEx: a `BepInExNativePlugin_Load` export is generated
 *   - For Direct:  a `nfsmw_run` export is generated
 *
 * Use exactly once per plugin source file.
 */
#if defined(__GNUC__)
#  define NFSMW__DIAG_PUSH _Pragma("GCC diagnostic push") \
        _Pragma("GCC diagnostic ignored \"-Wattributes\"")
#  define NFSMW__DIAG_POP  _Pragma("GCC diagnostic pop")
#else
#  define NFSMW__DIAG_PUSH
#  define NFSMW__DIAG_POP
#endif

/* Plugin metadata is exported with external linkage. The pointer/struct
 * objects are intentionally non-const so they have external linkage at
 * namespace scope (a C++ `const` at namespace scope is internal-linkage
 * and cannot be dllexport'd). */
#define NFSMW_PLUGIN_DECLARE(NAME, VERSION, AUTHOR)                          \
    static int nfsmw_plugin_main_impl(void);                                  \
    NFSMW__DIAG_PUSH                                                          \
    extern "C" nfsmw_plugin_main_fn nfsmw_plugin_main_ptr =                    \
        nfsmw_plugin_main_impl;                                                \
    extern "C" NFSMW_EXPORT nfsmw_plugin_info_t NFSMW_PluginInfo = {           \
        NAME, VERSION, AUTHOR, "NFSMW"                                         \
    };                                                                         \
    extern "C" NFSMW_EXPORT const char *NFSMW_PluginName    = NAME;            \
    extern "C" NFSMW_EXPORT const char *NFSMW_PluginVersion = VERSION;         \
    extern "C" NFSMW_EXPORT const char *NFSMW_PluginAuthor  = AUTHOR;          \
    NFSMW__DIAG_POP

/* ---- Entry point definition macro ----
 *
 * Body block runs once after game image is loaded + image-base verified.
 * Returns NFSMW_OK / NFSMW_FAIL.
 */
#define NFSMW_PLUGIN_MAIN() static int nfsmw_plugin_main_impl(void)

#endif /* NFSMW_SDK_H */

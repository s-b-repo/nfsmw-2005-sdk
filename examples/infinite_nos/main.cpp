/*
 * infinite_nos — the canonical "hello world" NFSMW mod.
 *
 * Sets Tweak_InfiniteNOS + Tweak_InfiniteRaceBreaker globals once at startup.
 * Mirrors the validated mods/infinite_trainer ASI but built via the SDK.
 *
 * Build (from Linux/Mac/Win host):
 *   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/nfsmw-toolchain-mingw-i686.cmake -B build
 *   cmake --build build --target infinite_nos
 * Output: build/examples/infinite_nos.dll (+ .asi copy)
 */

#include <nfsmw_sdk/nfsmw_sdk.h>

NFSMW_PLUGIN_DECLARE("Infinite NOS", "1.0.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    *nfsmw::Tweak_InfiniteNOS()         = true;
    *nfsmw::Tweak_InfiniteRaceBreaker() = true;
    OutputDebugStringA("[infinite_nos] enabled NOS + RaceBreaker\n");
    return NFSMW_OK;
}

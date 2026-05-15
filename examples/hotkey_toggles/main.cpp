/*
 * hotkey_toggles — toggle cheats at runtime with the keyboard.
 *
 *   F5  -> toggle Infinite NOS
 *   F6  -> toggle Infinite RaceBreaker
 *   F7  -> cycle game speed 1.0 / 0.5 / 2.0
 *
 * Shows the opt-in hotkeys.h helper (pulls user32 only because this mod
 * includes it; the core SDK stays kernel32-only).
 */

#include <nfsmw_sdk/nfsmw_sdk.h>
#include <nfsmw_sdk/hotkeys.h>

NFSMW_PLUGIN_DECLARE("Hotkey Toggles", "1.0.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    nfsmw_hotkey_register(VK_F5, [](void*) {
        bool *f = nfsmw::Tweak_InfiniteNOS();
        *f = !*f;
        OutputDebugStringA("[hotkey_toggles] InfiniteNOS toggled\n");
    }, nullptr);

    nfsmw_hotkey_register(VK_F6, [](void*) {
        bool *f = nfsmw::Tweak_InfiniteRaceBreaker();
        *f = !*f;
        OutputDebugStringA("[hotkey_toggles] InfiniteRaceBreaker toggled\n");
    }, nullptr);

    nfsmw_hotkey_register(VK_F7, [](void*) {
        float *s = nfsmw::Tweak_GameSpeed();
        *s = (*s >= 2.0f) ? 0.5f : (*s >= 1.0f ? 2.0f : 1.0f);
        OutputDebugStringA("[hotkey_toggles] GameSpeed cycled\n");
    }, nullptr);

    nfsmw_hotkeys_start();
    OutputDebugStringA("[hotkey_toggles] F5/F6/F7 armed\n");
    return NFSMW_OK;
}

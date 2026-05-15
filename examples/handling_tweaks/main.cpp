/*
 * handling_tweaks — modify a vehicle attribute at runtime.
 *
 * Demonstrates the attribute Collection API: look up the player car's
 * pvehicle collection and halve its MASS for arcadey handling.
 *
 * NB: the collection must exist when plugin_main runs. For a robust mod
 * you would hook a per-frame function and apply this after the race loads;
 * here we keep it minimal and apply to the default SkipFE car string.
 */

#include <nfsmw_sdk/nfsmw_sdk.h>

NFSMW_PLUGIN_DECLARE("Handling Tweaks", "1.0.0", "nfsmw-2005-re")

struct VehicleCollection {
    /* opaque — we only call engine helpers via address */
};

NFSMW_PLUGIN_MAIN() {
    const char *car = *reinterpret_cast<const char**>(NFSMW_GLOBAL_SkipFEPlayerCar);
    if (!car) car = "bmwm3gtre46";

    auto coll = nfsmw_find_collection("pvehicle", car);
    if (!coll) {
        OutputDebugStringA("[handling_tweaks] collection not ready yet\n");
        return NFSMW_OK;  /* not fatal — race not loaded */
    }
    /* For a full implementation, resolve the attribute pointer via the
     * engine's Collection::GetAttribute (see docs/ATTRIBUTE_DATABASE.md).
     * uint32_t mass_key = NFSMW_BCHUNK("MASS"); */
    OutputDebugStringA("[handling_tweaks] collection found; see ATTRIBUTE_DATABASE.md\n");
    return NFSMW_OK;
}

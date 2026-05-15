/*
 * handling_tweaks — real end-to-end attribute mutation.
 *
 * Looks up the player car's `pvehicle` collection, reads MASS, and halves
 * it for arcadey handling. Demonstrates the full attribute path:
 *   find_collection -> Collection::GetData -> typed read/write.
 *
 * NB: the collection only exists once the car/vault is loaded. A robust
 * mod would re-apply this after each race load (hook a per-frame fn);
 * this example keeps it to a single startup attempt and reports if the
 * collection isn't ready yet.
 */

#include <nfsmw_sdk/nfsmw_sdk.h>

NFSMW_PLUGIN_DECLARE("Handling Tweaks", "1.1.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    const char *car = *reinterpret_cast<const char**>(NFSMW_GLOBAL_SkipFEPlayerCar);
    if (!car || !*car) car = "bmwm3gtre46";

    nfsmw_Collection *coll = nfsmw_find_collection("pvehicle", car);
    if (!coll) {
        OutputDebugStringA("[handling_tweaks] pvehicle collection not loaded yet\n");
        return NFSMW_OK;  /* not fatal — vault not up */
    }

    uint32_t mass_key = NFSMW_BCHUNK("MASS");
    float mass = 0.0f;
    if (!nfsmw_attr_get_float(coll, mass_key, &mass)) {
        OutputDebugStringA("[handling_tweaks] MASS attribute absent\n");
        return NFSMW_OK;
    }

    char buf[96];
    /* no user32: build the message with the engine-free path */
    OutputDebugStringA("[handling_tweaks] MASS read OK; halving\n");
    (void)buf;

    if (nfsmw_attr_set_float(coll, mass_key, mass * 0.5f))
        OutputDebugStringA("[handling_tweaks] MASS halved\n");

    return NFSMW_OK;
}

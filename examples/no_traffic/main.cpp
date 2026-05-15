/*
 * no_traffic — disable civilian traffic via the SkipFE flag.
 *
 * SkipFEDisableTraffic suppresses traffic spawning when set before a race
 * loads. Also drops SkipFETrafficDensity to 0 for in-progress sessions.
 */

#include <nfsmw_sdk/nfsmw_sdk.h>

NFSMW_PLUGIN_DECLARE("No Traffic", "1.0.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    *reinterpret_cast<bool*>(NFSMW_GLOBAL_SkipFEDisableTraffic)  = true;
    *reinterpret_cast<float*>(NFSMW_GLOBAL_SkipFETrafficDensity) = 0.0f;
    OutputDebugStringA("[no_traffic] traffic disabled\n");
    return NFSMW_OK;
}

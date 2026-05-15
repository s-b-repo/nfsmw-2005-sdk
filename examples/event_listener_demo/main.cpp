/*
 * event_listener_demo — react to engine gameplay events.
 *
 * Subscribes to two well-known events on the global hashed bus and logs
 * when they fire. No code patched — pure listener registration.
 *
 *   MPursuitBreaker  — an environmental pursuit-breaker was triggered
 *   MNotifyMilestoneProgress — a milestone/career notification
 *
 * Also installs a bus snoop that prints the hash of EVERY event — the
 * fastest way to discover which event fires when (cross-ref the hash
 * with EVENT_BUSES.md §8).
 */

#include <nfsmw_sdk/nfsmw_sdk.h>
#include <nfsmw_sdk/events.h>

static void NFSMW_CDECL on_breaker(void* ev, void* ctx) {
    (void)ev; (void)ctx;
    OutputDebugStringA("[event_demo] MPursuitBreaker fired\n");
}

NFSMW_PLUGIN_DECLARE("Event Listener Demo", "1.0.0", "nfsmw-2005-re")

NFSMW_PLUGIN_MAIN() {
    nfsmw_events_subscribe("MPursuitBreaker", on_breaker, nullptr);

    nfsmw::events::on("MNotifyMilestoneProgress", [](void* /*ev*/) {
        OutputDebugStringA("[event_demo] milestone progress\n");
    });

    nfsmw::events::on_any([](uint32_t hash) {
        /* no user32: format the hash by hand */
        char b[32] = "[event_demo] 0x00000000\n";
        for (int i = 0; i < 8; ++i) {
            int nib = (hash >> ((7 - i) * 4)) & 0xF;
            b[15 + i] = (char)(nib < 10 ? '0' + nib : 'a' + nib - 10);
        }
        OutputDebugStringA(b);
    });

    OutputDebugStringA("[event_demo] listeners armed\n");
    return NFSMW_OK;
}

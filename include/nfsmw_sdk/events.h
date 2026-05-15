/*
 * nfsmw_sdk/events.h — the global hashed gameplay event bus (DAT_0091e0d0).
 *
 * NFSMW's cross-subsystem comms ride a single hash-keyed bus: race
 * finish, pursuit enter/over, milestone unlock, reputation, audio cues,
 * SpeedBreaker/PursuitBreaker triggers — all broadcast on this bus,
 * keyed by bChunk(name). Subscribing a C callback lets a mod react to
 * *any* of them ("make anyone do anything") without hooking code.
 *
 * Verified anchors (EVENT_BUSES.md §3):
 *   bus singleton          DAT_0091e0d0           @ 0x0091e0d0
 *   BusSubscribeListenerByHash                    @ 0x0061fd00
 *   BusBroadcastEventByHash                       @ 0x0061fc10
 * Event key = bChunk(name) (Jenkins mix3, seed 0xABCDEF00).
 *
 *   #include <nfsmw_sdk/events.h>
 *
 *   void on_pursuit_over(void* ev, void* ctx) { (void)ev;(void)ctx;
 *       OutputDebugStringA("[mod] pursuit ended\n");
 *   }
 *   nfsmw_events_subscribe("MPursuitOver", on_pursuit_over, 0);
 *
 * C++: nfsmw::events::on("MPursuitOver", [](void* ev){ ... });
 *
 * CALLBACK ABI — IMPORTANT: the engine listener convention is cdecl and
 * (per every documented subscriber) effectively (event_handle, ctx).
 * The event handle is engine-internal — treat it as OPAQUE. The event's
 * own hash lives at handle+0x18 (event[6]) if you need to disambiguate.
 * Before relying on any handle field, VERIFY by first subscribing to a
 * known event and inspecting it. cdecl means a real 3/4-arg engine
 * signature still won't corrupt the caller, so experimenting is safe.
 *
 * There is NO automatic unsubscribe — a listener lives until the bus is
 * destroyed (process lifetime). Register once (e.g. NFSMW_PLUGIN_MAIN).
 *
 * Opt-in: not in the umbrella.
 */

#ifndef NFSMW_SDK_EVENTS_H
#define NFSMW_SDK_EVENTS_H

#include "platform.h"
#include "functions.h"
#include "attributes.h"   /* nfsmw_bchunk */
#include "hooks.h"

#ifndef NFSMW_EVENT_BUS_SINGLETON
#define NFSMW_EVENT_BUS_SINGLETON 0x0091E0D0u   /* DAT_0091e0d0 */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Engine listener convention (see ABI note above). */
typedef void (NFSMW_CDECL *nfsmw_event_cb)(void *event_handle, void *ctx);

typedef void (NFSMW_CDECL *nfsmw__bus_subscribe_fn)(
    void *bus, uint32_t hash, nfsmw_event_cb cb, void *ctx);
typedef void (NFSMW_CDECL *nfsmw__bus_broadcast_fn)(
    void *bus, void *event_handle);

/* Subscribe `cb` to every broadcast of `hash` on the global bus. */
static inline void nfsmw_events_subscribe_hash(uint32_t hash,
                                               nfsmw_event_cb cb,
                                               void *ctx) {
    void *bus = (void*)NFSMW_EVENT_BUS_SINGLETON;
    ((nfsmw__bus_subscribe_fn)NFSMW_FN_BusSubscribeListenerByHash)(
        bus, hash, cb, ctx);
}

/* Same, keyed by event name (hashed with the engine's bChunk). */
static inline void nfsmw_events_subscribe(const char *name,
                                          nfsmw_event_cb cb, void *ctx) {
    nfsmw_events_subscribe_hash(nfsmw_bchunk(name), cb, ctx);
}

/* Broadcast a PRE-CONSTRUCTED engine event handle. We intentionally do
 * NOT offer a broadcast(name, ...) helper: BusBroadcastEventByHash takes
 * a constructed event object (hash at +0x18), not loose args —
 * fabricating one corrupts the heap. Build the handle the way the
 * engine does for that event (see EVENT_BUSES.md §8) and pass it here. */
static inline void nfsmw_events_broadcast_raw(void *event_handle) {
    void *bus = (void*)NFSMW_EVENT_BUS_SINGLETON;
    ((nfsmw__bus_broadcast_fn)NFSMW_FN_BusBroadcastEventByHash)(
        bus, event_handle);
}

#ifdef __cplusplus
} /* extern "C" */

namespace nfsmw {
namespace events {

/* Subscribe a stateless callable to an event by name. The callable gets
 * the opaque event handle. */
template <typename F>
inline void on(const char *name, F &&fn) {
    static F held = (F&&)fn;
    nfsmw_events_subscribe(name,
        [](void *ev, void*) { held(ev); }, nullptr);
}

/* Snoop EVERY event on the bus: inline-hooks BusBroadcastEventByHash and
 * calls `cb(hash)` (hash read from event_handle+0x18) before the engine
 * dispatches. Useful for discovering which event fires when. */
template <typename F>
inline bool on_any(F &&fn) {
    using BcFn = void (NFSMW_CDECL*)(void*, void*);
    static F held = (F&&)fn;
    static BcFn orig = nullptr;
    struct S {
        static void NFSMW_CDECL detour(void *bus, void *ev) {
            if (ev) held(*reinterpret_cast<uint32_t*>(
                          reinterpret_cast<char*>(ev) + 0x18));
            if (orig) orig(bus, ev);
        }
    };
    static InlineHook<BcFn> hook(
        NFSMW_FN_BusBroadcastEventByHash, &S::detour);
    orig = hook.original();
    return hook.installed();
}

}  // namespace events
}  // namespace nfsmw
#endif

#endif /* NFSMW_SDK_EVENTS_H */

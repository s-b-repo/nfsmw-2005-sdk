/*
 * nfsmw_sdk/input.h — typed access to the game's action-binding layer.
 *
 * NFSMW maps physical inputs to ~76 game actions (STEERLEFT, GAS, BRAKE,
 * HANDBRAKE, GAMEBREAKER, NITROUS, SHIFTUP, …) through a binding
 * template. The action id == the row index. Use the generated
 * `NFSMW_ActionID_GAMEACTION_*` enum for ids.
 *
 * RE-DB anchors (wave-8): binding template @ 0x008f6d80 (76 rows, stride
 * 0x34, 4 slots/row), runtime mirror @ 0x0091f418,
 * PollAllGameActionBindingsPerFrame @ 0x6349b0.
 *
 * Two robust ways to use this:
 *
 *  1. Inspect / rebind a slot — `nfsmw_input_binding_row(id)` gives the
 *     0x34-byte template row to read or edit.
 *
 *  2. Read or OVERRIDE live action state every frame — hook the poller
 *     with `nfsmw::input::on_poll(cb)`; your callback runs right after
 *     the engine has refreshed all action values, so you can read them
 *     or write the runtime mirror to inject input.
 *
 * Opt-in: not in the umbrella. Include explicitly.
 */

#ifndef NFSMW_SDK_INPUT_H
#define NFSMW_SDK_INPUT_H

#include "platform.h"
#include "functions.h"
#include "enums.h"
#include "hooks.h"

#ifndef NFSMW_INPUT_BINDING_TEMPLATE
#define NFSMW_INPUT_BINDING_TEMPLATE 0x008F6D80u  /* 76 rows */
#endif
#ifndef NFSMW_INPUT_BINDING_STRIDE
#define NFSMW_INPUT_BINDING_STRIDE   0x34u        /* 13 dwords / row */
#endif
#ifndef NFSMW_INPUT_BINDING_ROWS
#define NFSMW_INPUT_BINDING_ROWS     76u
#endif
#ifndef NFSMW_INPUT_RUNTIME_MIRROR
#define NFSMW_INPUT_RUNTIME_MIRROR   0x0091F418u  /* polled-state base */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Pointer to the 0x34-byte binding template row for an action id
 * (read its slots, or write to rebind). NULL if id out of range. */
static inline void *nfsmw_input_binding_row(unsigned action_id) {
    if (action_id >= NFSMW_INPUT_BINDING_ROWS) return 0;
    return (void*)(NFSMW_INPUT_BINDING_TEMPLATE
                   + (uintptr_t)action_id * NFSMW_INPUT_BINDING_STRIDE);
}

/* Base of the per-frame polled-state mirror (engine-internal layout;
 * indexable but the element type is version-specific — prefer the
 * on_poll hook below to read/inject values version-safely). */
static inline void *nfsmw_input_runtime_mirror(void) {
    return (void*)NFSMW_INPUT_RUNTIME_MIRROR;
}

/* Engine poller signature is internal; we only need to chain through. */
typedef void (NFSMW_CDECL *nfsmw_input_poll_fn)(void);

#ifdef __cplusplus
} /* extern "C" */

namespace nfsmw {
namespace input {

/* Run `fn` immediately AFTER the engine polls all action bindings each
 * frame. Inside `fn` the runtime mirror is fresh — read it for current
 * action state, or write it to inject/override input. Uses the robust
 * inline-hook backend. Call once (e.g. from NFSMW_PLUGIN_MAIN). */
template <typename F>
inline bool on_poll(F &&fn) {
    static F held = (F&&)fn;
    static nfsmw_input_poll_fn orig = nullptr;
    struct S {
        static void NFSMW_CDECL detour() {
            if (orig) orig();
            held();
        }
    };
    static InlineHook<nfsmw_input_poll_fn> hook(
        NFSMW_FN_PollAllGameActionBindingsPerFrame, &S::detour);
    orig = hook.original();
    return hook.installed();
}

}  // namespace input
}  // namespace nfsmw
#endif

#endif /* NFSMW_SDK_INPUT_H */

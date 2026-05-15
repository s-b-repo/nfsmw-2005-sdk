# nfsmw_sdk cookbook

Real, copy-pasteable recipes that **compose the verified primitives**.
Every address/offset/hash used here is shipped in the SDK headers — no
guessing required. Each recipe is a complete `NFSMW_PLUGIN_MAIN` body
unless noted.

> Build any of these like the examples:
> `cmake --build build` → `<name>.dll` (BepInEx) + `<name>.asi` (ASI).

---

## 1. Infinite NOS + RaceBreaker

```cpp
#include <nfsmw_sdk/nfsmw_sdk.h>
NFSMW_PLUGIN_DECLARE("Infinite", "1.0", "you")
NFSMW_PLUGIN_MAIN() {
    *nfsmw::Tweak_InfiniteNOS()         = true;
    *nfsmw::Tweak_InfiniteRaceBreaker() = true;
    return NFSMW_OK;
}
```

## 2. Slow-mo / fast-forward

```cpp
*nfsmw::Tweak_GameSpeed() = 0.35f;   // bullet time;  2.0f = fast
```

## 3. No cops, no traffic (free-roam sandbox)

```cpp
*NFSMW_PTR_AT(NFSMW_GLOBAL_SkipFEDisableCops,    bool)  = true;
*NFSMW_PTR_AT(NFSMW_GLOBAL_SkipFEDisableTraffic, bool)  = true;
*NFSMW_PTR_AT(NFSMW_GLOBAL_SkipFETrafficDensity, float) = 0.0f;
```

## 4. Double every bounty award (robust inline hook)

```cpp
#include <nfsmw_sdk/nfsmw_sdk.h>
using AwardFn = void (NFSMW_THISCALL*)(void*, void*, int);
static nfsmw::InlineHook<AwardFn>* g;
static void NFSMW_THISCALL x2(void* s, void* v, int amt) {
    g->original()(s, v, amt * 2);
}
NFSMW_PLUGIN_DECLARE("2x Bounty","1.0","you")
NFSMW_PLUGIN_MAIN() {
    static nfsmw::InlineHook<AwardFn> h(NFSMW_FN_AwardPlayerBounty_Impl, &x2);
    g = &h;
    return h.installed() ? NFSMW_OK : NFSMW_FAIL;
}
```

## 5. Halve the player car's mass (attribute edit)

```cpp
const char* car = *NFSMW_PTR_AT(NFSMW_GLOBAL_SkipFEPlayerCar, const char*);
if (!car || !*car) car = "bmwm3gtre46";
auto c = nfsmw_find_collection("pvehicle", car);
float m;
if (c && nfsmw_attr_get_float(c, NFSMW_BCHUNK("MASS"), &m))
    nfsmw_attr_set_float(c, NFSMW_BCHUNK("MASS"), m * 0.5f);
```

## 6. Do something when a pursuit ends

```cpp
#include <nfsmw_sdk/events.h>
nfsmw_events_subscribe("MPursuitOver",
    [](void*, void*){ *nfsmw::Tweak_InfiniteNOS() = true; }, nullptr);
// discover other event names live:
nfsmw::events::on_any([](uint32_t h){ /* log h, cross-ref EVENT_BUSES */ });
```

## 7. Draw an overlay every frame

```cpp
#include <nfsmw_sdk/d3d9_hooks.h>
nfsmw::on_endscene([](void* dev){
    // cast dev to IDirect3DDevice9* (include <d3d9.h>), init ImGui, draw
});
```

## 8. Toggle a cheat with a hotkey

```cpp
#include <nfsmw_sdk/hotkeys.h>
nfsmw_hotkey_register(VK_F5, [](void*){
    bool* f = nfsmw::Tweak_InfiniteNOS(); *f = !*f;
}, nullptr);
nfsmw_hotkeys_start();
```

## 9. Read/write a struct field by verified offset

For the 79 ABI-invariant structs the SDK ships `NFSMW_OFF_*`. Get the
object pointer from a hook or global, then:

```cpp
#include <nfsmw_sdk/structs.h>
// rp = RaceParameters* received from a hook on race-setup:
nfsmw::field<unsigned char>(rp, NFSMW_OFF_RaceParameters_TrafficDensity) = 0;
unsigned tn = nfsmw::field<unsigned>(rp, NFSMW_OFF_RaceParameters_TrackNumber);
```

## 10. Patch a code byte (force a branch / nop a check)

```cpp
nfsmw_write_byte(0x00XXYYZZ, 0xEB);   // jz -> jmp, etc.
// always page-protect-safe + icache-flushed; returns 0 if it couldn't.
```

---

## Why-not (deliberate omissions — so you don't go looking)

- **No `nfsmw::race_status()` typed accessor.** `GRaceStatus` is a
  multiple-inheritance struct; its field offsets are *withheld*
  (Itanium↔MSVC base placement can differ, no in-binary ground truth).
  The singleton pointer `NFSMW_SINGLETON_GRaceStatus` is exposed as
  opaque — use it with hooks, not field offsets.
- **No `current_fe_screen()` helper.** The indirection level of
  `NFSMW_SINGLETON_CFEManager` (pointer-to-pointer vs object) isn't
  verified; a wrong deref corrupts the heap. Use the FNG event bus
  (`PostUIEventToNamedNode` / `events.h`) instead of poking FE memory.
- **No `set_heat(n)` helper.** There is no verified heat *write* site.
  Hook `AwardPlayerBounty_Impl` (recipe 4) or subscribe to heat/pursuit
  events instead.
- **No 64-bit / other-NFS support, no managed config UI, no hot-reload.**
  Inherent to a native Win32 game with no managed runtime — see
  `CAPABILITIES.md`.

If a recipe you need isn't here and isn't in *why-not*, it's probably a
straight composition of the primitives in `API_REFERENCE.md` — the
pattern is always: *get a pointer (global/hook/event) → apply an
offset/attribute/value*.

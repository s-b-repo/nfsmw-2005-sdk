# nfsmw_sdk API reference

All headers live under `<nfsmw_sdk/...>`. Include the umbrella
`<nfsmw_sdk/nfsmw_sdk.h>` for everything, or pull individual headers.

## Entry-point macros (`nfsmw_sdk.h`)

| Macro | Purpose |
|-------|---------|
| `NFSMW_PLUGIN_DECLARE(name, ver, author)` | Emits exported metadata + binds your entry. Use once per plugin. |
| `NFSMW_PLUGIN_MAIN() { ... }` | Defines the body that runs once after the game image is verified. Return `NFSMW_OK` / `NFSMW_FAIL`. |

The body runs on the ASI worker thread (~2 s post-attach) **or** from
`BepInExNativePlugin_Load`, whichever loader is active, exactly once.

## platform.h

- `NFSMW_IMAGE_BASE` — `0x00400000`.
- `NFSMW_EXPORT`, `NFSMW_NAKED` — compiler-portable attributes.
- `NFSMW_THISCALL/FASTCALL/CDECL/STDCALL` — calling-convention attributes
  matching `speed.exe`.
- `int nfsmw_validate_speed_exe(void)` — true iff loaded into retail
  `speed.exe` (image-base check).
- Memory helpers (page-protection-aware):
  - `nfsmw_write_bytes(addr, src, n)`
  - `nfsmw_write_byte/word/int/float(addr, v)`
  - `nfsmw_read_byte/word/int/float(addr)`

## globals.h

- `NFSMW_GLOBAL_<Name>` — address macros for 28 SDK-documented globals
  (e.g. `NFSMW_GLOBAL_Tweak_InfiniteNOS`, `NFSMW_GLOBAL_DrawHUD`).
- `NFSMW_SINGLETON_<Name>` — game-master object pointers.
- `NFSMW_VTBL_<Class>` — vtable addresses (HUD, pvehicle, AI).
- `NFSMW_HUD_SLOT_<Widget>` — `CHudWidgetArray` member offsets.
- `NFSMW_GEN_G_<Name>` / `NFSMW_GEN_F_<Fullname>` — the full
  machine-generated table (auto-generated, 28 globals + 153 functions).
- C++ typed accessor functions in `namespace nfsmw`:
  `*nfsmw::Tweak_InfiniteNOS()`, `*nfsmw::DrawHUD()`,
  `*nfsmw::Tweak_GameSpeed()`, etc.; generic `nfsmw::global<T>(addr)`.
  (These are functions, not C++17 inline variables, so the SDK works on
  C++14 — note the trailing `()`.)

## functions.h

- `NFSMW_FN_<Name>` — address constants for the curated, named functions
  (hash API, HUD, FNG bus, cop AI, save/load, Lua VM, …).
- `nfsmw::as_fn<Sig>(addr)` — cast an address to a callable pointer:

```cpp
auto setBreaker =
    nfsmw::as_fn<void(NFSMW_THISCALL*)(void*, char)>(
        NFSMW_FN_SetSpeedBreakerActive);
setBreaker(obj, 1);
```

## enums.h

- `NFSMW_<EnumName>` typedef enums generated from the SDK (65 enums,
  `ActionID`, `CARPART_LOD`, race modes, …). Collisions between
  same-named nested SDK enums are header-qualified.
- Aliases: `NFSMW_ACTION_GAMEBREAKER`, `NFSMW_ACTION_NITROUS`, etc.

## attributes.h

bChunk = Bob Jenkins 1996 mix3, seed `0xABCDEF00`.

- `nfsmw_bchunk(s)` / `nfsmw::bchunk(s)` — calls the engine's
  `StringToKey @ 0x454640` (exact engine behavior).
- `nfsmw_bchunk_inline(s)` / `nfsmw::bchunk_local(s)` — self-contained
  reimplementation; usable before the engine is up.
- `NFSMW_BCHUNK("STR")` — convenience macro.
- `NFSMW_ATTR_TYPE_*` — 7 verified attribute *type* hashes.
- `NFSMW_ATTR_<NAME>` — 294 verified attribute *name* hashes
  (auto-generated from `docs/attribute_cracks_verified.json`).
- `nfsmw_find_collection(class, name)` — wraps `FindCollection @
  0x455FD0`; hashes both args with bChunk.
- `nfsmw_collection_get_data(coll, key, index)` — wraps
  `Collection::GetData @ 0x454190`; returns a pointer to the live
  attribute value (write through it to mutate).
- Typed helpers (return 1 on success, 0 if absent):
  `nfsmw_attr_get_float/set_float`, `nfsmw_attr_get_int/set_int`.

```cpp
nfsmw_Collection *c = nfsmw_find_collection("pvehicle", "bmwm3gtre46");
float mass;
if (nfsmw_attr_get_float(c, NFSMW_BCHUNK("MASS"), &mass))
    nfsmw_attr_set_float(c, NFSMW_BCHUNK("MASS"), mass * 0.5f);
```

(See the `handling_tweaks` example for the full flow.)

## scan.h — signature scanner

Hardcoded `NFSMW_FN_*` addresses are exact for retail `speed.exe` but
break on repacked / patched / modded binaries. Locate code by byte
pattern instead:

- `nfsmw_aob_scan("8B 0D ?? ?? ?? ??")` / `nfsmw::aob(...)` — scan the
  main module image; `??` or `?` = wildcard byte. Returns the match
  address or `0`.
- `nfsmw_aob_scan_range(start, len, pattern)` — scan an explicit range.
- `nfsmw_resolve_rel32(at, instr_len)` / `nfsmw::resolve_rel32(at, n=5)`
  — turn an `E8`/`E9` rel32 at `at` into the absolute target.
- `nfsmw_main_module_range(&base, &size)` — the scanned image bounds.

```cpp
uintptr_t a = nfsmw::aob("E8 ?? ?? ?? ?? 8B 4C 24 10");
if (a) { auto fn = nfsmw::resolve_rel32(a); /* call target */ }
```

Pattern-matcher logic is covered by `tests/host_tests.py` (run in CI).

## hotkeys.h — runtime keybinds (opt-in)

Not pulled in by the umbrella header. `#include <nfsmw_sdk/hotkeys.h>`
explicitly; it adds a user32 dependency (the core SDK stays
kernel32-only). Edge-triggered (fires once per press) via a background
`GetAsyncKeyState` poll thread — works in menus/loading and on patched
executables.

- `nfsmw_hotkey_register(vk, cb, user)` — register a `VK_*` key; `cb` is
  `void(*)(void*)`. Returns 1, or 0 if the table is full.
- `nfsmw_hotkeys_start()` — start the poll thread (idempotent).
- `nfsmw::on_hotkey(vk, callable)` — C++ convenience for stateless
  lambdas; auto-starts the pump.
- Tunables: `NFSMW_HOTKEYS_MAX` (32), `NFSMW_HOTKEYS_POLL_MS` (30).

```cpp
#include <nfsmw_sdk/hotkeys.h>
nfsmw_hotkey_register(VK_F5, [](void*){ /* non-capturing */ }, nullptr);
nfsmw_hotkeys_start();
```

## hooks.h

C API:
- `nfsmw_vtable_hook(vtbl_addr, slot, new_fn, &orig)` — swap one vtable
  slot; restores protection.
- `nfsmw_jmp_detour(target, new_fn, &trampoline, sizeof_steal)` — 5-byte
  `E9` detour; returns a trampoline to call the original.
- `nfsmw_free_trampoline(t)` — release on unload.

C++ RAII:
- `nfsmw::VTableHook<Sig> h(vtbl, slot, &cb);` — `h.orig()`,
  `h.installed()`; auto-restores in destructor. Keep it `static` /
  long-lived.
- `nfsmw::JmpDetour<Sig> d(target, &cb);` — `d.trampoline()`.

```cpp
using Fn = void (NFSMW_THISCALL*)(void*, void*);
static nfsmw::VTableHook<Fn> hook(NFSMW_VTBL_AIVehicleHelicopter, 17, &cb);
```

## Return codes

| Constant | Value |
|----------|-------|
| `NFSMW_OK`   | `0`  |
| `NFSMW_FAIL` | `-1` |

## Address provenance

Every constant traces to the project RE database:
`docs/sdk_addrs.json`, `docs/attribute_cracks_verified.json`,
`docs/sdk_enums.json`. Regenerate the `_generated_*.h` tables with
`python3 sdk/tools/codegen.py`. CI guard: `--check`.

# Changelog

## Unreleased

### Added (struct field offsets — honest, partial)

- **`structs.h`** + `_generated_structs.h`: real `NFSMW_OFF_<S>_<f>` /
  `NFSMW_SIZEOF_<S>` for **79** no/single-inheritance engine structs.
  Offsets are compiler-computed from berkayylmao's NFSPluginSDK MW05
  type definitions (BSD-3) built with MSVC-layout-matching flags
  (`g++ -m32 -malign-double`); for these shapes the layout is
  ABI-invariant vs the retail MSVC-7.10 game. The **21**
  multiple/virtual-inheritance structs are opaque typedefs with offsets
  **withheld** (Itanium↔MSVC base placement can differ, no in-binary
  ground truth) — withheld, never guessed. `data/struct_offsets.json`
  vendored; codegen `--check` covers it. `struct_offsets_demo` example
  (12 total).

### Added / Fixed (events + CI)

- **`events.h`** (opt-in): subscribe to the global hashed gameplay
  event bus (`DAT_0091e0d0`) — `nfsmw_events_subscribe(name/hash,…)`,
  `nfsmw::events::on`, and `nfsmw::events::on_any` (snoop every event
  for discovery). Broadcast is passthrough-only by design (no
  fabricated event handles). React to race/pursuit/milestone/audio
  events with zero code patching. New `event_listener_demo` (11 total).
- **CI**: bumped `actions/checkout@v4` → `@v5` (Node 24); silences the
  Node 20 deprecation that would have started failing runs after
  2026-09-16.

### Added (v1.2 — full mod surface)

- **`iat_hook.h`** (opt-in): non-invasive import-table hooking
  (`nfsmw_iat_hook` / `nfsmw::IatHook`). Kernel32-only PE walk.
  `iat_hook_demo` example.
- **`midhook.h`** (opt-in, MinHook backend): mid-function
  register-context hook — `nfsmw::MidHook`, `nfsmw_regs`. Runtime-emitted
  capture thunk (no inline asm). `midhook_demo` example.
- **`input.h`** (opt-in): typed binding-template/mirror access +
  `nfsmw::input::on_poll` (per-frame, robust inline-hook on the action
  poller — read or inject input).
- **`lua.h`** (opt-in): register C functions as Lua 5.0.2 script natives
  — `nfsmw::lua::on_register_natives` + `nfsmw_lua_register`. Verified
  anchors shipped; the build-specific registrar is an overridable macro
  (`NFSMW_LUA_REGISTRAR`). `lua_native_demo` example.
- `docs/CAPABILITIES.md` updated: all v1.2 + the previously "deferred"
  Lua item flipped to ✅; "bottom line" reflects the full mod surface.

### Added (v1.1 — robust hooking)

- **Vendored MinHook** (`extern/minhook/`, © Tsuda Kageyu, BSD-2) as the
  default inline-hook backend. `nfsmw::InlineHook<Sig>` /
  `nfsmw_inline_hook` now hook **any** function prologue reliably
  (instruction relocation + trampoline + safe enable/disable) instead
  of the 5-byte-E9-only `JmpDetour`. Selectable
  `-DNFSMW_HOOKS_BACKEND=minhook|simple` (default `minhook`);
  `nfsmw_add_plugin()` compiles/links MinHook automatically, install +
  `find_package` ship and resolve it.
- New `inline_hook` example (2x bounty via `AwardPlayerBounty_Impl`).
- `docs/CAPABILITIES.md` — honest can/can't matrix + BepInEx framing
  correction + roadmap.
- **`d3d9_hooks.h`** (opt-in): hook the D3D9 render loop —
  `nfsmw_d3d9_install` / `nfsmw::on_endscene` (+ Reset). Self-contained
  COM-vtable hook, **no d3d9.lib**. New `overlay_demo` example draws a
  bar every frame with zero external deps. Unlocks overlays / ImGui /
  custom HUD.

### Fixed (CI)

- **codegen.py / sdk/data**: vendored the source JSON
  (`sdk_addrs.json`, `attribute_cracks_verified.json`, `sdk_enums.json`)
  into `sdk/data/`. The standalone repo previously had no source for
  `codegen.py --check`, so CI failed every run (headers reported stale).
  codegen now reads `sdk/data/` first and falls back to `../../docs/`
  for in-place main-repo regen. Verified the vendored JSON yields
  byte-identical generated headers.
- **CI**: added a downstream `find_package(nfsmw_sdk CONFIG)` consumer
  build step (catches helpers-path / export regressions the
  install-only step misses).
- **scan.h / API_REFERENCE**: documented that `nfsmw_aob_scan` covers
  the whole image (.text/.rdata/.data) — use long, code-distinctive
  patterns.

### Fixed (correctness — P0)

- **entry.c**: removed `wsprintfA` (user32 dependency that broke the MSVC
  link path; also a banned/unsafe API). Startup line is now emitted via
  plain `OutputDebugStringA` calls.
- **platform.h `nfsmw_write_bytes`**: now checks `VirtualProtect` and
  returns `int` (0 = page could not be made writable, nothing written).
  Previously an unchecked failure led to an access violation in `memcpy`.
  All `nfsmw_write_byte/word/int/float` now return `int` too.
- **platform.h / hooks.h**: added `FlushInstructionCache` after writing to
  executable memory (`nfsmw_write_bytes`, `nfsmw_jmp_detour`), as required
  by the Win32 contract when another thread may execute the patched code.
- **platform.h `nfsmw_validate_speed_exe`**: added a case-insensitive
  "ends with speed.exe" filename gate on top of the image-base check, to
  reject repacked / wrapped binaries that share the base.
- **functions.h**: removed the `0x0u` placeholder constants
  `NFSMW_FN_FindSceneNodeByName` / `NFSMW_FN_RemoveSceneNodeByName`.
  They are now undefined, so use fails to compile instead of calling
  address 0.

### Changed (source-compatibility — ACTION REQUIRED)

- **globals.h**: the `nfsmw::` typed globals are now **accessor
  functions**, not C++17 `inline` variables. This makes the SDK usable on
  C++14 and removes ODR concerns.

  Migration — add `()`:

  ```diff
  - *nfsmw::Tweak_InfiniteNOS = true;
  + *nfsmw::Tweak_InfiniteNOS() = true;
  ```

  The `NFSMW_GLOBAL_*` address macros are unchanged; mods that use those
  directly need no edits.

### Added

- **entry.c**: `NFSMW_ASI_INIT_DELAY_MS` compile-time override for the
  ASI worker startup delay (default 2000 ms).
- **CMake**: `find_package(nfsmw_sdk CONFIG)` support — ships
  `nfsmw_sdkConfig`, version file, installed headers + `entry.c` +
  shared `nfsmw_sdkHelpers.cmake`. Consumers get `nfsmw_add_plugin()`
  and the `nfsmw::sdk` target with zero hardcoded paths.
- **CI**: GitHub Actions — `codegen --check`, host unit tests,
  cross-build all examples, i386 verification, install/find_package
  smoke test, and a C++14 build job.
- **scan.h**: IDA-style AOB signature scanner (`nfsmw_aob_scan`,
  `nfsmw_aob_scan_range`, `nfsmw_resolve_rel32`,
  `nfsmw_main_module_range`; `nfsmw::aob` / `nfsmw::resolve_rel32`).
  Lets mods locate code by byte pattern instead of hardcoded addresses,
  surviving patched/repacked executables.
- **tests/host_tests.py**: native unit tests for the
  platform-independent logic (bChunk known-value + AOB matcher),
  wired into CI.
- **attributes.h**: real attribute get/set via `Collection::GetData @
  0x454190` — `nfsmw_collection_get_data` plus typed
  `nfsmw_attr_get/set_float` and `_int`. The `handling_tweaks` example
  now actually reads and halves the player car's MASS end-to-end
  instead of pointing at a doc.
- **functions.h**: `NFSMW_FN_CollectionGetData`,
  `NFSMW_FN_ClassGetCollection`, `NFSMW_FN_ClassGetNumCollections`.
- **hotkeys.h** (opt-in): edge-triggered keyboard hotkeys via a
  background poll thread — `nfsmw_hotkey_register`,
  `nfsmw_hotkeys_start`, `nfsmw::on_hotkey`. Adds user32 only for mods
  that include it; core SDK stays kernel32-only. New `hotkey_toggles`
  example (F5/F6/F7 cheat toggles).

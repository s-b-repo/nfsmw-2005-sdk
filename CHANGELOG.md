# Changelog

## Unreleased

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

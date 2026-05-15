# Changelog

## Unreleased

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

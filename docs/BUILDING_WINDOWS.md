# Building nfsmw_sdk mods on Windows

Two supported toolchains. MSVC is the most native; MinGW matches the
Linux/Mac path exactly.

## Option A — MSVC (Visual Studio 2019/2022)

NFSMW is 32-bit, so you must target **x86**, not x64.

1. Install "Desktop development with C++" + CMake.
2. From an **x86 Native Tools** developer prompt:

```cmd
cd sdk
cmake -S . -B build -A Win32
cmake --build build --config Release
```

`-A Win32` forces the 32-bit target. The SDK's `platform.h` will
`#error` if a 64-bit target slips through.

Single-file build:
```cmd
cl /LD /O2 /MT /EHsc /I sdk\include ^
   mymod.cpp sdk\src\entry.c /Fe:mymod.dll
copy mymod.dll mymod.asi
```

## Option B — MinGW-w64 (i686)

Install via [MSYS2](https://www.msys2.org/):
```bash
pacman -S mingw-w64-i686-gcc mingw-w64-i686-cmake
```
Then from the **MINGW32** shell, follow the exact commands in
[BUILDING_LINUX.md](BUILDING_LINUX.md) §2–§3.

## Installing

- ASI: copy `mymod.asi` to `<NFSMW>\scripts\` (needs Ultimate-ASI-Loader's
  `dinput8.dll` next to `speed.exe`).
- BepInEx: copy `mymod.dll` to `<NFSMW>\BepInEx\plugins\` — see
  [BEPINEX_INTEGRATION.md](BEPINEX_INTEGRATION.md).

## Gotchas

- The original retail `speed.exe` is i386. A 64-bit DLL will not load.
  Always verify: `dumpbin /headers mymod.dll | findstr machine` → `14C`
  (x86), not `8664`.
- Keep `/MT` (static CRT) so users don't need the VC++ redistributable.
- MSVC name-decoration: the SDK already applies `extern "C"` to all
  loader exports; no `.def` file is required.

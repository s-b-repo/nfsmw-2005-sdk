# Building nfsmw_sdk mods on Linux

The mod is a 32-bit Windows DLL, so you cross-compile with **MinGW-w64
(i686)**. No Wine needed to *build*; you only need Wine (or Windows) to
*run* NFSMW.

## 1. Install the toolchain

Arch / Manjaro:
```bash
sudo pacman -S mingw-w64-gcc cmake
```
Debian / Ubuntu:
```bash
sudo apt install gcc-mingw-w64-i686 g++-mingw-w64-i686 cmake
```
Fedora:
```bash
sudo dnf install mingw32-gcc mingw32-gcc-c++ cmake
```

Verify:
```bash
i686-w64-mingw32-g++ --version
```

## 2. Build with CMake

```bash
cd sdk
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/nfsmw-toolchain-mingw-i686.cmake
cmake --build build -j
```

Outputs land in `build/examples/`:
- `<mod>.dll`  → BepInEx 6 native plugin
- `<mod>.asi`  → Ultimate-ASI-Loader (identical bytes, renamed)

## 3. Build a single file without CMake

```bash
i686-w64-mingw32-g++ -std=c++17 -m32 -Os -shared \
  -static -static-libgcc -static-libstdc++ -Wl,--kill-at \
  -Isdk/include \
  mymod.cpp sdk/src/entry.c -o mymod.dll
cp mymod.dll mymod.asi
```

`-Wl,--kill-at` strips the `@N` stdcall decoration so the ASI loader and
BepInEx see clean export names. `-static*` removes the libgcc/libstdc++
DLL dependency so the mod is a single self-contained file.

## 4. Install for testing under Wine

ASI route (recommended for quick iteration):
```
cp mymod.asi  "$WINEPREFIX/.../Need for Speed Most Wanted/scripts/"
```
(Requires Ultimate-ASI-Loader's `dinput8.dll` already in the game dir —
this RE project ships one in `mods/`.)

BepInEx route: see [BEPINEX_INTEGRATION.md](BEPINEX_INTEGRATION.md).

## Notes

- A native-clang IDE will flag `#error "...Win32 PE only..."` and
  `windows.h not found` in the headers. That is **expected** — the SDK
  intentionally refuses to compile for non-Win32 targets. Only the MinGW
  cross build is supported; the IDE squiggles are not real build errors.
- `-Wattributes` / "initialized and declared extern" warnings from the
  `NFSMW_PLUGIN_DECLARE` macro are benign — that is the correct idiom for
  dllexported C symbols.

# BepInEx Template for NFSMW (2005)

NFSMW is a **native Win32** game (no Mono/.NET runtime), so it uses
**BepInEx 6 with the NativeBootstrap (BepInEx.NativePass / Doorstop)**.

## Layout this template produces

```
<NFSMW install>/
├── speed.exe
├── winhttp.dll              ← BepInEx Doorstop proxy (from BepInEx 6 native pack)
├── doorstop_config.ini      ← points Doorstop at the native bootstrap
├── BepInEx/
│   ├── core/                ← BepInEx 6 native core DLLs
│   └── plugins/
│       └── infinite_nos.dll ← your nfsmw_sdk plugin (one DLL, dual loader)
```

## Install steps

1. Download **BepInEx 6 (BepInEx_x86)** — the *native* x86 build, NOT the
   Unity/IL2CPP build. NFSMW is x86 (32-bit), so the x86 pack is required.
2. Extract the pack into the NFSMW install folder (next to `speed.exe`).
   This places `winhttp.dll` + `doorstop_config.ini` + `BepInEx/`.
3. NFSMW does **not** import `winhttp.dll` by default. Two options:
   - **A (recommended):** Use the `dinput8.dll` proxy variant of Doorstop
     (rename the proxy to `dinput8.dll`). NFSMW *does* import `dinput8.dll`.
   - **B:** Add `winhttp.dll` to NFSMW's import table with a PE editor.
   Set `doorstop_config.ini` → `[General] enabled=true`.
4. Build your plugin (the unified `src/entry.c` makes every DLL work
   under **both** loaders automatically — no loader flag):
   ```
   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/nfsmw-toolchain-mingw-i686.cmake -B build
   cmake --build build --target infinite_nos
   ```
   Or grab a pre-built DLL from the repo's Releases.
5. Copy `build/examples/infinite_nos.dll` → `BepInEx/plugins/`.
6. Launch `speed.exe`. Check `BepInEx/LogOutput.log` and DebugView for the
   `[nfsmw_sdk] BepInEx plugin loading` line.

## Why both ASI *and* BepInEx?

The same DLL can export both `BepInExNativePlugin_Load` (BepInEx) and a
`DllMain` worker thread (ASI). Users pick whichever loader they already have.
There is no loader flag — `src/entry.c` always emits both entry points,
so one built DLL works in either loader.

See `../docs/BEPINEX_INTEGRATION.md` for troubleshooting.

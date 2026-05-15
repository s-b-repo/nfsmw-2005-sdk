# nfsmw_sdk — Need for Speed: Most Wanted (2005) Mod SDK

![CI](https://github.com/s-b-repo/nfsmw-2005-sdk/actions/workflows/ci.yml/badge.svg)

A header-only C/C++ SDK for writing native mods for **NFSMW (2005)
`speed.exe`** (PE32, i386, image base `0x00400000`). Build the mod source
from **Linux, Windows, or macOS**; the output is always a 32-bit Win32 DLL
that loads under **both** the Ultimate-ASI-Loader **and** BepInEx 6.

This SDK is the modder-facing front end of the
[nfsmw-2005-re](https://github.com/s-b-repo/nfsmw-2005-re) reverse-engineering
project. Addresses come from the project's verified Ghidra database
(`docs/sdk_addrs.json`, `docs/attribute_cracks_verified.json`,
`docs/sdk_enums.json`) and the BSD-3 NFSPluginSDK by berkayylmao.

## Why "works with BepInEx" needs a note

NFSMW is a **native Win32 game** — there is no Mono/.NET/IL2CPP runtime.
BepInEx 6's managed plugin model does not apply. What *does* work is
**BepInEx 6 NativeBootstrap (Doorstop)**: BepInEx injects, then loads
*native* DLLs that export `BepInExNativePlugin_Load`. This SDK emits that
export **and** a classic ASI `DllMain` worker from a single shared
`entry.c`, with a one-shot interlock so the plugin runs exactly once no
matter which loader fires. One DLL, both ecosystems.

See [`docs/BEPINEX_INTEGRATION.md`](docs/BEPINEX_INTEGRATION.md).

## Layout

```
sdk/
├── include/nfsmw_sdk/      header-only SDK
│   ├── nfsmw_sdk.h         umbrella + NFSMW_PLUGIN_DECLARE/MAIN macros
│   ├── platform.h          target detection, mem read/write, typed sugar
│   ├── globals.h           typed global pointers + NFSMW_GLOBAL_* macros
│   ├── functions.h         function address constants + typedef helpers
│   ├── enums.h             engine enums
│   ├── attributes.h        bChunk (Jenkins mix3) hash + Collection get/set
│   ├── hooks.h             vtable / inline (MinHook) / JMP detour
│   ├── scan.h              AOB signature scanner
│   ├── structs.h           verified struct field offsets (opt-in)
│   ├── d3d9_hooks.h        D3D9 EndScene/Reset render hook (opt-in)
│   ├── iat_hook.h          import-table hook (opt-in)
│   ├── midhook.h           mid-function register hook (opt-in)
│   ├── input.h             action-binding access + poll hook (opt-in)
│   ├── lua.h               register Lua 5.0.2 script natives (opt-in)
│   ├── events.h            global hashed event bus (opt-in)
│   ├── hotkeys.h           runtime keybinds (opt-in)
│   └── _generated_*.h      AUTO-GENERATED — do not edit
├── src/entry.c             unified ASI + BepInEx entry shim
├── extern/minhook/         vendored MinHook backend (BSD-2)
├── data/*.json             codegen sources (addrs/attrs/enums/structs)
├── examples/               12 buildable example mods
├── bepinex-template/       drop-in BepInEx install skeleton
├── cmake/                  MinGW-w64 i686 cross toolchain + pkg config
├── tools/codegen.py        regenerates _generated_*.h from data/*.json
├── tests/host_tests.py     host unit tests (bChunk + AOB), run in CI
└── CMakeLists.txt          cross-platform build + nfsmw_add_plugin()
```

## Pre-built mods (no toolchain needed)

Don't want to build? Every tagged release ships the 12 example mods
pre-compiled (PE32 i386, dual ASI + BepInEx) — see
**[Releases](https://github.com/s-b-repo/nfsmw-2005-sdk/releases)**
(`nfsmw-sdk-examples-<tag>.zip`, built by CI from the tag). Drop a
`.asi` into `<NFSMW>/scripts/` or a `.dll` into
`<NFSMW>/BepInEx/plugins/`. The repo itself stays source-only; binaries
live on Releases by design.

## Quick start

```bash
# from any host (Linux/macOS/Windows) with i686-w64-mingw32 installed:
cd sdk
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/nfsmw-toolchain-mingw-i686.cmake
cmake --build build
# -> build/examples/infinite_nos.dll  (BepInEx)  + infinite_nos.asi (ASI)
```

Drop `infinite_nos.asi` into `<NFSMW>/scripts/` (ASI Loader) **or**
`infinite_nos.dll` into `<NFSMW>/BepInEx/plugins/` (BepInEx 6 native).

## Use it in your own project

Install once, then consume via `find_package` — no hardcoded paths:

```bash
cmake -S sdk -B sdk/build -DNFSMW_BUILD_EXAMPLES=OFF \
  -DCMAKE_TOOLCHAIN_FILE=sdk/cmake/nfsmw-toolchain-mingw-i686.cmake \
  -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build sdk/build --target install
```

```cmake
# your mod's CMakeLists.txt
find_package(nfsmw_sdk CONFIG REQUIRED)
nfsmw_add_plugin(my_mod SOURCES my_mod.cpp)   # -> my_mod.dll + my_mod.asi
```

(Or just vendor/submodule the repo and `add_subdirectory(sdk)`.)

## Minimal mod

```cpp
#include <nfsmw_sdk/nfsmw_sdk.h>

NFSMW_PLUGIN_DECLARE("My Mod", "1.0.0", "me")

NFSMW_PLUGIN_MAIN() {
    *nfsmw::Tweak_InfiniteNOS() = true;
    return NFSMW_OK;
}
```

## Build docs

- [`docs/BUILDING_LINUX.md`](docs/BUILDING_LINUX.md)
- [`docs/BUILDING_WINDOWS.md`](docs/BUILDING_WINDOWS.md)
- [`docs/BUILDING_MAC.md`](docs/BUILDING_MAC.md)
- [`docs/BEPINEX_INTEGRATION.md`](docs/BEPINEX_INTEGRATION.md)
- [`docs/API_REFERENCE.md`](docs/API_REFERENCE.md)
- [`docs/CAPABILITIES.md`](docs/CAPABILITIES.md) — **what you can/can't
  hook & modify** + how BepInEx actually relates to this SDK + roadmap
- [`docs/COOKBOOK.md`](docs/COOKBOOK.md) — **copy-pasteable recipes**
  composing the primitives (incl. struct-offset field edits) + an
  honest "why-not" list

## Regenerating address tables

`tools/codegen.py` rebuilds `_generated_addrs.h`, `_generated_attrs.h`,
`_generated_enums.h` from the vendored JSON in **`sdk/data/`** — the
SDK's self-contained source of truth (so CI can verify the headers).

```bash
python3 sdk/tools/codegen.py            # regenerate
python3 sdk/tools/codegen.py --check    # CI guard: nonzero if stale
```

When the upstream RE database (`nfsmw-2005-re/docs/`) changes, re-sync
and regenerate:

```bash
cp ../nfsmw-2005-re/docs/{sdk_addrs,attribute_cracks_verified,sdk_enums}.json sdk/data/
python3 sdk/tools/codegen.py
```

(Running inside the main RE-project tree, `codegen.py` falls back to
`../../docs/` automatically if `sdk/data/` is absent.)

## Legal

BSD-3-Clause. This SDK ships **no copyrighted EA game data** — only
addresses/offsets derived from clean-room RE for interoperability. The
mirrored NFSPluginSDK retains berkayylmao's BSD-3 license
(`docs/nfsplugin_sdk_mw05/LICENSE`). The vendored MinHook backend
(`extern/minhook/`) is © Tsuda Kageyu, BSD-2-Clause
(`extern/minhook/LICENSE.txt`) — compatible with this SDK's BSD-3. The
struct field offsets in `data/struct_offsets.json` / `structs.h` are
compiler-derived from berkayylmao's NFSPluginSDK MW05 type definitions
(BSD-3, `docs/nfsplugin_sdk_mw05/LICENSE`); only the resulting offset
table is shipped, not his source. You must own a legitimate copy of
NFSMW to use any mod built with this SDK.

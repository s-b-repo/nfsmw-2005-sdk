# nfsmw_sdk capability matrix — what you can and can't mod

## Read this first: how BepInEx relates to this SDK

A common misconception: *"BepInEx should expose a native C++ plugin base
class like it does for Unity, and the SDK should plug into that."*

That model does not exist for NFSMW, and **cannot**, because:

- BepInEx's `BaseUnityPlugin` / `BasePlugin` exist because Unity/IL2CPP/
  .NET games ship a **managed runtime** BepInEx can host plugins inside.
- **NFSMW (2005) is a native Win32 C++ game. There is no managed
  runtime.** For native games BepInEx is *only* Doorstop — an injector
  that gets a DLL into the process. Nothing more.

So the division of labour is:

| Layer | Role | For Unity games | For NFSMW (native) |
|-------|------|-----------------|--------------------|
| **BepInEx / Doorstop** | Get code into the process | injector + managed plugin host | injector **only** |
| **The SDK** | The actual modding API (hook, patch, scan, read/write game state) | (BepInEx's managed API does this) | **`nfsmw_sdk` does this** |

**`nfsmw_sdk` *is* the native equivalent of `BaseUnityPlugin` for this
game.** There is no other native-C++ plugin contract in BepInEx to
conform to beyond the Doorstop entry point — which the SDK already
exports (`BepInExNativePlugin_Load`), alongside an ASI `DllMain` so the
same DLL also works under Ultimate-ASI-Loader.

In short: **BepInEx provides the foothold; this SDK provides everything
you'd actually do with it.**

## The matrix

Legend: ✅ supported · ⚠️ supported with caveats · 🔜 planned (v1.1/v1.2) ·
❌ not possible / out of scope

### Code execution & hooking

| Capability | Status | How / limitation |
|---|---|---|
| Run code at game start | ✅ | `NFSMW_PLUGIN_MAIN`; one-shot, post-image-load, validated `speed.exe` |
| Replace a C++ virtual method | ✅ | `nfsmw::VTableHook<Sig>` — swap any vtable slot, chain to original |
| Detour a free function (clean ≥5-byte prologue) | ✅ | `nfsmw::JmpDetour` (simple 5-byte) or — recommended — `nfsmw::InlineHook` |
| Hook **any** function reliably (incl. tiny/relative prologues) | ✅ | `nfsmw::InlineHook<Sig>` — vendored **MinHook** backend (default CMake build); relocates prologue instructions, safe enable/disable |
| Mid-function hook (hook an arbitrary instruction, not just entry) | 🔜 v1.2 | Needs the trampoline engine; roadmap |
| Hook imported API calls (D3D9, file IO, Win32) | 🔜 v1.2 | IAT hook helper; roadmap |
| Hook the D3D9 render loop (overlays, ImGui, custom HUD) | 🔜 v1.1 | `<nfsmw_sdk/d3d9_hooks.h>` → `nfsmw::on_endscene` / `on_reset` |
| Unhook / restore | ✅ | RAII wrappers restore on destruction; MinHook backend adds safe disable |
| Thread-safe hook install while game runs | ⚠️ | NFSMW game code is effectively single-threaded; MinHook suspends threads on (un)install but uninstalling while a thread is *in* your trampoline is still UB |

### Memory & data

| Capability | Status | How / limitation |
|---|---|---|
| Read/write any process memory | ✅ | `nfsmw_write_bytes/byte/word/int/float` (page-protect-safe, icache-flushed, checked) |
| Patch code bytes (nop, force jump, etc.) | ✅ | Same write helpers |
| Typed access to 28 documented globals | ✅ | `nfsmw::DrawHUD()` etc.; `NFSMW_GLOBAL_*` macros for all |
| Call ~150 documented engine functions | ✅ | `NFSMW_FN_*` + `nfsmw::as_fn<Sig>()`; 153 auto-generated from RE DB |
| Locate code/data by signature (patched/cracked exe) | ✅ | `nfsmw::aob("48 8B ?? ??")`, `resolve_rel32` |
| Read/write vehicle & game attributes by name | ✅ | `nfsmw_find_collection` + `nfsmw_attr_get/set_float/int` (`Collection::GetData @ 0x454190`) |
| 294 verified attribute name hashes + bChunk | ✅ | `NFSMW_ATTR_*`, `NFSMW_BCHUNK("MASS")` (Jenkins mix3, exact) |
| 65 engine enums | ✅ | `NFSMW_<Enum>_<Member>` |

### Input & UX

| Capability | Status | How / limitation |
|---|---|---|
| Runtime keyboard hotkeys / toggles | ✅ | opt-in `<nfsmw_sdk/hotkeys.h>` (edge-triggered poll thread) |
| Draw an on-screen overlay / debug UI | 🔜 v1.1 | via the D3D9 EndScene hook (ImGui-ready) |
| Read the game's own input bindings | ⚠️ | Binding table mapped in RE DB (`0x008f6d80`); no typed wrapper yet — use raw addresses |
| Managed config UI (BepInEx `ConfigEntry` style) | ❌ | No managed runtime — native game. Use your own INI/file, or hotkeys |

### Distribution & build

| Capability | Status | How |
|---|---|---|
| One DLL loads under ASI **and** BepInEx native | ✅ | Unified `entry.c`, one-shot interlock |
| Build from Linux / Windows / macOS | ✅ | MinGW-w64 i686 cross toolchain; CMake |
| Consume via `find_package(nfsmw_sdk CONFIG)` | ✅ | Ships Config + helpers + headers + entry.c |
| C++14 / C++17 / C / MSVC / GCC / Clang | ✅ | Header-only core, kernel32-only |
| Hot-reload a plugin without restarting the game | ❌ | Not supported (native injection model) |
| Automatic plugin load-ordering / dependency resolution | ❌ | Mods load in loader-discovery order; no manifest graph |

### Hard limits (won't change)

| | Why |
|---|---|
| ❌ 64-bit | retail `speed.exe` is i386 PE32 |
| ❌ Other NFS titles | addresses are MW-2005-specific |
| ❌ Online/anticheat bypass | out of scope; single-player RE for interop only |
| ❌ Redistribute game assets | legal — SDK ships addresses only |

## Roadmap

**v1.1 — done:**
- ✅ Vendored **MinHook** (BSD-2) detour backend → `nfsmw::InlineHook`
  hooks *any* function prologue reliably. Selectable
  `-DNFSMW_HOOKS_BACKEND=minhook|simple` (default `minhook`).

**v1.1 — remaining:**
- `<nfsmw_sdk/d3d9_hooks.h>` — `nfsmw::on_endscene` / `on_reset` +
  `overlay_demo` example. Unlocks overlays / ImGui / custom rendering.

**v1.2 (planned):**
- IAT (import) hook helper.
- Typed wrapper over the game input-binding table.
- Mid-function hook helper (on top of the trampoline engine).

**Deferred (needs a real use case):**
- Native registration of new Lua script functions (the 5.0.2 VM is
  mapped, but exposing C to scripts is large design surface — auth,
  sandboxing — and there's no demand yet).

## Bottom line

For a native Win32 game, "can I mod anything?" reduces to "can I hook
any function and read/write any memory?" Today: **any vtable method, any
memory, any documented function, any attribute, any clean-prologue
function.** After v1.1 (MinHook + D3D9): **any function, plus the render
loop** — which is the practical ceiling for native game modding. The
remaining ❌ items are inherent to a no-managed-runtime native game, not
SDK shortcomings.

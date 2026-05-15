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
| Mid-function hook (hook an arbitrary instruction, not just entry) | ✅ | `nfsmw::MidHook` (`midhook.h`) — runtime-emitted register-capture thunk + MinHook trampoline; read/modify CPU regs, resume. Needs MinHook backend |
| Hook imported API calls (D3D9, file IO, Win32) | ✅ | `nfsmw_iat_hook` (`iat_hook.h`) — non-invasive IAT thunk swap, kernel32-only. (Doesn't catch GetProcAddress/cached ptrs — inline-hook those) |
| Hook the D3D9 render loop (overlays, ImGui, custom HUD) | ✅ | opt-in `<nfsmw_sdk/d3d9_hooks.h>` → `nfsmw_d3d9_install` / `nfsmw::on_endscene` (+ Reset); self-contained vtable hook, no d3d9.lib |
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
| Typed engine struct field offsets | ✅ (partial, honest) | `structs.h` — `NFSMW_OFF_<S>_<f>` + `NFSMW_SIZEOF_<S>` for **79** no/single-inheritance structs (compiler-derived from berkayylmao's NFSPluginSDK type defs under MSVC-matching flags; ABI-invariant vs the retail game). **21** multiple/virtual-inheritance structs are opaque typedefs with offsets withheld (Itanium↔MSVC base placement can differ; no in-binary ground truth). No fabricated numbers |

### Input & UX

| Capability | Status | How / limitation |
|---|---|---|
| Runtime keyboard hotkeys / toggles | ✅ | opt-in `<nfsmw_sdk/hotkeys.h>` (edge-triggered poll thread) |
| Draw an on-screen overlay / debug UI | ✅ | via the D3D9 EndScene hook (ImGui-ready); `overlay_demo` example draws every frame with zero external deps |
| Read / override the game's own input | ✅ | `input.h` — `nfsmw_input_binding_row(action)` to inspect/rebind; `nfsmw::input::on_poll(cb)` runs each frame right after the engine refreshes action state (read or inject) |
| Add a C function callable from game scripts | ✅ | `lua.h` — `nfsmw::lua::on_register_natives` + `nfsmw_lua_register` (vanilla Lua 5.0.2). One build-specific registrar address (`NFSMW_LUA_REGISTRAR`) resolved per target |
| React to engine gameplay events (race finish, pursuit enter/over, milestone, reputation, audio cues) | ✅ | `events.h` — `nfsmw_events_subscribe("MPursuitOver", cb, ctx)` / `nfsmw::events::on(name, …)` on the global hashed bus; `on_any` snoops every event for discovery |
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

**v1.1 — done:**
- ✅ `<nfsmw_sdk/d3d9_hooks.h>` — `nfsmw::on_endscene` / Reset +
  `overlay_demo`. Overlays / ImGui / custom rendering.

**v1.2 — done:**
- ✅ IAT (import) hook helper — `iat_hook.h`.
- ✅ Typed input-binding access + per-frame poll hook — `input.h`.
- ✅ Mid-function (register-context) hook — `midhook.h`.
- ✅ Lua 5.0.2 native registration — `lua.h` (was "deferred"; shipped
  as a mechanism + verified anchors; the one build-specific registrar
  address is an overridable macro, by design — no guessed addresses).

**Struct layouts — done (partial, honest):** `structs.h` ships
compiler-derived field offsets for the 79 ABI-invariant structs; the 21
multiple/virtual-inheritance ones stay opaque (no certifiable offsets).
The remaining ~145 `sdk_structs.json` names are nested/other-namespace
and were excluded rather than guessed.

**Roadmap (future, on demand):** ImGui drop-in helper; INI/config
helper; automatic signature-backed address resolution for the
`NFSMW_FN_*` table; offsets for the MI structs if/when extracted from
the binary (the only authoritative source for those).

## Bottom line

For a native Win32 game, "can I mod anything?" reduces to "can I hook
any function and read/write any memory?" The SDK now does: **any vtable
method, any function entry (MinHook), any *instruction* (mid-hook with
register context), any imported API (IAT), the D3D9 render loop, any
memory, any documented function, any attribute, typed struct fields (79
ABI-invariant structs), the input layer, and new Lua script natives.**
That is the practical ceiling for native game
modding — the remaining ❌ items are inherent to a no-managed-runtime
native game, not SDK shortcomings.

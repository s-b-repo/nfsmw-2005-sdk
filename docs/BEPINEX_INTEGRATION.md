# BepInEx 6 integration for NFSMW (2005)

## The key fact

NFSMW is a **native Win32 C++ game**. It has **no** Mono, .NET, or
IL2CPP runtime. Therefore:

- BepInEx's **managed (C#) plugin** model does **not** apply.
- You use **BepInEx 6 with the NativeBootstrap / Doorstop** injector,
  which loads **native** DLLs that export `BepInExNativePlugin_Load`.

`nfsmw_sdk` builds exactly that export (plus a fallback ASI `DllMain`
worker) from `sdk/src/entry.c`. A one-shot `InterlockedCompareExchange`
guard makes the plugin body run exactly once even if both an ASI loader
and BepInEx are installed simultaneously.

## Which BepInEx package

Download **BepInEx 6 (bleeding-edge), the `x86` *native* build** — NOT
the Unity/IL2CPP variant. NFSMW's `speed.exe` is 32-bit, so the x86 pack
is mandatory; the x64 pack will silently fail to inject.

## Injection: NFSMW does not import winhttp.dll

BepInEx 6 Doorstop normally ships as `winhttp.dll` and relies on the game
importing WinHTTP. `speed.exe` does **not**. Two fixes:

### Option A (recommended) — dinput8 proxy chain

NFSMW *does* import `dinput8.dll`. Rename the Doorstop proxy:

```
<NFSMW>/
├── speed.exe
├── dinput8.dll          ← Doorstop proxy (renamed from winhttp.dll)
├── doorstop_config.ini  ← enabled=true
└── BepInEx/
    ├── core/
    └── plugins/
        └── yourmod.dll
```

If you already use Ultimate-ASI-Loader (which also claims `dinput8.dll`),
pick **one** loader — don't chain two proxies on the same name. In that
case prefer the ASI route for that DLL and skip BepInEx, or use Option B.

### Option B — patch the import table

Use a PE editor (CFF Explorer / `LIEF`) to add `winhttp.dll` to
`speed.exe`'s import directory, then keep the stock BepInEx `winhttp.dll`.
More invasive; only if you can't use the dinput8 proxy.

## doorstop_config.ini

NFSMW is neither Mono nor IL2CPP — leave those sections empty. The
template in `sdk/bepinex-template/doorstop_config.ini` is preconfigured:

```ini
[General]
enabled=true
[Logging]
debug_output=true
```

## Install steps

1. Extract the BepInEx 6 x86 native pack next to `speed.exe`.
2. Rename `winhttp.dll` → `dinput8.dll` (Option A).
3. Drop your built `yourmod.dll` into `BepInEx/plugins/`.
4. Launch. Confirm in `BepInEx/LogOutput.log` and via DebugView:
   `[nfsmw_sdk] plugin starting via BepInEx`.

## Dual-loader behavior

| Loader present            | What runs the plugin            |
|---------------------------|---------------------------------|
| ASI loader only           | `DllMain` worker thread (2 s)   |
| BepInEx native only       | `BepInExNativePlugin_Load`      |
| Both                      | Whichever fires first; the      |
|                           | interlock blocks the second     |

Build once with the default CMake target — the same DLL satisfies all
three rows. Copy it as `.asi` for ASI, leave it `.dll` for BepInEx.

## Troubleshooting

- **Nothing loads**: wrong arch. `file yourmod.dll` must say *Intel
  80386*. The BepInEx pack must be x86 too.
- **`BepInExNativePlugin_Load` not found**: the export got C++-mangled.
  Always link `sdk/src/entry.c` (it forces C linkage); don't reimplement
  the export in a `.cpp` without `extern "C"`.
- **Plugin runs twice**: you linked two entry shims. Use only
  `sdk/src/entry.c`.
- **Crash at startup**: the SDK's `nfsmw_validate_speed_exe()` checks
  image base `0x00400000`; a mismatch means it's not retail `speed.exe`
  (repacked/packed exe) — the plugin refuses rather than corrupt memory.

/*
 * nfsmw_sdk/platform.h — host-OS detection + Win32 PE32 entry-point macros.
 *
 * NFSMW is a 32-bit Win32 PE binary. The mod target is ALWAYS a Win32 i386 DLL,
 * regardless of the host OS where the source is compiled. This header makes
 * cross-compilation explicit:
 *
 *   - Linux host:   i686-w64-mingw32-gcc / clang -target i686-w64-windows-gnu
 *   - Windows host: cl /arch:IA32, or i686-w64-mingw32-gcc
 *   - macOS host:   `brew install mingw-w64`, then i686-w64-mingw32-gcc
 *
 * Loading mechanisms supported:
 *   - ASI plugin (Ultimate-ASI-Loader via app/dinput8.dll), output renamed .asi
 *   - BepInEx 6 Native plugin (drop into the BepInEx plugins folder)
 *   - Direct DLL injection (CreateRemoteThread / LoadLibraryA)
 */

#ifndef NFSMW_SDK_PLATFORM_H
#define NFSMW_SDK_PLATFORM_H

/* ----- Target check: must produce 32-bit Win32 PE ----- */
#if !defined(_WIN32)
#  error "nfsmw_sdk targets Win32 PE only. Cross-compile to i686-w64-mingw32."
#endif

#if defined(__x86_64__) || defined(_M_X64)
#  error "nfsmw_sdk requires a 32-bit build (NFSMW is i386). Use -m32 or i686-w64-mingw32."
#endif

/* ----- Compiler-portable PE export macro ----- */
#if defined(_MSC_VER)
#  define NFSMW_EXPORT __declspec(dllexport)
#  define NFSMW_NAKED  __declspec(naked)
#elif defined(__GNUC__) || defined(__clang__)
#  define NFSMW_EXPORT __attribute__((dllexport))
#  define NFSMW_NAKED  __attribute__((naked))
#else
#  define NFSMW_EXPORT
#  define NFSMW_NAKED
#endif

/* ----- Calling conventions used in speed.exe ----- */
#if defined(_MSC_VER)
#  define NFSMW_THISCALL __thiscall
#  define NFSMW_FASTCALL __fastcall
#  define NFSMW_CDECL    __cdecl
#  define NFSMW_STDCALL  __stdcall
#else /* GCC / Clang */
#  define NFSMW_THISCALL __attribute__((thiscall))
#  define NFSMW_FASTCALL __attribute__((fastcall))
#  define NFSMW_CDECL    __attribute__((cdecl))
#  define NFSMW_STDCALL  __attribute__((stdcall))
#endif

/* ----- Image-base ----- */
#define NFSMW_IMAGE_BASE 0x00400000u

/* ----- Loading-mechanism detection ----- */
#define NFSMW_LOADER_ASI       1   /* dinput8.dll proxy / Ultimate-ASI-Loader */
#define NFSMW_LOADER_BEPINEX   2   /* BepInEx 6 Native plugin */
#define NFSMW_LOADER_DIRECT    3   /* manual LoadLibrary / RemoteThread */

#ifndef NFSMW_LOADER
#  define NFSMW_LOADER NFSMW_LOADER_ASI
#endif

/* ----- Host-OS macros (for the build script, not runtime) ----- */
#if defined(__linux__)
#  define NFSMW_BUILD_HOST "Linux"
#elif defined(__APPLE__)
#  define NFSMW_BUILD_HOST "macOS"
#elif defined(_WIN32)
#  define NFSMW_BUILD_HOST "Windows"
#else
#  define NFSMW_BUILD_HOST "Unknown"
#endif

#include <windows.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Validate that the loaded module is retail speed.exe:
 *   1. image base == 0x00400000 (NFSMW has a fixed, non-ASLR base), AND
 *   2. the main module file name ends with "speed.exe".
 * The second gate catches repacked / wrapped / DRM-shimmed binaries that
 * happen to share the base. */
static inline int nfsmw_validate_speed_exe(void) {
    HMODULE m = GetModuleHandleA(NULL);
    if (!m) return 0;
    DWORD nt_off = *(DWORD*)((BYTE*)m + 0x3C);
    DWORD image_base = *(DWORD*)((BYTE*)m + nt_off + 0x34);
    if (image_base != NFSMW_IMAGE_BASE) return 0;

    char path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, path, (DWORD)sizeof(path));
    if (len == 0 || len >= sizeof(path)) return 0;
    /* case-insensitive endswith "speed.exe" */
    const char *want = "speed.exe";
    size_t wl = 9;
    if (len < wl) return 0;
    const char *tail = path + len - wl;
    for (size_t i = 0; i < wl; ++i) {
        char a = tail[i], b = want[i];
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (a != b) return 0;
    }
    return 1;
}

/* Write to a code/data page with a VirtualProtect RWX wrap, then flush the
 * instruction cache (required by the Win32 contract when patching code that
 * another thread may be executing). Returns 1 on success, 0 if the page
 * could not be made writable (in which case nothing is written). */
static inline int nfsmw_write_bytes(uintptr_t addr, const void *src, size_t n) {
    DWORD old;
    if (!VirtualProtect((LPVOID)addr, n, PAGE_EXECUTE_READWRITE, &old))
        return 0;
    memcpy((void*)addr, src, n);
    VirtualProtect((LPVOID)addr, n, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (LPCVOID)addr, n);
    return 1;
}

static inline int  nfsmw_write_byte (uintptr_t addr, uint8_t v)  { return nfsmw_write_bytes(addr, &v, 1); }
static inline int  nfsmw_write_word (uintptr_t addr, uint16_t v) { return nfsmw_write_bytes(addr, &v, 2); }
static inline int  nfsmw_write_int  (uintptr_t addr, uint32_t v) { return nfsmw_write_bytes(addr, &v, 4); }
static inline int  nfsmw_write_float(uintptr_t addr, float v)    { return nfsmw_write_bytes(addr, &v, 4); }
static inline uint8_t  nfsmw_read_byte (uintptr_t addr) { return *(uint8_t*)addr; }
static inline uint16_t nfsmw_read_word (uintptr_t addr) { return *(uint16_t*)addr; }
static inline uint32_t nfsmw_read_int  (uintptr_t addr) { return *(uint32_t*)addr; }
static inline float    nfsmw_read_float(uintptr_t addr) { return *(float*)addr; }

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NFSMW_SDK_PLATFORM_H */

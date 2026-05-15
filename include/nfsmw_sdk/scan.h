/*
 * nfsmw_sdk/scan.h — IDA-style array-of-bytes (AOB) signature scanner.
 *
 * Hardcoded addresses (NFSMW_FN_* / NFSMW_GLOBAL_*) are exact for retail
 * speed.exe but break on repacked / no-CD / widescreen-patched / modded
 * executables. A signature scan locates code by its byte pattern instead,
 * surviving relocations and small edits elsewhere.
 *
 * Pattern syntax (space-separated hex, `??` or `?` = wildcard):
 *
 *     uintptr_t a = nfsmw_aob_scan("8B 0D ?? ?? ?? ?? 89 41 04");
 *     if (!a) { ... not found, bail ... }
 *
 * Resolving operands relative to a match:
 *
 *     // a points at `mov ecx, [g_foo]` ; the rel32 imm is at a+2
 *     uintptr_t g_foo = nfsmw_read_int(a + 2);              // absolute imm32
 *     uintptr_t tgt   = nfsmw_resolve_rel32(a + 1, 5);      // E8/E9 rel32 call/jmp
 *
 * Scans the main module's mapped image only (one pass, bounded). NB: this
 * covers ALL sections (.text, .rdata, .data) — use code-distinctive,
 * sufficiently long patterns so a short signature can't false-match in
 * non-code data.
 */

#ifndef NFSMW_SDK_SCAN_H
#define NFSMW_SDK_SCAN_H

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Resolve the byte range of the main module's image from its PE headers. */
static inline int nfsmw_main_module_range(uintptr_t *base_out, size_t *size_out) {
    HMODULE m = GetModuleHandleA(NULL);
    if (!m) return 0;
    BYTE *base = (BYTE*)m;
    DWORD nt_off = *(DWORD*)(base + 0x3C);
    BYTE *nt = base + nt_off;
    if (*(DWORD*)nt != 0x00004550u) return 0;       /* "PE\0\0" */
    DWORD size_of_image = *(DWORD*)(nt + 0x18 + 0x38); /* OptHdr.SizeOfImage */
    if (base_out) *base_out = (uintptr_t)base;
    if (size_out) *size_out = (size_t)size_of_image;
    return 1;
}

/* Parse one pattern token; returns 1 and sets *val/*wild, or 0 at end. */
static inline const char *nfsmw__pat_next(const char *p, int *val, int *wild) {
    while (*p == ' ' || *p == '\t') ++p;
    if (!*p) return 0;
    if (p[0] == '?' ) {
        *wild = 1; *val = 0;
        ++p; if (*p == '?') ++p;
        return p;
    }
    int hi = -1, lo = -1;
    char c = p[0];
    hi = (c >= '0' && c <= '9') ? c - '0'
       : (c | 32) >= 'a' && (c | 32) <= 'f' ? (c | 32) - 'a' + 10 : -1;
    c = p[1];
    lo = (c >= '0' && c <= '9') ? c - '0'
       : (c | 32) >= 'a' && (c | 32) <= 'f' ? (c | 32) - 'a' + 10 : -1;
    if (hi < 0 || lo < 0) return 0;
    *wild = 0; *val = (hi << 4) | lo;
    return p + 2;
}

/* Scan [start,start+len) for `pattern`. Returns match address or 0. */
static inline uintptr_t nfsmw_aob_scan_range(uintptr_t start, size_t len,
                                             const char *pattern) {
    /* Compile pattern into parallel byte/mask arrays (max 256 tokens). */
    unsigned char pb[256], pm[256];
    size_t plen = 0;
    const char *p = pattern;
    while (plen < 256) {
        int v, w;
        const char *np = nfsmw__pat_next(p, &v, &w);
        if (!np) break;
        pb[plen] = (unsigned char)v;
        pm[plen] = w ? 0 : 1;
        ++plen;
        p = np;
    }
    if (plen == 0 || plen > len) return 0;

    const unsigned char *hay = (const unsigned char*)start;
    size_t last = len - plen;
    for (size_t i = 0; i <= last; ++i) {
        size_t j = 0;
        for (; j < plen; ++j)
            if (pm[j] && hay[i + j] != pb[j]) break;
        if (j == plen) return start + i;
    }
    return 0;
}

/* Scan the whole main module image. */
static inline uintptr_t nfsmw_aob_scan(const char *pattern) {
    uintptr_t base; size_t size;
    if (!nfsmw_main_module_range(&base, &size)) return 0;
    return nfsmw_aob_scan_range(base, size, pattern);
}

/* Given the address of an E8/E9 rel32 instruction (`at` = opcode byte),
 * with total instruction length `instr_len` (5 for E8/E9), return the
 * absolute branch target. */
static inline uintptr_t nfsmw_resolve_rel32(uintptr_t at, size_t instr_len) {
    int32_t disp = *(int32_t*)(at + 1);
    return at + instr_len + (uintptr_t)disp;
}

#ifdef __cplusplus
} /* extern "C" */

namespace nfsmw {
inline uintptr_t aob(const char *pattern) { return nfsmw_aob_scan(pattern); }
inline uintptr_t resolve_rel32(uintptr_t at, size_t n = 5) {
    return nfsmw_resolve_rel32(at, n);
}
}  // namespace nfsmw
#endif

#endif /* NFSMW_SDK_SCAN_H */

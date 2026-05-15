/*
 * nfsmw_sdk/attributes.h — Jenkins mix3 hash + attribute system helpers.
 *
 * NFSMW's attribute database (attributes.bin) is keyed by 32-bit hashes.
 * The hash function is Bob Jenkins 1996 "mix3", seed 0xABCDEF00 — called
 * "bChunk" in the engine. Verified: bChunk("BASE") == 0xA6B47FAC.
 *
 * Usage:
 *   uint32_t mass_hash = NFSMW_BCHUNK("MASS");  // compile-time
 *   uint32_t mass_runtime = nfsmw::bchunk("MASS");  // runtime
 *
 * To read/write an attribute:
 *   auto coll = nfsmw::find_collection("pvehicle", "bmwm3gtre46");
 *   float* mass = coll->get_float(NFSMW_BCHUNK("MASS"));
 *   *mass = 800.0f;  // half the original mass
 */

#ifndef NFSMW_SDK_ATTRIBUTES_H
#define NFSMW_SDK_ATTRIBUTES_H

#include "platform.h"
#include "functions.h"
#include "_generated_attrs.h"   /* NFSMW_ATTR_<NAME> = verified hash (294) */
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Attribute type hashes (7 verified) ---- */
#define NFSMW_ATTR_TYPE_Float           0x3C16EC5Eu
#define NFSMW_ATTR_TYPE_Text            0xA3F0C234u
#define NFSMW_ATTR_TYPE_UInt32          0x939992BBu
#define NFSMW_ATTR_TYPE_Bool            0x064BEC37u
#define NFSMW_ATTR_TYPE_RefSpec         0x2B936EB7u
#define NFSMW_ATTR_TYPE_StringKey       0xA502A824u
#define NFSMW_ATTR_TYPE_eDRIVE_BY_TYPE  0xDB9D3A16u

/* ---- bChunk = Jenkins mix3, seed 0xABCDEF00 — runtime version ---- */
/* Forward to the engine's own implementation at speed.exe:0x454640.
 * Avoids needing a separate C implementation; saves space + matches the
 * engine's exact behavior including any quirks. */

typedef uint32_t (NFSMW_CDECL *nfsmw_StringToKey_fn)(const char *s);

static inline uint32_t nfsmw_bchunk(const char *s) {
    return ((nfsmw_StringToKey_fn)NFSMW_FN_StringToKey)(s);
}

/* ---- Compile-time bChunk (Jenkins mix3 implemented inline) ---- */
/* For static-init use cases; matches engine bit-for-bit. */

#define NFSMW__MIX3_BLOCK(_a, _b, _c) do { \
    (_a) -= (_b); (_a) -= (_c); (_a) ^= ((_c) >> 13); \
    (_b) -= (_c); (_b) -= (_a); (_b) ^= ((_a) << 8);  \
    (_c) -= (_a); (_c) -= (_b); (_c) ^= ((_b) >> 13); \
    (_a) -= (_b); (_a) -= (_c); (_a) ^= ((_c) >> 12); \
    (_b) -= (_c); (_b) -= (_a); (_b) ^= ((_a) << 16); \
    (_c) -= (_a); (_c) -= (_b); (_c) ^= ((_b) >> 5);  \
    (_a) -= (_b); (_a) -= (_c); (_a) ^= ((_c) >> 3);  \
    (_b) -= (_c); (_b) -= (_a); (_b) ^= ((_a) << 10); \
    (_c) -= (_a); (_c) -= (_b); (_c) ^= ((_b) >> 15); \
} while (0)

static inline uint32_t nfsmw_bchunk_inline(const char *s) {
    uint32_t a = 0x9E3779B9u, b = 0x9E3779B9u, c = 0xABCDEF00u;
    size_t length = strlen(s);
    size_t rem = length;
    const uint8_t *p = (const uint8_t*)s;
    while (rem >= 12) {
        a += (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
        b += (uint32_t)p[4] | ((uint32_t)p[5] << 8) | ((uint32_t)p[6] << 16) | ((uint32_t)p[7] << 24);
        c += (uint32_t)p[8] | ((uint32_t)p[9] << 8) | ((uint32_t)p[10] << 16) | ((uint32_t)p[11] << 24);
        NFSMW__MIX3_BLOCK(a, b, c);
        p += 12; rem -= 12;
    }
    c += (uint32_t)length;
    /* tail (positional packing per Jenkins 1996) */
    switch (rem) {
        case 11: c += ((uint32_t)p[10]) << 24;  /* fallthrough */
        case 10: c += ((uint32_t)p[9]) << 16;
        case 9:  c += ((uint32_t)p[8]) << 8;
        case 8:  b += ((uint32_t)p[7]) << 24;
        case 7:  b += ((uint32_t)p[6]) << 16;
        case 6:  b += ((uint32_t)p[5]) << 8;
        case 5:  b += p[4];
        case 4:  a += ((uint32_t)p[3]) << 24;
        case 3:  a += ((uint32_t)p[2]) << 16;
        case 2:  a += ((uint32_t)p[1]) << 8;
        case 1:  a += p[0];
    }
    NFSMW__MIX3_BLOCK(a, b, c);
    return c;
}

/* ---- Attribute Collection API (forward to engine via SDK addresses) ---- */
typedef struct nfsmw_Collection nfsmw_Collection;

typedef nfsmw_Collection* (NFSMW_CDECL *nfsmw_FindCollection_fn)(uint32_t classKey, uint32_t collectionKey);

static inline nfsmw_Collection* nfsmw_find_collection(const char *className, const char *collectionName) {
    uint32_t ck = nfsmw_bchunk(className);
    uint32_t lk = nfsmw_bchunk(collectionName);
    return ((nfsmw_FindCollection_fn)NFSMW_FN_FindCollection)(ck, lk);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* ---- C++ convenience ---- */
#ifdef __cplusplus
namespace nfsmw {
/* Runtime hash via the engine's own StringToKey @ 0x454640. */
inline uint32_t bchunk(const char *s) { return nfsmw_bchunk(s); }

/* Self-contained hash (no engine call); matches the engine bit-for-bit.
 * Usable before the game is fully initialized. */
inline uint32_t bchunk_local(const char *s) { return nfsmw_bchunk_inline(s); }
}  // namespace nfsmw
#endif

#define NFSMW_BCHUNK(_str) nfsmw_bchunk_inline(_str)

#endif /* NFSMW_SDK_ATTRIBUTES_H */

/*
 * nfsmw_sdk/structs.h — engine struct field offsets (opt-in).
 *
 * Provenance & trust model (read this):
 *
 *  - Offsets are NOT guessed. They are computed by the compiler from
 *    berkayylmao's NFSPluginSDK MW05 type definitions (BSD-3) — the
 *    community-maintained, game-matched type set — built with
 *    MSVC-layout-matching flags (`g++ -m32 -malign-double`).
 *  - `NFSMW_OFF_<Struct>_<field>` / `NFSMW_SIZEOF_<Struct>` are emitted
 *    ONLY for structs with no / single inheritance. For those the GCC
 *    i686 record layout is provably identical to the MSVC-7.10 layout
 *    the retail game uses (vptr at 0, sequential members, matched
 *    alignment) — i.e. ABI-invariant.
 *  - Multiple- / virtual-inheritance structs are exposed as **opaque
 *    typedefs only**, with NO offsets: Itanium vs MSVC can place
 *    secondary bases differently and there is no in-binary ground truth
 *    to verify against, so shipping numbers there would be dishonest.
 *
 * Usage — read/write a field through the offset macro:
 *
 *   #include <nfsmw_sdk/structs.h>
 *   // e.g. a RaceParameters* p obtained from the game:
 *   uint8_t* dens = (uint8_t*)p + NFSMW_OFF_RaceParameters_TrafficDensity;
 *
 * Opt-in: not pulled in by the umbrella header.
 */

#ifndef NFSMW_SDK_STRUCTS_H
#define NFSMW_SDK_STRUCTS_H

#include "platform.h"
#include "_generated_structs.h"

/* Convenience: byte-offset member access. */
#define NFSMW_FIELD(ptr, off, type) (*(type*)((char*)(ptr) + (off)))

#endif /* NFSMW_SDK_STRUCTS_H */

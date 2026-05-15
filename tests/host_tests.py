#!/usr/bin/env python3
"""
host_tests.py — compile + run the platform-independent SDK logic natively.

The SDK headers are Win32-only (they #error otherwise), but several pieces
of logic are pure C and can be verified on the build host:

  * attributes.h  nfsmw_bchunk_inline  -> bChunk("BASE") must == 0xA6B47FAC
  * scan.h        nfsmw_aob_scan_range -> finds a known pattern + wildcards

We slice those functions out of the headers (so the test always reflects
the shipped code) and compile a tiny native harness.

Usage:  python3 tests/host_tests.py     (exit 0 = all pass)
"""

import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
INC = os.path.join(os.path.dirname(HERE), "include", "nfsmw_sdk")


def slice_between(text, start_marker, end_marker, include_end=True):
    i = text.index(start_marker)
    j = text.index(end_marker, i)
    return text[i : j + (len(end_marker) if include_end else 0)]


def extract_bchunk():
    src = open(os.path.join(INC, "attributes.h")).read()
    mac = src[src.index("#define NFSMW__MIX3_BLOCK"):
              src.index("static inline uint32_t nfsmw_bchunk_inline")]
    fn = src[src.index("static inline uint32_t nfsmw_bchunk_inline"):]
    fn = fn[: fn.index("\n}\n") + 3]
    return mac + fn


def extract_aob():
    src = open(os.path.join(INC, "scan.h")).read()
    a = src.index("static inline const char *nfsmw__pat_next")
    b = src.index("/* Scan the whole main module image. */")
    return src[a:b]


HARNESS = r"""
#include <stdint.h>
#include <string.h>
#include <stdio.h>
__BCHUNK__
__AOB__
int main(void) {
    int fails = 0;

    unsigned k = nfsmw_bchunk_inline("BASE");
    if (k == 0xA6B47FACu) printf("PASS bchunk(BASE)=0x%08X\n", k);
    else { printf("FAIL bchunk(BASE)=0x%08X want 0xA6B47FAC\n", k); fails++; }

    /* synthetic buffer: pattern with two wildcard bytes */
    unsigned char buf[32] = {
        0x90,0x90,0x8B,0x0D,0xAA,0xBB,0xCC,0xDD,0x89,0x41,0x04,0xC3,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    uintptr_t hit = nfsmw_aob_scan_range((uintptr_t)buf, sizeof buf,
                                         "8B 0D ?? ?? ?? ?? 89 41 04");
    if (hit == (uintptr_t)(buf + 2)) printf("PASS aob match at +2\n");
    else { printf("FAIL aob hit=%p want=%p\n",(void*)hit,(void*)(buf+2)); fails++; }

    uintptr_t miss = nfsmw_aob_scan_range((uintptr_t)buf, sizeof buf,
                                          "DE AD BE EF");
    if (miss == 0) printf("PASS aob no-match returns 0\n");
    else { printf("FAIL aob no-match returned %p\n",(void*)miss); fails++; }

    return fails ? 1 : 0;
}
"""


def main():
    code = (HARNESS
            .replace("__BCHUNK__", extract_bchunk())
            .replace("__AOB__", extract_aob()))
    with tempfile.TemporaryDirectory() as d:
        c = os.path.join(d, "t.c")
        exe = os.path.join(d, "t")
        open(c, "w").write(code)
        cc = os.environ.get("CC", "cc")
        r = subprocess.run([cc, "-O2", "-w", c, "-o", exe])
        if r.returncode != 0:
            print("compile failed", file=sys.stderr)
            sys.exit(2)
        sys.exit(subprocess.run([exe]).returncode)


if __name__ == "__main__":
    main()

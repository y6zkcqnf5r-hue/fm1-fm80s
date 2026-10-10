// SPDX-License-Identifier: GPL-3.0-only
/* White noise: a 32-bit xorshift (Marsaglia, "Xorshift RNGs", 2003), integers only. Deterministic:
 * the same seed gives the same samples on the host, in wasm and on the device (CLAUDE.md rule 7). */
#pragma once
#include <stdint.h>

#define NOISE_SEED 0x2545F491u          /* any non-zero value */

/* the next sample, Q15 (-32768..32767); *s must not be 0 */
static inline int32_t noise_q15(uint32_t *s)
{
    uint32_t x = *s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *s = x;
    return (int32_t)(x >> 16) - 32768;
}

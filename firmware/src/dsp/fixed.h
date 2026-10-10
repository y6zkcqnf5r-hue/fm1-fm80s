// SPDX-License-Identifier: GPL-3.0-only
/* Fixed-point helpers: the only arithmetic the DSP uses (no float, no 64-bit division).
 *
 *   Q15    levels and samples: int16, 32767 = 1.0
 *   Q12    gains: 4096 = 1.0
 *   phase  uint32 that wraps: one turn = 2^32; inc = f * 2^32 / FS
 *   pitch  1/16 semitone: note * 16 + fine; note 0 = C-0, NOTE_A4 (57) = 440 Hz
 *
 * Right shifts of negative values are arithmetic (clang and gcc, host, wasm and the device's
 * clang); the build uses -fwrapv. Tables come from tools/gen_tables.py (build/gen/tables.h). */
#pragma once
#include <stdint.h>
#include "tables.h"

typedef int16_t q15_t;
typedef uint32_t phase_t;

#define Q15_ONE 32767
#define Q12_ONE 4096

/* clamp to the int16 range */
static inline int16_t sat16(int32_t x)
{
    if (x > 32767)
        return 32767;
    if (x < -32768)
        return -32768;
    return (int16_t)x;
}

/* a * b in Q15, rounded to nearest (halves up); -1 * -1 saturates to 32767 */
static inline q15_t mul_q15(q15_t a, q15_t b)
{
    return sat16(((int32_t)a * b + 0x4000) >> 15);
}

/* x * g with g in Q12, rounded to nearest; 64-bit product, shift only. The result must fit an
 * int32 (it does for any 24-bit x and g < 2^19). */
static inline int32_t mul_q12(int32_t x, int32_t g)
{
    return (int32_t)(((int64_t)x * g + 0x800) >> 12);
}

/* sine of a phase, Q15: 1024-point table, linear interpolation on the next 16 bits */
static inline q15_t sin_q15(phase_t ph)
{
    uint32_t i = ph >> (32 - SINE_BITS);
    int32_t frac = (int32_t)((ph >> (16 - SINE_BITS)) & 0xFFFFu);
    int32_t a = SINE_Q15[i], b = SINE_Q15[i + 1];
    return (q15_t)(a + (((b - a) * frac + 0x8000) >> 16));
}

/* phase increment of a pitch (1/16 semitone, clamped to 0..PITCH_MAX). Only the top octave is
 * stored; an octave down is a shift right, bit-exact (Felucca's method). The octave block is
 * found by a multiply, (q * 21846) >> 22 == q / 192 on this range, so no division. */
static inline uint32_t pitch_inc(int32_t p)
{
    uint32_t q, blk;
    if (p < 0)
        p = 0;
    if (p > PITCH_MAX)
        p = PITCH_MAX;
    q = (uint32_t)p + PITCH_OFS;
    blk = (q * 21846u) >> 22;
    return PITCH_OCT[q - blk * PITCH_OCT_N] >> (PITCH_BLK_TOP - blk);
}

/* phase increment of a note (0..127) plus fine tune (1/16 semitone, signed) */
static inline uint32_t note_inc(int32_t note, int32_t fine)
{
    return pitch_inc(note * PITCH_FINE + fine);
}

/* floor(2^shift / d), for a quotient that fits 32 bits, d > 0, d < 2^31, shift <= 62: a long division
 * with 32-bit operations only (no 64-bit division on the device, CLAUDE.md rule 2). shift + 1
 * iterations: for control-rate use (filter coefficients), not per sample. d == 0 gives 0. */
static inline uint32_t div_pow2(int shift, uint32_t d)
{
    uint32_t q = 0, r = 0;
    int i;
    if (!d)
        return 0;
    for (i = shift; i >= 0; i--) {
        r = (r << 1) | (i == shift);
        if (r >= d) {
            r -= d;
            if (i < 32)
                q |= 1u << i;
        }
    }
    return q;
}

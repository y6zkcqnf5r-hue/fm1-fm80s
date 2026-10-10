// SPDX-License-Identifier: GPL-3.0-only
/* State-variable filter, low-pass / high-pass / band-pass, in fixed point. The topology-preserving
 * (trapezoidal) form of Andrew Simper's "Linear Trapezoidal Integrated SVF" (Cytomic, 2013): stable
 * at every cutoff and resonance, unlike the Chamberlin form. Written from the published equations.
 *
 *   g = tan(pi fc / FS) (table SVF_G_Q16, CUT 00..FF = 30 Hz..18 kHz), k = 2 - 2 res (damping)
 *   a1 = 1 / (1 + g (g + k)), a2 = g a1, a3 = g a2          coefficients in Q30
 *   v3 = x - ic2;  v1 = a1 ic1 + a2 v3;  v2 = ic2 + a2 ic1 + a3 v3;  ic1 = 2 v1 - ic1;  ic2 = 2 v2 - ic2
 *   LP = v2, BP = v1, HP = x - k v1 - v2
 *
 * svf_set() runs at control rate (one 32-bit long division); svf_run() per sample, 5 multiplies in
 * 64 bits, shifts only. The states keep SVF_FRAC bits of fraction. At RES FF the peak gain is about
 * 1/k = 43: for a full-scale input the states stay under 2^29 (tested), the int32 holds 2^31. */
#pragma once
#include <stdint.h>

enum { FLT_OFF, FLT_LP, FLT_HP, FLT_BP, FLT_COUNT };

#define SVF_FRAC 8                      /* fraction bits of the states: a 30 Hz low-pass moves by */
#define SVF_RND (1 << 29)               /* far less than 1 LSB per sample, it must not round to 0 */

typedef struct {
    int32_t ic1, ic2;                   /* integrator states, input * 2^SVF_FRAC */
    int32_t a1, a2, a3;                 /* Q30 */
    int32_t k;                          /* Q16 */
    uint8_t mode;
} svf_t;

void svf_reset(svf_t *f);
void svf_set(svf_t *f, int mode, int cut, int res);   /* FLT_*, 00..FF, 00..FF; keeps the states */

static inline int32_t svf_run(svf_t *f, int32_t x)
{
    int32_t v1, v2, v3, y;
    if (f->mode == FLT_OFF)
        return x;
    x <<= SVF_FRAC;
    v3 = x - f->ic2;
    v1 = (int32_t)(((int64_t)f->a1 * f->ic1 + (int64_t)f->a2 * v3 + SVF_RND) >> 30);
    v2 = f->ic2 + (int32_t)(((int64_t)f->a2 * f->ic1 + (int64_t)f->a3 * v3 + SVF_RND) >> 30);
    f->ic1 = 2 * v1 - f->ic1;
    f->ic2 = 2 * v2 - f->ic2;
    if (f->mode == FLT_LP)
        y = v2;
    else if (f->mode == FLT_BP)
        y = v1;
    else
        y = x - (int32_t)(((int64_t)f->k * v1) >> 16) - v2;
    return (y + (1 << (SVF_FRAC - 1))) >> SVF_FRAC;
}

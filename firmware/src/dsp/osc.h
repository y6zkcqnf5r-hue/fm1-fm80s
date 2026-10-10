// SPDX-License-Identifier: GPL-3.0-only
/* Oscillators of the WAV engine, Q15 out. Saw and square are band-limited by polyBLEP: the
 * 2-sample polynomial correction of Välimäki and Huovilainen ("Antialiasing oscillators in
 * subtractive synthesis", IEEE Signal Processing Magazine, 2007), in fixed point. The triangle and
 * the sine have no jump: they are computed directly (the triangle's corner aliases 40 dB below a
 * naive saw; a polyBLAMP may follow if it is ever heard).
 *
 *   t   the phase as a fraction, Q16 (ph >> 16)       dt   the increment, Q16 (inc >> 16)
 *   rdt = 2^30 / dt, set with the pitch (control rate): t / dt = t * rdt >> 14, no division per sample
 *
 * Shapes beyond SAW SQR TRI SIN come with P3-07 (PUL: the pulse width, NOI, the wavetables); until
 * then PUL plays SQR and the others SAW. */
#pragma once
#include <stdint.h>
#include "dsp/fixed.h"

enum { SHP_SAW, SHP_SQR, SHP_PUL, SHP_TRI, SHP_SIN, SHP_NOI, SHP_COUNT = 16 };

typedef struct { phase_t ph; uint32_t inc, dt, rdt; } osc_t;

static inline void osc_pitch(osc_t *o, uint32_t inc)
{
    o->inc = inc;
    o->dt = inc >> 16 ? inc >> 16 : 1;  /* below 0.7 Hz: never for a note */
    o->rdt = (1u << 30) / o->dt;        /* 32-bit division, at control rate */
}

/* the polyBLEP residual at t, Q15 (-32768..0 just after the jump, 0..32768 just before) */
static inline int32_t osc_blep(uint32_t t, const osc_t *o)
{
    uint32_t x;
    if (t < o->dt) {                    /* x = t / dt: 2x - x^2 - 1 */
        x = (t * o->rdt) >> 14;
        if (x > 65535)
            x = 65535;
        return (int32_t)(2 * x - ((x * x) >> 16) - 65536) >> 1;
    }
    if (t > 65536 - o->dt) {            /* x = (t - 1) / dt = -u: x^2 + 2x + 1 = u^2 - 2u + 1 */
        x = ((65536 - t) * o->rdt) >> 14;
        if (x > 65535)
            x = 65535;
        return (int32_t)(((x * x) >> 16) - 2 * x + 65536) >> 1;
    }
    return 0;
}

/* one sample, then the phase moves on */
static inline int32_t osc_run(osc_t *o, int shape)
{
    uint32_t t = o->ph >> 16;
    int32_t y;
    switch (shape) {
    case SHP_SQR:
    case SHP_PUL:
        y = t < 32768 ? 32767 : -32767;
        y += osc_blep(t, o) - osc_blep((t + 32768) & 0xFFFF, o);
        break;
    case SHP_TRI:
        y = t < 32768 ? 2 * (int32_t)t - 32768 : 98303 - 2 * (int32_t)t;
        break;
    case SHP_SIN:
        y = sin_q15(o->ph);
        break;
    default:                            /* SHP_SAW */
        y = (int32_t)t - 32768 - osc_blep(t, o);
    }
    o->ph += o->inc;
    return y;
}

// SPDX-License-Identifier: GPL-3.0-only
#include "dsp/env.h"
#include "dsp/fixed.h"

#define ONE (1 << 24)
#define FLOOR (ONE >> 12)               /* -72 dB (8 LSB of Q15): done */

static int clamp8(int v) { return v < 0 ? 0 : v > 0xFF ? 0xFF : v; }

void env_start(env_t *e, int atk, int dec, int sus, int rel)
{
    sus = clamp8(sus);
    e->atk = ENV_ATK_Q24[clamp8(atk)];
    e->dec = ENV_COEF_Q24[clamp8(dec)];
    e->rel = ENV_COEF_Q24[clamp8(rel)];
    e->sus = sus << 16 | sus << 8 | sus;   /* FF -> 0xFFFFFF, just under 1.0 */
    e->stage = ENV_ATK;
}

void env_release(env_t *e)
{
    if (e->stage != ENV_IDLE)
        e->stage = ENV_REL;
}

int32_t env_step(env_t *e)
{
    switch (e->stage) {
    case ENV_ATK:
        e->v += (int32_t)e->atk;
        if (e->v >= ONE) {
            e->v = ONE;
            e->stage = ENV_DEC;
        }
        break;
    case ENV_DEC:
        e->v += (int32_t)(((int64_t)(e->sus - e->v) * e->dec) >> 24);
        break;
    case ENV_REL: {
        int32_t d = (int32_t)(((int64_t)e->v * e->rel) >> 24);
        e->v -= d ? d : 1;              /* a long release still ends */
        if (e->v < FLOOR) {
            e->v = 0;
            e->stage = ENV_IDLE;
        }
        break;
    }
    default:
        e->v = 0;
    }
    return e->v >> 9 < Q15_ONE ? e->v >> 9 : Q15_ONE;
}

// SPDX-License-Identifier: GPL-3.0-only
#include "dsp/svf.h"
#include "dsp/fixed.h"

#define K_MAX 131072                    /* k = 2.0 in Q16: no resonance */
#define K_PER_RES 508                   /* RES FF: k = 2 - 255 * 508 / 65536 = 0.023 */

void svf_reset(svf_t *f)
{
    f->ic1 = f->ic2 = 0;
    svf_set(f, FLT_OFF, 0xFF, 0);
}

void svf_set(svf_t *f, int mode, int cut, int res)
{
    uint32_t g, den;
    if (cut < 0)
        cut = 0;
    if (cut > 0xFF)
        cut = 0xFF;
    if (res < 0)
        res = 0;
    if (res > 0xFF)
        res = 0xFF;
    f->mode = (uint8_t)(mode >= 0 && mode < FLT_COUNT ? mode : FLT_OFF);
    f->k = K_MAX - res * K_PER_RES;
    g = SVF_G_Q16[cut];                 /* < 8.0 in Q16 */
    /* 1 + g (g + k) in Q20: g < 2^18 and g + k < 2^19 in Q16, the product < 2^37 in Q32 */
    den = (1u << 20) + (uint32_t)(((uint64_t)g * (g + (uint32_t)f->k)) >> 12);
    f->a1 = (int32_t)div_pow2(50, den); /* 2^50 / den = a1 in Q30, <= 2^30 */
    f->a2 = (int32_t)(((int64_t)g * f->a1) >> 16);
    f->a3 = (int32_t)(((int64_t)g * f->a2) >> 16);
}

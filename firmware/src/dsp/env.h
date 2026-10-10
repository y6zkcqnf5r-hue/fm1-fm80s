// SPDX-License-Identifier: GPL-3.0-only
/* ADSR envelope, one step per sub-block of 16 frames (CLAUDE.md rule 5), in Q24 (1.0 = 2^24).
 *
 *   ATK  linear 0 -> 1 in t(ATK)            t(v) = 1 ms * 10000^(v/255): 00 = 1 ms, FF = 10 s
 *   DEC  exponential towards SUS, -60 dB in t(DEC)
 *   SUS  level, 00..FF
 *   REL  exponential towards 0, -60 dB in t(REL); below -72 dB the envelope is done (exactly 0)
 *
 * Tables ENV_ATK_Q24 and ENV_COEF_Q24 come from tools/gen_tables.py. */
#pragma once
#include <stdint.h>

enum { ENV_IDLE, ENV_ATK, ENV_DEC, ENV_REL };

typedef struct {
    int32_t v;                          /* Q24 */
    int32_t sus;                        /* Q24 */
    uint32_t atk;                       /* Q24 per sub-block */
    uint32_t dec, rel;                  /* Q24 coefficients */
    uint8_t stage;
} env_t;

void env_start(env_t *e, int atk, int dec, int sus, int rel);   /* from the current level */
void env_release(env_t *e);
int32_t env_step(env_t *e);             /* next sub-block: the level in Q15 */

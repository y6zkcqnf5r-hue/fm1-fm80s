// SPDX-License-Identifier: GPL-3.0-only
/* FM80S: the app the platform runs (plat.h). The interface (ui/) plays the tracks (core/), here the
 * drums alone; MASTER is the overall volume. */
#include "app/plat.h"
#include "core/track.h"
#include "dsp/fixed.h"
#include "ui/ui.h"

static volatile int32_t v_master;       /* MASTER, Q12: written by app_frame, read by app_audio */
static int32_t master_g;                /* the gain reached at the end of the last block */

void app_init(void)
{
    track_init();
    ui_init();
    v_master = master_g = plat_master();
}

void app_frame(void)
{
    ui_frame();
    v_master = plat_master();
}

void app_audio(int32_t *lr, int n)
{
    int32_t m0 = master_g, m1 = v_master, g = m0;
    int i;
    track_audio(lr, n);
    for (i = 0; i < n; i++) {           /* MASTER ramps across the block (n = 256): no zipper */
        g = i >= 255 ? m1 : m0 + (((m1 - m0) * (i + 1)) >> 8);
        lr[2 * i] = mul_q12(lr[2 * i], g);      /* 64-bit product: 16 loud sounds pass 2^19 */
        lr[2 * i + 1] = mul_q12(lr[2 * i + 1], g);
    }
    master_g = g;
}

int app_cost(void) { return track_cost(); }

// SPDX-License-Identifier: GPL-3.0-only
/* FM80S: the app the platform runs (plat.h). Until the Drums engine, a check that the platform taken
 * from Trawler works: the screen says FM80S, each key plays a sine through Trawler's voice, MASTER
 * is the overall volume. */
#include "app/cmdq.h"
#include "app/plat.h"
#include "dsp/fixed.h"
#include "dsp/voice.h"
#include "ui/gfx.h"

enum { CMD_NOTE, CMD_OFF };
#define CMD(op, arg) ((uint32_t)(op) << 8 | (uint32_t)(arg))

static volatile int32_t v_master;       /* MASTER, Q12: written by app_frame, read by app_audio */
static int32_t master_g;                /* the gain reached at the end of the last block */
static voice_t voice;                   /* audio side only */
static uint32_t prev_keys;
static int drawn;

static const patch_t SINE = {VOICE_WAV, SHP_SIN, FLT_OFF, 0xFF, 0, 0x00, 0x80, 0xC0, 0x60, 0x80, 0x80};

void app_init(void)
{
    cmdq_init();
    voice_reset(&voice);
    prev_keys = 0;
    drawn = 0;
    v_master = master_g = plat_master();
}

void app_frame(void)
{
    uint32_t keys = plat_keys(), down = keys & ~prev_keys;
    int k;
    for (k = 0; k < NKEYS; k++)
        if (down >> k & 1u)
            cmdq_push(CMD(CMD_NOTE, k));
    if (prev_keys && !keys)
        cmdq_push(CMD(CMD_OFF, 0));
    prev_keys = keys;
    v_master = plat_master();
    plat_leds(0, keys, 0);
    if (!drawn) {
        gfx_fill(0, 0, 240, 240, TH_BG);
        gfx_font(GFX_FONT_6X12);
        gfx_text((240 - gfx_text_w("FM80S", 4)) / 2, 96, "FM80S", 4, TH_HEAD, TH_BG);
        drawn = 1;
    }
}

void app_audio(int32_t *lr, int n)
{
    int32_t m0 = master_g, m1 = v_master, g = m0;
    uint32_t c;
    int i;
    while (cmdq_pop(&c)) {
        if (c >> 8 == CMD_NOTE)
            voice_note(&voice, (41 + (int)(c & 0xFF)) * PITCH_FINE, 0xFF, &SINE);   /* key 0 = F-3 */
        else
            voice_off(&voice);
    }
    voice_render(&voice, lr, n);
    for (i = 0; i < n; i++) {           /* MASTER ramps across the block (n = 256): no zipper */
        g = i >= 255 ? m1 : m0 + (((m1 - m0) * (i + 1)) >> 8);
        lr[2 * i] = (lr[2 * i] * g) >> 12;
        lr[2 * i + 1] = (lr[2 * i + 1] * g) >> 12;
    }
    master_g = g;
}

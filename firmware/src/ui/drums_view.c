// SPDX-License-Identifier: GPL-3.0-only
#include "ui/drums_view.h"
#include "core/track.h"
#include "ui/gfx.h"
#include "ui/ui.h"

/* the layout, in pixels */
#define HEAD_H 22
#define PAD_W 26
#define PAD_GAP 3
#define PAD_X0 ((240 - 8 * PAD_W - 7 * PAD_GAP) / 2)
#define PAD_H 58
#define PAD_TOP 30                      /* sounds 9-16 */
#define PAD_BOTTOM 94                   /* sounds 1-8 */
#define FOOT_Y 158
#define GAUGE_W 56
#define GAUGE_GAP 4
#define GAUGE_X0 ((240 - 4 * GAUGE_W - 3 * GAUGE_GAP) / 2)
#define LEVELS 8                        /* a pad's brightness, 0 (silent) .. 8 */

static const uint16_t FAMILY_RGB[DFAM_COUNT] = {TH_FAM_KICK, TH_FAM_SNARE, TH_FAM_METAL, TH_FAM_TOM,
                                                 TH_FAM_PERC};
static const uint16_t KNOB_RGB[4] = {TH_K1, TH_K2, TH_K3, TH_K4};
static const char *const KNOB_NAME[4] = {"TUNE", "DECAY", "TONE", "LEVEL"};

/* what the screen shows now, to redraw only what changed */
static int8_t shown_level[DRUM_SOUNDS];
static int8_t shown_sound;
static int16_t shown_value[4];

/* 6 dB per step: a pad fades with its sound's decay as the ear hears it */
static int level_of(int meter)
{
    int l = 0;
    while (l < LEVELS && meter >= 1 << l)
        l++;
    return l;
}

/* from a to b by f / LEVELS, per RGB565 field */
static uint16_t blend(uint16_t a, uint16_t b, int f)
{
    int r = (a >> 11) + ((((b >> 11) - (a >> 11)) * f) >> 3);
    int g = (a >> 5 & 63) + ((((b >> 5 & 63) - (a >> 5 & 63)) * f) >> 3);
    int bl = (a & 31) + ((((b & 31) - (a & 31)) * f) >> 3);
    return (uint16_t)(r << 11 | g << 5 | bl);
}

static void dec3(int v, char *s)
{
    s[0] = (char)(v >= 100 ? '0' + v / 100 : ' ');
    s[1] = (char)(v >= 10 ? '0' + v / 10 % 10 : ' ');
    s[2] = (char)('0' + v % 10);
    s[3] = 0;
}

static void draw_pad(int s, int level, int selected)
{
    int x = PAD_X0 + (s & 7) * (PAD_W + PAD_GAP), y = s < 8 ? PAD_BOTTOM : PAD_TOP;
    uint16_t fam = FAMILY_RGB[DRUM_FAMILY[s]], c = blend(TH_PAD, fam, level);
    const char *name = DRUM_SHORT[s];
    gfx_fill(x, y, PAD_W, PAD_H, c);
    gfx_font(GFX_FONT_6X10);
    gfx_text(x + (PAD_W - gfx_text_w(name, 1)) / 2, y + PAD_H - 14, name, 1, level >= 5 ? TH_BG : fam, c);
    if (selected) {
        gfx_frame(x, y, PAD_W, PAD_H, TH_SELECT);
        gfx_frame(x + 1, y + 1, PAD_W - 2, PAD_H - 2, TH_SELECT);
    }
}

static void draw_name(int s)
{
    const char *name = DRUM_NAME[s];
    gfx_fill(120, 0, 120, HEAD_H, TH_BG);
    gfx_font(GFX_FONT_8X12);
    gfx_text(234 - gfx_text_w(name, 1), 5, name, 1, TH_TEXT, TH_BG);
}

static void draw_gauge(int i, int v)
{
    int x = GAUGE_X0 + i * (GAUGE_W + GAUGE_GAP), fill = ((GAUGE_W - 4) * v) >> 8;
    char num[4];
    gfx_frame(x, FOOT_Y + 20, GAUGE_W, 14, KNOB_RGB[i]);
    gfx_fill(x + 2, FOOT_Y + 22, fill, 10, KNOB_RGB[i]);
    gfx_fill(x + 2 + fill, FOOT_Y + 22, GAUGE_W - 4 - fill, 10, TH_RULE);
    dec3(v, num);
    gfx_font(GFX_FONT_8X12);
    gfx_text(x + (GAUGE_W - gfx_text_w(num, 1)) / 2, FOOT_Y + 42, num, 1, TH_TEXT, TH_BG);
}

static void draw_frame(void)
{
    int i;
    gfx_fill(0, 0, 240, 240, TH_BG);
    gfx_font(GFX_FONT_8X12);
    gfx_text(6, 5, "FM80S", 1, TH_HEAD, TH_BG);
    gfx_text(54, 5, "DRUMS", 1, TH_ACCENT, TH_BG);
    gfx_hline(0, HEAD_H, 240, TH_RULE);
    gfx_hline(0, FOOT_Y, 240, TH_RULE);
    gfx_font(GFX_FONT_6X10);
    for (i = 0; i < 4; i++)
        gfx_text(GAUGE_X0 + i * (GAUGE_W + GAUGE_GAP) + (GAUGE_W - gfx_text_w(KNOB_NAME[i], 1)) / 2,
                 FOOT_Y + 6, KNOB_NAME[i], 1, KNOB_RGB[i], TH_BG);
}

void drums_view_draw(int full)
{
    const track_t *t = &track[0];
    int s, i, sel = ui.sound;
    if (full) {
        draw_frame();
        for (s = 0; s < DRUM_SOUNDS; s++)
            shown_level[s] = -1;
        shown_sound = -1;
        for (i = 0; i < 4; i++)
            shown_value[i] = -1;
    }
    for (s = 0; s < DRUM_SOUNDS; s++) {
        int l = level_of(t->st.head.meter[s]);
        int was_sel = s == shown_sound, is_sel = s == sel;
        if (l != shown_level[s] || was_sel != is_sel) {
            draw_pad(s, l, is_sel);
            shown_level[s] = (int8_t)l;
        }
    }
    if (sel != shown_sound) {
        draw_name(sel);
        shown_sound = (int8_t)sel;
        for (i = 0; i < 4; i++)
            shown_value[i] = -1;
    }
    for (i = 0; i < 4; i++) {
        int v = t->p[sel * DRUM_BYTES + DP_TUNE + i];
        if (v != shown_value[i]) {
            draw_gauge(i, v);
            shown_value[i] = (int16_t)v;
        }
    }
}

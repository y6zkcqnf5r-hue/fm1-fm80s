/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Floydus (FM80S); after SLOOP's splash.c (isod89) */
/* FM80S boot screen: the synthwave image (tools/gen_fm80s_splash.py -> fm80s_splash.h: 16 colours,
 * RLE runs of (run - 1) << 4 | colour), drawn into the canvas in bands of at most 120 rows, then the
 * credit; the version over the image. Replaces SLOOP's splash.c in felucca.c (that file stays as is). */
#include "fm80s_splash.h"
static void fm80s_image_draw(uint32_t y0, const char *tag)
{
    uint32_t r0;
    for (r0 = 0; r0 < FM80S_SPLASH_H; r0 += 120u) {
        uint32_t rows = FM80S_SPLASH_H - r0 > 120u ? 120u : FM80S_SPLASH_H - r0;
        uint32_t lo = r0 * FM80S_SPLASH_W, hi = (r0 + rows) * FM80S_SPLASH_W, pos = 0, i;
        cv_begin(FM80S_SPLASH_W, rows, C_BLACK);
        for (i = 0; i < sizeof FM80S_SPLASH_RLE && pos < hi; i++) {
            uint32_t run = (FM80S_SPLASH_RLE[i] >> 4) + 1u, c = FM80S_SPLASH_RLE[i] & 15u, p;
            if (c && pos + run > lo)
                for (p = pos < lo ? lo : pos; p < pos + run && p < hi; p++)
                    cv_px[p - lo] = swap16(FM80S_SPLASH_PAL[c]);
            pos += run;
        }
        if (r0 == 0)                                    /* the version, small, on the sky at the top right */
            cv_text((int32_t)FM80S_SPLASH_W - 4 - text_w(&FONT_S, tag), 2, &FONT_S, tag, RGB(150, 110, 170));
        cv_blit((240u - FM80S_SPLASH_W) / 2u, y0 + r0);
    }
}

/* the boot screen: the image with the version, who made it and what it comes from */
static void fm80s_splash(void)
{
    static const char by[] = "by Floydus", from[] = "inspired by SLOOP & Felucca";
    const char *v = FELUCCA_VERSION;
    char tag[16] = "v";
    if (v[0] == 'F' && v[1] == 'M' && v[2] == '8' && v[5] == ' ')
        v += 6;                                         /* "FM80S 0.1" -> "v0.1": the name is in the image */
    str_cpy(tag + 1, v, sizeof tag - 1);
    lcd_fill(0, 0, 240, 240, C_BLACK);
    fm80s_image_draw(10, tag);
    cv_begin(240, 36, C_BLACK);
    cv_text(120 - text_w(&FONT_S, by) / 2, 0, &FONT_S, by, RGB(196, 196, 204));
    cv_text(120 - text_w(&FONT_S, from) / 2, 18, &FONT_S, from, RGB(150, 110, 170));
    cv_blit(0, 202);
}

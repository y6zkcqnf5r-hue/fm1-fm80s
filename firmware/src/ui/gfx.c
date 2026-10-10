// SPDX-License-Identifier: GPL-3.0-only
#include "ui/gfx.h"
#include "app/plat.h"
#include "font.h"

#define CELL_W_MAX 8
#define CELL_H_MAX 12

typedef struct {
    uint8_t w, h;
    const uint8_t *rows;                /* FONT_GLYPHS * h bytes */
} face_t;

static const face_t FACES[GFX_FONTS] = {
    {FONT_6X12_W, FONT_6X12_H, &FONT_6X12[0][0]},
    {FONT_6X10_W, FONT_6X10_H, &FONT_6X10[0][0]},
    {FONT_8X12_W, FONT_8X12_H, &FONT_8X12[0][0]},
};

static const face_t *face = &FACES[GFX_FONT_6X12];

/* one glyph at the largest scale, sent with a single lcd_blit */
static uint16_t glyph[CELL_W_MAX * GFX_SCALE_MAX * CELL_H_MAX * GFX_SCALE_MAX];

/* the panel takes RGB565 with its bytes swapped (as lcd_fill does it) */
static uint16_t swap(uint16_t c) { return (uint16_t)((c >> 8) | (c << 8)); }

static int clamp_scale(int scale)
{
    if (scale < 1)
        return 1;
    if (scale > GFX_SCALE_MAX)
        return GFX_SCALE_MAX;
    return scale;
}

/* ------------------------------------------------------------- shapes --- */
void gfx_fill(int x, int y, int w, int h, uint16_t c)
{
    if (w > 0 && h > 0)
        lcd_fill(x, y, w, h, c);
}

void gfx_hline(int x, int y, int w, uint16_t c) { gfx_fill(x, y, w, 1, c); }
void gfx_vline(int x, int y, int h, uint16_t c) { gfx_fill(x, y, 1, h, c); }

void gfx_frame(int x, int y, int w, int h, uint16_t c)
{
    gfx_hline(x, y, w, c);
    gfx_hline(x, y + h - 1, w, c);
    gfx_vline(x, y + 1, h - 2, c);
    gfx_vline(x + w - 1, y + 1, h - 2, c);
}

void gfx_play_icon(int x, int y, int h, uint16_t c)
{
    int i, half = (h + 1) / 2;          /* 32-bit division by a constant */
    for (i = 0; i < half; i++)
        gfx_vline(x + i, y + i, h - 2 * i, c);
}

/* --------------------------------------------------------------- text --- */
void gfx_font(int f)
{
    if (f >= 0 && f < GFX_FONTS)
        face = &FACES[f];
}

int gfx_font_w(void) { return face->w; }
int gfx_font_h(void) { return face->h; }

void gfx_text(int x, int y, const char *s, int scale, uint16_t fg, uint16_t bg)
{
    uint16_t f = swap(fg), b = swap(bg);
    int w, h, r, c, k, i;
    scale = clamp_scale(scale);
    w = face->w * scale;
    h = face->h * scale;
    for (; *s; s++, x += w) {
        int ch = (unsigned char)*s;
        const uint8_t *rows;
        if (ch < FONT_FIRST || ch > FONT_LAST)
            ch = '?';
        rows = face->rows + (ch - FONT_FIRST) * face->h;
        for (r = 0; r < h; r++) {
            uint8_t bits = rows[r / scale];             /* 32-bit division, scale >= 1 */
            uint16_t *px = &glyph[r * w];
            for (c = 0, i = 0; c < face->w; c++) {
                uint16_t v = bits & (0x80u >> c) ? f : b;
                for (k = 0; k < scale; k++)
                    px[i++] = v;
            }
        }
        lcd_blit(x, y, w, h, glyph);
    }
}

int gfx_text_w(const char *s, int scale)
{
    int n = 0;
    while (s[n])
        n++;
    return n * face->w * clamp_scale(scale);
}

int gfx_text_h(int scale) { return face->h * clamp_scale(scale); }

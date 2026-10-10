// SPDX-License-Identifier: GPL-3.0-only
/* ui/gfx.c over a fake screen: shapes, the three faces bit for bit against build/gen/font.h,
 * scaling, the theme colours as RGB565. */
#include <string.h>
#include "test.h"
#include "ui/gfx.h"
#include "font.h"

#define W 240
#define H 240

static uint16_t fb[W * H];             /* plain RGB565 (the panel's byte swap undone) */

void lcd_fill(int x, int y, int w, int h, uint16_t rgb565)    /* plain colour in */
{
    int i, j;
    for (j = y; j < y + h; j++)
        for (i = x; i < x + w; i++)
            if (i >= 0 && i < W && j >= 0 && j < H)
                fb[j * W + i] = rgb565;
}

/* gfx_text sends byte-swapped pixels, as the panel takes them */

void lcd_blit(int x, int y, int w, int h, const uint16_t *px)
{
    int i, j;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            if (x + i >= 0 && x + i < W && y + j >= 0 && y + j < H)
                fb[(y + j) * W + x + i] = (uint16_t)((px[j * w + i] >> 8) | (px[j * w + i] << 8));
}

static int count(uint16_t c)
{
    int i, n = 0;
    for (i = 0; i < W * H; i++)
        n += fb[i] == c;
    return n;
}

static void test_shapes(void)
{
    memset(fb, 0, sizeof fb);
    gfx_fill(10, 10, 5, 4, 0x1234);
    CHECK_EQ(count(0x1234), 20);
    gfx_fill(10, 10, 0, 4, 0xFFFF);                  /* empty: nothing */
    gfx_fill(10, 10, 4, -1, 0xFFFF);
    CHECK_EQ(count(0xFFFF), 0);
    memset(fb, 0, sizeof fb);
    gfx_frame(0, 0, 10, 6, 0x00FF);
    CHECK_EQ(count(0x00FF), 2 * 10 + 2 * 4);         /* the outline only */
    CHECK_EQ(fb[3 * W + 5], 0);
    memset(fb, 0, sizeof fb);
    gfx_play_icon(100, 100, 8, 0x0F0F);
    CHECK_EQ(count(0x0F0F), 8 + 6 + 4 + 2);          /* a triangle of 8 rows */
    CHECK_EQ(fb[100 * W + 103], 0);
    CHECK_EQ(fb[103 * W + 103], 0x0F0F);
    memset(fb, 0, sizeof fb);
    gfx_fill(235, 235, 20, 20, 0x0101);              /* clipped by the screen */
    CHECK_EQ(count(0x0101), 25);
}

/* every glyph of a face, drawn at (x, y), equals its rows in font.h */
static int face_ok(int f, const uint8_t *rows, int w, int h)
{
    int ch, r, c, bad = 0;
    gfx_font(f);
    CHECK_EQ(gfx_font_w(), w);
    CHECK_EQ(gfx_font_h(), h);
    for (ch = FONT_FIRST; ch <= FONT_LAST; ch++) {
        char s[2] = {(char)ch, 0};
        gfx_text(7, 9, s, 1, 0xFFFF, 0x0000);
        for (r = 0; r < h; r++)
            for (c = 0; c < w; c++) {
                int on = rows[(ch - FONT_FIRST) * h + r] >> (7 - c) & 1;
                if ((fb[(9 + r) * W + 7 + c] == 0xFFFF) != on)
                    bad++;
            }
    }
    return bad;
}

static void test_faces(void)
{
    CHECK_EQ(face_ok(GFX_FONT_6X12, &FONT_6X12[0][0], 6, 12), 0);
    CHECK_EQ(face_ok(GFX_FONT_6X10, &FONT_6X10[0][0], 6, 10), 0);
    CHECK_EQ(face_ok(GFX_FONT_8X12, &FONT_8X12[0][0], 8, 12), 0);
    gfx_font(GFX_FONTS);                             /* refused: the face stays */
    CHECK_EQ(gfx_font_w(), 8);
    CHECK_EQ(gfx_text_w("C-4 7F", 1), 48);
    CHECK_EQ(gfx_text_w("ab", 3), 48);
    CHECK_EQ(gfx_text_h(2), 24);
    /* the 6x10 cut is Terminus 12 without its first and last rows */
    CHECK(!memcmp(FONT_6X10['A' - FONT_FIRST], &FONT_6X12['A' - FONT_FIRST][1], 10));
    /* a phrase row of the spec is 30 cells: 240 px in 8x12, 180 in 6x10 */
    gfx_font(GFX_FONT_8X12);
    CHECK_EQ(gfx_text_w("0 C-4 7F 03 ARP37 CUT40 -----", 1), 232);
    gfx_font(GFX_FONT_6X12);
}

static void test_scale_and_clip(void)
{
    int r, c, bad = 0;
    memset(fb, 0, sizeof fb);
    gfx_font(GFX_FONT_6X12);
    gfx_text(0, 0, "H", 2, 0xFFFF, 0x0001);
    for (r = 0; r < 24; r++)
        for (c = 0; c < 12; c++) {
            int on = FONT_6X12['H' - FONT_FIRST][r / 2] >> (7 - c / 2) & 1;
            if ((fb[r * W + c] == 0xFFFF) != on)
                bad++;
        }
    CHECK_EQ(bad, 0);
    gfx_text(-3, 0, "?", 9, 0xFFFF, 0x0001);         /* scale clamped to 4, off-screen part clipped */
    CHECK_EQ(gfx_text_w("?", 9), 24);
    gfx_text(0, 0, "\x7F", 1, 0xFFFF, 0x0001);       /* outside the font: drawn as '?' */
    CHECK(1);
}

static void test_theme(void)
{
    CHECK_EQ(RGB565(0xFF, 0xFF, 0xFF), 0xFFFF);
    CHECK_EQ(RGB565(0, 0, 0), 0);
    CHECK_EQ(RGB565(0xFF, 0, 0), 0xF800);
    CHECK_EQ(RGB565(0, 0xFF, 0), 0x07E0);
    CHECK_EQ(RGB565(0, 0, 0xFF), 0x001F);
    CHECK(TH_BG != TH_TEXT && TH_CURSOR != TH_BG && TH_PLAYROW != TH_BG);
}

TEST_MAIN(RUN(test_shapes); RUN(test_faces); RUN(test_scale_and_clip); RUN(test_theme);)

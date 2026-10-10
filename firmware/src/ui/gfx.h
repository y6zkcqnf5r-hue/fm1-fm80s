// SPDX-License-Identifier: GPL-3.0-only
/* Drawing on the 240x240 RGB565 screen, over plat.h's lcd_fill / lcd_blit: rectangles, lines,
 * text in one of three generated faces (build/gen/font.h, ADR-027), and the theme's colours.
 * Colours are plain RGB565 here; the byte swap the panel wants happens inside. */
#pragma once
#include <stdint.h>

#define GFX_SCALE_MAX 4

/* the faces, all from Terminus (SIL OFL 1.1) */
enum {
    GFX_FONT_6X12,                      /* Terminus 12 as is: the hello app, scaled titles */
    GFX_FONT_6X10,                      /* cut to 10 rows: 40 columns, a 12 px row with 2 px of air */
    GFX_FONT_8X12,                      /* Terminus 16 cut to 12 rows: 30 columns, a 12 px row */
    GFX_FONTS
};

/* RGB565 from 8-bit r g b, for constants */
#define RGB565(r, g, b) ((uint16_t)(((r) & 0xF8) << 8 | ((g) & 0xFC) << 3 | (b) >> 3))

/* The Trawler theme (a harbour at night, our own: no other tracker's colours). */
#define TH_BG RGB565(0x0B, 0x14, 0x24)          /* deep navy */
#define TH_TEXT RGB565(0xD8, 0xE2, 0xEA)        /* values */
#define TH_DIM RGB565(0x4A, 0x5A, 0x6E)         /* empty cells, row numbers */
#define TH_HEAD RGB565(0x7F, 0xD1, 0xC7)        /* the header: sea foam */
#define TH_NOTE RGB565(0xFF, 0xFF, 0xFF)        /* notes */
#define TH_FX RGB565(0x9A, 0xB8, 0xF0)          /* FX commands */
#define TH_CURSOR RGB565(0xF2, 0xB3, 0x3D)      /* the cursor's background: amber, text in TH_BG */
#define TH_PLAYROW RGB565(0x1E, 0x3A, 0x4F)     /* the row playing */
#define TH_RULE RGB565(0x24, 0x33, 0x48)        /* lines between the zones, every 4th row */
#define TH_K1 RGB565(0xE5, 0x60, 0x4E)          /* the four knob boxes of the footer */
#define TH_K2 RGB565(0xF2, 0xB3, 0x3D)
#define TH_K3 RGB565(0x5F, 0xC7, 0x7E)
#define TH_K4 RGB565(0x5A, 0x9B, 0xE6)

/* ------------------------------------------------------------- shapes --- */
void gfx_fill(int x, int y, int w, int h, uint16_t c);
void gfx_hline(int x, int y, int w, uint16_t c);
void gfx_vline(int x, int y, int h, uint16_t c);
void gfx_frame(int x, int y, int w, int h, uint16_t c);    /* 1 px outline */
void gfx_play_icon(int x, int y, int h, uint16_t c);       /* a right-pointing triangle, h rows */

/* --------------------------------------------------------------- text --- */
void gfx_font(int f);                   /* GFX_FONT_*: the face of the next calls (6x12 at boot) */
int gfx_font_w(void);                   /* cell of the current face, in pixels */
int gfx_font_h(void);
/* s at (x, y), each font pixel drawn as scale x scale (1..GFX_SCALE_MAX), fg on bg; a character
 * outside the font shows as '?' */
void gfx_text(int x, int y, const char *s, int scale, uint16_t fg, uint16_t bg);
int gfx_text_w(const char *s, int scale);   /* width in pixels */
int gfx_text_h(int scale);                  /* height in pixels */

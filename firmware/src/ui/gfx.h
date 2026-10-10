// SPDX-License-Identifier: GPL-3.0-only
/* Drawing on the 240x240 RGB565 screen, over plat.h's lcd_fill / lcd_blit: rectangles, lines,
 * text in one of three generated faces (build/gen/font.h, ADR-027), and the theme's colours.
 * Colours are plain RGB565 here; the byte swap the panel wants happens inside. */
#pragma once
#include <stdint.h>

#define GFX_SCALE_MAX 4

/* the faces, all from Terminus (SIL OFL 1.1) */
enum {
    GFX_FONT_6X12,                      /* Terminus 12 as is: scaled titles */
    GFX_FONT_6X10,                      /* cut to 10 rows: 40 columns, a 12 px row with 2 px of air */
    GFX_FONT_8X12,                      /* Terminus 16 cut to 12 rows: 30 columns, a 12 px row */
    GFX_FONTS
};

/* RGB565 from 8-bit r g b, for constants */
#define RGB565(r, g, b) ((uint16_t)(((r) & 0xF8) << 8 | ((g) & 0xFC) << 3 | (b) >> 3))

/* The FM80S theme: synthwave at night, our own (CLAUDE.md rule 9). */
#define TH_BG RGB565(0x14, 0x0A, 0x26)          /* violet night */
#define TH_TEXT RGB565(0xED, 0xE6, 0xFF)        /* values */
#define TH_DIM RGB565(0x6B, 0x5A, 0x8E)         /* labels */
#define TH_HEAD RGB565(0xFF, 0x3F, 0xA4)        /* the header: magenta */
#define TH_ACCENT RGB565(0x2E, 0xE6, 0xFF)      /* the engine's name: cyan */
#define TH_PAD RGB565(0x26, 0x18, 0x42)         /* a silent pad */
#define TH_RULE RGB565(0x33, 0x22, 0x55)        /* lines between the zones, empty gauges */
#define TH_SELECT RGB565(0xFF, 0xFF, 0xFF)      /* the outline of the sound the knobs set */
#define TH_FAM_KICK RGB565(0xFF, 0x3F, 0xA4)    /* the pads, by family */
#define TH_FAM_SNARE RGB565(0xFF, 0x8A, 0x3D)
#define TH_FAM_METAL RGB565(0xFF, 0xD8, 0x4A)
#define TH_FAM_TOM RGB565(0x2E, 0xE6, 0xFF)
#define TH_FAM_PERC RGB565(0x9B, 0x6B, 0xFF)
#define TH_K1 RGB565(0x2E, 0xE6, 0xFF)          /* KNOB1-4, as their caps on the panel */
#define TH_K2 RGB565(0xFF, 0x3F, 0xA4)
#define TH_K3 RGB565(0xFF, 0xD8, 0x4A)
#define TH_K4 RGB565(0x4C, 0xF0, 0x8A)

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

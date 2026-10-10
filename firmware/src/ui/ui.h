// SPDX-License-Identifier: GPL-3.0-only
/* The interface: reads the controls once per app_frame, plays the sounds, turns the knobs, lights
 * the LEDs and has the screen redraw what changed. In P0-01 one screen, the Drums one
 * (docs/04_SPEC.md §1.2 and §5). */
#pragma once
#include <stdint.h>
#include "app/plat.h"

/* the knobs (Q-04, to be tuned by hand): a detent moves a byte by UI_KNOB_STEP, or by
 * UI_KNOB_FAST when it comes less than UI_KNOB_FAST_MS after the previous one */
#define UI_KNOB_STEP 2
#define UI_KNOB_FAST 8
#define UI_KNOB_FAST_MS 40

/* a key's LED: lit while the key is held or its sound is above UI_LED_LIT, dim while it still
 * sounds (the FM-1 has three levels: off, dim, lit) */
#define UI_LED_LIT 64

typedef struct {
    uint8_t sound;                      /* the sound the knobs set: the last one hit, 0..15 */
} ui_t;

extern ui_t ui;

void ui_init(void);                     /* after track_init() */
void ui_frame(void);                    /* from app_frame */

/* keys: the white index of a key (0..15), -1 for a black one; the bit of white key w */
int ui_white(int key);
uint32_t ui_white_bit(int w);

/* app state for the scenario runner's expect (host/scenario.c) */
enum { UI_PROBE_SOUND };
int ui_probe(int what);

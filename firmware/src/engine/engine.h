// SPDX-License-Identifier: GPL-3.0-only
/* The interface every sound engine implements (docs/05_ARCHITECTURE.md §4, ADR-014).
 *
 * A track holds one engine, ENG_PARAMS bytes of sound (0..255 each, written by the interface only)
 * and the engine's state. The engine runs on the audio side: it plays notes, renders, publishes one
 * level per voice for the screen and the lights, and declares its cost. It never draws: its screen
 * lives in ui/. */
#pragma once
#include <stdint.h>

#define ENG_PARAMS 128                  /* bytes of sound per track */
#define ENG_METERS 16

/* the start of every engine's state */
typedef struct {
    volatile uint8_t meter[ENG_METERS]; /* written by the audio: each voice's level, 0..255 */
} eng_head_t;

typedef struct {
    const char *name;                   /* "DRUMS" */
    uint16_t state_size;                /* checked by core/track.c against the state union */
    const uint8_t *defaults;            /* ENG_PARAMS bytes: the factory sound */
    void (*init)(void *st);
    void (*note_on)(void *st, const uint8_t *p, int note, int vel);   /* drums: note = sound 0..15 */
    void (*note_off)(void *st, const uint8_t *p, int note);
    /* adds n frames L R, Q15, n a multiple of 16: the bytes p are read again every 16 frames, so
     * the knobs act on the notes that sound */
    void (*render)(void *st, const uint8_t *p, int32_t *lr, int n);
    int (*cost)(const void *st);        /* declared MAC per frame, sounding voices only */
} engine_t;

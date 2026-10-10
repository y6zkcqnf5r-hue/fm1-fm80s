// SPDX-License-Identifier: GPL-3.0-only
/* The tracks: one engine each, its bytes of sound and its state (ADR-014). The interface writes the
 * bytes and queues the notes (app/cmdq); the audio plays the notes and renders (ADR-015). In P0-01
 * there is one track, the drums. */
#pragma once
#include <stdint.h>
#include "app/config.h"
#include "engine/drums.h"
#include "engine/engine.h"

/* the state of any engine; a new engine adds its type here */
typedef union {
    eng_head_t head;
    drums_t drums;
} eng_state_t;

typedef struct {
    const engine_t *eng;
    uint8_t p[ENG_PARAMS];              /* written by the interface only, one byte at a time */
    eng_state_t st;                     /* the audio's */
} track_t;

extern track_t track[ENG_TRACKS];

void track_init(void);                  /* before the audio starts */

/* interface side */
void track_note_on(int t, int note);    /* queued for the next audio block */
int track_cost(void);                   /* declared MAC per frame of the last block */

/* audio side: the queued notes, then every track added to lr */
void track_audio(int32_t *lr, int n);

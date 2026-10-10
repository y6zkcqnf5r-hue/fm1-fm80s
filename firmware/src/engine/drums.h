// SPDX-License-Identifier: GPL-3.0-only
/* The Drums engine v1 (docs/04_SPEC.md §2.2, ADR-016 to ADR-020): a kit of 16 synthesized sounds,
 * one per white key, on one track. No samples: sines, square banks and noise through Trawler's
 * filter and envelopes.
 *
 * The track's 128 bytes are 16 sounds of DRUM_BYTES: TUNE, DECAY, TONE, LEVEL, then 4 reserved.
 * One voice per sound, all 16 can sound together. A sound is one-shot: it plays to the end of its
 * decay whether the key is held or not. Hitting it again fades the previous hit out in 2 ms first;
 * the closed hat fades the open one out the same way. */
#pragma once
#include <stdint.h>
#include "dsp/env.h"
#include "dsp/svf.h"
#include "engine/engine.h"

#define DRUM_SOUNDS 16
#define DRUM_BYTES 8
#define DRUM_OSC 6                      /* oscillators of the largest sound, the metallic bank */

enum { DP_TUNE, DP_DECAY, DP_TONE, DP_LEVEL };

enum { DRUM_KICK_L, DRUM_KICK_S, DRUM_SNARE, DRUM_SNARE2, DRUM_CLAP, DRUM_RIM, DRUM_CH, DRUM_OH,
       DRUM_TOM_LO, DRUM_TOM_MID, DRUM_TOM_HI, DRUM_COWBELL, DRUM_CRASH, DRUM_RIDE, DRUM_SHAKER,
       DRUM_ZAP };

/* the families, for the pads' colours */
enum { DFAM_KICK, DFAM_SNARE, DFAM_METAL, DFAM_TOM, DFAM_PERC, DFAM_COUNT };

typedef struct {
    env_t amp;                          /* the level */
    env_t aux;                          /* the snare's tone, the kick's click, the clap's bursts */
    env_t pit;                          /* the pitch sweep */
    uint32_t ph[DRUM_OSC];
    svf_t f1, f2;
    int32_t ga, gb;                     /* the gains reached at the end of the last sub-block, Q15 */
    int16_t cut;                        /* the cutoff the filters are set for, -1 = not yet */
    uint16_t t;                         /* sub-blocks since the hit */
    uint8_t state;                      /* DV_* */
    uint8_t fade;                       /* sub-blocks left in the fade */
    uint8_t again;                      /* a hit waits for the end of the fade */
} drum_voice_t;

typedef struct {
    eng_head_t head;
    uint32_t noise;                     /* one generator for the kit, always running */
    drum_voice_t v[DRUM_SOUNDS];
} drums_t;

extern const engine_t ENGINE_DRUMS;
extern const char *const DRUM_NAME[DRUM_SOUNDS];    /* "KICK L", for the header */
extern const char *const DRUM_SHORT[DRUM_SOUNDS];   /* "KL", 4 characters at most, for the pads */
extern const uint8_t DRUM_FAMILY[DRUM_SOUNDS];      /* DFAM_* */

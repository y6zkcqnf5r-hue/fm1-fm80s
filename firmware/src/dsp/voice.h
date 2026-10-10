// SPDX-License-Identifier: GPL-3.0-only
/* One voice per track (spec §2). Since P1-03 it plays the WAV engine: an oscillator (dsp/osc.h),
 * the state-variable filter (dsp/svf.h), the ADSR (dsp/env.h), VOL and PAN.
 *
 * A new note on a sounding voice first fades it to 0 in 96 frames (6 sub-blocks, 2.2 ms), then
 * starts at phase 0; on a silent voice it starts on the very sample it is called at. Gains and the
 * envelope move once per sub-block of 16 frames (CLAUDE.md rule 5), interpolated across it. The
 * sub-block counter belongs to the voice and restarts with each note, so the output does not depend
 * on how the caller cuts its blocks.
 *
 * Cost (docs/05_ARCHITECTURE.md §7): VOICE_WAV_COST multiply-accumulates per frame, declared, not
 * measured: oscillator 2, filter 5 (64-bit), gains 4, plus the control rate. */
#pragma once
#include <stdint.h>
#include "dsp/env.h"
#include "dsp/osc.h"
#include "dsp/svf.h"

#define VOICE_WAV_COST 12

enum { VOICE_NONE, VOICE_WAV };

/* what a note needs from its instrument, decoded by the tracker (tracker/instr.c) */
typedef struct {
    uint8_t engine;                     /* VOICE_* */
    uint8_t shape;                      /* SHP_* */
    uint8_t flt, cut, res;              /* FLT_*, 00..FF, 00..FF */
    uint8_t atk, dec, sus, rel;
    uint8_t vol, pan;                   /* v/0x80 (80 = 0 dB); 00 left, 80 centre, FF right */
} patch_t;

typedef struct {
    patch_t pt;
    osc_t osc;
    svf_t flt;
    env_t env;
    int32_t level;                      /* note VOL x instrument VOL, Q15 (up to 2.0) */
    int32_t pan_l, pan_r;               /* Q15 */
    int32_t fade;                       /* Q15: 1.0, down to 0 before a new note */
    int32_t gl0, gl1, gr0, gr1;         /* Q15 gains at the start and the end of the sub-block */
    patch_t next_pt;                    /* the note waiting for the fade */
    int32_t next_pitch, next_vol;
    uint8_t state, sub;
} voice_t;

void voice_reset(voice_t *v);           /* silent at once */
/* pitch in 1/16 semitone (note * 16 + fine), vol 00..FF; a patch with no engine releases the voice */
void voice_note(voice_t *v, int pitch, int vol, const patch_t *p);
void voice_legato(voice_t *v, int pitch, const patch_t *p);  /* new pitch; triggers if silent */
void voice_vol(voice_t *v, int vol);
void voice_off(voice_t *v);             /* release */
void voice_filter(voice_t *v, int cut, int res);    /* live CUT and RES of the sounding note */
int voice_active(const voice_t *v);     /* 0 = silent, adds nothing */
int voice_cost(const voice_t *v);       /* declared MAC per frame, 0 when silent */
void voice_render(voice_t *v, int32_t *lr, int n);  /* adds n interleaved L R frames, Q15 */

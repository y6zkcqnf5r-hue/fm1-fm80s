// SPDX-License-Identifier: GPL-3.0-only
/* The Drums engine v1. Each sound is a recipe for one of eight models; its four knobs are read
 * again every sub-block of 16 frames (CLAUDE.md rule 5):
 *
 *   TUNE   +-1 octave on the oscillators; on the noise sounds (clap, shaker) it moves the cutoff
 *   DECAY  the -60 dB time of the level, inside the sound's own range (dlo..dhi)
 *   TONE   the filter's cutoff, inside the sound's range (clo..chi); on the kicks, toms and zap the
 *          level of the click
 *   LEVEL  the volume, 0x80 = 0 dB
 *
 * A voice mixes two sources, A and B, each with its gain interpolated across the sub-block. Times
 * are in the envelope's units (t = 1 ms * 10000^(v/255), dsp/env.h) and cutoffs in the filter's
 * (30 Hz * 600^(v/255), dsp/svf.h). The factory sounds are a first pass, to be tuned by ear. */
#include "engine/drums.h"
#include "dsp/fixed.h"
#include "dsp/noise.h"

#define SUB SUB_FRAMES
#define FADE_SUBS 6                     /* 96 frames, 2.2 ms */
#define ENV_ONE (1 << 24)
#define ENV_END (ENV_ONE >> 12)         /* -72 dB: the hit is over */
#define CLAP_GAP 23                     /* sub-blocks between the clap's bursts: 8.3 ms */
#define CLAP_TAIL (3 * CLAP_GAP)        /* the tail starts after three bursts */
#define BANK_STEP 5461                  /* one square of the bank: 6 of them reach 32766 */

enum { DV_IDLE, DV_PLAY, DV_FADE };

/* A: sine body, B: noise click */          /* A: two sines, B: noise through f1 */
enum { M_KICK, M_SNARE,
       /* A: noise through f1 */            /* A: two sines through f1 */
       M_CLAP, M_RIM,
       /* A: square bank through f1, f2 */  /* A: bank and noise through f1, f2 */
       M_METAL, M_CRASH,
       /* A: two squares through f1 */      /* A: noise through f1, with an attack */
       M_BELL, M_SHAKER };

/* declared MAC per frame of a sounding voice, by model (docs/05_ARCHITECTURE.md §8) */
static const uint8_t COST[8] = {5, 11, 7, 8, 13, 14, 8, 7};

typedef struct {
    uint8_t model;
    int16_t pitch, pitch2;              /* 1/16 semitone; for the bank, an offset */
    int16_t pdepth;                     /* the pitch sweep's start, above the pitch */
    uint8_t pdec;                       /* the sweep's time */
    uint8_t adec;                       /* the aux envelope's time */
    uint8_t amix;                       /* the snare's tone against its noise, /256 */
    uint8_t dlo, dhi;                   /* DECAY 00..FF */
    uint8_t clo, chi;                   /* TONE 00..FF */
    uint8_t res;
    uint8_t mode1, mode2;               /* FLT_* of f1 and f2 (f2 a high-pass under f1, or off) */
    uint8_t atk;                        /* attack time, 0 = at once */
    uint16_t gain;                      /* the filtered source A's gain, /16 (16 = 0 dB) */
} recipe_t;

static const recipe_t R[DRUM_SOUNDS] = {
    /* model     pitch pitch2 pdep pdec adec amix dlo  dhi  clo  chi  res   mode1   mode2  atk gain */
    {M_KICK,     310,    0,  384,  83,  30,   0, 139, 222,   0,   0,    0, FLT_OFF, FLT_OFF, 0, 16},  /* kick L, 50 Hz */
    {M_KICK,     360,    0,  480,  75,  30,   0, 108, 185,   0,   0,    0, FLT_OFF, FLT_OFF, 0, 16},  /* kick S, 60 Hz */
    {M_SNARE,    664,  832,   64,  64, 113, 200, 108, 185, 156, 223,    0, FLT_BP,  FLT_OFF, 0, 38},  /* 180 + 330 Hz */
    {M_SNARE,    720,  878,   48,  58,  94, 110,  94, 172, 167, 232,    0, FLT_BP,  FLT_OFF, 0, 48},  /* 220 + 390 Hz */
    {M_CLAP,       0,    0,    0,   0,  75,   0,  94, 185, 131, 195, 0x50, FLT_BP,  FLT_OFF, 0, 52},
    {M_RIM,      921, 1281,    0,   0,   0,   0,  30, 121, 112, 204, 0x30, FLT_HP,  FLT_OFF, 0, 48},  /* 455 + 1667 Hz */
    {M_METAL,      0,    0,    0,   0,   0,   0,  58, 147, 204, 250, 0x40, FLT_BP,  FLT_HP,  0, 384},  /* closed hat */
    {M_METAL,      0,    0,    0,   0,   0,   0, 139, 210, 204, 250, 0x40, FLT_BP,  FLT_HP,  0, 126},  /* open hat */
    {M_KICK,     502,    0,  192, 128,  30,   0, 121, 203,   0,   0,    0, FLT_OFF, FLT_OFF, 0, 16},  /* tom, 100 Hz */
    {M_KICK,     614,    0,  192, 128,  30,   0, 121, 203,   0,   0,    0, FLT_OFF, FLT_OFF, 0, 16},  /* 150 Hz */
    {M_KICK,     720,    0,  192, 128,  30,   0, 121, 203,   0,   0,    0, FLT_OFF, FLT_OFF, 0, 16},  /* 220 Hz */
    {M_BELL,     969, 1078,    0,   0,   0,   0,  94, 177, 156, 211, 0x60, FLT_BP,  FLT_OFF, 0, 26},  /* 540 + 800 Hz */
    {M_CRASH,      0,    0,    0,   0,   0,   0, 166, 236, 195, 244, 0x20, FLT_BP,  FLT_HP,  0, 50},
    {M_METAL,     48,    0,    0,   0,   0,   0, 147, 230, 176, 232, 0x70, FLT_BP,  FLT_OFF, 0, 39},  /* ride */
    {M_SHAKER,     0,    0,    0,   0,   0,   0,  64, 158, 195, 239, 0x20, FLT_BP,  FLT_OFF, 40, 27},
    {M_KICK,     528,    0,  576,  94,   0,   0,  94, 177,   0,   0,    0, FLT_OFF, FLT_OFF, 0, 16},  /* zap */
};

/* the factory kit: TUNE DECAY TONE LEVEL, then 4 reserved bytes */
static const uint8_t DEFAULTS[ENG_PARAMS] = {
    0x80, 0x97, 0x60, 0x80, 0, 0, 0, 0,    /* kick L */
    0x80, 0xA6, 0xA0, 0x80, 0, 0, 0, 0,    /* kick S */
    0x80, 0x96, 0x80, 0x70, 0, 0, 0, 0,    /* snare */
    0x80, 0x94, 0x90, 0x70, 0, 0, 0, 0,    /* snare 2 */
    0x80, 0x95, 0x70, 0x80, 0, 0, 0, 0,    /* clap */
    0x80, 0xA6, 0x60, 0x68, 0, 0, 0, 0,    /* rim */
    0x80, 0x87, 0x80, 0x60, 0, 0, 0, 0,    /* closed hat */
    0x80, 0x77, 0x80, 0x58, 0, 0, 0, 0,    /* open hat */
    0x80, 0x96, 0x40, 0x68, 0, 0, 0, 0,    /* tom low */
    0x80, 0x96, 0x40, 0x68, 0, 0, 0, 0,    /* tom mid */
    0x80, 0x96, 0x40, 0x68, 0, 0, 0, 0,    /* tom high */
    0x80, 0xA3, 0x50, 0x58, 0, 0, 0, 0,    /* cowbell */
    0x80, 0x9A, 0x80, 0x50, 0, 0, 0, 0,    /* crash */
    0x80, 0x9A, 0x90, 0x50, 0, 0, 0, 0,    /* ride */
    0x80, 0x85, 0x70, 0x60, 0, 0, 0, 0,    /* shaker */
    0x80, 0x8B, 0x00, 0x70, 0, 0, 0, 0,    /* zap */
};

const char *const DRUM_NAME[DRUM_SOUNDS] = {
    "KICK L", "KICK S", "SNARE", "SNARE 2", "CLAP", "RIM", "HAT CL", "HAT OP",
    "TOM LO", "TOM MID", "TOM HI", "COWBELL", "CRASH", "RIDE", "SHAKER", "ZAP",
};
const char *const DRUM_SHORT[DRUM_SOUNDS] = {
    "KL", "KS", "SN", "SN2", "CLP", "RIM", "CH", "OH", "LT", "MT", "HT", "COW", "CRS", "RID", "SHK", "ZAP",
};
const uint8_t DRUM_FAMILY[DRUM_SOUNDS] = {
    DFAM_KICK, DFAM_KICK, DFAM_SNARE, DFAM_SNARE, DFAM_SNARE, DFAM_SNARE, DFAM_METAL, DFAM_METAL,
    DFAM_TOM, DFAM_TOM, DFAM_TOM, DFAM_PERC, DFAM_METAL, DFAM_METAL, DFAM_PERC, DFAM_PERC,
};

/* -fade gain: index = sub-blocks left */
static const int32_t FADE_Q15[FADE_SUBS + 1] = {0, 5461, 10923, 16384, 21845, 27307, 32767};

static int clamp8(int v) { return v < 0 ? 0 : v > 0xFF ? 0xFF : v; }

/* an envelope that starts at full level and decays (or rises first, with an attack) */
static void env_hit(env_t *e, int atk, int dec)
{
    env_start(e, atk, dec, 0, 0);
    if (!atk) {
        e->v = ENV_ONE;
        e->stage = ENV_DEC;
    } else
        e->v = 0;
}

static void env_off(env_t *e)
{
    e->v = 0;
    e->stage = ENV_IDLE;
}

static int decay_of(const recipe_t *r, const uint8_t *q)
{
    return r->dlo + (((r->dhi - r->dlo) * q[DP_DECAY]) >> 8);
}

static void hit(drum_voice_t *v, const recipe_t *r, const uint8_t *q)
{
    int i;
    for (i = 0; i < DRUM_OSC; i++)
        v->ph[i] = 0;
    svf_reset(&v->f1);
    svf_reset(&v->f2);
    v->cut = -1;
    v->ga = v->gb = 0;                  /* from 0: the first sub-block is a 0.36 ms attack */
    v->t = 0;
    v->fade = 0;
    v->again = 0;
    env_hit(&v->amp, r->atk, decay_of(r, q));
    if (r->model == M_CLAP)
        env_off(&v->amp);               /* the tail waits for the bursts */
    env_hit(&v->aux, 0, r->adec);
    env_hit(&v->pit, 0, r->pdec);
    v->state = DV_PLAY;
}

static void drums_init(void *st)
{
    drums_t *d = st;
    int s;
    d->noise = NOISE_SEED;
    for (s = 0; s < DRUM_SOUNDS; s++) {
        d->v[s].state = DV_IDLE;
        d->v[s].ga = d->v[s].gb = 0;
        env_off(&d->v[s].amp);
        d->head.meter[s] = 0;
    }
}

/* fade a sounding voice out; again = 1 hits it once the fade is over */
static void fade_out(drum_voice_t *v, int again)
{
    if (v->state == DV_PLAY) {
        v->state = DV_FADE;
        v->fade = FADE_SUBS;
    }
    if (v->state == DV_FADE)
        v->again = (uint8_t)again;
}

static void drums_note_on(void *st, const uint8_t *p, int note, int vel)
{
    drums_t *d = st;
    drum_voice_t *v;
    (void)vel;                          /* no velocity on the FM-1: every hit is the same */
    if (note < 0 || note >= DRUM_SOUNDS)
        return;
    v = &d->v[note];
    if (note == DRUM_CH)
        fade_out(&d->v[DRUM_OH], 0);
    if (v->state == DV_IDLE)
        hit(v, &R[note], p + note * DRUM_BYTES);
    else
        fade_out(v, 1);
}

static void drums_note_off(void *st, const uint8_t *p, int note)
{
    (void)st;                           /* one-shot */
    (void)p;
    (void)note;
}

static inline uint32_t sq_bit(uint32_t ph) { return ph >> 31; }

/* a filter's output times gain / 16, clipped to Q15 (a filtered bank or noise is quiet) */
static inline int32_t boost(int32_t y, int gain) { return sat16((int32_t)(((int64_t)y * gain) >> 4)); }

/* one sub-block of voice s, added to lr */
static void voice_sub(drums_t *d, int s, const uint8_t *q, int32_t *lr)
{
    const recipe_t *r = &R[s];
    drum_voice_t *v = &d->v[s];
    int tune = (((int)q[DP_TUNE] - 128) * 3) >> 1;     /* 1/16 semitone, -192..190 */
    int lvl = q[DP_LEVEL], tone = q[DP_TONE];
    int32_t a, x, sweep, ga0 = v->ga, gb0 = v->gb, ga1, gb1, m;
    uint32_t inc[DRUM_OSC];
    int i, k, nosc = 0;

    /* the envelopes: DECAY acts on the sounding hit */
    v->amp.dec = ENV_COEF_Q24[decay_of(r, q)];
    if (r->model == M_CLAP) {
        if (v->t == CLAP_GAP || v->t == 2 * CLAP_GAP)
            env_hit(&v->aux, 0, r->adec);
        else if (v->t == CLAP_TAIL)
            env_hit(&v->amp, 0, decay_of(r, q));
    }
    a = env_step(&v->amp);
    x = env_step(&v->aux);
    sweep = (r->pdepth * env_step(&v->pit)) >> 15;

    /* the oscillators and the filters */
    switch (r->model) {
    case M_KICK:
        inc[0] = pitch_inc(r->pitch + tune + sweep);
        nosc = 1;
        break;
    case M_SNARE:
    case M_RIM:
    case M_BELL:
        inc[0] = pitch_inc(r->pitch + tune + sweep);
        inc[1] = pitch_inc(r->pitch2 + tune + sweep);
        nosc = 2;
        break;
    case M_METAL:
    case M_CRASH:
        for (k = 0; k < DRUM_OSC; k++)
            inc[k] = pitch_inc(DRUM_METAL_P16[k] + r->pitch + tune);
        nosc = DRUM_OSC;
        break;
    }
    if (r->mode1 != FLT_OFF) {
        int cut = r->clo + (((r->chi - r->clo) * tone) >> 8);
        if (r->model == M_CLAP || r->model == M_SHAKER)
            cut += tune >> 3;
        cut = clamp8(cut);
        if (cut != v->cut) {
            svf_set(&v->f1, r->mode1, cut, r->res);
            svf_set(&v->f2, r->mode2, clamp8(cut - 20), 0);
            v->cut = (int16_t)cut;
        }
    }

    /* the gains at the end of this sub-block */
    switch (r->model) {
    case M_KICK:
        ga1 = a;
        gb1 = (x * tone) >> 8;
        break;
    case M_SNARE:
        ga1 = (x * r->amix) >> 8;
        gb1 = a;
        break;
    case M_CLAP:
        ga1 = x + a > Q15_ONE ? Q15_ONE : x + a;
        gb1 = 0;
        break;
    default:
        ga1 = a;
        gb1 = 0;
    }
    ga1 = (ga1 * lvl) >> 7;
    gb1 = (gb1 * lvl) >> 7;
    if (v->state == DV_FADE) {
        v->fade--;
        ga1 = (ga1 * FADE_Q15[v->fade]) >> 15;
        gb1 = (gb1 * FADE_Q15[v->fade]) >> 15;
    }

    for (i = 0; i < SUB; i++) {
        int32_t A = 0, B = 0, y, w;
        int32_t ga = ga0 + (((ga1 - ga0) * (i + 1)) >> 4), gb = gb0 + (((gb1 - gb0) * (i + 1)) >> 4);
        switch (r->model) {
        case M_KICK:
            A = sin_q15(v->ph[0]);
            B = gb ? noise_q15(&d->noise) : 0;
            break;
        case M_SNARE:
            A = (sin_q15(v->ph[0]) + sin_q15(v->ph[1])) >> 1;
            B = boost(svf_run(&v->f1, noise_q15(&d->noise)), r->gain);
            break;
        case M_CLAP:
        case M_SHAKER:
            A = boost(svf_run(&v->f1, noise_q15(&d->noise)), r->gain);
            break;
        case M_RIM:
            A = boost(svf_run(&v->f1, (sin_q15(v->ph[0]) + sin_q15(v->ph[1])) >> 1), r->gain);
            break;
        case M_BELL:
            w = (sq_bit(v->ph[0]) ? -16383 : 16383) + (sq_bit(v->ph[1]) ? -16383 : 16383);
            A = boost(svf_run(&v->f1, w), r->gain);
            break;
        case M_METAL:
        case M_CRASH:
            for (k = 0, w = 0; k < DRUM_OSC; k++)
                w += sq_bit(v->ph[k]) ? -BANK_STEP : BANK_STEP;
            if (r->model == M_CRASH)
                w = (w + noise_q15(&d->noise)) >> 1;
            A = boost(svf_run(&v->f2, sat16(svf_run(&v->f1, w))), r->gain);
            break;
        }
        for (k = 0; k < nosc; k++)
            v->ph[k] += inc[k];
        y = ((A * ga) >> 15) + ((B * gb) >> 15);
        y >>= 1;                        /* headroom: 16 sounds at once */
        lr[2 * i] += y;
        lr[2 * i + 1] += y;
    }
    v->ga = ga1;
    v->gb = gb1;
    v->t++;

    /* the end of the hit, or of the fade */
    if (v->state == DV_FADE && !v->fade) {
        if (v->again)
            hit(v, r, q);
        else
            v->state = DV_IDLE;
    } else if (v->state == DV_PLAY && v->amp.stage != ENV_ATK && v->amp.v < ENV_END &&
               (r->model != M_CLAP || v->t > CLAP_TAIL))
        v->state = DV_IDLE;
    m = ga1 > gb1 ? ga1 : gb1;
    d->head.meter[s] = (uint8_t)(v->state == DV_IDLE ? 0 : m >> 7 > 255 ? 255 : m >> 7);
}

static void drums_render(void *st, const uint8_t *p, int32_t *lr, int n)
{
    drums_t *d = st;
    int b, s;
    for (b = 0; b + SUB <= n; b += SUB)
        for (s = 0; s < DRUM_SOUNDS; s++)
            if (d->v[s].state != DV_IDLE)
                voice_sub(d, s, p + s * DRUM_BYTES, lr + 2 * b);
}

static int drums_cost(const void *st)
{
    const drums_t *d = st;
    int s, c = 0;
    for (s = 0; s < DRUM_SOUNDS; s++)
        if (d->v[s].state != DV_IDLE)
            c += COST[R[s].model];
    return c;
}

const engine_t ENGINE_DRUMS = {
    "DRUMS", (uint16_t)sizeof(drums_t), DEFAULTS,
    drums_init, drums_note_on, drums_note_off, drums_render, drums_cost,
};

// SPDX-License-Identifier: GPL-3.0-only
#include "dsp/voice.h"

#define SUB_SHIFT 4                     /* 16 frames */
#define FADE_STEP 5462                  /* 1.0 -> 0 in 6 sub-blocks: the 2 ms fade of a new note */

enum { V_IDLE, V_ON, V_REL, V_FADE };

static int32_t vol_q15(int vol) { return vol << 7 | vol >> 1; }   /* FF -> 32767 */

void voice_reset(voice_t *v)
{
    osc_pitch(&v->osc, 0);
    v->osc.ph = 0;
    svf_reset(&v->flt);
    v->env.v = 0;
    v->env.stage = ENV_IDLE;
    v->level = v->next_vol = v->next_pitch = 0;
    v->pan_l = v->pan_r = Q15_ONE;
    v->fade = Q15_ONE;
    v->gl0 = v->gl1 = v->gr0 = v->gr1 = 0;
    v->pt.engine = v->next_pt.engine = VOICE_NONE;
    v->state = V_IDLE;
    v->sub = 0;
}

int voice_active(const voice_t *v) { return v->state != V_IDLE || v->gl1 || v->gr1 || v->sub; }

int voice_cost(const voice_t *v) { return voice_active(v) && v->pt.engine == VOICE_WAV ? VOICE_WAV_COST : 0; }

static void set_level(voice_t *v, int vol) { v->level = (vol_q15(vol) * v->pt.vol) >> 7; }

static void start(voice_t *v, int pitch, int vol, const patch_t *p)
{
    v->pt = *p;
    osc_pitch(&v->osc, pitch_inc(pitch));
    v->osc.ph = 0;
    v->flt.ic1 = v->flt.ic2 = 0;
    svf_set(&v->flt, p->flt, p->cut, p->res);
    v->env.v = 0;
    env_start(&v->env, p->atk, p->dec, p->sus, p->rel);
    set_level(v, vol);
    /* balance: the far side goes down, the near one stays at 1.0 */
    v->pan_l = p->pan <= 0x80 ? Q15_ONE : (0xFF - p->pan) * 258;
    v->pan_r = p->pan >= 0x80 ? Q15_ONE : p->pan * 258;
    v->fade = Q15_ONE;
    v->state = V_ON;
    v->sub = 0;                         /* the attack starts on this sample */
}

/* end the sub-block here, at the gains reached so far: the fade starts on this sample */
static void cut_sub(voice_t *v)
{
    if (v->sub) {
        v->gl1 = v->gl0 + (((v->gl1 - v->gl0) * v->sub) >> SUB_SHIFT);
        v->gr1 = v->gr0 + (((v->gr1 - v->gr0) * v->sub) >> SUB_SHIFT);
        v->sub = 0;
    }
}

void voice_note(voice_t *v, int pitch, int vol, const patch_t *p)
{
    if (p->engine == VOICE_NONE) {
        voice_off(v);
        return;
    }
    if (!voice_active(v)) {
        start(v, pitch, vol, p);
        return;
    }
    cut_sub(v);
    v->next_pt = *p;
    v->next_pitch = pitch;
    v->next_vol = vol;
    v->state = V_FADE;
}

void voice_legato(voice_t *v, int pitch, const patch_t *p)
{
    if (v->state == V_ON)
        osc_pitch(&v->osc, pitch_inc(pitch));
    else if (v->state == V_FADE)
        v->next_pitch = pitch;
    else
        voice_note(v, pitch, 0xFF, p);
}

void voice_vol(voice_t *v, int vol)
{
    if (v->state == V_FADE)
        v->next_vol = vol;
    else
        set_level(v, vol);
}

void voice_off(voice_t *v)
{
    if (v->state == V_ON) {
        env_release(&v->env);
        v->state = V_REL;
    } else if (v->state == V_FADE)      /* the fade goes on, the waiting note will not start */
        v->next_pt.engine = VOICE_NONE;
}

void voice_filter(voice_t *v, int cut, int res)
{
    v->pt.cut = (uint8_t)cut;
    v->pt.res = (uint8_t)res;
    svf_set(&v->flt, v->pt.flt, cut, res);   /* the states go on: no click */
}

/* the gains of the next sub-block */
static void next_sub(voice_t *v)
{
    int32_t e, g;
    v->gl0 = v->gl1;
    v->gr0 = v->gr1;
    if (v->state == V_FADE && v->fade == 0) {
        if (v->next_pt.engine != VOICE_NONE)
            start(v, v->next_pitch, v->next_vol, &v->next_pt);
        else {
            v->env.v = 0;
            v->env.stage = ENV_IDLE;
            v->state = V_IDLE;
        }
    }
    e = env_step(&v->env);
    if (v->state == V_FADE)
        v->fade = v->fade > FADE_STEP ? v->fade - FADE_STEP : 0;
    else if (v->state == V_REL && v->env.stage == ENV_IDLE)
        v->state = V_IDLE;
    g = (((e * v->level) >> 15) * v->fade) >> 15;   /* up to 2.0 in Q15 */
    v->gl1 = (g * v->pan_l) >> 15;
    v->gr1 = (g * v->pan_r) >> 15;
}

void voice_render(voice_t *v, int32_t *lr, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        int32_t gl, gr, y;
        if (v->sub == 0) {
            if (!voice_active(v))
                return;                 /* silent: adds nothing, and nothing to keep in step */
            next_sub(v);
        }
        gl = v->gl0 + (((v->gl1 - v->gl0) * (v->sub + 1)) >> SUB_SHIFT);
        gr = v->gr0 + (((v->gr1 - v->gr0) * (v->sub + 1)) >> SUB_SHIFT);
        y = sat16(svf_run(&v->flt, osc_run(&v->osc, v->pt.shape)));   /* resonance clips here */
        lr[2 * i] += (y * gl) >> 16;    /* -6 dB: a full wave is +-16383 at 0 dB */
        lr[2 * i + 1] += (y * gr) >> 16;
        v->sub = (uint8_t)((v->sub + 1) & 15);
    }
}

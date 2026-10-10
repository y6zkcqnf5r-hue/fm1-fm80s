// SPDX-License-Identifier: GPL-3.0-only
/* The WAV engine (P1-03): polyBLEP oscillators against naive ones (spectrum), the SVF against a
 * double-precision model of the same equations, no overflow at full resonance, ADSR timing, VOL
 * and PAN, and the reference renders of each waveform (build/renders/eng_wav, golden). */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "render.h"
#include "dsp/env.h"
#include "dsp/osc.h"
#include "dsp/svf.h"
#include "dsp/voice.h"

#define N 4096

static double hann[N];

/* largest component away from the harmonics of f0 and below 12 kHz, where folding is heard
 * (dB below the fundamental), Hann window */
static double alias_db(const double *x, double f0)
{
    static double re[N / 2], im[N / 2];
    double fund = 0, worst = 0;
    int k, i;
    for (k = 1; k < N / 2; k++) {
        double r = 0, m = 0, w = 2 * M_PI * k / N;
        for (i = 0; i < N; i++) {
            r += x[i] * hann[i] * cos(w * i);
            m -= x[i] * hann[i] * sin(w * i);
        }
        re[k] = r;
        im[k] = m;
    }
    for (k = 1; k < N / 2; k++) {
        double mag = sqrt(re[k] * re[k] + im[k] * im[k]), fk = (double)k * FS / N;
        double h = fk / f0, near = fabs(h - floor(h + 0.5)) * f0 * N / FS;   /* bins to a harmonic */
        if (fabs(fk - f0) < 3.0 * FS / N)
            fund = mag > fund ? mag : fund;
        else if (near > 4 && fk < 12000 && mag > worst)
            worst = mag;
    }
    return 20 * log10(worst / fund);
}

static void test_blep(void)
{
    static double a[N], b[N];
    static const int shapes[2] = {SHP_SAW, SHP_SQR};
    osc_t o;
    int s, i, note = 93;                             /* A-7, 3520 Hz: the harmonics fold back */
    double f0 = 440.0 * pow(2.0, (note - NOTE_A4) / 12.0);
    for (i = 0; i < N; i++)
        hann[i] = 0.5 - 0.5 * cos(2 * M_PI * i / N);
    for (s = 0; s < 2; s++) {
        double da, db;
        osc_pitch(&o, note_inc(note, 0));
        o.ph = 0;
        for (i = 0; i < N; i++) {
            uint32_t t = o.ph >> 16;
            b[i] = shapes[s] == SHP_SAW ? (double)t - 32768 : (t < 32768 ? 32767 : -32767);
            a[i] = osc_run(&o, shapes[s]);
        }
        da = alias_db(a, f0);
        db = alias_db(b, f0);
        printf("  %s at 3520 Hz: aliasing below 12 kHz %.1f dB (polyBLEP), %.1f dB (naive)\n",
               s ? "square" : "saw", da, db);
        CHECK(da < db - 12);
    }
    /* the triangle and the sine stay in range */
    osc_pitch(&o, note_inc(48, 0));
    for (i = 0, o.ph = 0; i < 2000; i++) {
        int32_t t = osc_run(&o, SHP_TRI);
        CHECK(t >= -32768 && t <= 32767);
        if (test_fails)
            break;
    }
}

static void test_div(void)
{
    uint32_t d;
    CHECK_EQ(div_pow2(46, 65536), 1u << 30);
    CHECK_EQ(div_pow2(46, 0), 0);
    CHECK_EQ(div_pow2(10, 3), 341);
    for (d = 65536; d < 0x7FFFFFFFu - 99991; d += 99991) {
        uint64_t want = (1ull << 46) / d;
        if (div_pow2(46, d) != (uint32_t)want) {
            CHECK_EQ(div_pow2(46, d), want);
            break;
        }
    }
    CHECK(1);
}

/* the same equations in double, with the same g and k, as a reference */
typedef struct { double ic1, ic2, a1, a2, a3, k; int mode; } ref_t;

/* the coefficients of f, as svf_set() quantized them: the reference then checks the arithmetic;
 * their accuracy against the exact formula is checked on its own (test_svf_coefs) */
static void ref_set(ref_t *r, const svf_t *f, int mode)
{
    r->a1 = f->a1 / 1073741824.0;
    r->a2 = f->a2 / 1073741824.0;
    r->a3 = f->a3 / 1073741824.0;
    r->k = f->k / 65536.0;
    r->mode = mode;
}

static void test_svf_coefs(void)
{
    int cut, res;
    double worst = 0;
    for (cut = 0; cut < 256; cut++)
        for (res = 0; res < 256; res += 15) {
            svf_t f;
            double g = SVF_G_Q16[cut] / 65536.0, k = (131072 - res * 508) / 65536.0;
            double a1 = 1 / (1 + g * (g + k)), e;
            svf_set(&f, FLT_LP, cut, res);
            e = fabs(f.a1 / 1073741824.0 - a1) + fabs(f.a2 / 1073741824.0 - g * a1) +
                fabs(f.a3 / 1073741824.0 - g * g * a1);
            if (e > worst)
                worst = e;
        }
    printf("  svf coefficients: worst error %.2g (Q30 LSB = %.2g)\n", worst, 1 / 1073741824.0);
    CHECK(worst < 2e-6);
    CHECK_EQ(SVF_G_Q16[0], (uint32_t)lround(tan(M_PI * 30.0 / FS) * 65536));
}

static double ref_run(ref_t *r, double x)
{
    double v3 = x - r->ic2, v1 = r->a1 * r->ic1 + r->a2 * v3, v2 = r->ic2 + r->a2 * r->ic1 + r->a3 * v3;
    r->ic1 = 2 * v1 - r->ic1;
    r->ic2 = 2 * v2 - r->ic2;
    return r->mode == FLT_LP ? v2 : r->mode == FLT_BP ? v1 : x - r->k * v1 - v2;
}

/* fixed point against the double model, white noise in, every mode, low to high cutoff and resonance */
static void test_svf(void)
{
    static const int cuts[4] = {0x00, 0x40, 0xA0, 0xFF}, ress[3] = {0x00, 0x80, 0xFF};
    int m, c, r, i;
    double worst = 0;
    for (m = FLT_LP; m <= FLT_BP; m++)
        for (c = 0; c < 4; c++)
            for (r = 0; r < 3; r++) {
                svf_t f;
                ref_t rf = {0, 0, 0, 0, 0, 0, 0};
                uint32_t lcg = 1;
                double peak = 1, err = 0;
                svf_reset(&f);
                svf_set(&f, m, cuts[c], ress[r]);
                ref_set(&rf, &f, m);
                for (i = 0; i < 20000; i++) {
                    int32_t x;
                    double y, e;
                    lcg = lcg * 1103515245u + 12345u;
                    x = (int32_t)(lcg >> 17) - 16384;   /* white noise, -6 dB */
                    y = ref_run(&rf, x);
                    e = fabs(svf_run(&f, x) - y);
                    if (fabs(y) > peak)
                        peak = fabs(y);
                    if (e > err)
                        err = e;
                }
                if (err / peak > worst)
                    worst = err / peak;
                CHECK(err <= 16 + peak * 0.005);
            }
    printf("  svf: worst error %.4f %% of the peak, against double\n", 100 * worst);
}

/* RES FF, a full-scale square, the cutoff swept up and down: the states stay far from overflowing,
 * and the fixed-point filter still follows the double one */
static void test_svf_saturation(void)
{
    static const int notes[4] = {24, 48, 72, 96};
    int m, n, i;
    int32_t worst_state = 0;
    double worst_out = 0, worst_err = 0;
    for (m = FLT_LP; m <= FLT_BP; m++)
        for (n = 0; n < 4; n++) {
            svf_t f;
            ref_t rf = {0, 0, 0, 0, 0, 0, 0};
            osc_t o;
            svf_reset(&f);
            osc_pitch(&o, note_inc(notes[n], 0));
            o.ph = 0;
            for (i = 0; i < 2 * 256 * 64; i++) {
                int cut = (i / 64) % 512, x;
                double y, e;
                if (cut > 255)
                    cut = 511 - cut;
                if (i % 64 == 0) {
                    svf_set(&f, m, cut, 0xFF);
                    ref_set(&rf, &f, m);
                }
                x = osc_run(&o, SHP_SQR);
                y = ref_run(&rf, x);
                e = fabs(svf_run(&f, x) - y);
                if (abs(f.ic1) > worst_state)
                    worst_state = abs(f.ic1);
                if (abs(f.ic2) > worst_state)
                    worst_state = abs(f.ic2);
                if (fabs(y) > worst_out)
                    worst_out = fabs(y);
                if (e > worst_err)
                    worst_err = e;
            }
        }
    printf("  svf at RES FF: largest state %d (2^%.1f), largest output %.0f, error %.0f\n", worst_state,
           log2(worst_state), worst_out, worst_err);
    CHECK(worst_state < (1 << 29));                  /* the int32 holds 2^31 */
    CHECK(worst_err < 0.01 * worst_out + 64);
}

static int steps_to(env_t *e, int stage_left)
{
    int n = 0;
    while (e->stage != stage_left && n < 100000) {
        env_step(e);
        n++;
    }
    return n;
}

static void test_env(void)
{
    env_t e = {0, 0, 0, 0, 0, ENV_IDLE};
    int n, want = (int)(0.1018 * FS / 16);           /* t(80) = 101.8 ms, in sub-blocks */
    double dist;
    env_start(&e, 0x80, 0x80, 0x80, 0x80);
    n = steps_to(&e, ENV_DEC);                       /* linear attack */
    CHECK(abs(n - want) <= 2);
    for (n = 0; n < want; n++)                       /* -60 dB of the way to SUS in t(DEC) */
        env_step(&e);
    dist = fabs((double)(e.v - e.sus)) / (double)((1 << 24) - e.sus);
    CHECK(dist < 0.0015);
    env_release(&e);
    n = steps_to(&e, ENV_IDLE);                      /* from 0.5 to -72 dB: 66 dB, at 60 dB per t(REL) */
    CHECK(n > want * 66 / 60 * 95 / 100 && n < want * 66 / 60 * 105 / 100);
    CHECK_EQ(e.v, 0);
    CHECK_EQ(env_step(&e), 0);
    env_start(&e, 0, 0, 0xFF, 0);                    /* 1 ms attack: 3 sub-blocks */
    e.v = 0;
    CHECK_EQ(steps_to(&e, ENV_DEC), 3);
    env_start(&e, 0, 0, 0, 0xFF);                    /* a 10 s release still ends */
    env_release(&e);
    e.v = 1 << 24;
    n = steps_to(&e, ENV_IDLE);
    CHECK(n < 100000);
    printf("  env: attack %d sub-blocks (want %d), 10 s release ends after %d\n", want, want, n);
}

static patch_t patch(int shape, int flt, int cut, int res)
{
    patch_t p;
    p.engine = VOICE_WAV;
    p.shape = (uint8_t)shape;
    p.flt = (uint8_t)flt;
    p.cut = (uint8_t)cut;
    p.res = (uint8_t)res;
    p.atk = 0x20;
    p.dec = 0x90;
    p.sus = 0xA0;
    p.rel = 0x80;
    p.vol = 0x80;
    p.pan = 0x80;
    return p;
}

static int32_t lr[2 * FS];

static int32_t peak_of(int ch, int n)
{
    int32_t pk = 0;
    int i;
    for (i = 0; i < n; i++)
        if (abs(lr[2 * i + ch]) > pk)
            pk = abs(lr[2 * i + ch]);
    return pk;
}

static void play(const patch_t *p, int note, int gate, int total)
{
    voice_t v;
    memset(lr, 0, sizeof lr);
    voice_reset(&v);
    voice_note(&v, note * 16, 0xFF, p);
    voice_render(&v, lr, gate);
    voice_off(&v);
    voice_render(&v, lr + 2 * gate, total - gate);
}

static void test_vol_pan(void)
{
    patch_t p = patch(SHP_SIN, FLT_OFF, 0xFF, 0);
    int32_t full, half, twice;
    int i, same = 1;
    p.sus = 0xFF;
    play(&p, 57, 8000, 8000);
    full = peak_of(0, 8000);
    for (i = 0; i < 8000; i++)
        if (lr[2 * i] != lr[2 * i + 1])
            same = 0;
    CHECK(same);                                     /* 80: centre, both sides at 0 dB */
    CHECK(full > 16000 && full <= 16383);
    p.vol = 0x40;
    play(&p, 57, 8000, 8000);
    half = peak_of(0, 8000);
    p.vol = 0xFF;
    play(&p, 57, 8000, 8000);
    twice = peak_of(0, 8000);
    CHECK(abs(2 * half - full) < full / 100);
    CHECK(abs(twice - 2 * full * 255 / 256) < full / 50);
    p.vol = 0x80;
    p.pan = 0x00;
    play(&p, 57, 8000, 8000);
    CHECK_EQ(peak_of(1, 8000), 0);
    CHECK(peak_of(0, 8000) == full);
    p.pan = 0xFF;
    play(&p, 57, 8000, 8000);
    CHECK_EQ(peak_of(0, 8000), 0);
    p.engine = VOICE_NONE;                           /* no engine: nothing */
    play(&p, 57, 8000, 8000);
    CHECK_EQ(peak_of(0, 8000), 0);
}

/* the reference renders: C-3, 0.6 s held, then 0.4 s of release */
static void test_renders(void)
{
    static const struct { const char *name; int shape, flt, cut, res; } R[] = {
        {"saw.wav", SHP_SAW, FLT_OFF, 0xFF, 0},
        {"square.wav", SHP_SQR, FLT_OFF, 0xFF, 0},
        {"triangle.wav", SHP_TRI, FLT_OFF, 0xFF, 0},
        {"saw_lp_res.wav", SHP_SAW, FLT_LP, 0x70, 0xE0},
        {"square_hp.wav", SHP_SQR, FLT_HP, 0x90, 0x40},
        {"saw_bp_res_max.wav", SHP_SAW, FLT_BP, 0x80, 0xFF},
    };
    int i;
    for (i = 0; i < (int)(sizeof R / sizeof *R); i++) {
        patch_t p = patch(R[i].shape, R[i].flt, R[i].cut, R[i].res);
        int32_t pk;
        play(&p, 36, FS * 6 / 10, FS);
        pk = peak_of(0, FS);
        CHECK(pk > 1000 && pk <= 32767);
        CHECK_EQ(lr[2 * (FS - 1)], 0);               /* released to silence */
        CHECK_EQ(render_wav("eng_wav", R[i].name, lr, FS), 0);
    }
}

TEST_MAIN(RUN(test_blep); RUN(test_div); RUN(test_svf_coefs); RUN(test_svf); RUN(test_svf_saturation); RUN(test_env);
          RUN(test_vol_pan); RUN(test_renders);)

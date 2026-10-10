// SPDX-License-Identifier: GPL-3.0-only
/* The Drums engine v1 (P0-01): each of the 16 sounds rendered alone with the factory kit
 * (build/renders/drums/, golden after listening), the closed hat cutting the open one, a hit again
 * fading the previous one in 2 ms, the knobs acting on a sounding hit, the 16 sounds at once with
 * room left for MASTER, the noise's determinism, the declared cost. */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "render.h"
#include "dsp/fixed.h"
#include "dsp/noise.h"
#include "engine/drums.h"

#define MAXF (4 * FS)                   /* the longest render: 4 s */
#define BLK 256
/* times as whole blocks: the engine renders by sub-blocks of 16 frames */
#define T50 (9 * BLK)                   /* 52 ms */
#define T100 (17 * BLK)                 /* 99 ms */
#define T200 (34 * BLK)                 /* 197 ms */
#define T1S (172 * BLK)                 /* 0.998 s */

static int32_t lr[2 * (MAXF + BLK)];
static drums_t d;
static uint8_t p[ENG_PARAMS];

static void reset(void)
{
    memcpy(p, ENGINE_DRUMS.defaults, sizeof p);
    ENGINE_DRUMS.init(&d);
}

/* render frames [from, to) of lr, by blocks of 256 like app_audio */
static void run(int from, int to)
{
    int f;
    for (f = from; f < to; f += BLK)
        ENGINE_DRUMS.render(&d, p, lr + 2 * f, to - f < BLK ? to - f : BLK);
}

static int active(int s) { return d.v[s].state != 0; }

static int32_t peak(int from, int to)
{
    int32_t pk = 0;
    int i;
    for (i = 2 * from; i < 2 * to; i++)
        if (abs(lr[i]) > pk)
            pk = abs(lr[i]);
    return pk;
}

static double rms(int from, int to)
{
    double e = 0;
    int i;
    for (i = from; i < to; i++)
        e += (double)lr[2 * i] * lr[2 * i];
    return sqrt(e / (to - from));
}

/* zero crossings per second of the left channel, upward */
static double freq(int from, int to)
{
    int i, n = 0;
    for (i = from + 1; i < to; i++)
        n += lr[2 * (i - 1)] < 0 && lr[2 * i] >= 0;
    return n * (double)FS / (to - from);
}

static void test_noise(void)
{
    uint32_t a = NOISE_SEED, b = NOISE_SEED;
    int i, lo = 0, hi = 0, same = 1;
    double sum = 0;
    for (i = 0; i < 100000; i++) {
        int32_t x = noise_q15(&a);
        same &= x == noise_q15(&b);
        lo = x < lo ? x : lo;
        hi = x > hi ? x : hi;
        sum += x;
    }
    CHECK(same);
    CHECK(lo < -32000 && hi > 32000);
    CHECK(fabs(sum / 100000) < 300);
    CHECK(a != 0);
}

/* every sound alone: it sounds, stays in range, ends, and its render is kept for listening */
static void test_renders(void)
{
    static const char *const FILE_NAME[DRUM_SOUNDS] = {
        "01_kick_l.wav", "02_kick_s.wav", "03_snare.wav", "04_snare2.wav", "05_clap.wav", "06_rim.wav",
        "07_hat_closed.wav", "08_hat_open.wav", "09_tom_low.wav", "10_tom_mid.wav", "11_tom_high.wav",
        "12_cowbell.wav", "13_crash.wav", "14_ride.wav", "15_shaker.wav", "16_zap.wav",
    };
    int s;
    for (s = 0; s < DRUM_SOUNDS; s++) {
        int n = 0;
        int32_t pk;
        reset();
        memset(lr, 0, sizeof lr);
        ENGINE_DRUMS.note_on(&d, p, s, 0xFF);
        CHECK(ENGINE_DRUMS.cost(&d) > 0);
        while (n < MAXF && (active(s) || n < FS / 10)) {
            run(n, n + BLK);
            n += BLK;
        }
        pk = peak(0, n);
        printf("  %-8s %5.0f ms, peak %5d (%5.1f dBFS), rms 50 ms %5.0f\n", DRUM_NAME[s], 1000.0 * n / FS, pk,
               20 * log10(pk / 32768.0), rms(0, FS / 20));
        CHECK(!active(s));                          /* every sound ends within 4 s */
        CHECK(pk > 2000 && pk < 32768);
        CHECK_EQ(d.head.meter[s], 0);
        CHECK_EQ(ENGINE_DRUMS.cost(&d), 0);
        CHECK_EQ(render_wav("drums", FILE_NAME[s], lr, n), 0);
    }
}

static void test_choke(void)
{
    int f;
    reset();
    memset(lr, 0, sizeof lr);
    ENGINE_DRUMS.note_on(&d, p, DRUM_OH, 0xFF);
    run(0, T100);
    CHECK(active(DRUM_OH) && d.head.meter[DRUM_OH] > 0);
    ENGINE_DRUMS.note_on(&d, p, DRUM_CH, 0xFF);
    f = T100;
    run(f, f + 96);                                 /* the fade: 6 sub-blocks */
    CHECK(!active(DRUM_OH));
    CHECK_EQ(d.head.meter[DRUM_OH], 0);
    CHECK(active(DRUM_CH));
    /* the other way round: the open hat leaves the closed one alone */
    ENGINE_DRUMS.note_on(&d, p, DRUM_OH, 0xFF);
    run(f + 96, f + 192);
    CHECK(active(DRUM_CH) && active(DRUM_OH));
}

/* a hit again: the sounding one fades to 0 in 96 frames, then the new one starts from 0 */
static void test_retrigger(void)
{
    int f = T50, i, worst = 0;
    reset();
    memset(lr, 0, sizeof lr);
    ENGINE_DRUMS.note_on(&d, p, DRUM_KICK_L, 0xFF);
    run(0, f);
    ENGINE_DRUMS.note_on(&d, p, DRUM_KICK_L, 0xFF);
    CHECK_EQ(d.v[DRUM_KICK_L].state, 2);           /* DV_FADE */
    run(f, f + 96);
    CHECK_EQ(d.v[DRUM_KICK_L].state, 1);           /* hit again */
    CHECK_EQ(d.v[DRUM_KICK_L].t, 0);
    CHECK(abs(lr[2 * (f + 95)]) < 200);             /* the end of the fade is near 0 */
    run(f + 96, f + 4096);
    CHECK(peak(f + 96, f + 4096) > 8000);           /* and the new hit sounds */
    for (i = f - 1; i < f + 200; i++)               /* no step: the fade is smooth */
        if (abs(lr[2 * (i + 1)] - lr[2 * i]) > worst)
            worst = abs(lr[2 * (i + 1)] - lr[2 * i]);
    printf("  retrigger: largest step %d around the fade\n", worst);
    CHECK(worst < 4000);
}

/* the knobs act on the sounding hit */
static void test_live_knobs(void)
{
    double slow, fast, f0, f1;
    int a = T100, b = T200;
    reset();
    memset(lr, 0, sizeof lr);
    ENGINE_DRUMS.note_on(&d, p, DRUM_KICK_L, 0xFF);
    run(0, b);
    slow = rms(a + T50, b);
    f0 = freq(a, b);
    reset();
    memset(lr, 0, sizeof lr);
    ENGINE_DRUMS.note_on(&d, p, DRUM_KICK_L, 0xFF);
    run(0, a);
    p[DRUM_KICK_L * DRUM_BYTES + DP_DECAY] = 0;     /* DECAY to its shortest */
    p[DRUM_KICK_L * DRUM_BYTES + DP_TUNE] = 0xFF;   /* an octave up */
    run(a, b);
    fast = rms(a + T50, b);
    f1 = freq(a, b);
    printf("  kick L: %.0f Hz, rms %.0f; after DECAY 00 and TUNE FF: %.0f Hz, rms %.0f\n", f0, slow, f1, fast);
    CHECK(fast < slow / 4);
    CHECK(f0 > 40 && f0 < 70);
    CHECK(f1 > 1.8 * f0 && f1 < 2.2 * f0);
    /* LEVEL 0: silent; 0x80 = 0 dB, 0xFF about +6 dB */
    reset();
    p[DRUM_SNARE * DRUM_BYTES + DP_LEVEL] = 0;
    memset(lr, 0, sizeof lr);
    ENGINE_DRUMS.note_on(&d, p, DRUM_SNARE, 0xFF);
    run(0, T100);
    CHECK_EQ(peak(0, T100), 0);
}

/* all 16 at once, every LEVEL at FF: the sum leaves room for MASTER's Q12 multiply */
static void test_all(void)
{
    int s, cost;
    int32_t pk;
    reset();
    for (s = 0; s < DRUM_SOUNDS; s++)
        p[s * DRUM_BYTES + DP_LEVEL] = 0xFF;
    memset(lr, 0, sizeof lr);
    for (s = 0; s < DRUM_SOUNDS; s++)
        ENGINE_DRUMS.note_on(&d, p, s, 0xFF);
    cost = ENGINE_DRUMS.cost(&d);
    run(0, T1S);
    pk = peak(0, T1S);
    printf("  16 sounds at once: declared cost %d MAC per frame, peak %d (2^%.1f)\n", cost, pk, log2(pk));
    CHECK(cost > 100 && cost < 200);
    CHECK(pk < 1 << 19);
    CHECK(sizeof(drums_t) < 4096);
}

TEST_MAIN(RUN(test_noise); RUN(test_renders); RUN(test_choke); RUN(test_retrigger); RUN(test_live_knobs); RUN(test_all);)

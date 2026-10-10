// SPDX-License-Identifier: GPL-3.0-only
/* dsp/fixed.h: saturation, rounding, sine and pitch tables, against double-precision references. */
#include <math.h>
#include "test.h"
#include "dsp/fixed.h"

static void test_sat16(void)
{
    CHECK_EQ(sat16(0), 0);
    CHECK_EQ(sat16(32767), 32767);
    CHECK_EQ(sat16(32768), 32767);
    CHECK_EQ(sat16(-32768), -32768);
    CHECK_EQ(sat16(-32769), -32768);
    CHECK_EQ(sat16(INT32_MAX), 32767);
    CHECK_EQ(sat16(INT32_MIN), -32768);
}

static void test_mul_q15(void)
{
    int a, b, worst = 0;
    CHECK_EQ(mul_q15(16384, 16384), 8192);           /* 0.5 * 0.5 */
    CHECK_EQ(mul_q15(-16384, 16384), -8192);
    CHECK_EQ(mul_q15(-32768, -32768), 32767);        /* -1 * -1 saturates */
    CHECK_EQ(mul_q15(-32768, 32767), -32767);
    CHECK_EQ(mul_q15(0, -32768), 0);
    CHECK_EQ(mul_q15(1, 16384), 1);                  /* 0.5 LSB rounds up */
    CHECK_EQ(mul_q15(-1, 16384), 0);                 /* -0.5 LSB rounds up too */
    CHECK_EQ(mul_q15(1, 16383), 0);
    /* identity within 1 LSB, and nearest rounding everywhere on a grid */
    for (a = -32768; a <= 32767; a += 257) {
        CHECK_NEAR(mul_q15((q15_t)a, Q15_ONE), a, 1);
        for (b = -32768; b <= 32767; b += 4099) {
            double r = (double)a * b / 32768.0;
            int e = (int)fabs(mul_q15((q15_t)a, (q15_t)b) - (r > 32767 ? 32767 : r));
            if (e > worst)
                worst = e;
        }
    }
    CHECK(worst <= 1);
}

static void test_mul_q12(void)
{
    CHECK_EQ(mul_q12(12345, Q12_ONE), 12345);
    CHECK_EQ(mul_q12(-12345, Q12_ONE), -12345);
    CHECK_EQ(mul_q12(1000, 2048), 500);
    CHECK_EQ(mul_q12(3, 2048), 2);                   /* 1.5 rounds to 2 */
    CHECK_EQ(mul_q12(8388607, 4 * Q12_ONE), 33554428);   /* 24-bit input, gain 4: no overflow */
    CHECK_EQ(mul_q12(-8388608, 4 * Q12_ONE), -33554432);
    CHECK_EQ(mul_q12(8388607, (1 << 19) - 1), 1073739648); /* the documented limit */
}

static void test_sine(void)
{
    uint32_t i;
    double worst = 0;
    CHECK_EQ(sin_q15(0), 0);
    CHECK_EQ(sin_q15(0x40000000u), 32767);           /* pi/2 */
    CHECK_EQ(sin_q15(0x80000000u), 0);               /* pi */
    CHECK_EQ(sin_q15(0xC0000000u), -32767);          /* 3pi/2 */
    for (i = 0; i < 65536; i++) {
        phase_t ph = i * 65536u + i * 7u;
        double e = fabs(sin_q15(ph) - 32767.0 * sin(2.0 * M_PI * ph / 4294967296.0));
        if (e > worst)
            worst = e;
        CHECK_EQ(sin_q15(ph) + sin_q15(ph + 0x80000000u), 0) ;   /* odd symmetry */
        if (test_fails)
            break;
    }
    CHECK(worst < 2.0);
    printf("  sine: max error %.3f LSB\n", worst);
}

static void test_phase_wrap(void)
{
    phase_t ph = 0xFFFFFFF0u;
    ph += 0x20u;
    CHECK_EQ(ph, 0x10u);
    CHECK_EQ(sin_q15(0xFFFFFFFFu), sin_q15(0xFFFFFFFFu));
    CHECK_NEAR(sin_q15(0xFFFFFFFFu), 0, 1);
}

static double inc_ref(int p) { return 440.0 * pow(2.0, (p / 16.0 - NOTE_A4) / 12.0) / FS * 4294967296.0; }

static void test_pitch(void)
{
    int p;
    uint32_t prev = 0;
    CHECK_NEAR(note_inc(NOTE_A4, 0), floor(440.0 / FS * 4294967296.0), 1);
    CHECK_NEAR(note_inc(0, 0) * (double)FS / 4294967296.0, 16.3516, 0.001);  /* C-0 */
    CHECK_NEAR(note_inc(NOTE_A4 + 12, 0), 2.0 * note_inc(NOTE_A4, 0), 1);    /* an octave doubles */
    for (p = 0; p <= PITCH_MAX; p++) {
        uint32_t v = pitch_inc(p);
        CHECK_NEAR(v, inc_ref(p), 1);
        CHECK(v > prev);
        prev = v;
        if (test_fails)
            break;
    }
    CHECK(note_inc(119, 0) < 0x80000000u);           /* B-9 below Nyquist */
    CHECK_EQ(pitch_inc(-5), pitch_inc(0));           /* clamped */
    CHECK_EQ(pitch_inc(99999), pitch_inc(PITCH_MAX));
    CHECK_EQ(note_inc(60, -16), note_inc(59, 0));    /* fine tune = 1/16 semitone */
}

TEST_MAIN(RUN(test_sat16); RUN(test_mul_q15); RUN(test_mul_q12); RUN(test_sine); RUN(test_phase_wrap);
          RUN(test_pitch);)

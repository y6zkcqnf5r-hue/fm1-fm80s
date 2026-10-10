// SPDX-License-Identifier: GPL-3.0-only
/* Reference renders of the unit tests (CLAUDE.md: a new DSP engine gets a WAV and its golden hash).
 *
 *   render_wav("eng_wav", "saw.wav", lr, frames);
 *
 * writes build/renders/eng_wav/saw.wav, 16-bit stereo 44.1 kHz, the format of host/wav.c (samples
 * saturated to int16). `make unit` then checks every file of build/renders against
 * tests/golden/renders/<suite>.sha256 (tests/host/golden.py), and `make golden` accepts them. */
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

static void render_put16(FILE *f, unsigned v) { fputc(v & 0xFF, f); fputc(v >> 8 & 0xFF, f); }
static void render_put32(FILE *f, uint32_t v) { render_put16(f, v & 0xFFFF); render_put16(f, v >> 16); }

static int render_wav(const char *suite, const char *name, const int32_t *lr, int frames)
{
    char path[256];
    FILE *f;
    int i;
    mkdir("build/renders", 0777);
    snprintf(path, sizeof path, "build/renders/%s", suite);
    mkdir(path, 0777);
    snprintf(path, sizeof path, "build/renders/%s/%s", suite, name);
    f = fopen(path, "wb");
    if (!f) {
        printf("  FAIL cannot write %s\n", path);
        return -1;
    }
    fwrite("RIFF", 1, 4, f);
    render_put32(f, 36 + (uint32_t)frames * 4);
    fwrite("WAVEfmt ", 1, 8, f);
    render_put32(f, 16);
    render_put16(f, 1);
    render_put16(f, 2);
    render_put32(f, 44100);
    render_put32(f, 44100 * 4);
    render_put16(f, 4);
    render_put16(f, 16);
    fwrite("data", 1, 4, f);
    render_put32(f, (uint32_t)frames * 4);
    for (i = 0; i < 2 * frames; i++) {
        int32_t s = lr[i] > 32767 ? 32767 : lr[i] < -32768 ? -32768 : lr[i];
        render_put16(f, (unsigned)(uint16_t)(int16_t)s);
    }
    fclose(f);
    return 0;
}

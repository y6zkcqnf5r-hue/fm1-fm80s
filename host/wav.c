// SPDX-License-Identifier: GPL-3.0-only
/* WAV recording of the simulator's output: 16-bit stereo PCM at 44.1 kHz, little-endian whatever
 * the host, header filled in on close. */
#include <stdio.h>
#include <string.h>
#include "io.h"
#include "host.h"

static FILE *wav;
static uint32_t frames;

static void put16(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}

static void put32(uint8_t *p, uint32_t v)
{
    put16(p, v);
    put16(p + 2, v >> 16);
}

int wav_open(const char *path)
{
    static const uint8_t blank[44];
    wav_close();
    wav = fopen(path, "wb");
    if (!wav)
        return -1;
    frames = 0;
    fwrite(blank, 1, sizeof blank, wav);
    return 0;
}

void wav_write(const int16_t *lr, int n)
{
    uint8_t b[4 * BLOCK];
    int i, m;
    if (!wav)
        return;
    while (n > 0) {
        m = n > BLOCK ? BLOCK : n;
        for (i = 0; i < 2 * m; i++)
            put16(b + 2 * i, (uint16_t)lr[i]);
        fwrite(b, 4, (size_t)m, wav);
        frames += (uint32_t)m;
        lr += 2 * m;
        n -= m;
    }
}

void wav_close(void)
{
    uint8_t h[44];
    uint32_t bytes = frames * 4;
    if (!wav)
        return;
    memcpy(h, "RIFF", 4);
    put32(h + 4, 36 + bytes);
    memcpy(h + 8, "WAVEfmt ", 8);
    put32(h + 16, 16);
    put16(h + 20, 1);                   /* PCM */
    put16(h + 22, 2);                   /* stereo */
    put32(h + 24, FS);
    put32(h + 28, FS * 4);
    put16(h + 32, 4);
    put16(h + 34, 16);
    memcpy(h + 36, "data", 4);
    put32(h + 40, bytes);
    fseek(wav, 0, SEEK_SET);
    fwrite(h, 1, sizeof h, wav);
    fclose(wav);
    wav = 0;
}

int wav_recording(void) { return wav != 0; }
uint32_t wav_frames(void) { return frames; }

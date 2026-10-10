// SPDX-License-Identifier: GPL-3.0-only
/* PNG screenshot of the simulated screen: 8-bit RGB, deflate in "stored" (uncompressed) blocks,
 * so no zlib is needed. Method from FoMni's host/omni_host.c (Charles Vestal), rewritten. */
#include <stdio.h>
#include <string.h>
#include "io.h"
#include "host.h"

#define ROW (1 + LCD_W * 3)             /* filter byte + RGB */
#define RAW (LCD_H * ROW)
#define NBLK ((RAW + 65534) / 65535)
#define ZLEN (2 + RAW + NBLK * 5 + 4)   /* zlib header, blocks, Adler-32 */

static uint32_t crc_t[256];

static uint32_t crc(uint32_t c, const uint8_t *p, uint32_t n)
{
    uint32_t i, k;
    if (!crc_t[1])
        for (i = 0; i < 256; i++) {
            uint32_t v = i;
            for (k = 0; k < 8; k++)
                v = v & 1 ? 0xEDB88320u ^ (v >> 1) : v >> 1;
            crc_t[i] = v;
        }
    for (i = 0; i < n; i++)
        c = crc_t[(c ^ p[i]) & 255] ^ (c >> 8);
    return c;
}

static void be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

/* one chunk: length, type + data, CRC of type + data */
static void chunk(FILE *f, const uint8_t *td, uint32_t len)
{
    uint8_t b[4];
    be32(b, len);
    fwrite(b, 1, 4, f);
    fwrite(td, 1, 4 + len, f);
    be32(b, ~crc(0xFFFFFFFFu, td, 4 + len));
    fwrite(b, 1, 4, f);
}

int png_write(const char *path, const uint16_t *fb)
{
    static const uint8_t sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    static uint8_t raw[RAW], idat[4 + ZLEN];
    uint8_t ihdr[4 + 13] = {'I', 'H', 'D', 'R', 0, 0, 0, 0, 0, 0, 0, 0, 8, 2, 0, 0, 0};
    uint8_t *q = raw;
    uint32_t a = 1, b = 0, pos, i;
    int x, y;
    FILE *f = fopen(path, "wb");
    if (!f)
        return -1;
    for (y = 0; y < LCD_H; y++) {
        *q++ = 0;                       /* filter: none */
        for (x = 0; x < LCD_W; x++) {
            uint16_t p = fb[y * LCD_W + x];
            p = (uint16_t)((p >> 8) | (p << 8));    /* back from the panel's byte order */
            *q++ = (uint8_t)(((p >> 11) & 31) * 255 / 31);
            *q++ = (uint8_t)(((p >> 5) & 63) * 255 / 63);
            *q++ = (uint8_t)((p & 31) * 255 / 31);
        }
    }
    for (i = 0; i < RAW; i++) {
        a = (a + raw[i]) % 65521;
        b = (b + a) % 65521;
    }
    memcpy(idat, "IDAT", 4);
    q = idat + 4;
    *q++ = 0x78;                        /* deflate, 32 KiB window */
    *q++ = 0x01;
    for (pos = 0; pos < RAW;) {
        uint32_t m = RAW - pos > 65535 ? 65535 : RAW - pos;
        *q++ = pos + m >= RAW;          /* BFINAL, BTYPE = stored */
        *q++ = (uint8_t)m;
        *q++ = (uint8_t)(m >> 8);
        *q++ = (uint8_t)~m;
        *q++ = (uint8_t)(~m >> 8);
        memcpy(q, raw + pos, m);
        q += m;
        pos += m;
    }
    be32(q, b << 16 | a);
    be32(ihdr + 4, LCD_W);
    be32(ihdr + 8, LCD_H);
    fwrite(sig, 1, sizeof sig, f);
    chunk(f, ihdr, 13);
    chunk(f, idat, ZLEN);
    chunk(f, (const uint8_t *)"IEND", 0);
    return fclose(f) ? -1 : 0;
}

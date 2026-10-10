// SPDX-License-Identifier: GPL-3.0-only
/* plat.h on the host: the FM-1 simulated in memory, driven by host_run_ms(). No stdio and no
 * files here (the scenario runner and the web wrapper do the I/O), so the same file builds
 * natively and to wasm. Integer clock and integer audio: both builds give the same bytes.
 * Shape from FoMni's host/omni_host.c (Charles Vestal), rewritten for Trawler. */
#include <string.h>
#include "host.h"
#include "dsp/fixed.h"

struct host host;
void (*host_audio_sink)(const int16_t *lr, int n);

/* F3 .. G5; a key is black when (k + 5) % 12 is 1, 3, 6, 8 or 10 (F3 is the 5th semitone of C) */
const uint8_t host_white[16] = {0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26};
const uint8_t host_black[11] = {1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25};

static uint32_t audio_acc;              /* FS per ms, a block is due every BLOCK * 1000 */
static uint32_t ui_due;

/* ---------------------------------------------------------------- time --- */
uint32_t plat_ms(void) { return host.now_ms; }

/* --------------------------------------------------------------- input --- */
uint32_t plat_buttons(void) { return host.buttons; }
uint32_t plat_keys(void) { return host.keys; }
uint16_t plat_master(void) { return host.master; }

int plat_enc(int role)
{
    int32_t v;
    if (role < 0 || role >= EN_COUNT)
        return 0;
    v = host.enc[role];
    host.enc[role] = 0;
    return (int)v;
}

/* -------------------------------------------------------------- output --- */
void plat_leds(uint32_t buttons, uint32_t keys, uint32_t keys_dim)
{
    host.lit_buttons = buttons;
    host.lit_keys = keys;
    host.dim_keys = keys_dim;
}

void plat_play_red(int on) { host.play_red = on != 0; }

/* clip a rectangle to the screen; 0 = nothing left */
static int clip(int *x, int *y, int *w, int *h, int *sx, int *sy)
{
    *sx = *sy = 0;
    if (*x < 0) {
        *sx = -*x;
        *w += *x;
        *x = 0;
    }
    if (*y < 0) {
        *sy = -*y;
        *h += *y;
        *y = 0;
    }
    if (*x + *w > LCD_W)
        *w = LCD_W - *x;
    if (*y + *h > LCD_H)
        *h = LCD_H - *y;
    return *w > 0 && *h > 0;
}

void lcd_fill(int x, int y, int w, int h, uint16_t rgb565)
{
    uint16_t c = (uint16_t)((rgb565 >> 8) | (rgb565 << 8));   /* as the panel takes it */
    int sx, sy, i, j;
    host.blits++;
    if (!clip(&x, &y, &w, &h, &sx, &sy))
        return;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            host.fb[(y + j) * LCD_W + x + i] = c;
}

void lcd_blit(int x, int y, int w, int h, const uint16_t *px)
{
    int w0 = w, sx, sy, j;
    host.blits++;
    if (!clip(&x, &y, &w, &h, &sx, &sy))
        return;
    for (j = 0; j < h; j++)
        memcpy(&host.fb[(y + j) * LCD_W + x], &px[(sy + j) * w0 + sx], (size_t)w * sizeof *px);
}

/* ---------------------------------------------------------------- MIDI --- */
int plat_midi_in(uint32_t *pkt)
{
    if (host.midi_r == host.midi_w)
        return 0;
    *pkt = host.midi_q[host.midi_r++ & (HOST_MIDI_Q - 1)];
    return 1;
}

void plat_midi_out(uint32_t pkt)
{
    host.midi_out++;
    host.midi_last = pkt;
}

void host_midi_push(uint32_t pkt)
{
    if (host.midi_w - host.midi_r < HOST_MIDI_Q)
        host.midi_q[host.midi_w++ & (HOST_MIDI_Q - 1)] = pkt;
}

/* cable 0, code index = the status' high nibble (0xF for system messages) */
uint32_t host_midi_msg(uint32_t st, uint32_t d1, uint32_t d2)
{
    st &= 0xFF;
    return (st >= 0xF0 ? 0x0Fu : st >> 4) | st << 8 | (d1 & 0x7F) << 16 | (d2 & 0x7F) << 24;
}

/* ------------------------------------------------------------- storage --- */
int plat_store_load(int slot, uint8_t *buf, int max)
{
    int n;
    if (slot < 0 || slot >= HOST_SLOTS || (n = host.store_len[slot]) < 0)
        return -1;
    if (n > max)
        n = max;
    memcpy(buf, host.store[slot], (size_t)n);
    return n;
}

int plat_store_save(int slot, const uint8_t *buf, int len)
{
    if (slot < 0 || slot >= HOST_SLOTS || len < 0 || len > HOST_SLOT_MAX)
        return -1;
    memcpy(host.store[slot], buf, (size_t)len);
    host.store_len[slot] = len;
    host.store_writes++;
    return 0;
}

/* --------------------------------------------------------- diagnostics --- */
int plat_cpu_pct(void) { return 0; }    /* the cost model arrives in P3-09; measuring would not be deterministic */
int plat_xruns(void) { return 0; }

/* ---------------------------------------------------------- the device --- */
void host_store_init(void)
{
    static int done;
    int i;
    if (done)
        return;
    for (i = 0; i < HOST_SLOTS; i++)
        host.store_len[i] = -1;
    done = 1;
}

void host_boot(void)
{
    host_store_init();
    host.now_ms = 0;
    host.buttons = host.keys = 0;
    memset(host.enc, 0, sizeof host.enc);
    host.master = 2048;
    host.lit_buttons = host.lit_keys = host.dim_keys = 0;
    host.play_red = 0;
    memset(host.fb, 0, sizeof host.fb);
    host.blits = 0;
    host.peak = 0;
    host.midi_w = host.midi_r = 0;
    host.midi_out = host.midi_last = 0;
    audio_acc = 0;
    ui_due = 0;
    app_init();
}

static void render_block(void)
{
    static int32_t lr[2 * BLOCK];
    static int16_t out[2 * BLOCK];
    int i;
    memset(lr, 0, sizeof lr);
    app_audio(lr, BLOCK);
    for (i = 0; i < 2 * BLOCK; i++) {
        int16_t s = sat16(lr[i]);
        int32_t a = s < 0 ? -(int32_t)s : s;
        if (a > host.peak)
            host.peak = a;
        out[i] = s;
    }
    if (host_audio_sink)
        host_audio_sink(out, BLOCK);
}

void host_run_ms(uint32_t ms)
{
    while (ms--) {
        host.now_ms++;
        audio_acc += FS;
        while (audio_acc >= BLOCK * 1000u) {
            audio_acc -= BLOCK * 1000u;
            render_block();
        }
        if (host.now_ms >= ui_due) {
            ui_due = host.now_ms + UI_MS;
            app_frame();
        }
    }
}

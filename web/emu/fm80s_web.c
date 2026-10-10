// SPDX-License-Identifier: GPL-3.0-only
/* FM80S in the browser: the host simulator (host/fm80s_host.c, the whole app against a
 * simulated FM-1) compiled to WebAssembly without Emscripten's glue, and run in an AudioWorklet
 * (web/emu/worklet.js). Asking for audio is what advances the device's clock: web_render() runs
 * host_run_ms() one millisecond at a time, which renders the app by blocks of 256 frames into a
 * ring and calls app_frame every 16 ms, exactly as the scenario runner does. No real clock comes in,
 * so the device's time cannot drift from the frames it produced. Integers only, like the native
 * build: both give the same bytes (tests/host/emu_test.mjs). Shape from X0X's and FoMni's
 * web/emu/x0x_web.c and omni_web.c (Charles Vestal), rewritten for Trawler. */
#include <string.h>
#include "host.h"

#define EXPORT __attribute__((used, visibility("default")))

#define RING 4096u                      /* stereo frames, power of two: > 1023 asked + 255 left over */
#define OUT_MAX 1024u

static int16_t ring[2 * RING];
static uint32_t ring_w, ring_r;         /* free-running frame counters */
static int16_t out_l[OUT_MAX], out_r[OUT_MAX];

/* host_audio_sink: BLOCK frames at a time; when full the oldest go (the page's silent copy only) */
static void to_ring(const int16_t *lr, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        uint32_t k = (ring_w++ & (RING - 1)) * 2u;
        ring[k] = lr[2 * i];
        ring[k + 1] = lr[2 * i + 1];
    }
    if (ring_w - ring_r > RING)
        ring_r = ring_w - RING;
}

/* ---------------------------------------------------------------- boot --- */
EXPORT void web_boot(void)
{
    host_audio_sink = to_ring;
    ring_w = ring_r = 0;
    host_boot();
}

/* --------------------------------------------------------------- audio --- */
/* n frames (<= 1024) into web_out_l / web_out_r, int16: the device runs until they exist */
EXPORT void web_render(uint32_t n)
{
    uint32_t i;
    if (n > OUT_MAX)
        n = OUT_MAX;
    while (ring_w - ring_r < n)
        host_run_ms(1);
    for (i = 0; i < n; i++) {
        uint32_t k = (ring_r++ & (RING - 1)) * 2u;
        out_l[i] = ring[k];
        out_r[i] = ring[k + 1];
    }
}
EXPORT int16_t *web_out_l(void) { return out_l; }
EXPORT int16_t *web_out_r(void) { return out_r; }
EXPORT uint32_t web_avail(void) { return ring_w - ring_r; }   /* frames rendered, not yet taken */
EXPORT void web_run_ms(uint32_t ms) { host_run_ms(ms); }      /* the test and the silent copy */

/* --------------------------------------------------------------- input --- */
EXPORT void web_buttons(uint32_t mask) { host.buttons = mask & ((1u << B_COUNT) - 1); }
EXPORT void web_keys(uint32_t mask) { host.keys = mask & ((1u << NKEYS) - 1); }
EXPORT void web_enc(int role, int n)
{
    if (role >= 0 && role < EN_COUNT)
        host.enc[role] += n;
}
EXPORT void web_master(uint32_t v) { host.master = (uint16_t)(v > 4096u ? 4096u : v); }
EXPORT void web_midi(uint32_t st, uint32_t d1, uint32_t d2) { host_midi_push(host_midi_msg(st, d1, d2)); }

/* ------------------------------------------------------ screen, lights --- */
EXPORT uint16_t *web_fb(void) { return host.fb; }
EXPORT uint32_t web_blits(void) { return host.blits; }
EXPORT uint32_t web_lit_buttons(void) { return host.lit_buttons; }
EXPORT uint32_t web_lit_keys(void) { return host.lit_keys; }
EXPORT uint32_t web_dim_keys(void) { return host.dim_keys & ~host.lit_keys; }
EXPORT int web_play_red(void) { return host.play_red; }

/* ------------------------------------------------------------- storage --- */
/* the page keeps the slots and gives them back before web_boot */
EXPORT uint32_t web_slots(void) { return HOST_SLOTS; }
EXPORT uint32_t web_store_max(void) { return HOST_SLOT_MAX; }
EXPORT uint8_t *web_store_ptr(int slot) { return slot >= 0 && slot < HOST_SLOTS ? host.store[slot] : 0; }
EXPORT int web_store_len(int slot)
{
    host_store_init();
    return slot >= 0 && slot < HOST_SLOTS ? (int)host.store_len[slot] : -1;
}
EXPORT void web_store_set_len(int slot, int n)
{
    host_store_init();
    if (slot >= 0 && slot < HOST_SLOTS)
        host.store_len[slot] = n < 0 ? -1 : n > HOST_SLOT_MAX ? HOST_SLOT_MAX : n;
}
EXPORT uint32_t web_store_writes(void) { return host.store_writes; }

/* --------------------------------------------------------- diagnostics --- */
/* the largest estimated CPU of a block since the last call (the drums are peaks) */
EXPORT int web_cpu(void)
{
    int c = host.cpu_peak;
    host.cpu_peak = 0;
    return c;
}
EXPORT uint32_t web_now(void) { return host.now_ms; }
EXPORT int32_t web_peak(void) { return host.peak; }

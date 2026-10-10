// SPDX-License-Identifier: GPL-3.0-only
/* FM80S platform interface: everything the app needs from the hardware, and nothing else.
 *
 * Implemented once per target: host/fm80s_host.c (scripted simulator, also compiled to wasm for
 * the browser) and firmware/src/app/plat_fm1.c (the FM-1, P0-02, over Felucca's HAL). The shared
 * code in dsp/, engine/, core/ and ui/ only talks to the device through this file (CLAUDE.md rule 6).
 * Shape from FoMni / X0X (Charles Vestal); taken from Trawler (docs/05_ARCHITECTURE.md §3). */
#pragma once
#include <stdint.h>

/* big buffers go in the .pool RAM region on the device (336 KiB) */
#ifdef APP_DEVICE
#define APP_POOL __attribute__((section(".pool")))
#else
#define APP_POOL
#endif

/* ---------------------------------------------------------------- time --- */
uint32_t plat_ms(void);                 /* ms since boot */

/* ----------------------------------------- input (read in app_frame) --- */
/* the 14 buttons, by their printed label */
enum { B_OCTDN, B_OCTUP, B_FX, B_SEL, B_ENV, B_LFO, B_EDIT, B_GLO,
       B_HOME, B_SAVE, B_ARP, B_SEQ, B_PLAY, B_REC, B_COUNT };
/* the 7 endless encoders */
enum { EN_SELECT, EN_ALGO, EN_K1, EN_K2, EN_K3, EN_K4, EN_PRESETS, EN_COUNT };
#define NKEYS 27                        /* F3 .. G5: 16 white, 11 black */

uint32_t plat_buttons(void);            /* held: bit B_* */
uint32_t plat_keys(void);               /* held: bit k = key k (0 = F3) */
int plat_enc(int role);                 /* detents since the last call, + = clockwise; EN_* */
uint16_t plat_master(void);             /* MASTER pot, 0..4096 (Q12) */

/* ------------------------------------------------------------- output --- */
void plat_leds(uint32_t buttons, uint32_t keys, uint32_t keys_dim);  /* lit: bit B_*, bit key */
void plat_play_red(int on);             /* PLAY's red LED (recording); call before plat_leds (ADR-013) */
/* the 240x240 RGB565 screen; pixels byte-swapped as the panel takes them */
void lcd_fill(int x, int y, int w, int h, uint16_t rgb565);
void lcd_blit(int x, int y, int w, int h, const uint16_t *px);

/* ------------------------------------- MIDI (4-byte USB-MIDI packets) --- */
int plat_midi_in(uint32_t *pkt);        /* 1 = got one */
void plat_midi_out(uint32_t pkt);

/* --------------------------------- storage, by slot (no raw flash) --- */
int plat_store_load(int slot, uint8_t *buf, int max);       /* bytes read, or -1 */
int plat_store_save(int slot, const uint8_t *buf, int len); /* 0 = ok; the device writes only when stopped */

/* -------------------------------------------------------- diagnostics --- */
int plat_cpu_pct(void);                 /* audio load: estimated (cost model), measured on the device */
int plat_xruns(void);                   /* audio underruns since boot */

/* ------------------------------------- provided by the app, called by the platform --- */
void app_init(void);
/* n frames, interleaved L R, Q15 in an int32 (+-32767 full scale, saturated on output, ADR-012);
 * called from the audio callback: no flash, no screen, no waiting */
void app_audio(int32_t *lr, int n);
void app_frame(void);                   /* main loop, ~60 Hz: input, UI, drawing */
int app_cost(void);                     /* declared MAC per frame of the last audio block (cost model) */

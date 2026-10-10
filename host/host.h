// SPDX-License-Identifier: GPL-3.0-only
/* The simulated FM-1: state and clock shared by the native scenario runner (host/scenario.c) and,
 * the wasm build (web/emu/fm80s_web.c). host/fm80s_host.c implements plat.h over
 * this state and uses no stdio, so it compiles as is for both. Shape from FoMni's omni_host.c. */
#pragma once
#include <stdint.h>
#include "app/plat.h"

#define LCD_W 240
#define LCD_H 240
#define FS 44100
#define BLOCK 256                       /* frames per app_audio call, as on the device */
#define UI_MS 16                        /* app_frame period */

/* storage, host side only until Q-12 settles the slot count */
#define HOST_SLOTS 8
#define HOST_SLOT_MAX (48 * 1024)

#define HOST_MIDI_Q 256                 /* incoming packets, power of two */

struct host {
    uint32_t now_ms;
    /* input, as a script or the page sets it */
    uint32_t buttons, keys;             /* held: bit B_*, bit key */
    int32_t enc[EN_COUNT];              /* detents not yet read by the app */
    uint16_t master;                    /* 0..4096 */
    /* output, as the app last set it */
    uint32_t lit_buttons, lit_keys, dim_keys;
    int play_red;
    uint16_t fb[LCD_W * LCD_H];         /* RGB565, byte-swapped as the panel takes it */
    uint32_t blits;                     /* lcd_fill + lcd_blit calls, to see if the screen changed */
    int32_t peak;                       /* largest |sample| out, after saturation */
    int32_t cpu_peak;                   /* largest plat_cpu_pct() of a block since the page last read it */
    /* MIDI */
    uint32_t midi_q[HOST_MIDI_Q], midi_w, midi_r;
    uint32_t midi_out;                  /* packets sent */
    uint32_t midi_last;                 /* the last one */
    /* storage, kept across host_boot */
    uint8_t store[HOST_SLOTS][HOST_SLOT_MAX];
    int32_t store_len[HOST_SLOTS];      /* -1 = empty */
    uint32_t store_writes;
};
extern struct host host;

/* where the saturated output goes, BLOCK interleaved L R frames at a time (0 = nowhere) */
extern void (*host_audio_sink)(const int16_t *lr, int n);

/* the keyboard: key index (0 = F3) of the white keys, left to right, and of the black ones */
extern const uint8_t host_white[16], host_black[11];

void host_store_init(void);             /* every slot empty, once: before the storage is first touched */
void host_boot(void);                   /* reset the device (not the storage), then app_init */
void host_run_ms(uint32_t ms);          /* run the device: audio blocks when due, app_frame every UI_MS */
void host_midi_push(uint32_t pkt);      /* an incoming USB-MIDI packet */
uint32_t host_midi_msg(uint32_t st, uint32_t d1, uint32_t d2);  /* the USB-MIDI packet of a MIDI message */

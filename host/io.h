// SPDX-License-Identifier: GPL-3.0-only
/* Files the native simulator writes: WAV recordings and PNG screenshots (not in the wasm build). */
#pragma once
#include <stdint.h>

int wav_open(const char *path);         /* 0 = ok; closes any recording in progress */
void wav_write(const int16_t *lr, int n);   /* n interleaved L R frames, if recording */
void wav_close(void);                   /* fills in the header; nothing if not recording */
int wav_recording(void);
uint32_t wav_frames(void);              /* frames written to the current or last recording */

int png_write(const char *path, const uint16_t *fb);    /* 240x240 RGB565 byte-swapped; 0 = ok */

// SPDX-License-Identifier: GPL-3.0-only
/* Commands from the interface to the audio path (docs/05_ARCHITECTURE.md §5): a lock-free queue
 * with one writer (app_frame) and one reader (app_audio). 32-bit indexes and slots, each written by
 * one side only: a 32-bit store is atomic on the device and on the host. */
#pragma once
#include <stdint.h>

#define CMDQ_N 16                       /* power of two */

void cmdq_init(void);                   /* before the audio starts */
int cmdq_push(uint32_t cmd);            /* interface side: 0, or -1 if full (the command is dropped) */
int cmdq_pop(uint32_t *cmd);            /* audio side: 1 = got one */

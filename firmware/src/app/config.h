// SPDX-License-Identifier: GPL-3.0-only
/* FM80S limits and memory budgets, in one place.
 *
 * The BUDGET_* figures are the FM-1's (CLAUDE.md rule 4); tools/size_report.py reads them from this
 * file (keep the "(N * 1024)" form, the script parses it). The size of a saved project is set in
 * P4-03 (spec §4). */
#pragma once

/* ---------------------------------------------------------- budgets --- */
#define BUDGET_RAM (96 * 1024)          /* .data + .bss */
#define BUDGET_POOL (336 * 1024)        /* .pool: buffers marked APP_POOL */
#define BUDGET_CODE (560 * 1024)        /* .text + .rodata + .data image, in flash */

/* ---------------------------------------------------------- tracks --- */
#define ENG_TRACKS 1                    /* the drums only until P2-02 brings 8 tracks */

/* -------------------------------------------- estimated CPU (P0-01) --- */
/* 100 % of the audio budget in declared MAC per frame (docs/05_ARCHITECTURE.md §8): a cautious
 * guess, set again in P0-02 from the time measured on the device */
#define CPU_REF_MAC 600

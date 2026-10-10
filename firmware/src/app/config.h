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

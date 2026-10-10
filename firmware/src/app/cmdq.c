// SPDX-License-Identifier: GPL-3.0-only
#include "app/cmdq.h"

static volatile uint32_t q[CMDQ_N];
static volatile uint32_t w, r;          /* free-running; w is the writer's, r the reader's */

void cmdq_init(void) { w = r = 0; }

int cmdq_push(uint32_t cmd)
{
    uint32_t i = w;
    if (i - r >= CMDQ_N)
        return -1;
    q[i & (CMDQ_N - 1)] = cmd;
    w = i + 1;                          /* after the slot: the reader never sees it half written */
    return 0;
}

int cmdq_pop(uint32_t *cmd)
{
    uint32_t i = r;
    if (i == w)
        return 0;
    *cmd = q[i & (CMDQ_N - 1)];
    r = i + 1;
    return 1;
}

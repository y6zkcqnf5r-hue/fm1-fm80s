// SPDX-License-Identifier: GPL-3.0-only
#include "core/track.h"
#include "app/cmdq.h"

/* a command: the op, the track and the note */
enum { CMD_NOTE_ON };
#define CMD(op, t, note) ((uint32_t)(op) << 16 | (uint32_t)(t) << 8 | (uint32_t)(note))

track_t track[ENG_TRACKS];

static volatile int32_t v_cost;         /* written by the audio, read by the interface */

void track_init(void)
{
    int t, i;
    cmdq_init();
    for (t = 0; t < ENG_TRACKS; t++) {
        track[t].eng = &ENGINE_DRUMS;
        for (i = 0; i < ENG_PARAMS; i++)
            track[t].p[i] = track[t].eng->defaults[i];
        if (track[t].eng->state_size <= sizeof track[t].st)
            track[t].eng->init(&track[t].st);
        else
            track[t].eng = 0;           /* cannot happen: the union holds every engine */
    }
    v_cost = 0;
}

void track_note_on(int t, int note)
{
    if (t >= 0 && t < ENG_TRACKS && note >= 0 && note < 256)
        cmdq_push(CMD(CMD_NOTE_ON, t, note));
}

int track_cost(void) { return v_cost; }

void track_audio(int32_t *lr, int n)
{
    uint32_t c;
    int t, cost = 0;
    while (cmdq_pop(&c)) {
        t = (int)(c >> 8 & 0xFF);
        if (t < ENG_TRACKS && c >> 16 == CMD_NOTE_ON && track[t].eng)
            track[t].eng->note_on(&track[t].st, track[t].p, (int)(c & 0xFF), 0xFF);
    }
    for (t = 0; t < ENG_TRACKS; t++)
        if (track[t].eng) {
            cost += track[t].eng->cost(&track[t].st);   /* the voices this block will play */
            track[t].eng->render(&track[t].st, track[t].p, lr, n);
        }
    v_cost = cost;
}

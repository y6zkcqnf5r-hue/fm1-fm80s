// SPDX-License-Identifier: GPL-3.0-only
#include "ui/ui.h"
#include "core/track.h"
#include "ui/drums_view.h"

ui_t ui;

static uint32_t prev_keys;
static uint32_t knob_at[4];             /* plat_ms() of each knob's last detent */
static int drawn;

static int is_black(int k)
{
    int s = (k + 5) % 12;               /* key 0 is F3 */
    return s == 1 || s == 3 || s == 6 || s == 8 || s == 10;
}

int ui_white(int key)
{
    int i, w = 0;
    if (key < 0 || key >= NKEYS || is_black(key))
        return -1;
    for (i = 0; i < key; i++)
        w += !is_black(i);
    return w;
}

uint32_t ui_white_bit(int w)
{
    int k;
    for (k = 0; k < NKEYS; k++)
        if (ui_white(k) == w)
            return 1u << k;
    return 0;
}

void ui_init(void)
{
    int i;
    ui.sound = DRUM_KICK_L;
    prev_keys = 0;
    for (i = 0; i < 4; i++)
        knob_at[i] = 0;
    drawn = 0;
}

/* KNOB1-4: TUNE DECAY TONE LEVEL of the sound last hit, live */
static void knobs(uint32_t now)
{
    int i;
    for (i = 0; i < 4; i++) {
        int n = plat_enc(EN_K1 + i), fast, v;
        uint8_t *b = &track[0].p[ui.sound * DRUM_BYTES + DP_TUNE + i];
        if (!n)
            continue;
        fast = n > 1 || n < -1 || now - knob_at[i] < UI_KNOB_FAST_MS;
        knob_at[i] = now;
        v = *b + n * (fast ? UI_KNOB_FAST : UI_KNOB_STEP);
        *b = (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v);   /* one byte write: the audio reads it whole */
    }
}

void ui_frame(void)
{
    uint32_t keys = plat_keys(), down = keys & ~prev_keys, lit = keys, dim = 0;
    int k, s;
    prev_keys = keys;

    /* a white key hits its sound and hands it to the knobs; the black keys do nothing yet */
    for (k = 0; k < NKEYS; k++)
        if (down >> k & 1u && (s = ui_white(k)) >= 0) {
            track_note_on(0, s);
            ui.sound = (uint8_t)s;
        }
    knobs(plat_ms());

    for (s = 0; s < DRUM_SOUNDS; s++) {
        int m = track[0].st.head.meter[s];
        if (m >= UI_LED_LIT)
            lit |= ui_white_bit(s);
        else if (m)
            dim |= ui_white_bit(s);
    }
    plat_leds(plat_buttons(), lit, dim);

    drums_view_draw(!drawn);
    drawn = 1;
}

int ui_probe(int what)
{
    switch (what) {
    case UI_PROBE_SOUND:
        return ui.sound;
    }
    return -1;
}

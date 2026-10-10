// SPDX-License-Identifier: GPL-3.0-only
/* The Drums screen (docs/04_SPEC.md §5, ADR-021): the header, the 16 pads in the keyboard's order
 * (keys 1-8 at the bottom, 9-16 on top) lit by the audio's levels, and the four gauges of the
 * sound the knobs set. Only what changed is redrawn. */
#pragma once

void drums_view_draw(int full);         /* full = the whole screen, else what changed */

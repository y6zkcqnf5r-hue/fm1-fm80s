#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Floydus (FM80S)
"""The FM80S boot image: a striped sunset over a perspective grid, a night sky, and the FM80S wordmark
in chrome cyan. Drawn with Pillow from simple shapes (no third-party font), in the format of SLOOP's
splash (tools/gen_logo.py): 240 x 188, a fixed 16-colour palette, RLE runs of (run - 1) << 4 | colour.

  tools/gen_fm80s_splash.py OUT.h          firmware boot image (build/gen/fm80s_splash.h)
  tools/gen_fm80s_splash.py --png OUT.png  the same image at 2x, to look at
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

W, H = 240, 188
SS = 4                                            # supersampling, then LANCZOS down
HORIZON = 112

BLACK = (0, 0, 0)                                 # palette index 0: never drawn (splash.c skips it)
SKY = [(14, 4, 34), (30, 8, 62), (54, 12, 94), (88, 18, 122)]          # top to horizon
SUN = [(255, 222, 84), (255, 152, 52), (255, 92, 96), (236, 40, 150)]  # top to bottom
GRID, GRID_DIM, GLOW = (255, 52, 212), (138, 28, 140), (255, 140, 228)
CYAN, CYAN_DK, CHROME, INK = (64, 232, 255), (22, 118, 214), (238, 250, 255), (8, 0, 24)
PALETTE = [BLACK, *SKY, *SUN, GRID, GRID_DIM, GLOW, CYAN, CYAN_DK, CHROME, INK]
assert len(PALETTE) == 16

STARS = [(18, 10), (52, 26), (88, 8), (150, 18), (196, 30), (228, 38), (34, 52), (210, 58), (120, 4),
         (70, 44), (176, 46)]


def sky_and_sun(im, d, k):
    bands = [0, 30, 60, 88, HORIZON]
    for i, c in enumerate(SKY):
        d.rectangle([0, bands[i] * k, W * k, bands[i + 1] * k], fill=c)
    for x, y in STARS:
        d.rectangle([x * k, y * k, (x + 1) * k - 1, (y + 1) * k - 1], fill=CHROME)
    cx, cy, r = 120, 98, 56
    top = cy - r
    rows = [top, top + 26, top + 46, top + 62, HORIZON]          # the sun's four flat colours
    for i, c in enumerate(SUN):
        layer = Image.new("L", (W * k, H * k), 0)
        ImageDraw.Draw(layer).ellipse([(cx - r) * k, top * k, (cx + r) * k, (cy + r) * k], fill=255)
        ImageDraw.Draw(layer).rectangle([0, 0, W * k, rows[i] * k], fill=0)
        ImageDraw.Draw(layer).rectangle([0, rows[i + 1] * k, W * k, H * k], fill=0)
        im.paste(c, (0, 0), layer)
    y, gap = 74, 1                                # the stripes: wider gaps towards the horizon
    while y < HORIZON:
        d.rectangle([(cx - r) * k, y * k, (cx + r) * k, (y + gap) * k - 1], fill=SKY[3] if y > 88 else SKY[2])   # the sky band behind
        y += gap + 6
        gap += 1


def grid(d, k):
    d.rectangle([0, HORIZON * k, W * k, H * k], fill=SKY[0])
    vx = 120
    t0 = 0.08                                     # rays towards the vanishing point, cut just under the
    for i in range(-9, 10):                       # horizon (where they would merge into a blot)
        x1, y0 = vx + i * 34, HORIZON + (H - HORIZON) * t0
        d.line([((vx + (x1 - vx) * t0) * k, y0 * k), (x1 * k, H * k)], fill=GRID_DIM if abs(i) > 6 else GRID,
               width=max(1, k))
    dy = 3.0
    y = HORIZON + dy
    while y < H:                                  # rows closer together towards the horizon
        d.line([(0, y * k), (W * k, y * k)], fill=GRID, width=max(1, k))
        dy *= 1.42
        y += dy
    d.rectangle([0, (HORIZON - 1) * k, W * k, (HORIZON + 1) * k], fill=GLOW)


# ---- the wordmark: square, rounded, monoline letters, slanted -----------------------------------------
GH, GW, GAP, ST = 44, 30, 6, 7                    # glyph height, width, gap, stroke (px)


def glyph(d, ch, x, y, k):
    w, h, s = GW * k, GH * k, ST * k
    hs = s / 2
    L, R, T, B, M = x + hs, x + w - hs, y + hs, y + h - hs, y + h * 0.48

    def line(pts):
        pts = [(px, py) for px, py in pts]
        d.line(pts, fill=255, width=round(s), joint="curve")
        for px, py in (pts[0], pts[-1]):
            d.ellipse([px - hs, py - hs, px + hs, py + hs], fill=255)

    def box(y0, y1):
        d.rounded_rectangle([x, y0, x + w, y1], radius=s * 1.1, outline=255, width=round(s))
    if ch == "F":
        line([(R, T), (L, T), (L, B)])
        line([(L, M), (R - 4 * k, M)])
    elif ch == "M":
        line([(L, B), (L, T), (x + w / 2, M + 4 * k), (R, T), (R, B)])
    elif ch == "8":
        box(y, M + hs)
        box(M - hs, y + h)
    elif ch == "0":
        box(y, y + h)
    elif ch == "S":
        line([(R, T), (L, T), (L, M), (R, M), (R, B), (L, B)])


def wordmark(k):
    """the wordmark's mask (L) on the full canvas, slanted, and its top and bottom rows"""
    text = "FM80S"
    total = len(text) * GW + (len(text) - 1) * GAP
    x0, y0 = (W - total) / 2 - 4, 126
    m = Image.new("L", (W * k, H * k), 0)
    d = ImageDraw.Draw(m)
    for i, ch in enumerate(text):
        glyph(d, ch, (x0 + i * (GW + GAP)) * k, y0 * k, k)
    sh = 0.22                                     # slant: x' = x + sh * (y_base - y)
    base = (y0 + GH) * k
    m = m.transform(m.size, Image.AFFINE, (1, sh, -sh * base, 0, 1, 0), resample=Image.BICUBIC)
    return m, y0, y0 + GH


def render(k=SS):
    im = Image.new("RGB", (W * k, H * k), BLACK)
    d = ImageDraw.Draw(im)
    sky_and_sun(im, d, k)
    grid(d, k)
    m, top, bot = wordmark(k)
    ink = m.filter(ImageFilter.MaxFilter(2 * 3 * k + 1))        # a dark outline around the letters
    im.paste(INK, (0, 0), ink)
    mid = top + (bot - top) * 0.52                 # chrome: light above the middle, a bright line, dark below
    chrome = Image.new("RGB", im.size, CYAN)
    cd = ImageDraw.Draw(chrome)
    cd.rectangle([0, top * k, W * k, (top + 5) * k], fill=CHROME)
    cd.rectangle([0, (mid - 1) * k, W * k, (mid + 1) * k], fill=CHROME)
    cd.rectangle([0, (mid + 1) * k, W * k, H * k], fill=CYAN_DK)
    im.paste(chrome, (0, 0), m)
    return im.resize((W, H), Image.LANCZOS)


def quantize(im):
    src, cache, px = im.tobytes(), {}, []
    for i in range(0, len(src), 3):
        p = (src[i], src[i + 1], src[i + 2])
        if p not in cache:
            cache[p] = min(range(16), key=lambda j: sum((p[c] - PALETTE[j][c]) ** 2 for c in range(3)))
        px.append(cache[p])
    return px


def rgb565(c):
    r, g, b = c
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def rle_of(px):
    rle, i = bytearray(), 0                       # (run - 1) << 4 | index, runs of 1..16
    while i < len(px):
        j = i
        while j < len(px) and px[j] == px[i] and j - i < 16:
            j += 1
        rle.append(((j - i - 1) << 4) | px[i])
        i = j
    return rle


def header(path):
    rle = rle_of(quantize(render()))
    L = ["/* generated by tools/gen_fm80s_splash.py: the FM80S boot image */", "#pragma once",
         "#include <stdint.h>", f"#define FM80S_SPLASH_W {W}", f"#define FM80S_SPLASH_H {H}",
         "static const uint16_t FM80S_SPLASH_PAL[16] = {" + ", ".join(f"0x{rgb565(c):04X}" for c in PALETTE) + "};",
         f"static const uint8_t FM80S_SPLASH_RLE[{len(rle)}] = {{"]
    for i in range(0, len(rle), 24):
        L.append("    " + ", ".join(str(b) for b in rle[i:i + 24]) + ",")
    L.append("};")
    Path(path).write_text("\n".join(L) + "\n")
    print(f"fm80s splash: {W}x{H}, {len(rle)} B RLE -> {path}")


def png(path):
    px = quantize(render())
    im = Image.new("RGB", (W, H))
    im.putdata([PALETTE[i] for i in px])
    im.resize((W * 2, H * 2), Image.NEAREST).save(path)
    print(f"fm80s splash: {len(rle_of(px))} B RLE, preview -> {path}")


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "--png":
        png(sys.argv[2])
    elif len(sys.argv) == 2:
        header(sys.argv[1])
    else:
        sys.exit(__doc__)

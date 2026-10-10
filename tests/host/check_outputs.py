# SPDX-License-Identifier: GPL-3.0-only
"""Check the files the scenarios wrote, independently of the simulator's own code.

  python3 tests/host/check_outputs.py DIR

Every PNG under DIR: signature, 240x240 8-bit RGB, chunk CRCs, IDAT that zlib inflates to the
right size. Every WAV: PCM 16-bit stereo 44.1 kHz, sizes that match the file, at least one frame.
Exit 1 on the first bad file, or if DIR holds none.
"""
import pathlib
import struct
import sys
import zlib


def check_png(p):
    b = p.read_bytes()
    assert b[:8] == b"\x89PNG\r\n\x1a\n", "bad signature"
    pos, chunks, idat = 8, [], b""
    while pos < len(b):
        n, = struct.unpack(">I", b[pos:pos + 4])
        td = b[pos + 4:pos + 8 + n]
        crc, = struct.unpack(">I", b[pos + 8 + n:pos + 12 + n])
        assert zlib.crc32(td) == crc, f"bad CRC on {td[:4]!r}"
        chunks.append(td[:4])
        if td[:4] == b"IDAT":
            idat += td[4:]
        pos += 12 + n
    assert chunks[0] == b"IHDR" and chunks[-1] == b"IEND", f"chunks {chunks}"
    w, h, depth, ctype = struct.unpack(">IIBB", b[16:26])
    assert (w, h, depth, ctype) == (240, 240, 8, 2), f"IHDR {w}x{h} depth {depth} type {ctype}"
    raw = zlib.decompress(idat)
    assert len(raw) == 240 * (1 + 240 * 3), f"{len(raw)} bytes of pixels"
    return f"{w}x{h}"


def check_wav(p):
    b = p.read_bytes()
    assert b[:4] == b"RIFF" and b[8:16] == b"WAVEfmt ", "not a WAV"
    riff, = struct.unpack("<I", b[4:8])
    assert riff == len(b) - 8, f"RIFF size {riff}, file {len(b)}"
    fmt = struct.unpack("<IHHIIHH", b[16:36])
    assert fmt == (16, 1, 2, 44100, 44100 * 4, 4, 16), f"fmt {fmt}"
    assert b[36:40] == b"data", "no data chunk"
    data, = struct.unpack("<I", b[40:44])
    assert data == len(b) - 44, f"data size {data}, file {len(b)}"
    assert data > 0, "no frames"
    return f"{data // 4} frames"


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    files = sorted(pathlib.Path(sys.argv[1]).rglob("*"))
    seen = 0
    for p in files:
        check = {".png": check_png, ".wav": check_wav}.get(p.suffix)
        if not check:
            continue
        try:
            what = check(p)
        except (AssertionError, struct.error, zlib.error) as e:
            print(f"  FAIL {p}: {e}")
            sys.exit(1)
        print(f"  ok {p} ({what})")
        seen += 1
    if not seen:
        print(f"  FAIL no PNG or WAV under {sys.argv[1]}")
        sys.exit(1)


if __name__ == "__main__":
    main()

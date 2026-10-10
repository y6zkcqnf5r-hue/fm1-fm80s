# SPDX-License-Identifier: GPL-3.0-only
"""Golden outputs: the WAV and PNG files a scenario writes must not change unless we want them to.

  python3 tests/host/golden.py check  OUTDIR GOLDENDIR
  python3 tests/host/golden.py accept OUTDIR GOLDENDIR

OUTDIR holds one folder per scenario (build/scenarios/<name>/). GOLDENDIR holds one
<name>.sha256 per scenario, in the `shasum -a 256` format ("<hex>  <file>", sorted by file).
check: every WAV and PNG has its line and the same hash, every line has its file; exit 1 if not.
accept: rewrites the .sha256 of every scenario that wrote files (make golden).
"""
import hashlib
import pathlib
import sys


def outputs(scen):
    """{file name: sha256} of the WAV and PNG files of one scenario folder"""
    return {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(scen.iterdir()) if p.suffix in (".wav", ".png")}


def read_golden(path):
    want = {}
    for line in path.read_text().splitlines():
        if line.strip():
            h, name = line.split(None, 1)
            want[name.strip()] = h
    return want


def check(out, gold):
    fails = 0
    scens = sorted(d for d in out.iterdir() if d.is_dir())
    names = {d.name for d in scens}
    for d in scens:
        got = outputs(d)
        g = gold / (d.name + ".sha256")
        want = read_golden(g) if g.exists() else {}
        for name in sorted(set(got) | set(want)):
            if name not in want:
                print(f"  FAIL {d.name}/{name}: no golden")
            elif name not in got:
                print(f"  FAIL {d.name}/{name}: in {g}, not written")
            elif got[name] != want[name]:
                print(f"  FAIL {d.name}/{name}: hash {got[name][:12]}, golden {want[name][:12]}")
            else:
                print(f"  ok golden {d.name}/{name}")
                continue
            fails += 1
    for g in sorted(gold.glob("*.sha256")):
        if g.stem not in names:
            print(f"  FAIL {g}: no scenario {g.stem}")
            fails += 1
    if fails:
        print(f"golden: {fails} faute(s); si le changement est voulu : make golden")
        sys.exit(1)


def accept(out, gold):
    gold.mkdir(parents=True, exist_ok=True)
    for d in sorted(x for x in out.iterdir() if x.is_dir()):
        got = outputs(d)
        if not got:
            continue
        g = gold / (d.name + ".sha256")
        g.write_text("".join(f"{h}  {name}\n" for name, h in sorted(got.items())))
        print(f"  wrote {g} ({len(got)} files)")


def main():
    if len(sys.argv) != 4 or sys.argv[1] not in ("check", "accept"):
        sys.exit(__doc__)
    out, gold = pathlib.Path(sys.argv[2]), pathlib.Path(sys.argv[3])
    if not out.is_dir():
        sys.exit(f"golden: no {out}")
    (check if sys.argv[1] == "check" else accept)(out, gold)


if __name__ == "__main__":
    main()

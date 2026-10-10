#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""FM80S size report: RAM, pool and code against the FM-1 budgets (CLAUDE.md rule 4).

    python3 tools/size_report.py [--config firmware/src/app/config.h] FILE.o|FILE.elf ...

`make size` compiles firmware/src as 32-bit ELF objects (clang --target=thumbv7m-none-eabi
-DAPP_DEVICE, so APP_POOL buffers land in .pool) and passes them here (ADR-020). In P5 the same script
reads the device's linked ELF. Python 3.9+, no dependency: a small ELF reader.

    RAM      .data* .bss* (and .sdata .sbss)
    pool     .pool*
    code     every other allocated section (.text .rodata ...) plus the .data image, in flash

Exit 1 if a budget is exceeded, or if a symbol is left to a 64-bit division or
soft-float helper (rules 1 and 2): the device has neither. From objects, the code figure is only
indicative (Thumb-2, not pi32v2), and unreferenced code is still counted (an upper bound).
"""
import argparse
import re
import struct
import sys
from pathlib import Path

SHF_ALLOC = 0x2
SHT_SYMTAB, SHT_NOBITS = 2, 8
ET_REL = 1
STB_LOCAL = 0
STT_OBJECT, STT_FUNC = 1, 2

BUDGETS = ("RAM", "POOL", "CODE")
FORBIDDEN = re.compile(r"^(__aeabi_(l|ul)divmod|__aeabi_[fd]\w+|__aeabi_\w+2[fd]|__u?(div|mod)di3"
                       r"|__u?divmoddi4|__\w+[sdt]f\d|__float\w+|__fix\w+)$")
RAM_PREFIX = (".data", ".bss", ".sdata", ".sbss")


def kind(name):
    if name.startswith(".pool"):
        return "pool"
    if name.startswith(RAM_PREFIX):
        return "ram"
    return "code"


class Elf:
    def __init__(self, path):
        b = Path(path).read_bytes()
        if b[:4] != b"\x7fELF":
            raise ValueError("%s: pas un fichier ELF" % path)
        wide = b[4] == 2
        e = "<" if b[5] == 1 else ">"
        if wide:
            (self.type,) = struct.unpack_from(e + "H", b, 16)
            shoff, = struct.unpack_from(e + "Q", b, 40)
            shentsize, shnum, shstrndx = struct.unpack_from(e + "HHH", b, 58)
            sh_fmt, sym_fmt = e + "IIQQQQIIQQ", e + "IBBHQQ"
        else:
            (self.type,) = struct.unpack_from(e + "H", b, 16)
            shoff, = struct.unpack_from(e + "I", b, 32)
            shentsize, shnum, shstrndx = struct.unpack_from(e + "HHH", b, 46)
            sh_fmt, sym_fmt = e + "IIIIIIIIII", e + "IIIBBH"
        secs = []
        for i in range(shnum):
            f = struct.unpack_from(sh_fmt, b, shoff + i * shentsize)
            secs.append({"name_off": f[0], "type": f[1], "flags": f[2], "off": f[4], "size": f[5],
                         "link": f[6], "entsize": f[9]})

        def cstr(off):
            return b[off:b.index(b"\0", off)].decode("latin-1")

        names = secs[shstrndx]["off"]
        for s in secs:
            s["name"] = cstr(names + s["name_off"])
        self.sections = [s for s in secs if s["flags"] & SHF_ALLOC and s["size"]]
        for s in self.sections:
            s["nobits"] = s["type"] == SHT_NOBITS
        self.symbols = []
        for s in secs:
            if s["type"] != SHT_SYMTAB:
                continue
            strtab = secs[s["link"]]["off"]
            for i in range(s["size"] // s["entsize"]):
                f = struct.unpack_from(sym_fmt, b, s["off"] + i * s["entsize"])
                if wide:
                    name, info, shndx, size = f[0], f[1], f[3], f[5]
                else:
                    name, size, info, shndx = f[0], f[2], f[3], f[5]
                sym = {"name": cstr(strtab + name), "size": size, "bind": info >> 4,
                       "stype": info & 15, "shndx": shndx}
                sym["section"] = secs[shndx]["name"] if 0 < shndx < len(secs) else ""
                self.symbols.append(sym)


def read_budgets(path):
    text = Path(path).read_text()
    out = {}
    for k in BUDGETS:
        m = re.search(r"#define\s+BUDGET_%s\s+\((\d+)\s*\*\s*1024\)" % k, text)
        if not m:
            sys.exit("size_report: BUDGET_%s introuvable dans %s" % (k, path))
        out[k] = int(m.group(1)) * 1024
    return out


def fmt(n):
    return "{:,}".format(n).replace(",", " ")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--config", default="firmware/src/app/config.h")
    ap.add_argument("files", nargs="+")
    a = ap.parse_args()
    budget = read_budgets(a.config)
    elves = [(f, Elf(f)) for f in a.files]
    relocatable = all(e.type == ET_REL for _, e in elves)

    total = {"ram": 0, "pool": 0, "code": 0}
    for _, e in elves:
        for s in e.sections:
            k = kind(s["name"])
            total[k] += s["size"]
            if k == "ram" and not s["nobits"]:
                total["code"] += s["size"]          # the .data image is stored in flash

    tops = {"ram": [], "pool": [], "code": []}
    defined, undefined = set(), set()
    for f, e in elves:
        for s in e.symbols:
            if s["shndx"] == 0:
                if s["name"] and s["bind"] != STB_LOCAL:
                    undefined.add(s["name"])
                continue
            if s["bind"] != STB_LOCAL:
                defined.add(s["name"])
            if s["stype"] in (STT_OBJECT, STT_FUNC) and s["size"] and s["section"]:
                tops[kind(s["section"])].append((s["size"], s["name"], Path(f).name))
    external = sorted(undefined - defined)

    fails = []
    print("taille : %d fichier(s) %s" % (len(elves), "objets ELF (avant liaison)" if relocatable
                                         else "ELF lié"))
    rows = [("RAM", total["ram"], "RAM"), ("pool", total["pool"], "POOL"), ("code", total["code"], "CODE")]
    for label, n, k in rows:
        ok = n <= budget[k]
        if not ok:
            fails.append("%s : %s o > %s o" % (label, fmt(n), fmt(budget[k])))
        note = "  (indicatif : Thumb-2, pas pi32v2)" if label == "code" and relocatable else ""
        print("  %-7s %9s o / %9s (%3d Kio)  %3d %%  %s%s" % (label, fmt(n), fmt(budget[k]),
              budget[k] // 1024, n * 100 // budget[k], "ok" if ok else "DÉPASSÉ", note))
    for k, label in (("ram", "RAM"), ("pool", "pool"), ("code", "code")):
        big = sorted(tops[k], reverse=True)[:5]
        if big:
            print("  plus gros, %s : %s" % (label, ", ".join("%s %s (%s)" % (n, fmt(sz), f)
                                                              for sz, n, f in big)))
    if external:
        print("  externes (plateforme, libc fournie) : %s" % ", ".join(external))
    bad = [n for n in external if FORBIDDEN.match(n)]
    if bad:
        fails.append("division 64 bits ou flottant logiciel (règles 1 et 2) : %s" % ", ".join(bad))

    for f in fails:
        print("size_report: ÉCHEC : " + f)
    if not fails:
        print("size_report: ok")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())

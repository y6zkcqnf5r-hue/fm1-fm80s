#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# FM80S lint: the chip rules of CLAUDE.md, checked on the shared firmware code, plus SPDX headers.
#
#   tools/lint.sh             firmware/src (code rules) + every source file of the repo (SPDX)
#   tools/lint.sh PATH...     the given files or directories only (both checks)
#
# Code rules run on firmware/src/**/*.{c,h} once comments and string literals are stripped, so a
# comment may say "float". Each fault prints "file:line: [rule] message"; exit 1 if any.
#   [float]     float / double              (no FPU, ADR-009)
#   [alloc]     malloc / calloc / realloc / free
#   [printf]    the printf family
#   [mathh]     #include <math.h>
#   [hostmacro] APP_WEB / APP_HOST / __EMSCRIPTEN__ (the shared code does not know where it runs)
#   [appdevice] APP_DEVICE outside app/plat.h
#   [romaddr]   a 0xFFCxxxxx constant (looks like a ROM address to the device linker)
#   [div64]     / or % on a line that mentions a 64-bit type (heuristic: no 64-bit division)
#   [spdx]      no "SPDX-License-Identifier: GPL-3.0-only" in the first 3 lines (.c .h .py .sh .js .mjs
#               .html .yml, Makefile)
set -u
cd "$(dirname "$0")/.."

fails=0
tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT

say() { echo "$1:$2: [$3] $4"; fails=$((fails + 1)); }

# blank out comments (also across lines) and the inside of string / char literals, line by line
strip() {
    awk '
    BEGIN { inc = 0 }
    {
        s = $0; n = length(s); i = 1; out = ""
        while (i <= n) {
            c = substr(s, i, 1); c2 = substr(s, i, 2)
            if (inc) { if (c2 == "*/") { inc = 0; i += 2 } else i++; continue }
            if (c2 == "/*") { inc = 1; i += 2; out = out " "; continue }
            if (c2 == "//") break
            if (c == "\"" || c == "\047") {
                q = c; out = out q; i++
                while (i <= n) {
                    c = substr(s, i, 1)
                    if (c == "\\") { i += 2; continue }
                    if (c == q) break
                    i++
                }
                out = out q; i++; continue
            }
            out = out c; i++
        }
        print out
    }' "$1"
}

# grep the stripped copy, report each hit against the original file
rule() {    # file rule message regex
    grep -nE "$4" "$tmp" | while IFS=: read -r ln _; do
        echo "$1:$ln: [$2] $3"
    done
}

W='(^|[^A-Za-z0-9_])'    # word start
E='([^A-Za-z0-9_]|$)'    # word end

code_file() {
    f="$1"
    strip "$f" > "$tmp"
    {
        rule "$f" float "float/double interdit (pas de FPU, point fixe)" "${W}(float|double)${E}"
        rule "$f" alloc "allocation dynamique interdite (mémoire statique)" "${W}(malloc|calloc|realloc|free)[[:space:]]*\("
        rule "$f" printf "printf interdit dans firmware/src" "${W}(printf|fprintf|sprintf|snprintf|vprintf|vfprintf|vsprintf|vsnprintf)[[:space:]]*\("
        rule "$f" hostmacro "le code commun ne doit pas savoir où il tourne" "${W}(APP_WEB|APP_HOST|__EMSCRIPTEN__)${E}"
        case "$f" in
            */app/plat.h) ;;
            *) rule "$f" appdevice "APP_DEVICE seulement dans app/plat.h" "${W}APP_DEVICE${E}" ;;
        esac
        rule "$f" romaddr "constante 0xFFC….. (ressemble à une adresse de la ROM)" "${W}0[xX][fF][fF][cC][0-9a-fA-F]{5}[uUlL]*${E}"
        grep -nE "${W}(u?int64_t|long[[:space:]]+long)${E}" "$tmp" | grep -E '[/%]' | while IFS=: read -r ln _; do
            echo "$f:$ln: [div64] division ou modulo sur une ligne en 64 bits (décaler, ou diviser en 32 bits)"
        done
    } > "$tmp.out"
    grep -nE '^[[:space:]]*#[[:space:]]*include[[:space:]]*[<"]math\.h[>"]' "$f" | while IFS=: read -r ln _; do
        echo "$f:$ln: [mathh] math.h interdit (tables générées par tools/gen_tables.py)"
    done >> "$tmp.out"
    if [ -s "$tmp.out" ]; then
        sort -t: -k2,2n "$tmp.out"
        fails=$((fails + $(wc -l < "$tmp.out")))
    fi
    rm -f "$tmp.out"
}

spdx_file() {
    head -n 3 "$1" | grep -q 'SPDX-License-Identifier: GPL-3.0-only' || say "$1" 1 spdx "en-tête SPDX GPL-3.0-only manquant"
}

is_code() { case "$1" in *.c | *.h) return 0 ;; esac; return 1; }
is_source() { case "$1" in *.c | *.h | *.py | *.sh | *.js | *.mjs | *.html | *.yml | Makefile | */Makefile) return 0 ;; esac; return 1; }

list() {    # every file under the given paths
    for p in "$@"; do
        if [ -d "$p" ]; then find "$p" -type f | sort; else echo "$p"; fi
    done
}

if [ $# -gt 0 ]; then
    for f in $(list "$@"); do
        is_code "$f" && code_file "$f"
        is_source "$f" && spdx_file "$f"
    done
else
    for f in $(list firmware/src); do
        is_code "$f" && code_file "$f"
    done
    # every source file git would track (committed or not), minus the lint's own bad examples
    for f in $( (git ls-files --cached --others --exclude-standard 2>/dev/null || find . -type f -not -path './.git/*' -not -path './build/*' -not -path './vendor-ref/*' | sed 's|^\./||') | grep -v '^tests/lint/bad/' | sort -u); do
        [ -f "$f" ] && is_source "$f" && spdx_file "$f"
    done
fi

if [ "$fails" -gt 0 ]; then
    echo "lint: $fails faute(s)"
    exit 1
fi
echo "lint: ok"

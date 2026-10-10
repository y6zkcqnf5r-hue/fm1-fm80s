#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# The lint checks itself: good.c must pass, and each bad/*.c must fail on the rule its
# "// expect: RULE" line names (and on that rule only).
set -u
cd "$(dirname "$0")/../.."
fails=0
if out=$(sh tools/lint.sh tests/lint/good.c 2>&1); then
    echo "  ok   good.c"
else
    echo "  FAIL good.c should pass:"; echo "$out" | sed 's/^/       /'; fails=$((fails + 1))
fi
for f in tests/lint/bad/*.c; do
    want=$(sed -n 's|^// expect: *||p' "$f")
    out=$(sh tools/lint.sh "$f" 2>&1)
    code=$?
    rules=$(echo "$out" | sed -n 's/.*: \[\([a-z0-9]*\)\].*/\1/p' | sort -u | tr '\n' ' ')
    if [ $code -ne 0 ] && [ "$rules" = "$want " ]; then
        echo "  ok   $(basename "$f") -> [$want]"
    else
        echo "  FAIL $(basename "$f"): want [$want], got exit $code, rules: $rules"
        echo "$out" | sed 's/^/       /'
        fails=$((fails + 1))
    fi
done
[ $fails -eq 0 ] && echo "lint selftest: ok" || echo "lint selftest: $fails FAILED"
exit $fails

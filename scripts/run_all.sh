#!/bin/sh
# Runs every problem binary passed as an argument and prints one line per
# problem plus a final summary. Exits non-zero if any problem fails.
# Used by `make test` and `make sanitize`.

pass=0
fail=0

for bin in "$@"; do
    name=${bin#build/*/}
    if out=$("./$bin" 2>&1); then
        summary=$(printf '%s\n' "$out" | tail -n 1)
        printf 'PASS  %-45s %s\n' "$name" "$summary"
        pass=$((pass + 1))
    else
        printf 'FAIL  %s\n' "$name"
        printf '%s\n' "$out" | grep -E 'FAIL|ERROR|runtime error|Sanitizer' | head -n 20 | sed 's/^/        /'
        fail=$((fail + 1))
    fi
done

echo
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]

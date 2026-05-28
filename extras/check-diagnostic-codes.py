#!/usr/bin/env python3
"""Catalog-hygiene invariant for Slang diagnostic codes.

Scans the Slang source tree for diagnostic catalog entries:

  * `source/slang/slang-diagnostics.lua` — `err(name, code, ...)` and
    `warning(name, code, ...)` and `note(name, code, ...)` blocks.
  * Every `source/.../*-diagnostic-defs.h` — `DIAGNOSTIC(code, severity,
    name, ...)` macro calls.

Builds a `int -> [(name, file:line)]` map and reports any integer code
that is bound to two or more *distinct* names. Multiple entries that
share both code AND name (e.g. severity-bucket umbrellas like E39999
where many keyed message templates fan in) are intentional and not
flagged; they are listed in `INTENTIONAL_UMBRELLAS` below.

Exit status:
  0   — no unallowed collisions.
  >0  — at least one unallowed collision; the script prints each one.

This is meant to run in CI as a build-time invariant so the catalog
does not silently accumulate new double-binds (cf. shader-slang/slang
issue #11318). Run locally with:

    python3 extras/check-diagnostic-codes.py

To intentionally suppress an existing collision while it is being
fixed, append the integer code to `INTENTIONAL_UMBRELLAS` with a
`# TODO: ...` comment naming the tracking issue.
"""

import os
import re
import sys
from collections import defaultdict


# Codes whose multi-binding is intentional. Keep sorted, and pair every
# entry with a comment naming why or with the tracking issue.
INTENTIONAL_UMBRELLAS = {
    10000,   # `illegalCharacter*` printable / hex variants in slang-lexer-diagnostic-defs.h
    39999,   # overload-resolution / lookup umbrella — many keyed templates per call site
    99999,   # internal-compiler-error catch-all
    # TODO(#11318): the parallel E20001-E20012 range in
    # slang-json-diagnostic-defs.h is still bound to the lua-side names
    # of the same codes (12 collisions). Renumbering the JSON side to a
    # free range is tracked as a follow-up; remove these once it lands.
    20001, 20002, 20003, 20004, 20005, 20006,
    20007, 20008, 20009, 20010, 20011, 20012,
}


LUA_PATH = "source/slang/slang-diagnostics.lua"
DEFS_GLOB = "source/**/*-diagnostic-defs.h"


_lua_block_re = re.compile(
    r"""(?mx)
    ^\s*(?:err|warning|note|abort)\(
    \s*"(?P<name>[^"]+)",
    \s*(?P<code>\d+),
    """
)


_defs_re = re.compile(
    r"""(?mx)
    ^\s*DIAGNOSTIC\(
    \s*(?P<code>\d+)
    \s*,
    \s*[A-Za-z_][\w]*       # severity
    \s*,
    \s*(?P<name>[A-Za-z_][\w]*)
    """
)


def scan_lua(path: str, codes: dict):
    if not os.path.exists(path):
        return
    with open(path, encoding="utf-8") as f:
        text = f.read()
    line = 1
    pos = 0
    for m in _lua_block_re.finditer(text):
        line += text.count("\n", pos, m.start())
        pos = m.start()
        codes[int(m.group("code"))].append((m.group("name"), f"{path}:{line}"))


def scan_defs(path: str, codes: dict):
    with open(path, encoding="utf-8") as f:
        text = f.read()
    line = 1
    pos = 0
    for m in _defs_re.finditer(text):
        line += text.count("\n", pos, m.start())
        pos = m.start()
        codes[int(m.group("code"))].append((m.group("name"), f"{path}:{line}"))


def main(argv):
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
    os.chdir(repo_root)

    codes: dict[int, list[tuple[str, str]]] = defaultdict(list)
    scan_lua(LUA_PATH, codes)

    # Walk source/ for *-diagnostic-defs.h.
    for root, _, files in os.walk("source"):
        for fn in files:
            if fn.endswith("-diagnostic-defs.h"):
                scan_defs(os.path.join(root, fn), codes)

    bad = []
    for code, bindings in sorted(codes.items()):
        if code in INTENTIONAL_UMBRELLAS:
            continue
        names = sorted({n for n, _ in bindings})
        if len(names) > 1:
            bad.append((code, bindings))

    if not bad:
        print(f"OK: scanned {sum(len(v) for v in codes.values())} catalog "
              f"entries across {len(codes)} distinct codes; no collisions.")
        return 0

    print("ERROR: integer-code collisions in the diagnostic catalogs:", file=sys.stderr)
    for code, bindings in bad:
        print(f"\n  E{code:05d}:", file=sys.stderr)
        for name, where in bindings:
            print(f"      {name:48s}  {where}", file=sys.stderr)
    print(
        "\nEvery integer code must be bound to exactly one name across all "
        "catalogs. If a multi-binding is intentional (severity-bucket "
        "umbrella, etc.), add the code to INTENTIONAL_UMBRELLAS with a "
        "comment naming the reason.",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))

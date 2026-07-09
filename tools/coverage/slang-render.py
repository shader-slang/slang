#!/usr/bin/env python3
"""Slang wrapper around `tools/coverage-html/slang-coverage-html.py`.

Pre-injects Slang's `--filter-exclude-regex` patterns (from
`slang_filters.SLANGC_EXCLUDE_PATTERNS`) so the rendered report
shows only the slangc compiler-only file set. All other CLI flags
pass through unchanged.

Usage mirrors the underlying tool:

    tools/coverage/slang-render.py <input.lcov> [--auth-summary FILE]
        [--output-dir DIR] [--title TEXT] [--source-root PATH]

Plain customers (e.g. shader-coverage-demo) should call the
underlying renderer directly — this wrapper exists only to add
Slang-specific defaults that aren't relevant outside this repo.
"""

import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
RENDERER = os.path.normpath(
    os.path.join(HERE, os.pardir, "coverage-html", "slang-coverage-html.py")
)
sys.path.insert(0, HERE)

from slang_filters import SLANGC_EXCLUDE_PATTERNS  # noqa: E402


def main() -> int:
    cmd = [sys.executable, RENDERER]
    for pat in SLANGC_EXCLUDE_PATTERNS:
        cmd += ["--filter-exclude-regex", pat]
    cmd += sys.argv[1:]
    return subprocess.call(cmd)


if __name__ == "__main__":
    sys.exit(main())

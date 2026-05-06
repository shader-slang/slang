#!/usr/bin/env python3
"""Slang wrapper around `tools/coverage-html/slang-coverage-merge.py`.

Pre-injects:

  - `--strip-prefix` for each Slang CI runner root, so per-OS LCOVs
    collapse to a stable repo-relative shape before merging.
  - `--filter-exclude-regex` for the slangc-only file set.

All other CLI flags (--auth-summary, --auth-summary-out,
--synthesize-functions, -o, etc.) pass through unchanged.

Usage:

    tools/coverage/slang-merge.py linux.lcov.gz macos.lcov.gz \\
        windows.lcov.gz -o merged.lcov \\
        --auth-summary linux-report.txt \\
        --auth-summary macos-report.txt \\
        --auth-summary-out merged-report.txt
"""

import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
MERGER = os.path.normpath(
    os.path.join(HERE, os.pardir, "coverage-html", "slang-coverage-merge.py")
)
sys.path.insert(0, HERE)

from slang_filters import (  # noqa: E402
    SLANGC_EXCLUDE_PATTERNS,
    SLANG_CI_STRIP_PREFIXES,
)


def main() -> int:
    cmd = [sys.executable, MERGER]
    for prefix in SLANG_CI_STRIP_PREFIXES:
        cmd += ["--strip-prefix", prefix]
    for pat in SLANGC_EXCLUDE_PATTERNS:
        cmd += ["--filter-exclude-regex", pat]
    cmd += sys.argv[1:]
    return subprocess.call(cmd)


if __name__ == "__main__":
    sys.exit(main())

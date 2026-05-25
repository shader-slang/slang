#!/usr/bin/env python3
"""Synthesize the merged ``coverage-summary.json`` from the auth-summary
text files written by ``slang-coverage-merge.py --auth-summary-out``.

Same JSON schema as the per-OS ``coverage-summary.json`` files produced
by ``ci-slang-coverage.yml`` so the landing-page generator can read all
four summaries (linux / macos / windows / merged) with one set of
``get_json_value`` helpers. Percent fields keep their trailing ``%`` to
match the per-OS files.

Used by the renderer step in ``.github/workflows/coverage-nightly.yml``:

    python3 tools/coverage/merge_summary.py \\
        coverage-reports/merged/full/coverage-report.txt \\
        coverage-reports/merged/slangc/coverage-report.txt \\
        "$(date -u +%Y-%m-%d)" "$(git rev-parse --short HEAD)" \\
        > coverage-reports/merged/coverage-summary.json

Stand-alone Python so it can be unit-tested at
``tools/coverage/tests/test_merge_summary.py``.
"""

from __future__ import annotations

import json
import os
import sys
from typing import Any, Dict, Optional


def _pct(s: str) -> str:
    """Return a percent string verbatim if it ends with ``%``, otherwise
    the empty string. ``llvm-cov report`` emits ``-`` in the Cover column
    when the metric is unmeasured (zero total regions, etc.); we surface
    that as an empty string so the landing-page generator treats it the
    same as missing data."""
    return s if s.endswith("%") else ""


def parse_total(path: Optional[str]) -> Optional[Dict[str, Any]]:
    """Parse the TOTAL line of an llvm-cov-style report text file.

    Returns a dict matching the per-OS coverage-summary.json field shape
    (region/function/line/branch coverage, hit/found counts) or ``None``
    if the file is missing, has no TOTAL line, or has a malformed TOTAL.
    Failure reasons are logged to stderr so a future column-layout change
    in ``write_llvm_cov_report`` surfaces in the workflow logs.

    Column layout (whitespace-separated, 13 fields incl. leading TOTAL):

        TOTAL  <regions> <missed_r> <r_cov>
               <functions> <missed_f> <f_cov>
               <lines> <missed_l> <l_cov>
               <branches> <missed_b> <b_cov>
    """
    if not path or not os.path.isfile(path):
        return None
    with open(path) as f:
        for line in f:
            if not line.startswith("TOTAL"):
                continue
            p = line.split()
            if len(p) < 13:
                print(
                    f"merge_summary: malformed TOTAL line in {path} "
                    f"({len(p)} fields, expected >=13). Merged row will "
                    f"be omitted from the landing page.",
                    file=sys.stderr,
                )
                return None
            # Count columns are integer hit/miss totals -- llvm-cov never
            # emits ``-`` here (only in the percent columns). A non-integer
            # in these slots means the report layout has drifted and we
            # cannot trust any of the derived counts, so omit the row
            # rather than publish misleading metrics.
            try:
                r_tot, r_miss = int(p[1]), int(p[2])
                f_tot, f_miss = int(p[4]), int(p[5])
                l_tot, l_miss = int(p[7]), int(p[8])
                b_tot, b_miss = int(p[10]), int(p[11])
            except ValueError:
                print(
                    f"merge_summary: non-integer TOTAL counts in {path}. "
                    f"Merged row will be omitted from the landing page.",
                    file=sys.stderr,
                )
                return None
            return {
                "region_coverage": _pct(p[3]),
                "regions_hit": r_tot - r_miss,
                "regions_found": r_tot,
                "function_coverage": _pct(p[6]),
                "functions_hit": f_tot - f_miss,
                "functions_found": f_tot,
                "line_coverage": _pct(p[9]),
                "lines_hit": l_tot - l_miss,
                "lines_found": l_tot,
                "branch_coverage": _pct(p[12]),
                "branches_hit": b_tot - b_miss,
                "branches_found": b_tot,
            }
    print(
        f"merge_summary: no TOTAL line in {path}. Merged row will be "
        f"omitted from the landing page.",
        file=sys.stderr,
    )
    return None


def build_summary(
    full_txt: Optional[str],
    slangc_txt: Optional[str],
    date: str,
    commit: str,
) -> Dict[str, Any]:
    """Combine the full and slangc TOTAL parses into one summary dict.

    The slangc fields are prefixed ``slangc_`` to match the per-OS
    schema. The dict always carries ``date`` / ``commit`` / ``platform``
    / ``platform_detail`` / ``_version`` metadata; coverage fields are
    present only when ``parse_total`` succeeds for the corresponding
    input."""
    out: Dict[str, Any] = {
        "date": date,
        "commit": commit,
        "platform": "merged",
        "platform_detail": "linux+macos+windows",
    }
    full = parse_total(full_txt)
    if full:
        out.update(full)
    slangc = parse_total(slangc_txt)
    if slangc:
        out.update({"slangc_" + k: v for k, v in slangc.items()})
    out["_version"] = 2
    return out


def main(argv: Optional[list] = None) -> int:
    args = sys.argv[1:] if argv is None else argv
    if len(args) != 4:
        print(
            "usage: merge_summary.py FULL_TXT SLANGC_TXT DATE COMMIT",
            file=sys.stderr,
        )
        return 2
    full_txt, slangc_txt, date, commit = args
    summary = build_summary(full_txt, slangc_txt, date, commit)
    json.dump(summary, sys.stdout, indent=2)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())

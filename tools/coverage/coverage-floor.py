#!/usr/bin/env python3
"""Coverage ratchet for the generated-test suite.

A suite-level floor on slangc compiler coverage: once recorded, a later
coverage run may not drop the covered-line / region / branch counts below
the floor without an explicit, justified re-record. This is the guardrail
that would have caught the 2026-06 regression where a doc-scope regen
silently deleted ~800 emission tests and dropped line coverage 53.10% ->
51.77% with nothing failing.

Design note: the floor is *suite-level*, not per-bundle. Per-bundle
attribution would need each bundle measured in its own profile; the merged
suite total is what the runner already produces and is enough to catch a
regression. The floor is intentionally adjustable (re-record with a reason)
because a legitimate compiler refactor can move coverage.

Usage:
  coverage-floor.py record <label-slangc-report.txt> [--reason TEXT]
  coverage-floor.py check  <label-slangc-report.txt>

`record` writes coverage-snapshot/floors.json from the report's TOTAL line.
`check` parses the report's TOTAL and exits 1 if covered lines/regions/
branches fall below the recorded floor (0 if at or above, 2 on error).
"""
import json
import sys
from pathlib import Path

SNAP_DIR = Path(__file__).resolve().parent.parent.parent / (
    "docs/generated/tests/_meta/coverage-snapshot"
)
FLOORS = SNAP_DIR / "floors.json"


def parse_total(report_path):
    """Return dict of covered lines/regions/branches from a llvm-cov TOTAL line.

    Columns: Regions MissedReg Cov% Func MissedF Exec% Lines MissedLines
    Cov% Branch MissedBr Cov%.
    """
    for ln in Path(report_path).read_text(encoding="utf-8").splitlines():
        if ln.startswith("TOTAL"):
            n = ln.split()
            regions, missed_r = int(n[1]), int(n[2])
            lines, missed_l = int(n[7]), int(n[8])
            branches, missed_b = int(n[10]), int(n[11])
            return {
                "lines_covered": lines - missed_l,
                "lines_total": lines,
                "regions_covered": regions - missed_r,
                "branches_covered": branches - missed_b,
            }
    raise SystemExit(f"no TOTAL line found in {report_path}")


def main(argv):
    if len(argv) < 2 or argv[0] not in ("record", "check"):
        print(__doc__)
        return 2
    mode, report = argv[0], argv[1]
    cur = parse_total(report)
    if mode == "record":
        reason = ""
        if "--reason" in argv:
            reason = argv[argv.index("--reason") + 1]
        FLOORS.write_text(
            json.dumps({"slangc": cur, "reason": reason}, indent=2) + "\n",
            encoding="utf-8",
        )
        print(f"recorded floor from {report}:")
        for k, v in cur.items():
            print(f"  {k}: {v}")
        return 0
    # check
    if not FLOORS.exists():
        print(f"no floor recorded yet at {FLOORS}; run `record` first.")
        return 2
    floor = json.loads(FLOORS.read_text(encoding="utf-8"))["slangc"]
    regressed = False
    for k in ("lines_covered", "regions_covered", "branches_covered"):
        delta = cur[k] - floor[k]
        flag = "OK " if delta >= 0 else "FAIL"
        if delta < 0:
            regressed = True
        print(f"{flag} {k}: {cur[k]} vs floor {floor[k]} ({delta:+d})")
    if regressed:
        print(
            "\nCOVERAGE REGRESSION below recorded floor. Either restore the "
            "lost coverage, or re-record the floor with a justification "
            "(`record --reason ...`) if the drop is a legitimate refactor."
        )
        return 1
    print("\ncoverage at or above floor.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

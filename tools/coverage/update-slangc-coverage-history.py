#!/usr/bin/env python3
"""Append today's slangc coverage metrics to the canonical history JSON.

Reads the combined-summary.json written by the nightly coverage job,
extracts slangc metrics for all available platforms (Linux, macOS, Windows),
and upserts the record into slangc-coverage-history.json.

Usage:
    python3 update-slangc-coverage-history.py \
        --summary reports/history/2026-06-24-abc1234/combined-summary.json \
        --history reports/slangc-coverage-history.json
"""
import argparse
import json
import os
import sys

# Fields stored per platform (Windows only has line coverage)
SLANGC_FIELDS = [
    "slangc_line_coverage", "slangc_lines_hit", "slangc_lines_found",
    "slangc_function_coverage", "slangc_functions_hit", "slangc_functions_found",
    "slangc_branch_coverage",   "slangc_branches_hit", "slangc_branches_found",
    "slangc_region_coverage",   "slangc_regions_hit",  "slangc_regions_found",
]


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--summary", required=True,
                    help="combined-summary.json for today's run")
    ap.add_argument("--history", required=True,
                    help="slangc-coverage-history.json to update")
    args = ap.parse_args()

    if not os.path.exists(args.summary):
        print(f"summary not found: {args.summary}; skipping history update")
        return

    summary = json.load(open(args.summary, encoding="utf-8"))
    date   = summary.get("date") or ""
    commit = summary.get("commit") or ""
    if not date:
        print("no date in summary; skipping history update")
        return

    platforms = summary.get("platforms", {})
    linux = platforms.get("linux", {})
    if not linux or "slangc_line_coverage" not in linux:
        print("no Linux slangc data in summary; skipping history update")
        return

    new_rec = {"date": date, "commit": commit}
    # Store each platform's slangc fields with a prefix
    for platform in ("linux", "macos", "windows"):
        pd = platforms.get(platform, {})
        for field in SLANGC_FIELDS:
            if field in pd:
                new_rec[f"{platform}_{field}"] = pd[field]

    # Load existing history
    history = []
    if os.path.exists(args.history):
        history = json.load(open(args.history, encoding="utf-8"))

    by_date = {r["date"]: r for r in history}
    by_date[date] = new_rec
    history = sorted(by_date.values(), key=lambda r: r["date"])

    with open(args.history, "w", encoding="utf-8") as fh:
        json.dump(history, fh, indent=2)
    print(f"updated {args.history}: {len(history)} records (date={date})")


if __name__ == "__main__":
    main()

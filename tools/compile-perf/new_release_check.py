#!/usr/bin/env python3
"""Sweep newly published releases into the tracked history.

Run by the nightly after its own bench, BEFORE the results push: compares the
GitHub release tags visible in the local slang checkout against the tracked
index.json, and for every new tag at or after the subrelease cutoff fetches
the prebuilt binaries and benches them like the release sweep does — so a
release (or patch release) appears on the site's release charts the night it
ships instead of waiting for a manual resync.

Patch releases (vYYYY.N.M) are tracked from SUBRELEASE_CUTOFF on; older patch
releases are deliberately not backfilled (maintainer decision: the historical
release charts stay at minor granularity before the cutoff).

Fails WITHOUT leaving partial baselines: if the sweep of the new tags does not
complete cleanly, their results directories are removed so the tracked history
never contains a partial release point; the next nightly retries.
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import fetch_releases

# Track majors AND patch releases from here on; nothing older is backfilled.
SUBRELEASE_CUTOFF = (2026, 13)

_VER = re.compile(r"^v(\d{4})\.(\d+)(?:\.(\d+))?$")


def version_tuple(tag):
    m = _VER.match(tag)
    if not m:
        return None
    return tuple(int(x) for x in m.groups(default="0"))


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--results", required=True, help="perf results repo checkout")
    ap.add_argument("--repo", default=fetch_releases.DEFAULT_REPO,
                    help="slang checkout (release tags are read from its git)")
    ap.add_argument("--samples", type=int, default=5)
    args = ap.parse_args()

    ipath = os.path.join(args.results, "index.json")
    known = set()
    if os.path.exists(ipath):
        known = {r["tag"] for r in json.load(open(ipath))}

    candidates = [tag for _date, tag in fetch_releases.window_tags(
        args.repo, "2000-01-01", "9999-12-31", include_patches=True)]
    new = [t for t in candidates
           if t not in known
           and (v := version_tuple(t)) is not None
           and v >= SUBRELEASE_CUTOFF]
    if not new:
        print("new-release check: nothing new to sweep")
        return

    print(f"new-release check: sweeping {', '.join(new)}")
    py = sys.executable

    def run(*cmd):
        return subprocess.run([py, *cmd], cwd=HERE).returncode

    rc = run("fetch_releases.py", "--repo", args.repo, "--tags", ",".join(new))
    if rc == 0:
        rc = run("sweep.py", "--index", os.path.join(HERE, "releases", "index.json"),
                 "--results", args.results, "--samples", str(args.samples), "--api")
    if rc == 0:
        rc = run("track.py", "merge-index", "--results", args.results,
                 "--index", os.path.join(HERE, "releases", "index.json"))
    if rc == 0:
        rc = run("track.py", "rebuild", "--results", args.results,
                 "--index", os.path.join(args.results, "index.json"))
    if rc != 0:
        # No partial baselines: drop whatever the failed attempt wrote so the
        # tracked history stays clean; the next nightly retries from scratch.
        for t in new:
            shutil.rmtree(os.path.join(args.results, "releases", t),
                          ignore_errors=True)
        sys.exit(f"new-release sweep of {', '.join(new)} failed; "
                 f"partial results removed, next nightly retries")
    print(f"new-release check: {', '.join(new)} added to the tracked history")


if __name__ == "__main__":
    main()

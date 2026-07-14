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
# The cutoff is deliberately a 2-tuple compared against version_tuple's
# 3-tuple: Python compares the common prefix first and a longer tuple wins
# ties, so (2026, 13, 0) >= (2026, 13) is True (v2026.13 itself is in) and
# (2026, 12, 5) >= (2026, 13) is False (older patch releases stay out). Keep
# the cutoff at 2 elements — a 3-element cutoff silently changes which
# releases enter the permanent tracked history.
SUBRELEASE_CUTOFF = (2026, 13)
assert len(SUBRELEASE_CUTOFF) == 2, \
    "cutoff must be (year, minor); a 3-tuple changes patch-release selection"

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
    # Snapshot of the tracked index BEFORE any step runs: merge-index (step 3)
    # writes the new tags into it, so a failure in a LATER step (rebuild)
    # must restore this snapshot along with removing the result dirs —
    # otherwise the tag stays in `known` forever and is never re-swept.
    index_before = None
    if os.path.exists(ipath):
        with open(ipath, "rb") as fh:
            index_before = fh.read()
        known = {r["tag"] for r in json.loads(index_before)}

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
        # No partial baselines: drop whatever the failed attempt wrote AND
        # restore the pre-run index.json (merge-index may already have added
        # the new tags to it) so the tracked history stays clean and the tags
        # still read as new; the next nightly retries from scratch.
        for t in new:
            shutil.rmtree(os.path.join(args.results, "releases", t),
                          ignore_errors=True)
        if index_before is not None:
            with open(ipath, "wb") as fh:
                fh.write(index_before)
        elif os.path.exists(ipath):
            # merge-index CREATED the index this run; a leftover copy would
            # make the failed tags read as known and never retried.
            os.remove(ipath)
        sys.exit(f"new-release sweep of {', '.join(new)} failed; "
                 f"partial results removed, next nightly retries")
    print(f"new-release check: {', '.join(new)} added to the tracked history")


if __name__ == "__main__":
    main()

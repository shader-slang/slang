#!/usr/bin/env python3
"""Assemble and maintain the compile-perf TRACKING series for CI.

The tracking series is the per-release history (one swept point per release, all
measured on the *current* runner) followed by the daily tip-of-tree (ToT) points
recorded since the last release:

    tracking = [release points ...]  ++  [daily ToT points with date > last release]

Absolute compile times are runner-specific, so the release history and the daily
points are comparable only when measured on the same machine. The per-release
history is (re)built by the manual release-sweep CI job whenever the benchmark
runner changes — "sync the history to the current runner" — and each rebuild
stamps a runner fingerprint. The daily ToT job appends one point and rebuilds.

Layout under --results (a checkout of the perf results repo in CI):
    index.json                   release list {tag,date,version} (fetch_releases.py)
    releases/<tag>/results.json  per-release sweep (bench.py output) — the history
    daily/<label>/results.json   one ToT sweep per night (label = <date>-<shortsha>)
    daily/<label>/meta.json      {date, commit, runner, kind:"daily"}
    runner.json                  {fingerprint, label} the history was built on
    tracking/tracking.json      derived series consumed by trend.py / plots

Commands:
    track.py register --label <date>-<sha> --commit <sha> [--date YYYY-MM-DD]
                                 stamp meta.json for a freshly written daily sweep, then rebuild
    track.py rebuild             recompute tracking/tracking.json
    track.py stamp-runner --label <run>   record runner.json (after a history rebuild)
    track.py runner-id           print this machine's runner fingerprint
    track.py summary             print the assembled tracking series
"""
import argparse
import json
import os
import platform
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)  # allow running from any directory

from lib import analyze


def runner_id():
    """A stable-per-machine fingerprint. Absolute timings only compare within one
    fingerprint; a change means the history must be re-swept on the new runner."""
    cpu = platform.processor() or platform.machine()
    # On Linux, platform.processor() often returns the architecture string
    # (e.g. "x86_64") rather than a CPU model name — the same value as
    # platform.machine(). In that case fall through to /proc/cpuinfo for the
    # actual model name, which produces a more stable per-machine fingerprint.
    # Note: `not cpu` is unreachable because the `or platform.machine()` above
    # ensures cpu is always non-empty.
    if cpu == platform.machine():
        try:
            with open("/proc/cpuinfo") as fh:
                for line in fh:
                    if line.lower().startswith("model name"):
                        cpu = line.split(":", 1)[1].strip()
                        break
        except OSError:
            pass
    cpus = os.cpu_count() or 0
    return f"{platform.system()}-{platform.machine()}-{cpu}-{cpus}cpu".replace("  ", " ")


def _point_metrics(results_json_path):
    """{ 'workload|timer': median_ms } for the default-size run of each workload.
    canonical_runs() collapses any swept (multi-size) data to default_size so
    history and daily points compare like-with-like. The median (not min) is the
    saved/compared value: it reflects the typical run rather than the single
    luckiest one, and is steadier when a build's run-to-run spread shifts."""
    runs = analyze.canonical_runs(json.load(open(results_json_path)))
    out = {}
    for r in runs:
        for timer, st in r["timers"].items():
            if st is not None:
                out[f"{r['workload']}|{timer}"] = st["median"]
    return out


def _release_points(results_dir, index_path):
    if not os.path.exists(index_path):
        return []
    pts = []
    for rec in json.load(open(index_path)):
        rj = analyze.results_path(results_dir, rec.get("tag", ""))
        if "slangc" not in rec or not os.path.exists(rj):
            continue
        pts.append({"label": rec["tag"], "date": rec.get("date", ""), "kind": "release",
                    "commit": rec.get("version", ""), "metrics": _point_metrics(rj)})
    pts.sort(key=lambda p: p["date"])
    return pts


def _daily_points(results_dir):
    ddir = os.path.join(results_dir, "daily")
    if not os.path.isdir(ddir):
        return []
    pts = []
    for label in sorted(os.listdir(ddir)):
        rj = os.path.join(ddir, label, "results.json")
        if not os.path.exists(rj):
            continue
        meta = {}
        mp = os.path.join(ddir, label, "meta.json")
        if os.path.exists(mp):
            meta = json.load(open(mp))
        date = meta.get("date") or label[:10]  # label prefix is YYYY-MM-DD
        pts.append({"label": label, "date": date, "kind": "daily",
                    "commit": meta.get("commit", ""),
                    "commit_time": meta.get("commit_time", ""),
                    "runner": meta.get("runner", ""),
                    "metrics": _point_metrics(rj)})
    # Within one date the label tiebreak is the short SHA — lexicographic hex,
    # unrelated to code order (labels carry only the commit's DATE, and e.g.
    # master's HEAD is usually committed the previous day, so same-date
    # siblings are common). Sort by the commit's full timestamp when meta
    # carries it so siblings land in true code order; the label remains the
    # deterministic fallback for points registered before commit_time existed.
    pts.sort(key=lambda p: (p["date"], p.get("commit_time") or "", p["label"]))
    return pts


def assemble(results_dir, index_path):
    """The tracking series: every release point, then the daily points dated
    strictly after the last release (the post-release ToT tail)."""
    rel = _release_points(results_dir, index_path)
    daily = _daily_points(results_dir)
    last_release_date = rel[-1]["date"] if rel else ""
    tail = [d for d in daily if d["date"] > last_release_date]
    runner = ""
    rp = os.path.join(results_dir, "runner.json")
    if os.path.exists(rp):
        runner = json.load(open(rp)).get("fingerprint", "")
    return {"runner": runner, "last_release": rel[-1]["label"] if rel else None,
            "points": rel + tail}


def rebuild(results_dir, index_path):
    series = assemble(results_dir, index_path)
    outdir = os.path.join(results_dir, "tracking")
    os.makedirs(outdir, exist_ok=True)
    out = os.path.join(outdir, "tracking.json")
    with open(out, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(series, fh, indent=2)
    n_rel = sum(1 for p in series["points"] if p["kind"] == "release")
    n_day = sum(1 for p in series["points"] if p["kind"] == "daily")
    print(f"wrote {out}: {n_rel} release + {n_day} daily point(s), "
          f"runner={series['runner'] or 'unset'}")
    return out


def register(results_dir, index_path, label, commit, date, corpus_sha="", commit_time=""):
    ddir = os.path.join(results_dir, "daily", label)
    rj = os.path.join(ddir, "results.json")
    if not os.path.exists(rj):
        raise SystemExit(f"no daily sweep at {rj}; run bench.py --out "
                         f"{os.path.join(results_dir, 'daily')} --label {label} first")
    meta = {"date": date or label[:10], "commit": commit,
            "runner": runner_id(), "kind": "daily"}
    if corpus_sha:
        meta["corpus_sha"] = corpus_sha
    # Full committer timestamp (git log -1 --format=%cI): orders same-date
    # sibling points by code chronology in the tracking series and reports.
    if commit_time:
        meta["commit_time"] = commit_time
    with open(os.path.join(ddir, "meta.json"), "w", encoding="utf-8", newline="\n") as fh:
        json.dump(meta, fh, indent=2)
    print(f"registered daily {label} (commit {commit[:9] or '?'}, runner {meta['runner']})")
    rebuild(results_dir, index_path)


def merge_index(results_dir, new_index_path):
    """Merge newly swept release entries into the results repo index.

    Reads the existing index.json (if any) from results_dir, merges in the
    entries from new_index_path (new entries override existing ones by tag),
    and writes the merged result back. This ensures the report is generated
    from all historical data, not just the releases swept in this run.
    """
    dest = os.path.join(results_dir, "index.json")
    existing = {}
    if os.path.exists(dest):
        for r in json.load(open(dest)):
            existing[r["tag"]] = r
    n_before = len(existing)
    for r in json.load(open(new_index_path)):
        existing[r["tag"]] = r
    merged = sorted(existing.values(), key=lambda r: r.get("date", ""))
    with open(dest, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(merged, fh, indent=2)
    n_added = len(existing) - n_before
    print(f"merged index: {len(merged)} releases total "
          f"({n_added} new, {len(merged) - n_added} existing)")


def stamp_runner(results_dir, label):
    rp = os.path.join(results_dir, "runner.json")
    with open(rp, "w", encoding="utf-8", newline="\n") as fh:
        json.dump({"fingerprint": runner_id(), "label": label}, fh, indent=2)
    print(f"stamped {rp}: {runner_id()} (built by {label})")


def summary(results_dir, index_path):
    series = assemble(results_dir, index_path)
    print(f"runner: {series['runner'] or 'unset'}   last release: {series['last_release']}")
    print(f"{'label':22s}{'date':12s}{'kind':9s}{'workloads':>10}")
    for p in series["points"]:
        wls = len({k.split('|', 1)[0] for k in p["metrics"]})
        print(f"{p['label']:22s}{p['date']:12s}{p['kind']:9s}{wls:>10}")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("cmd", choices=["register", "rebuild", "stamp-runner",
                                    "runner-id", "summary", "merge-index"])
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--index", default=os.path.join(HERE, "releases", "index.json"))
    ap.add_argument("--label", default="")
    ap.add_argument("--commit", default="")
    ap.add_argument("--date", default="")
    ap.add_argument("--corpus-sha", default="", dest="corpus_sha")
    ap.add_argument("--commit-time", default="", dest="commit_time",
                    help="the swept commit's full committer timestamp "
                         "(git log -1 --format=%%cI); orders same-date points")
    args = ap.parse_args()

    if args.cmd == "runner-id":
        print(runner_id())
    elif args.cmd == "rebuild":
        rebuild(args.results, args.index)
    elif args.cmd == "summary":
        summary(args.results, args.index)
    elif args.cmd == "register":
        if not args.label:
            ap.error("register needs --label")
        register(args.results, args.index, args.label, args.commit, args.date,
                 args.corpus_sha, args.commit_time)
    elif args.cmd == "merge-index":
        if not args.index or not os.path.exists(args.index):
            ap.error("merge-index needs --index <new-index.json>")
        merge_index(args.results, args.index)
    elif args.cmd == "stamp-runner":
        stamp_runner(args.results, args.label or "manual")


if __name__ == "__main__":
    main()

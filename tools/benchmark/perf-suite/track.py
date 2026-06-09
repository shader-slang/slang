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
    <tag>/results.json           per-release sweep (bench.py output) — the history
    daily/<label>/results.json   one ToT sweep per night (label = <date>-<shortsha>)
    daily/<label>/meta.json      {date, commit, runner, kind:"daily"}
    runner.json                  {fingerprint, label} the history was built on
    _tracking/tracking.json      derived series consumed by trend.py / plots

Commands:
    track.py register --label <date>-<sha> --commit <sha> [--date YYYY-MM-DD]
                                 stamp meta.json for a freshly written daily sweep, then rebuild
    track.py rebuild             recompute _tracking/tracking.json
    track.py stamp-runner --label <run>   record runner.json (after a history rebuild)
    track.py runner-id           print this machine's runner fingerprint
    track.py summary             print the assembled tracking series
"""
import argparse
import json
import os
import platform

import analyze

HERE = os.path.dirname(os.path.abspath(__file__))


def runner_id():
    """A stable-per-machine fingerprint. Absolute timings only compare within one
    fingerprint; a change means the history must be re-swept on the new runner."""
    cpu = platform.processor() or platform.machine()
    if not cpu or cpu == platform.machine():
        # Linux: platform.processor() is often empty — use the cpuinfo model name.
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
        rj = os.path.join(results_dir, rec.get("tag", ""), "results.json")
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
                    "commit": meta.get("commit", ""), "runner": meta.get("runner", ""),
                    "metrics": _point_metrics(rj)})
    pts.sort(key=lambda p: (p["date"], p["label"]))
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
    outdir = os.path.join(results_dir, "_tracking")
    os.makedirs(outdir, exist_ok=True)
    out = os.path.join(outdir, "tracking.json")
    with open(out, "w") as fh:
        json.dump(series, fh, indent=2)
    n_rel = sum(1 for p in series["points"] if p["kind"] == "release")
    n_day = sum(1 for p in series["points"] if p["kind"] == "daily")
    print(f"wrote {out}: {n_rel} release + {n_day} daily point(s), "
          f"runner={series['runner'] or 'unset'}")
    return out


def register(results_dir, index_path, label, commit, date):
    ddir = os.path.join(results_dir, "daily", label)
    rj = os.path.join(ddir, "results.json")
    if not os.path.exists(rj):
        raise SystemExit(f"no daily sweep at {rj}; run bench.py --out "
                         f"{os.path.join(results_dir, 'daily')} --label {label} first")
    meta = {"date": date or label[:10], "commit": commit,
            "runner": runner_id(), "kind": "daily"}
    with open(os.path.join(ddir, "meta.json"), "w") as fh:
        json.dump(meta, fh, indent=2)
    print(f"registered daily {label} (commit {commit[:9] or '?'}, runner {meta['runner']})")
    rebuild(results_dir, index_path)


def stamp_runner(results_dir, label):
    rp = os.path.join(results_dir, "runner.json")
    with open(rp, "w") as fh:
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
                                    "runner-id", "summary"])
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--index", default=os.path.join(HERE, "releases", "index.json"))
    ap.add_argument("--label", default="")
    ap.add_argument("--commit", default="")
    ap.add_argument("--date", default="")
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
        register(args.results, args.index, args.label, args.commit, args.date)
    elif args.cmd == "stamp-runner":
        stamp_runner(args.results, args.label or "manual")


if __name__ == "__main__":
    main()

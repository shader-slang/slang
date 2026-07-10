#!/usr/bin/env python3
"""Render the per-workload phase-composition report as a self-contained HTML file.

One panel per workload: frontEndExecute / generateOutput stacked areas across the
release + daily-ToT history, with per-workload detail pages linked from each panel
title.

    python3 report.py            # -> results/analysis/report_per_workload.html
"""
import argparse
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)  # allow running from any directory

import breakdown
from lib import analyze, manifest

CSS = """
body{font:14px/1.5 -apple-system,Segoe UI,Roboto,sans-serif;margin:0;color:#1a1a1a;background:#fafafa}
.wrap{max-width:1240px;margin:0 auto;padding:28px}
h1{font-size:24px;margin:0 0 4px} h2{font-size:18px;margin:34px 0 10px;border-bottom:2px solid #eee;padding-bottom:4px}
.sub{color:#666;margin:0 0 18px}
.chart{background:#fff;border:1px solid #eee;border-radius:6px;padding:8px;margin:10px 0;overflow:auto}
.chart svg{display:block;max-width:100%;height:auto}
.small{color:#777;font-size:12px}
"""


def combined_index(release_index, results_dir):
    """Release records (from releases/index.json) + synthetic daily records (one
    per daily/<label>/ with a results.json), date-ordered. Lets the existing
    release charts span the post-release daily ToT runs too: daily recs carry a
    placeholder 'slangc' so they pass the loaders' presence filter."""
    recs = [dict(r, kind="release") for r in release_index]
    ddir = os.path.join(results_dir, "daily")
    if os.path.isdir(ddir):
        for label in sorted(os.listdir(ddir)):
            if not os.path.exists(os.path.join(ddir, label, "results.json")):
                continue
            mp = os.path.join(ddir, label, "meta.json")
            meta = json.load(open(mp)) if os.path.exists(mp) else {}
            recs.append({"tag": label, "date": meta.get("date", label[:10]),
                         "version": meta.get("commit", ""), "slangc": "tot",
                         "kind": "daily",
                         "commit_time": meta.get("commit_time", "")})
    # Same-date daily points order by the commit's full timestamp (true code
    # order) when meta carries it; the tag is the deterministic fallback for
    # points registered before commit_time existed. See track.py.
    recs.sort(key=lambda r: (r.get("date", ""), r.get("kind") == "daily",
                             r.get("commit_time") or "", r.get("tag", "")))
    return recs


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("--index", default=os.path.join(HERE, "releases", "index.json"))
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--metric", default="median", choices=["min", "median", "mean"])
    args = ap.parse_args()

    with open(args.index) as fh:
        index = json.load(fh)

    outdir = os.path.join(args.results, "analysis")
    os.makedirs(outdir, exist_ok=True)

    # Combined index: releases + daily ToT runs, written to disk so breakdown's
    # index_path-based loaders also see the daily points.
    cindex = combined_index(index, args.results)
    cindex_path = os.path.join(outdir, "_combined_index.json")
    with open(cindex_path, "w", encoding="utf-8") as fh:
        json.dump(cindex, fh, indent=2)

    series, _, order = analyze.load_series(cindex, args.results, args.metric)
    if not order:
        raise SystemExit("no results; run sweep.py first")
    tags = [t for t, _ in order]
    n_rel = sum(1 for r in cindex if r.get("kind") != "daily")
    n_day = len(cindex) - n_rel
    n_wl = len({wl for (wl, _t) in series})

    # api workloads (mode="api": driven through libslang by the api-driver)
    # report apiTotal + api* phase timers instead of compileInner, so they get
    # their own section below rather than empty panels in the compiler grid.
    present = {wl for (wl, _t) in series}
    api_names = manifest.display_order(
        [w.name for w in manifest.WORKLOADS if w.mode == "api" and w.name in present])
    compiler_names = manifest.display_order([wl for wl in present
                                             if wl not in set(api_names)])

    wl_links = breakdown.write_workload_pages(args.results, cindex_path, args.metric, outdir)
    svg_pw_coarse = breakdown.render_stacked_multiples(
        args.results, cindex_path, args.metric,
        os.path.join(outdir, "perf_per_benchmark_coarse.svg"),
        breakdown.FE_GO_ORDER, breakdown.coarse_buckets, link_for=wl_links,
        names=compiler_names)
    svg_coarse_html = open(svg_pw_coarse, encoding="utf-8").read()

    svg_api_html = None
    if api_names:
        svg_api = breakdown.render_stacked_multiples(
            args.results, cindex_path, args.metric,
            os.path.join(outdir, "perf_per_benchmark_api.svg"),
            breakdown.API_BUCKET_ORDER, breakdown.api_buckets, link_for=wl_links,
            names=api_names,
            title=f"API-path workloads — apiTotal phase composition ({args.metric} ms)")
        svg_api_html = open(svg_api, encoding="utf-8").read()

    PW = ['<!doctype html><meta charset="utf-8">',
          f"<title>Slang compile-perf — per workload</title><style>{CSS}</style>",
          '<div class="wrap">',
          "<h1>Slang compile-time performance — per benchmark</h1>",
          f'<p class="sub">{n_rel} releases + {n_day} daily ToT runs, '
          f'<b>{analyze.short_tag(tags[0])}</b> → <b>{analyze.short_tag(tags[-1])}</b> · '
          f'metric: {args.metric} (ms) · {n_wl} workloads. '
          f'The {n_day} rightmost points per panel are daily top-of-tree builds.</p>',
          '<h2>Phase composition across releases — main phases</h2>',
          '<p class="small">One panel per workload; the two top-level phases '
          '<b>frontEndExecute</b> (green) and <b>generateOutput</b> (orange) stacked as '
          'areas across the release history, top edge tracing <code>compileInner</code> '
          '(own zero-based y-axis per panel; × = first→last ratio). '
          '<b>Click a workload name</b> to open its own page with the full sub-counter '
          'breakdown (specializeModule, simplifyIR, SemanticChecking, …).</p>',
          f'<div class="chart">{svg_coarse_html}</div>',
          *(['<h2>API-path workloads — apiTotal phase composition</h2>',
             '<p class="small">Workloads driven through <code>libslang</code> by the '
             'api-driver (session creation, module loading/linking, reflection, '
             'link-time specialization). The top edge traces <b>apiTotal</b> — the '
             'API-side cost a one-shot <code>slangc</code> run pays once and cannot '
             'separate; the compiler grid above does not include these panels '
             'because they have no <code>compileInner</code>.</p>',
             f'<div class="chart">{svg_api_html}</div>'] if svg_api_html else []),
          '<p class="small">Generated by compile-perf/report.py. '
          'Source data: results/&lt;tag&gt;/results.json.</p>',
          "</div>"]

    out = os.path.join(outdir, "report_per_workload.html")
    with open(out, "w", encoding="utf-8") as fh:
        fh.write("\n".join(PW))
    print(f"wrote {out}  ({n_rel} releases + {n_day} daily)")


if __name__ == "__main__":
    main()

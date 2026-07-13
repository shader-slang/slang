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
import daily_movers
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


SECTION_CSS = CSS + """
.secrow{background:#fff;border:1px solid #e3e6ea;border-radius:8px;
        padding:16px 20px;box-shadow:0 1px 2px rgba(0,0,0,.06);margin:14px 0}
.secrow b{font-size:17px}
.secrow .links{margin:8px 0 0}
.secrow a{font-weight:600;text-decoration:none;color:#1a5fb4}
.secrow p{color:#666;font-size:13px;margin:6px 0 0}
.status{color:#444;font-size:13px;margin:2px 0 16px}\n.worse{color:#c0392b;font-weight:600} .better{color:#1e8449;font-weight:600}\ntd.num,th.num{text-align:right}
"""

PROVENANCE_NOTE = (
    '<div class="note"><b>Methodology:</b> release points are the OFFICIAL '
    "prebuilt binaries, re-measured on the perf runner; daily tip-of-tree "
    "points are built on the runner with release-matched flags but a "
    "different MSVC toolset. Each page's points share one build provenance; "
    "absolute times are NOT comparable between the release and ToT pages (a "
    "uniform few-% offset, and more on individual hot loops).</div>")


def grid_page(path, title, sub, note, svg):
    """One cadence page: per-workload panels, two per row."""
    H = ['<!doctype html><meta charset="utf-8">',
         f"<title>{title}</title><style>{SECTION_CSS}</style>",
         '<div class="wrap">',
         '<p class="small"><a href="index.html">&larr; overview</a></p>',
         f"<h1>{title}</h1>", f'<p class="sub">{sub}</p>',
         f'<p class="small">{note}</p>',
         f'<div class="chart">{svg}</div>' if svg
         else '<p class="small">no data yet</p>',
         PROVENANCE_NOTE, "</div>"]
    with analyze.open_output(path) as fh:
        fh.write("\n".join(H))
    print(f"wrote {path}")


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("--index", default=os.path.join(HERE, "releases", "index.json"))
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--metric", default="median", choices=["min", "median", "mean"])
    ap.add_argument("--daily-window", type=int, default=30,
                    help="trailing daily points shown on the ToT pages")
    args = ap.parse_args()

    with open(args.index) as fh:
        index = json.load(fh)

    outdir = os.path.join(args.results, "analysis")
    os.makedirs(outdir, exist_ok=True)

    # Three index views: combined (per-workload detail pages show both
    # cadences), release-only, and the trailing daily window. Separate files so
    # breakdown's index_path-based loaders stay unchanged.
    cindex = combined_index(index, args.results)
    releases = [r for r in cindex if r.get("kind") != "daily"]
    dailies = [r for r in cindex if r.get("kind") == "daily"][-args.daily_window:]
    paths = {}
    for name, recs in (("combined", cindex), ("releases", releases), ("daily", dailies)):
        p = os.path.join(outdir, f"_{name}_index.json")
        with analyze.open_output(p) as fh:
            json.dump(recs, fh, indent=2)
        paths[name] = p

    series, _, order = analyze.load_series(cindex, args.results, args.metric)
    if not order:
        raise SystemExit("no results; run sweep.py first")
    present = {wl for (wl, _t) in series}
    api_names = manifest.display_order(
        [w.name for w in manifest.WORKLOADS if w.mode == "api" and w.name in present])
    compiler_names = manifest.display_order([wl for wl in present
                                             if wl not in set(api_names)])

    # Per-workload detail pages: one chart per cadence, both families.
    wl_links = breakdown.write_workload_pages(
        args.results,
        [("Across releases", paths["releases"]),
         (f"Daily tip-of-tree (last {args.daily_window} days)", paths["daily"])],
        args.metric, outdir, back="../index.html")

    rel_note = ("Official release binaries — minor releases, plus patch releases "
                "from v2026.13 on — re-measured on the current perf runner. "
                "Click a workload name for its full sub-counter breakdown.")
    day_note = (f"Runner-built master HEAD, one point per calendar day, trailing "
                f"{args.daily_window} points. Click a workload name for its full "
                "sub-counter breakdown.")

    # Four cadence pages: {api, microbench} x {tot, releases}, each a single
    # stacked column of panels.
    PAGES = []
    if api_names:
        PAGES += [("api", api_names, breakdown.API_BUCKET_ORDER, breakdown.api_buckets,
                   "API-path & RT workloads", "apiTotal")]
    PAGES += [("microbench", compiler_names, breakdown.FE_GO_ORDER,
               breakdown.coarse_buckets, "Compiler microbenchmarks", "compileInner")]
    for prefix, names, border, bfn, title, edge in PAGES:
        for cad, ipath, cad_title, note in (
                ("tot", paths["daily"], "daily tip-of-tree", day_note),
                ("releases", paths["releases"], "across releases", rel_note)):
            idx = json.load(open(ipath))
            svg = None
            if idx:
                svgp = os.path.join(outdir, f"perf_{prefix}_{cad}.svg")
                breakdown.render_stacked_multiples(
                    args.results, ipath, args.metric, svgp, border, bfn,
                    cols=2, names=names, link_for=wl_links,
                    title=f"{title} — {cad_title} ({args.metric} ms)")
                svg = open(svgp, encoding="utf-8").read()
            grid_page(os.path.join(outdir, f"{prefix}-{cad}.html"),
                      f"{title} — {cad_title}",
                      f"{len(names)} workloads · stacked phases, top edge = "
                      f"{edge} ({args.metric} ms)",
                      note, svg)

    # Landing page: stacked section rows — API & RT on top, microbenchmarks,
    # then the sweeps archive.
    n_rel, n_day = len(releases), len(dailies)
    last_daily = dailies[-1]["tag"] if dailies else "-"
    last_rel = releases[-1]["tag"] if releases else "-"
    d0 = dailies[0]["date"] if dailies else "-"
    d1 = dailies[-1]["date"] if dailies else "-"
    r0 = releases[0]["tag"] if releases else "-"
    r1 = releases[-1]["tag"] if releases else "-"
    daily_link = (f"Daily ToT: {html_escape(d0)} &rarr; {html_escape(d1)} "
                  f"({n_day} days, trailing {args.daily_window})")
    rel_link = f"Releases: {html_escape(r0)} &rarr; {html_escape(r1)} ({n_rel})"

    def secrow(title, prefix, desc):
        return ('<div class="secrow"><b>' + title + "</b>"
                f'<div class="links"><a href="{prefix}-tot.html">{daily_link}</a></div>'
                f'<div class="links"><a href="{prefix}-releases.html">{rel_link}</a></div>'
                f"<p>{desc}</p></div>")

    rows = []
    if api_names:
        rows.append(secrow(
            "API &amp; RT workloads", "api",
            f"{len(api_names)} application-integration shapes driven through "
            "libslang: RT programs, session cost, module graphs, reflection, "
            "per-variant specialization."))
    rows.append(secrow(
        "Compiler microbenchmarks", "microbench",
        f"{len(compiler_names)} workloads, one compiler pass each — parse "
        "&rarr; sema &rarr; IR &rarr; specialization &rarr; backends."))
    rows.append('<div class="secrow"><b>Complexity sweeps</b>'
                '<div class="links"><a href="sweep/">all sweeps</a></div>'
                "<p>Compile time vs workload size N — scaling curves and per-pass "
                "growth attribution; every archived sweep on one page.</p></div>")
    # Recent movers: the day boundaries that moved the suite most, with the
    # timers that moved — attribution-ready (each row names the commit range
    # to bisect). Daily points only: release points differ in build
    # provenance and would masquerade as steps.
    movers_html = ""
    try:
        dpoints = daily_movers.daily_points(args.results, args.metric)

        # Suite boundary strip: % of the previous day's suite total, so a
        # "big day" means the same thing whatever the suite's absolute size.
        def suite_pct(v0, v1):
            common = [(wl, t) for (wl, t) in v1
                      if t == daily_movers.headline(wl) and (wl, t) in v0]
            base = sum(v0[k] for k in common)
            return (sum(v1[k] for k in common) - base) / base * 100 if base else 0.0

        bounds = [(suite_pct(b[5], b[6]),) + b for b in daily_movers.boundaries(dpoints)]
        bounds.sort(key=lambda b: -abs(b[0]))
        rows_m = []
        biggest_html = ""
        if bounds:
            pct, _total, bd0, bd1, bc0, bc1, _v0, _v1 = bounds[0]
            cls = "worse" if pct > 0 else "better"
            biggest_html = (
                f'<p class="small">Largest daily change in the window: '
                f"{html_escape(bd0)} &rarr; {html_escape(bd1)} "
                f"(<code>{html_escape(bc0)}..{html_escape(bc1)}</code>), suite "
                f"<b class='{cls}'>{pct:+.1f}%</b>.</p>")

        # Top-10 window movers: per-workload headline change over the whole
        # daily window, improved and regressed together, ranked by |%|.
        per_wl = {}
        for _d, _c, vals in dpoints:
            for (wl, t), v in vals.items():
                if t == daily_movers.headline(wl):
                    per_wl.setdefault(wl, []).append(v)
        movers10 = sorted(
            ((vs[-1] / vs[0] - 1) * 100, wl, vs[0], vs[-1])
            for wl, vs in per_wl.items() if len(vs) >= 2 and vs[0] > 0)
        movers10.sort(key=lambda r: -abs(r[0]))
        if movers10:
            d0w = dpoints[0][0]
            d1w = dpoints[-1][0]
            rows_m += [f"<h2>Top movers over the daily window "
                       f"({html_escape(d0w)} &rarr; {html_escape(d1w)})</h2>",
                       biggest_html,
                       "<table><tr><th>benchmark</th><th class=num>start (ms)</th>"
                       "<th class=num>end (ms)</th><th class=num>change</th></tr>"]
            for pct, wl, v0w, v1w in movers10[:10]:
                cls = "worse" if pct > 0 else "better"
                link = f"workloads/{html_escape(wl)}.html"
                rows_m.append(
                    f"<tr><td><a href='{link}'>{html_escape(wl)}</a></td>"
                    f"<td class=num>{v0w:.1f}</td><td class=num>{v1w:.1f}</td>"
                    f"<td class='num {cls}'>{pct:+.1f}%</td></tr>")
            rows_m.append("</table>")
        movers_html = "".join(rows_m)
    except Exception as e:  # noqa: BLE001 — the strip must never sink the report
        print(f"note: recent-movers strip skipped: {e}")

    H = ['<!doctype html><meta charset="utf-8">',
         f"<title>Slang compile-time performance</title><style>{SECTION_CSS}</style>",
         '<div class="wrap">', "<h1>Slang compile-time performance</h1>",
         f'<p class="status">latest nightly: <b>{html_escape(last_daily)}</b> &nbsp;·&nbsp; '
         f'latest release in charts: <b>{html_escape(last_rel)}</b> &nbsp;·&nbsp; '
         f'{n_rel} releases + {n_day} daily points · metric: {args.metric}</p>',
         *rows,
         movers_html,
         '<p class="small">Data: <a href="https://github.com/shader-slang/slang-compile-perf">'
         "slang-compile-perf</a> · methodology: tools/compile-perf/DESIGN.md in the slang "
         "repo · alerts: the nightly trend check (daily-baseline, "
         "median-of-5).</p>", "</div>"]
    with analyze.open_output(os.path.join(outdir, "index.html")) as fh:
        fh.write("\n".join(H))

    # Old bookmark compatibility: the previous single-grid page name redirects
    # to the landing page.
    with analyze.open_output(os.path.join(outdir, "report_per_workload.html")) as fh:
        fh.write('<!doctype html><meta charset="utf-8">'
                 '<meta http-equiv="refresh" content="0; url=index.html">'
                 '<a href="index.html">moved: compile-perf overview</a>')
    print(f"wrote {os.path.join(outdir, 'index.html')}  "
          f"({n_rel} releases + {n_day} daily)")


def html_escape(s):
    return str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


if __name__ == "__main__":
    main()

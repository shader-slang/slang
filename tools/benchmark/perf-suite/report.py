#!/usr/bin/env python3
"""Render the full perf-suite analysis as a single self-contained HTML file
(charts inline as SVG + tables). Stdlib only. Reuses analyze.py and plot.py.

    python3 report.py            # -> results/_analysis/report.html
"""
import argparse
import html
import json
import os

import analyze
import plot

HERE = os.path.dirname(os.path.abspath(__file__))

CSS = """
body{font:14px/1.5 -apple-system,Segoe UI,Roboto,sans-serif;margin:0;color:#1a1a1a;background:#fafafa}
.wrap{max-width:1240px;margin:0 auto;padding:28px}
h1{font-size:24px;margin:0 0 4px} h2{font-size:18px;margin:34px 0 10px;border-bottom:2px solid #eee;padding-bottom:4px}
.sub{color:#666;margin:0 0 18px}
table{border-collapse:collapse;width:100%;background:#fff;box-shadow:0 1px 2px rgba(0,0,0,.06);margin:8px 0 4px}
th,td{padding:6px 10px;text-align:left;border-bottom:1px solid #eee;font-variant-numeric:tabular-nums}
th{background:#f4f5f7;font-weight:600;font-size:12px;text-transform:uppercase;letter-spacing:.03em;color:#555}
td.num{text-align:right} tr:hover td{background:#fafbfc}
.reg{color:#c0392b;font-weight:600} .imp{color:#1e8449;font-weight:600} .flat{color:#888}
.chart{background:#fff;border:1px solid #eee;border-radius:6px;padding:8px;margin:10px 0;overflow:auto}
.chart svg{display:block;max-width:100%;height:auto}
.pill{display:inline-block;padding:1px 7px;border-radius:10px;font-size:12px;font-weight:600}
.pill.r{background:#fdecea;color:#c0392b} .pill.g{background:#e9f7ef;color:#1e8449}
code{background:#f0f1f3;padding:1px 5px;border-radius:3px;font-size:13px}
.note{background:#fff8e1;border-left:4px solid #f0c000;padding:10px 14px;border-radius:4px;margin:10px 0}
.small{color:#777;font-size:12px}
"""


def cls(ratio):
    if ratio >= 1.15:
        return "reg"
    if ratio <= 0.9:
        return "imp"
    return "flat"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--index", default=os.path.join(HERE, "releases", "index.json"))
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--metric", default="min")
    ap.add_argument("--rel", type=float, default=1.15)
    ap.add_argument("--abs", type=float, default=2.0)
    args = ap.parse_args()

    with open(args.index) as fh:
        index = json.load(fh)
    series, lookup, order = analyze.load_series(index, args.results, args.metric)
    if not order:
        raise SystemExit("no results; run sweep.py first")
    tags = [t for t, _ in order]

    # primary timers per workload
    primary = {}
    for rec in index:
        p = os.path.join(args.results, rec.get("tag", ""), "results.json")
        if rec.get("slangc") and os.path.exists(p):
            for run in json.load(open(p)):
                primary[run["workload"]] = set(run.get("primary_timers", []))
            break

    # charts (generate SVGs, embed inline)
    outdir = os.path.join(args.results, "_analysis")
    os.makedirs(outdir, exist_ok=True)
    _, cseries = plot.load(args.index, args.results, "compileInner")
    svg_norm = plot.render(tags, cseries, "compileInner — normalized to first release",
                           "x baseline", False, True, "mdl_dxr",
                           os.path.join(outdir, "perf_normalized.svg"))
    svg_abs = plot.render(tags, cseries, "compileInner — absolute (log scale)",
                          "ms (log)", True, False, "mdl_dxr",
                          os.path.join(outdir, "perf_compileinner.svg"))
    inline = {p: open(p).read().split("?>")[-1] for p in (svg_norm, svg_abs)}

    H = ['<!doctype html><meta charset="utf-8">',
         f"<title>Slang compile-perf — release sweep</title><style>{CSS}</style>",
         '<div class="wrap">']
    H.append("<h1>Slang compile-time performance — release sweep</h1>")
    H.append(f'<p class="sub">{len(tags)} releases, <b>{tags[0]}</b> → <b>{tags[-1]}</b> · '
             f'metric: {args.metric} (ms) · 11 workloads (9 synthetic + diagnostics control + 1 real shader). '
             f'A jump is flagged at ratio ≥ {args.rel} and ≥ {args.abs} ms.</p>')

    # charts
    H.append("<h2>Charts</h2>")
    H.append(f'<div class="chart">{inline[svg_norm]}</div>')
    H.append(f'<div class="chart">{inline[svg_abs]}</div>')

    # end-to-end drift
    H.append("<h2>End-to-end drift (compileInner, first → last)</h2>")
    H.append("<table><tr><th>Workload</th><th class=num>first (ms)</th><th class=num>last (ms)</th>"
             "<th class=num>ratio</th><th>verdict</th></tr>")
    rows = []
    for (wl, timer), vals in series.items():
        if timer != "compileInner" or len(vals) < 2:
            continue
        f, l = vals[0][2], vals[-1][2]
        rows.append((wl, f, l, l / f if f else 0))
    for wl, f, l, r in sorted(rows, key=lambda x: -x[3]):
        verdict = ("<span class='pill r'>regressed</span>" if r >= args.rel
                   else "<span class='pill g'>faster</span>" if r <= 0.9 else "—")
        H.append(f"<tr><td>{html.escape(wl)}</td><td class=num>{f:.0f}</td><td class=num>{l:.0f}</td>"
                 f"<td class='num {cls(r)}'>{r:.2f}×</td><td>{verdict}</td></tr>")
    H.append("</table>")

    # ranked compileInner step-changes with leaf attribution
    H.append("<h2>Flagged step-changes (compileInner, leaf-attributed)</h2>")
    H.append("<table><tr><th>Workload</th><th>boundary</th><th class=num>prev</th><th class=num>cur</th>"
             "<th class=num>ratio</th><th class=num>Δ ms</th><th>leaf cause</th></tr>")
    flags = []
    for (wl, timer), vals in series.items():
        if timer != "compileInner":
            continue
        for ptag, tag, pv, cv, rel, delta in analyze.flag_steps(vals, args.rel, args.abs):
            d = analyze.leaf_deltas(lookup, ptag, tag, wl)
            cause = ""
            if d:
                b = max(d, key=lambda k: d[k])
                if d[b] > 0:
                    cause = f"{b} (+{d[b]:.0f} ms)"
            flags.append((wl, ptag, tag, pv, cv, rel, delta, cause))
    for wl, ptag, tag, pv, cv, rel, delta, cause in sorted(flags, key=lambda x: -x[6]):
        H.append(f"<tr><td>{html.escape(wl)}</td><td><code>{ptag}→{tag}</code></td>"
                 f"<td class=num>{pv:.0f}</td><td class=num>{cv:.0f}</td>"
                 f"<td class='num reg'>{rel:.2f}×</td><td class=num>+{delta:.0f}</td>"
                 f"<td>{html.escape(cause)}</td></tr>")
    H.append("</table>")

    # per-release series for primary timers
    H.append("<h2>Per-release series (primary timer per workload)</h2>")
    H.append('<p class="small">Cell = min ms; <span class="reg">red</span> marks a ≥1.4× jump vs the previous release.</p>')
    H.append("<table><tr><th>Workload · timer</th>" +
             "".join(f"<th class=num>{html.escape(t.replace('v20',''))}</th>" for t in tags) + "</tr>")
    for (wl, timer), vals in sorted(series.items()):
        if timer not in primary.get(wl, set()):
            continue
        byt = {t: v for t, _, v in vals}
        cells, prev = [], None
        for t in tags:
            v = byt.get(t)
            if v is None:
                cells.append("<td class=num>-</td>")
            else:
                jump = prev and v / prev >= 1.4
                cells.append(f"<td class='num{' reg' if jump else ''}'>{v:.0f}</td>")
                prev = v
        H.append(f"<tr><td>{html.escape(wl)} · {html.escape(timer)}</td>" + "".join(cells) + "</tr>")
    H.append("</table>")

    # diagnostics path cost
    diag = []
    for t in tags:
        e, c = lookup.get((t, "diagnostics_errors", "SemanticChecking")), lookup.get((t, "diagnostics_clean", "SemanticChecking"))
        if e is not None and c is not None:
            diag.append((t, e - c))
    if diag:
        H.append("<h2>Diagnostics path cost (SemanticChecking: errors − clean)</h2>")
        H.append("<table><tr><th>release</th><th class=num>extra ms on error path</th></tr>")
        for t, v in diag:
            H.append(f"<tr><td>{t}</td><td class=num>{v:.0f}</td></tr>")
        H.append("</table>")

    H.append('<div class="note"><b>Reading these numbers:</b> the synthetic workloads are '
             '<i>stress tests built to amplify</i> one compiler pass each — multipliers are sensitivity figures, '
             'not user-facing slowdowns. <code>mdl_dxr</code> is the only real production shader and is the realistic '
             'end-to-end signal. Leaf attribution avoids double-counting nested timers '
             '(generateOutput ⊃ linkAndOptimizeIR ⊃ specializeModule/simplifyIR).</div>')
    H.append('<p class="small">Generated by perf-suite/report.py from results/_analysis. '
             'Source data: results/&lt;tag&gt;/results.json.</p>')
    H.append("</div>")

    out = os.path.join(outdir, "report.html")
    with open(out, "w") as fh:
        fh.write("\n".join(H))
    print(f"wrote {out}  ({len(tags)} releases)")


if __name__ == "__main__":
    main()

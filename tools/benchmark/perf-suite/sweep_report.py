#!/usr/bin/env python3
"""Render a complexity-sweep report for a single slangc build as a self-contained
HTML file (inline SVG scaling curves + a fit table). Stdlib only.

Where report.py shows compile time *across releases* at a fixed size, this shows
compile time *across sizes* for one binary: the simple->complex scaling curve of
each swept workload (one point per sweep_size), with a floor+slope*N fit and a
super-linearity check. Complements ladder_scaling.py (which prints the same fit
as a cross-release table).

    python3 bench.py --slangc <slangc> --label dev --sweep --only <wl,...>
    python3 sweep_report.py --label dev            # -> results/dev/_sweep/sweep_report.html
"""
import argparse
import html
import json
import os

import analyze
import plot  # for esc() + PALETTE

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
.reg{color:#c0392b;font-weight:600} .flat{color:#888}
.chart{background:#fff;border:1px solid #eee;border-radius:6px;padding:8px;margin:10px 0;overflow:auto}
.chart svg{display:block;max-width:100%;height:auto}
code{background:#f0f1f3;padding:1px 5px;border-radius:3px;font-size:13px}
.note{background:#fff8e1;border-left:4px solid #f0c000;padding:10px 14px;border-radius:4px;margin:10px 0}
.small{color:#777;font-size:12px}
"""


def load_sweeps(results_dir, label, metric):
    """{workload: {"timers": {timer: [(size, val), ...]}, "primary": [...]}} for
    every workload in results/<label>/results.json that has >=2 distinct sizes."""
    path = os.path.join(results_dir, label, "results.json")
    if not os.path.exists(path):
        raise SystemExit(f"no results at {path}; run bench.py --label {label} --sweep first")
    runs = json.load(open(path))
    by_wl = {}
    for r in runs:
        if r["size"] <= 0:
            continue
        st = r["timers"]
        wl = by_wl.setdefault(r["workload"], {"timers": {}, "primary": r.get("primary_timers", [])})
        for tname, s in st.items():
            if s is None:
                continue
            wl["timers"].setdefault(tname, []).append((r["size"], s[metric]))
    out = {}
    for wl, d in by_wl.items():
        sizes = {sz for pts in d["timers"].values() for sz, _ in pts}
        if len(sizes) < 2:
            continue
        d["timers"] = {t: sorted(set(pts)) for t, pts in d["timers"].items()}
        out[wl] = d
    return out


def fit(pts):
    """floor, slope, R^2, and top-doubling ratio (>2 = super-linear) for a curve."""
    xs, ys = [x for x, _ in pts], [y for _, y in pts]
    a, b, r2 = analyze._linfit(xs, ys)
    top = (ys[-1] / ys[-2]) if (len(ys) >= 2 and ys[-2]) else None
    return a, b, r2, top


def render_panels(sweeps, metric, out, cols=3):
    """One panel per workload: each primary timer (+ compileInner) as a line vs N,
    linear zero-based y, with the compileInner fit annotated."""
    names = sorted(sweeps)
    n = len(names)
    cols = min(cols, n) or 1
    rows = (n + cols - 1) // cols
    pw, ph = 300, 188
    # right margin holds the per-panel legend AND the (long) fit annotation, so it
    # must fit e.g. "fit: -4051ms + 62.13·N  (R²=0.966)" (~200px) without spilling
    # into the next column or off the canvas.
    ml, mt, mr, mb = 56, 40, 218, 58
    cw, chh = ml + pw + mr, mt + ph + mb
    W = cols * cw + 24       # +pad so the rightmost legend/fit text isn't clipped
    H = rows * chh + 60      # +pad below the last row

    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
         f'viewBox="0 0 {W} {H}" font-family="sans-serif" font-size="11">',
         f'<rect width="{W}" height="{H}" fill="white"/>',
         f'<text x="12" y="30" font-size="17" font-weight="bold">'
         f'Complexity sweep — compile time vs workload size N ({metric} ms, linear)</text>']

    for k, wl in enumerate(names):
        timers = sweeps[wl]["timers"]
        primary = [t for t in sweeps[wl]["primary"] if t in timers]
        # always show compileInner; primaries first, compileInner last (bold)
        lines = [t for t in primary if t != "compileInner"]
        if "compileInner" in timers:
            lines.append("compileInner")
        sizes = sorted({sz for t in lines for sz, _ in timers[t]})
        vmax = max((v for t in lines for _, v in timers[t]), default=1.0) or 1.0
        nmin, nmax = sizes[0], sizes[-1]
        hi = vmax * 1.08

        r, c = divmod(k, cols)
        ox = 8 + c * cw
        oy = 40 + r * chh
        xmap = lambda x: ox + ml + (x - nmin) / (nmax - nmin) * pw if nmax > nmin else ox + ml + pw / 2
        ymap = lambda v: oy + mt + ph - (v / hi) * ph

        s.append(f'<text x="{ox+ml}" y="{oy+18}" font-size="13" font-weight="600">{plot.esc(wl)}</text>')

        # y gridlines (0, mid, max)
        for frac in (0.0, 0.5, 1.0):
            yv = hi * frac
            y = ymap(yv)
            s.append(f'<line x1="{ox+ml:.1f}" y1="{y:.1f}" x2="{ox+ml+pw:.1f}" y2="{y:.1f}" stroke="#eee"/>')
            lbl = f"{yv:.0f}" if yv >= 10 else f"{yv:.1f}"
            s.append(f'<text x="{ox+ml-4:.1f}" y="{y+3:.1f}" text-anchor="end" fill="#999">{lbl}</text>')
        # axes
        s.append(f'<line x1="{ox+ml:.1f}" y1="{oy+mt:.1f}" x2="{ox+ml:.1f}" y2="{oy+mt+ph:.1f}" stroke="#333"/>')
        s.append(f'<line x1="{ox+ml:.1f}" y1="{oy+mt+ph:.1f}" x2="{ox+ml+pw:.1f}" y2="{oy+mt+ph:.1f}" stroke="#333"/>')
        # x ticks: every swept size
        for sz in sizes:
            x = xmap(sz)
            s.append(f'<line x1="{x:.1f}" y1="{oy+mt+ph:.1f}" x2="{x:.1f}" y2="{oy+mt+ph+3:.1f}" stroke="#999"/>')
            s.append(f'<text x="{x:.1f}" y="{oy+mt+ph+15:.1f}" text-anchor="middle" fill="#666">{sz}</text>')
        s.append(f'<text x="{ox+ml+pw/2:.0f}" y="{oy+mt+ph+34:.0f}" text-anchor="middle" fill="#444">N</text>')

        # lines
        for li, tname in enumerate(lines):
            color = plot.PALETTE[li % len(plot.PALETTE)]
            bold = tname == "compileInner"
            pts = [(xmap(x), ymap(v)) for x, v in timers[tname]]
            if len(pts) >= 2:
                path = " ".join(f"{x:.1f},{y:.1f}" for x, y in pts)
                s.append(f'<polyline points="{path}" fill="none" stroke="{color}" '
                         f'stroke-width="{2.6 if bold else 1.6}"/>')
            for x, y in pts:
                s.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{2.4 if bold else 1.8}" fill="{color}"/>')
            ly = oy + mt + 4 + li * 16
            weight = ' font-weight="600"' if bold else ""
            s.append(f'<line x1="{ox+ml+pw+10}" y1="{ly}" x2="{ox+ml+pw+28}" y2="{ly}" '
                     f'stroke="{color}" stroke-width="{3 if bold else 2}"/>')
            s.append(f'<text x="{ox+ml+pw+32}" y="{ly+4}" fill="#222"{weight}>{plot.esc(tname)}</text>')

        # fit annotation (compileInner)
        if "compileInner" in timers:
            a, b, r2, top = fit(timers["compileInner"])
            note = f"fit: {a:.0f}ms + {b:.2f}·N  (R²={r2:.3f})"
            sl = f"top-2×: {top:.2f}×" if top else ""
            ly = oy + mt + 4 + len(lines) * 16 + 8
            s.append(f'<text x="{ox+ml+pw+10}" y="{ly}" fill="#555">{plot.esc(note)}</text>')
            if sl:
                cl = "#c0392b" if (top and top > 2.15) else "#555"
                s.append(f'<text x="{ox+ml+pw+10}" y="{ly+15}" fill="{cl}">{plot.esc(sl)}</text>')

    s.append("</svg>")
    with open(out, "w") as fh:
        fh.write("\n".join(s))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--label", default="dev", help="results/<label>/ to report on")
    ap.add_argument("--metric", default="min", choices=["min", "median", "mean"])
    args = ap.parse_args()

    sweeps = load_sweeps(args.results, args.label, args.metric)
    if not sweeps:
        raise SystemExit(f"no multi-size (swept) workloads in results/{args.label}; "
                         "re-run bench.py with --sweep")

    outdir = os.path.join(args.results, args.label, "_sweep")
    os.makedirs(outdir, exist_ok=True)
    svg = render_panels(sweeps, args.metric, os.path.join(outdir, "sweep_curves.svg"))
    inline = open(svg).read()

    names = sorted(sweeps)
    H = ['<!doctype html><meta charset="utf-8">',
         f"<title>Slang compile-perf — complexity sweep ({html.escape(args.label)})</title>",
         f"<style>{CSS}</style>", '<div class="wrap">',
         "<h1>Slang compile-time performance — complexity sweep</h1>",
         f'<p class="sub">build <b>{html.escape(args.label)}</b> · metric: {args.metric} (ms) · '
         f'{len(names)} swept workload(s) · compile time vs workload size <code>N</code>, '
         "each curve a single binary scaled simple→complex.</p>",
         "<h2>Scaling curves</h2>",
         '<p class="small">Each panel: the workload\'s primary phase timers and '
         '<b>compileInner</b> (bold) on a zero-based linear axis vs <code>N</code>. '
         'Annotated with the floor+slope·N least-squares fit and the top-doubling '
         "ratio (>2× ⇒ super-linear at the high end, shown red).</p>",
         f'<div class="chart">{inline}</div>',
         "<h2>Scaling fit (compileInner)</h2>",
         '<p class="small"><b>floor</b> = fixed per-compile cost · <b>slope</b> = ms per '
         'unit of N · <b>top-2×</b> = t(N<sub>max</sub>)/t(N<sub>max</sub>/2): 2.00 ≈ linear, '
         '&gt;2 super-linear.</p>',
         "<table><tr><th>Workload</th><th class=num>N range</th><th class=num>floor (ms)</th>"
         "<th class=num>slope (ms/N)</th><th class=num>R²</th><th class=num>t(N<sub>min</sub>)</th>"
         "<th class=num>t(N<sub>max</sub>)</th><th class=num>top-2×</th></tr>"]
    for wl in names:
        pts = sweeps[wl]["timers"].get("compileInner")
        if not pts:
            continue
        a, b, r2, top = fit(pts)
        n0, y0, n1, y1 = pts[0][0], pts[0][1], pts[-1][0], pts[-1][1]
        topcls = "reg" if (top and top > 2.15) else "flat"
        tops = f"{top:.2f}×" if top else "—"
        H.append(f"<tr><td>{html.escape(wl)}</td><td class=num>{n0}–{n1}</td>"
                 f"<td class=num>{a:.0f}</td><td class=num>{b:.2f}</td><td class=num>{r2:.3f}</td>"
                 f"<td class=num>{y0:.0f}</td><td class=num>{y1:.0f}</td>"
                 f"<td class='num {topcls}'>{tops}</td></tr>")
    H.append("</table>")
    H.append('<div class="note"><b>Reading these:</b> these synthetic workloads amplify one '
             'compiler pass each — the curve <i>shape</i> (floor vs slope vs super-linearity), not '
             'the absolute ms, is the signal. A clean straight line ⇒ the cost is linear in code '
             'size; an upward bend ⇒ a pass whose cost grows faster than the input.</div>')
    H.append('<p class="small">Generated by perf-suite/sweep_report.py from '
             f'results/{html.escape(args.label)}/results.json. Companion to report.py '
             '(cross-release) and ladder_scaling.py (cross-release fit table).</p>')
    H.append("</div>")

    out = os.path.join(outdir, "sweep_report.html")
    with open(out, "w") as fh:
        fh.write("\n".join(H))
    print(f"wrote {out}  ({len(names)} workloads)")


if __name__ == "__main__":
    main()

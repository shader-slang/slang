#!/usr/bin/env python3
"""Render per-release perf charts as self-contained SVG (stdlib only — no
matplotlib, no JS/CDN). Reads the sweep results + index and emits, into
results/_analysis/:
  - perf_normalized.svg : every workload's compileInner relative to the first
                          release (1.0 baseline) — shows where step-changes land.
  - perf_compileinner.svg : absolute compileInner (log y) for all workloads.
  - perf_<timer>.svg (optional via --timer) : one timer across workloads.
"""
import argparse
import json
import math
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))

PALETTE = ["#e6194b", "#3cb44b", "#4363d8", "#f58231", "#911eb4", "#46f0f0",
           "#f032e6", "#bcf60c", "#fabebe", "#008080", "#9a6324", "#000075"]


def vkey(tag):
    m = re.match(r"v(\d+)\.(\d+)(?:\.(\d+))?", tag)
    return (int(m[1]), int(m[2]), int(m[3] or 0)) if m else (0, 0, 0)


def load(index_path, results_dir, timer):
    with open(index_path) as fh:
        index = json.load(fh)
    order = []
    series = {}  # workload -> [value or None per release]
    recs = []
    for rec in index:
        p = os.path.join(results_dir, rec["tag"], "results.json")
        if "slangc" not in rec or not os.path.exists(p):
            continue
        recs.append(rec)
    # Preserve index.json order (authored chronologically). This lets non-release
    # labels (e.g. pr9808-before/after) slot in at the right position; they don't
    # match the version regex used as the fallback sort key.
    if all(vkey(r["tag"]) != (0, 0, 0) for r in recs):
        recs.sort(key=lambda r: vkey(r["tag"]))
    order = [r["tag"] for r in recs]
    import analyze
    for i, rec in enumerate(recs):
        raw = json.load(open(os.path.join(results_dir, rec["tag"], "results.json")))
        runs = {r["workload"]: r for r in analyze.canonical_runs(raw)}
        for wl, r in runs.items():
            st = r["timers"].get(timer)
            series.setdefault(wl, [None] * len(recs))
            series[wl][i] = st["min"] if st else None
    return order, series


def esc(s):
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def render(order, series, title, ylabel, logy, normalize, highlight, out):
    W, H = 1180, 640
    ml, mr, mt, mb = 70, 230, 56, 96  # margins (right margin holds legend)
    pw, ph = W - ml - mr, H - mt - mb
    n = len(order)

    # transform each series
    data = {}
    for wl, vals in series.items():
        ys = list(vals)
        if normalize:
            base = next((v for v in ys if v), None)
            ys = [(v / base if (v and base) else None) for v in ys]
        data[wl] = ys

    allv = [v for ys in data.values() for v in ys if v]
    if not allv:
        raise SystemExit("no data to plot")
    vmin, vmax = min(allv), max(allv)
    if logy:
        vmin = max(vmin, 1e-3)
        lo, hi = math.log10(vmin) - 0.02, math.log10(vmax) + 0.02
        ymap = lambda v: mt + ph - (math.log10(max(v, 1e-3)) - lo) / (hi - lo) * ph
    else:
        lo = 0 if normalize else 0
        hi = vmax * 1.05
        ymap = lambda v: mt + ph - (v - lo) / (hi - lo) * ph
    xmap = lambda i: ml + (i * pw / (n - 1) if n > 1 else pw / 2)

    s = []
    s.append(f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" font-family="sans-serif" font-size="12">')
    s.append(f'<rect width="{W}" height="{H}" fill="white"/>')
    s.append(f'<text x="{ml}" y="28" font-size="17" font-weight="bold">{esc(title)}</text>')

    # y gridlines + labels
    if logy:
        ticks = []
        e = math.floor(lo)
        while e <= math.ceil(hi):
            for m_ in (1, 2, 5):
                val = m_ * 10 ** e
                if vmin <= val <= vmax * 1.05:
                    ticks.append(val)
            e += 1
    else:
        steps = 8
        ticks = [hi * k / steps for k in range(steps + 1)]
    for tv in ticks:
        y = ymap(tv)
        s.append(f'<line x1="{ml}" y1="{y:.1f}" x2="{ml+pw}" y2="{y:.1f}" stroke="#eee"/>')
        lbl = f"{tv:.0f}" if tv >= 10 else f"{tv:.1f}"
        if normalize:
            lbl = f"{tv:.1f}x"
        s.append(f'<text x="{ml-8}" y="{y+4:.1f}" text-anchor="end" fill="#666">{lbl}</text>')
    if normalize:  # baseline at 1.0
        y1 = ymap(1.0)
        s.append(f'<line x1="{ml}" y1="{y1:.1f}" x2="{ml+pw}" y2="{y1:.1f}" stroke="#999" stroke-dasharray="4 3"/>')

    # x labels (release tags)
    for i, tag in enumerate(order):
        x = xmap(i)
        s.append(f'<line x1="{x:.1f}" y1="{mt+ph}" x2="{x:.1f}" y2="{mt+ph+4}" stroke="#999"/>')
        t = tag.replace("v20", "")
        s.append(f'<text x="{x:.1f}" y="{mt+ph+18:.1f}" text-anchor="end" fill="#444" '
                 f'transform="rotate(-55 {x:.1f} {mt+ph+18:.1f})">{esc(t)}</text>')
    s.append(f'<text x="{ml+pw/2:.0f}" y="{H-8}" text-anchor="middle" fill="#444">release</text>')
    s.append(f'<text x="18" y="{mt+ph/2:.0f}" text-anchor="middle" fill="#444" '
             f'transform="rotate(-90 18 {mt+ph/2:.0f})">{esc(ylabel)}</text>')
    # axes
    s.append(f'<line x1="{ml}" y1="{mt}" x2="{ml}" y2="{mt+ph}" stroke="#333"/>')
    s.append(f'<line x1="{ml}" y1="{mt+ph}" x2="{ml+pw}" y2="{mt+ph}" stroke="#333"/>')

    # series, sorted by final value desc for legend ordering
    names = sorted(data, key=lambda w: -(next((v for v in reversed(data[w]) if v), 0)))
    for idx, wl in enumerate(names):
        color = PALETTE[idx % len(PALETTE)]
        thick = 3.4 if wl == highlight else 1.8
        pts = [(xmap(i), ymap(v)) for i, v in enumerate(data[wl]) if v is not None]
        if len(pts) >= 2:
            path = " ".join(f"{x:.1f},{y:.1f}" for x, y in pts)
            s.append(f'<polyline points="{path}" fill="none" stroke="{color}" stroke-width="{thick}"/>')
        for x, y in pts:
            s.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{2.6 if wl==highlight else 2.0}" fill="{color}"/>')
        ly = mt + 6 + idx * 18
        last = next((v for v in reversed(data[wl]) if v), None)
        suffix = f"  ({last:.1f}x)" if (normalize and last) else ""
        label = wl + (" ★" if wl == highlight else "") + suffix
        s.append(f'<line x1="{ml+pw+14}" y1="{ly}" x2="{ml+pw+34}" y2="{ly}" stroke="{color}" stroke-width="3"/>')
        s.append(f'<text x="{ml+pw+38}" y="{ly+4}" fill="#222">{esc(label)}</text>')

    s.append("</svg>")
    with open(out, "w") as fh:
        fh.write("\n".join(s))
    return out


def render_small_multiples(order, series, title, out, cols=4):
    """One linear, zero-based panel per workload (small multiples). Unlike the
    two overview charts — normalized (hides absolute magnitude) and log-absolute
    (distorts curve shape) — this shows each benchmark's true absolute curve on
    its own linear y-axis, so the per-benchmark shape and scale are both honest."""
    names = sorted(series, key=lambda w: -(next((v for v in reversed(series[w]) if v), 0)))
    n = len(names)
    rows = (n + cols - 1) // cols
    pw, ph = 252, 150          # panel plot area
    ml, mt = 46, 26            # per-panel left/top margin (axis labels / title)
    mr, mb = 12, 34            # per-panel right/bottom margin (x labels)
    cw, chh = ml + pw + mr, mt + ph + mb
    W = cols * cw + 16
    H = rows * chh + 56
    nrel = len(order)

    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
         f'font-family="sans-serif" font-size="11">',
         f'<rect width="{W}" height="{H}" fill="white"/>',
         f'<text x="12" y="26" font-size="17" font-weight="bold">{esc(title)}</text>']

    # sparse x ticks: first, middle, last release
    tick_idx = sorted(set([0, nrel // 2, nrel - 1]))

    for k, wl in enumerate(names):
        r, c = divmod(k, cols)
        ox = 8 + c * cw
        oy = 44 + r * chh
        vals = series[wl]
        present = [(i, v) for i, v in enumerate(vals) if v is not None]
        vmax = max((v for _, v in present), default=1.0) or 1.0
        hi = vmax * 1.08
        xmap = lambda i: ox + ml + (i * pw / (nrel - 1) if nrel > 1 else pw / 2)
        ymap = lambda v: oy + mt + ph - (v / hi) * ph

        last = next((v for v in reversed(vals) if v), None)
        first = next((v for v in vals if v), None)
        ratio = (last / first) if (first and last) else None
        rtxt = f"  {ratio:.2f}×" if ratio else ""
        s.append(f'<text x="{ox+ml}" y="{oy+14}" font-size="12" font-weight="600">'
                 f'{esc(wl)}<tspan fill="#888" font-weight="400">{esc(rtxt)}</tspan></text>')

        # panel frame + y gridlines (0, mid, max) with absolute-ms labels
        for frac in (0.0, 0.5, 1.0):
            yv = hi * frac
            y = ymap(yv)
            s.append(f'<line x1="{ox+ml:.1f}" y1="{y:.1f}" x2="{ox+ml+pw:.1f}" y2="{y:.1f}" stroke="#eee"/>')
            lbl = f"{yv:.0f}" if yv >= 10 else f"{yv:.1f}"
            s.append(f'<text x="{ox+ml-4:.1f}" y="{y+3:.1f}" text-anchor="end" fill="#999">{lbl}</text>')
        s.append(f'<line x1="{ox+ml:.1f}" y1="{oy+mt:.1f}" x2="{ox+ml:.1f}" y2="{oy+mt+ph:.1f}" stroke="#333"/>')
        s.append(f'<line x1="{ox+ml:.1f}" y1="{oy+mt+ph:.1f}" x2="{ox+ml+pw:.1f}" y2="{oy+mt+ph:.1f}" stroke="#333"/>')

        # x ticks (sparse, release tags)
        for i in tick_idx:
            x = xmap(i)
            t = order[i].replace("v20", "")
            s.append(f'<line x1="{x:.1f}" y1="{oy+mt+ph:.1f}" x2="{x:.1f}" y2="{oy+mt+ph+3:.1f}" stroke="#999"/>')
            s.append(f'<text x="{x:.1f}" y="{oy+mt+ph+15:.1f}" text-anchor="middle" fill="#666">{esc(t)}</text>')

        # the curve
        pts = [(xmap(i), ymap(v)) for i, v in present]
        if len(pts) >= 2:
            path = " ".join(f"{x:.1f},{y:.1f}" for x, y in pts)
            s.append(f'<polyline points="{path}" fill="none" stroke="#3060d0" stroke-width="1.8"/>')
        for x, y in pts:
            s.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="2.0" fill="#3060d0"/>')

    s.append("</svg>")
    with open(out, "w") as fh:
        fh.write("\n".join(s))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--index", default=os.path.join(HERE, "releases", "index.json"))
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--timer", default="compileInner")
    ap.add_argument("--highlight", default="mdl_dxr", help="workload to emphasize")
    args = ap.parse_args()

    outdir = os.path.join(args.results, "_analysis")
    os.makedirs(outdir, exist_ok=True)
    order, series = load(args.index, args.results, args.timer)

    p1 = render(order, series,
                f"Slang compile time per release — {args.timer}, normalized to first release",
                "x baseline", logy=False, normalize=True, highlight=args.highlight,
                out=os.path.join(outdir, "perf_normalized.svg"))
    p2 = render(order, series,
                f"Slang compile time per release — {args.timer} (absolute, log scale)",
                "ms (log)", logy=True, normalize=False, highlight=args.highlight,
                out=os.path.join(outdir, "perf_compileinner.svg"))
    p3 = render_small_multiples(
        order, series,
        f"Per-benchmark {args.timer} — absolute, linear (own y-axis each)",
        out=os.path.join(outdir, "perf_per_benchmark.svg"))
    print(f"wrote {p1}\nwrote {p2}\nwrote {p3}")
    print(f"({len(order)} releases, {len(series)} workloads)")


if __name__ == "__main__":
    main()

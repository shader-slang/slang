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
import breakdown  # stacked phase-composition vs N
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
    path = analyze.results_path(results_dir, label)
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


def fit(pts, floor=0.0):
    """Power-law fit of the *feature work* (compile time minus the fixed
    per-compile floor): (t − floor) = a·N^k. Returns {a, k, pow_r2, top}.

    The floor must be subtracted first or k is meaningless: at small N the curve
    is floor-dominated, which flattens the raw log-log slope and understates k.
    The honest floor is the measured `minimal` workload (passed in), not a fitted
    intercept. `k` is the global exponent (k≈1 linear, >1 super-linear); `top` is
    the high-end local doubling ratio — when they disagree, the exponent is
    changing across the range (e.g. linear at small N, bending up at large N)."""
    xs, ys = [x for x, _ in pts], [y for _, y in pts]
    work = [y - floor for y in ys]
    a, k, pow_r2 = analyze._powfit(xs, work)
    top = (ys[-1] / ys[-2]) if (len(ys) >= 2 and ys[-2]) else None
    return {"a": a, "k": k, "pow_r2": pow_r2, "top": top}


def superlinear(k):
    """A power-law exponent meaningfully above 1."""
    return k is not None and k > 1.15


def render_panels(sweeps, metric, out, floor=0.0, cols=3, link_for=None):
    """One panel per workload: just the compileInner scaling curve vs N (zero-based
    linear y). The panel title links to the workload's own page (full sub-counter
    stacked view + analysis + numbers); a compact ∝N^k flag stays for at-a-glance
    super-linearity. The per-timer lines and fit text block live on those pages."""
    names = sorted(sweeps)
    n = len(names)
    cols = min(cols, n) or 1
    rows = (n + cols - 1) // cols
    pw, ph = 300, 188
    ml, mt, mr, mb = 56, 40, 24, 58
    cw, chh = ml + pw + mr, mt + ph + mb
    W = cols * cw + 24
    H = rows * chh + 60

    s = [f'<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" '
         f'width="{W}" height="{H}" viewBox="0 0 {W} {H}" preserveAspectRatio="xMidYMid meet" '
         f'font-family="sans-serif" font-size="11">',
         f'<rect width="{W}" height="{H}" fill="white"/>',
         f'<text x="12" y="30" font-size="17" font-weight="bold">'
         f'Complexity sweep — compileInner vs workload size N ({metric} ms, linear)</text>']

    for k, wl in enumerate(names):
        timers = sweeps[wl]["timers"]
        if "compileInner" not in timers:
            continue
        pts_ci = timers["compileInner"]
        sizes = [sz for sz, _ in pts_ci]
        vmax = max((v for _, v in pts_ci), default=1.0) or 1.0
        nmin, nmax = sizes[0], sizes[-1]
        hi = vmax * 1.08

        r, c = divmod(k, cols)
        ox = 8 + c * cw
        oy = 40 + r * chh
        xmap = lambda x: ox + ml + (x - nmin) / (nmax - nmin) * pw if nmax > nmin else ox + ml + pw / 2
        ymap = lambda v: oy + mt + ph - (v / hi) * ph

        ft = fit(pts_ci, floor)
        kk = ft["k"]
        kcl = "#c0392b" if superlinear(kk) else "#888"
        link = (link_for or {}).get(wl)
        nfill = "#1a5fb4" if link else "#1a1a1a"
        deco = ' text-decoration="underline"' if link else ""
        ttl = (f'<text x="{ox+ml}" y="{oy+18}" font-size="13" font-weight="600" fill="{nfill}"{deco}>'
               f'{plot.esc(wl)}</text>')
        if link:
            ttl = f'<a xlink:href="{plot.esc(link)}" href="{plot.esc(link)}" target="_top">{ttl}</a>'
        s.append(ttl)
        s.append(f'<text x="{ox+ml+pw}" y="{oy+18}" text-anchor="end" font-size="11" '
                 f'fill="{kcl}" font-weight="600">{plot.esc(f"∝N^{kk:.2f}")}</text>')

        for frac in (0.0, 0.5, 1.0):
            yv = hi * frac
            y = ymap(yv)
            s.append(f'<line x1="{ox+ml:.1f}" y1="{y:.1f}" x2="{ox+ml+pw:.1f}" y2="{y:.1f}" stroke="#eee"/>')
            lbl = f"{yv:.0f}" if yv >= 10 else f"{yv:.1f}"
            s.append(f'<text x="{ox+ml-4:.1f}" y="{y+3:.1f}" text-anchor="end" fill="#999">{lbl}</text>')
        s.append(f'<line x1="{ox+ml:.1f}" y1="{oy+mt:.1f}" x2="{ox+ml:.1f}" y2="{oy+mt+ph:.1f}" stroke="#333"/>')
        s.append(f'<line x1="{ox+ml:.1f}" y1="{oy+mt+ph:.1f}" x2="{ox+ml+pw:.1f}" y2="{oy+mt+ph:.1f}" stroke="#333"/>')
        for sz in sizes:
            x = xmap(sz)
            s.append(f'<line x1="{x:.1f}" y1="{oy+mt+ph:.1f}" x2="{x:.1f}" y2="{oy+mt+ph+3:.1f}" stroke="#999"/>')
            s.append(f'<text x="{x:.1f}" y="{oy+mt+ph+15:.1f}" text-anchor="middle" fill="#666">{sz}</text>')
        s.append(f'<text x="{ox+ml+pw/2:.0f}" y="{oy+mt+ph+34:.0f}" text-anchor="middle" fill="#444">N</text>')

        pts = [(xmap(x), ymap(v)) for x, v in pts_ci]
        if len(pts) >= 2:
            path = " ".join(f"{x:.1f},{y:.1f}" for x, y in pts)
            s.append(f'<polyline points="{path}" fill="none" stroke="#2171b5" stroke-width="2.6"/>')
        for x, y in pts:
            s.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="2.4" fill="#2171b5"/>')

    s.append("</svg>")
    with open(out, "w") as fh:
        fh.write("\n".join(s))
    return out


def write_sweep_pages(results_dir, label, metric, sweeps, floor, outdir):
    """One detail page per swept workload: the stacked sub-counter view vs N, the
    scaling analysis (floor / k / top-2×), and the raw per-size sweep numbers.
    Returns {workload: href relative to the sweep report} for linking."""
    import inspect
    import manifest
    wdir = os.path.join(outdir, "workloads")
    os.makedirs(wdir, exist_ok=True)
    pre = ("background:#fff;border:1px solid #eee;border-radius:6px;padding:8px;overflow:auto")
    links = {}
    for wl in sorted(sweeps):
        spec = manifest.BY_NAME.get(wl)
        timers = sweeps[wl]["timers"]
        ci = timers.get("compileInner")
        if not ci:
            continue
        svgp = os.path.join(wdir, f"{wl}.svg")
        breakdown.render_stacked_sweep(
            results_dir, label, metric, svgp, cols=1, names=[wl], panel=(1000, 420),
            title=f"{wl} — phase composition vs N ({label}, {metric} ms)")
        svg = open(svgp).read()

        ft = fit(ci, floor)
        kk, top = ft["k"], ft["top"]
        kcls = "reg" if superlinear(kk) else "flat"
        topcls = "reg" if (top and top > 2.15) else "flat"
        desc = (inspect.getdoc(spec.gen) if spec and spec.gen else "") or "(no description)"
        flags = " ".join(spec.extra_flags) if spec and spec.extra_flags else "(none)"
        meta = (f"<b>bucket:</b> {plot.esc(spec.bucket)} &nbsp;·&nbsp; <b>mode:</b> "
                f"{plot.esc(spec.mode)} &nbsp;·&nbsp; <b>flags:</b> <code>{plot.esc(flags)}</code>"
                if spec else "")

        # sweep numbers: one row per size; compileInner + the workload's primary timers
        cols_t = ["compileInner"] + [t for t in sweeps[wl]["primary"]
                                     if t != "compileInner" and t in timers]
        sizes = [sz for sz, _ in ci]
        bytimer = {t: dict(timers[t]) for t in cols_t}
        num = ["<table><tr><th class=num>N</th>"
               + "".join(f"<th class=num>{html.escape(t)}</th>" for t in cols_t) + "</tr>"]
        for sz in sizes:
            cells = "".join(f"<td class=num>{bytimer[t].get(sz, float('nan')):.0f}</td>"
                            if bytimer[t].get(sz) is not None else "<td class=num>–</td>"
                            for t in cols_t)
            num.append(f"<tr><td class=num>{sz}</td>{cells}</tr>")
        num.append("</table>")

        body = (f'<!doctype html><meta charset="utf-8">'
                f"<title>{html.escape(wl)} — sweep ({html.escape(label)})</title>"
                f"<style>{CSS}</style><div class='wrap'>"
                f"<p><a href='../sweep_report.html'>&larr; all workloads</a></p>"
                f"<h1>{html.escape(wl)}</h1>"
                f"<p style='color:#444;white-space:pre-wrap'>{html.escape(desc)}</p>"
                f"<p class='small'>{meta}</p>"
                f"<h2>Phase composition vs N (stacked sub-counters)</h2>"
                f"<p class='small'>compileInner split into phase buckets (named leaves + "
                f"<code>(self)</code> residuals) stacked across the sweep sizes — the top edge "
                f"is compileInner, so you can see <i>which</i> phase drives the scaling.</p>"
                f"<div class='chart'>{svg}</div>"
                f"<h2>Scaling analysis</h2>"
                f"<p class='small'>floor-subtracted power-law fit "
                f"<code>(t − floor) = a·N<sup>k</sup></code>; <b>floor</b> = the "
                f"<code>minimal</code> workload (fixed per-compile cost), <b>k</b> the global "
                f"exponent, <b>top-2×</b> the local high-end doubling ratio.</p>"
                f"<table><tr><th class=num>N range</th><th class=num>floor (ms)</th>"
                f"<th class=num>k (work)</th><th class=num>fit R²</th>"
                f"<th class=num>t(N<sub>min</sub>)</th><th class=num>t(N<sub>max</sub>)</th>"
                f"<th class=num>top-2×</th></tr>"
                f"<tr><td class=num>{sizes[0]}–{sizes[-1]}</td><td class=num>{floor:.0f}</td>"
                f"<td class='num {kcls}'>{kk:.2f}</td><td class=num>{ft['pow_r2']:.3f}</td>"
                f"<td class=num>{ci[0][1]:.0f}</td><td class=num>{ci[-1][1]:.0f}</td>"
                f"<td class='num {topcls}'>{(f'{top:.2f}×' if top else '–')}</td></tr></table>"
                f"<h2>Sweep numbers ({metric} ms)</h2>" + "".join(num)
                + "</div>")
        with open(os.path.join(wdir, f"{wl}.html"), "w") as fh:
            fh.write(body)
        links[wl] = f"workloads/{wl}.html"
    return links


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--label", default="dev", help="results/<label>/ to report on")
    ap.add_argument("--metric", default="median", choices=["min", "median", "mean"])
    args = ap.parse_args()

    sweeps = load_sweeps(args.results, args.label, args.metric)
    if not sweeps:
        raise SystemExit(f"no multi-size (swept) workloads in results/{args.label}; "
                         "re-run bench.py with --sweep")

    # Fixed per-compile floor = the `minimal` workload's compileInner on this
    # build (the suite's floor canary). Subtracted before the power-law fit so the
    # exponent reflects feature work, not the floor. 0 if minimal wasn't run.
    floor = 0.0
    raw = json.load(open(analyze.results_path(args.results, args.label)))
    for r in raw:
        if r["workload"] == "minimal" and r["timers"].get("compileInner"):
            floor = r["timers"]["compileInner"][args.metric]
            break

    outdir = os.path.join(analyze.results_dir_for(args.results, args.label), "_sweep")
    os.makedirs(outdir, exist_ok=True)
    # Per-workload pages (stacked sub-counters + analysis + numbers), then the
    # index's compileInner-only curves linking into them.
    links = write_sweep_pages(args.results, args.label, args.metric, sweeps, floor, outdir)
    svg = render_panels(sweeps, args.metric, os.path.join(outdir, "sweep_curves.svg"),
                        floor, link_for=links)
    inline = open(svg).read()

    names = sorted(sweeps)
    H = ['<!doctype html><meta charset="utf-8">',
         f"<title>Slang compile-perf — complexity sweep ({html.escape(args.label)})</title>",
         f"<style>{CSS}</style>", '<div class="wrap">',
         "<h1>Slang compile-time performance — complexity sweep</h1>",
         f'<p class="sub">build <b>{html.escape(args.label)}</b> · metric: {args.metric} (ms) · '
         f'{len(names)} swept workload(s) · compile time vs workload size <code>N</code>, '
         "each curve a single binary scaled simple→complex.</p>",
         "<h2>Scaling curves (compileInner)</h2>",
         '<p class="small">Each panel is one workload\'s <b>compileInner</b> vs '
         '<code>N</code> on a zero-based linear axis, flagged with the floor-subtracted '
         'exponent <b>∝N<sup>k</sup></b> (red ⇒ super-linear, k&gt;1.15). '
         f'<b>Click a workload name</b> for its sub-counter stacked view, the full scaling '
         f'analysis (floor {floor:.0f} ms / k / top-2×), and the raw sweep numbers.</p>',
         f'<div class="chart">{inline}</div>']
    H.append('<div class="note"><b>Reading these:</b> these synthetic workloads amplify one '
             'compiler pass each — the curve <i>shape</i> (the exponent <b>k</b> of the '
             'floor-subtracted fit <code>(t − floor) = a·N<sup>k</sup></code>), not the '
             'absolute ms, is the signal. <b>k≈1</b> ⇒ cost is linear in code size; '
             '<b>k&gt;1</b> ⇒ a pass whose cost grows faster than its input, where regressions '
             'on large real shaders hide. The fixed <b>floor</b> is the <code>minimal</code> '
             'workload (subtracted before the fit, not refitted per workload), also the suite\'s '
             'standalone canary for "the stdlib got heavier". Per-workload pages carry the full '
             'analysis and numbers.</div>')
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

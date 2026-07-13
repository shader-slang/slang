#!/usr/bin/env python3
"""Render a complexity-sweep report for a single slangc build as a self-contained
HTML file (inline SVG scaling curves + a fit table). Stdlib only.

Where report.py shows compile time *across releases* at a fixed size, this shows
compile time *across sizes* for one binary: the simple->complex scaling curve of
each swept workload (one point per sweep_size), with a floor-subtracted
power-law fit ((t - floor) = a*N^k) and a super-linearity check. Complements
ladder_scaling.py (the cross-release linear floor+slope table over the same
sweep data).

    python3 bench.py --slangc <slangc> --label dev --sweep --only <wl,...>
    python3 sweep_report.py --label dev            # -> results/dev/sweep/sweep_report.html
"""
import argparse
import html
import json
import os

import breakdown  # stacked phase-composition vs N
from lib import analyze, manifest


def canonical_order(names):
    """Order workload names the same way the default cross-release report
    orders its panels: the manifest's WORKLOADS list position, unknown names
    last (alphabetically), so the sweep report and report.py stay aligned."""
    pos = {w.name: i for i, w in enumerate(manifest.WORKLOADS)}
    return sorted(names, key=lambda n: (pos.get(n, len(pos)), n))

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
    a, k, pow_r2 = analyze.powfit(xs, work)
    top = (ys[-1] / ys[-2]) if (len(ys) >= 2 and ys[-2]) else None
    return {"a": a, "k": k, "pow_r2": pow_r2, "top": top}


# The 0.15 noise buffer shared by both super-linearity thresholds: a fit over
# 3-5 points is noisy, and readings within 15% of the linear expectation are
# not actionable. K_NOISE_BUFFER couples the exponent cutoff (linear k=1.0)
# and the top-step cutoff (linear = 2.0 on the suite's x2 doubling ladders) so
# they move together if the ladders' spacing or length ever changes.
K_NOISE_BUFFER = 0.15
SUPERLINEAR_K = 1.0 + K_NOISE_BUFFER
SUPERLINEAR_TOP2X = 2.0 + K_NOISE_BUFFER


def superlinear(k):
    """True when the power-law exponent k is meaningfully above linear (k=1.0),
    i.e. past the shared noise buffer documented on K_NOISE_BUFFER."""
    return k is not None and k > SUPERLINEAR_K


def bucket_series(results_dir, label, metric):
    """Per-workload phase-bucket totals at each sweep size plus the per-bucket
    floor: ({workload: {size: {bucket: ms}}}, {bucket: floor_ms}), using
    breakdown's mutually-exclusive buckets (named leaf timers + `(self)`
    residuals). Because the buckets partition compileInner exactly, growth
    deltas computed from them attribute the compileInner increase without
    double-counting nested timers — which raw timer names (e.g.
    SemanticChecking containing its own children) would.

    The floor is the `minimal` workload's bucket decomposition — the same
    fixed-per-compile canary the top-level panels anchor their linear
    expectation to, split by bucket ({} if minimal wasn't run)."""
    runs = json.load(open(analyze.results_path(results_dir, label)))
    per, floor_b = {}, {}
    for r in runs:
        if not r.get("timers"):
            continue
        timers = {k: v[metric] for k, v in r["timers"].items() if v}
        if r["workload"] == "minimal" and not floor_b:
            floor_b = breakdown.buckets(timers)
        if r.get("size", 0) <= 0:
            continue
        per.setdefault(r["workload"], {})[r["size"]] = breakdown.buckets(timers)
    return per, floor_b


def growth_attribution(by_size, floor_b=None, top_n=5):
    """Attribute the compileInner growth between the smallest and largest sweep
    size to individual phase buckets, answering "which timer grows, which stays
    constant". Returns (rows, others, flat, total_delta, (n_min, n_max)):

      rows   — the top growing buckets (Δ ≥ max(1 ms, 2% of total growth), at
               most `top_n`), each {name, v0, v1, delta, k, lin}. `k` is the
               bucket's own power-law exponent on its floor-subtracted work,
               (t_bucket − floor_bucket) = a·N^k — the same subtraction fit()
               does for compileInner, since a fixed cost inside the bucket
               flattens a raw log-log slope (None when the work is ~0
               somewhere, where a log-log fit is undefined).
               `lin` is the same end-point-vs-linear-expectation ratio the
               top-level panels show, per bucket: the expectation is anchored
               to the bucket's share of the `minimal` floor (`floor_b`, 0 for
               buckets absent there) with the slope fitted through-origin on
               the low-N half of (t − floor); lin = t(N_max) / expectation, so
               1.0 reads "grew exactly linearly" regardless of the ladder's N
               ratio (None with <3 points or a non-positive expectation).
      others — buckets above that same Δ threshold but cut by the top-`top_n`
               limit: still real growth, just not the headline. Kept separate
               so they are never mislabeled as constant.
      flat   — the genuinely minor remainder (each ≤ the threshold, i.e. ≤2%
               of total growth), sorted by size at N_max so the sentence reads
               biggest-first.
      total_delta — compileInner(N_max) − compileInner(N_min); since buckets
              partition compileInner, the per-bucket shares sum to 100%."""
    sizes = sorted(by_size)
    if len(sizes) < 2:
        return [], [], [], 0.0, (0, 0)
    n0, n1 = sizes[0], sizes[-1]
    names = sorted({b for bm in by_size.values() for b in bm})
    total = sum(by_size[n1].values()) - sum(by_size[n0].values())
    entries = []
    for name in names:
        pts = [(sz, by_size[sz].get(name, 0.0)) for sz in sizes]
        v0, v1 = pts[0][1], pts[-1][1]
        fb = (floor_b or {}).get(name, 0.0)
        work = [(x, v - fb) for x, v in pts]
        k = None
        if all(w > 0 for _, w in work):
            _, k, _ = analyze.powfit([x for x, _ in work], [w for _, w in work])
        lin = None
        if len(pts) >= 3:
            lo = pts[:max(2, (len(pts) + 1) // 2)]
            slope = (sum(x * (v - fb) for x, v in lo)
                     / (sum(x * x for x, _ in lo) or 1.0))
            expect = fb + slope * pts[-1][0]
            if expect > 0:
                lin = v1 / expect
        entries.append({"name": name, "v0": v0, "v1": v1, "delta": v1 - v0,
                        "k": k, "lin": lin})
    entries.sort(key=lambda e: -e["delta"])
    thresh = max(1.0, 0.02 * total) if total > 0 else 1.0
    growing = [e for e in entries if e["delta"] >= thresh]
    rows, others = growing[:top_n], growing[top_n:]
    picked = {e["name"] for e in growing}
    flat = sorted((e for e in entries if e["name"] not in picked),
                  key=lambda e: -e["v1"])
    return rows, others, flat, total, (n0, n1)


def render_growth_table(by_size, floor_b=None):
    """HTML for the growth-attribution section of a workload page: a top-5
    'growing buckets' table (Δms, share of total growth, the × lin end-point
    ratio, and the bucket's own ∝N^k exponent), an 'also growing' line for
    growers cut by the top-5 limit, and a one-line near-constant remainder, so
    a reader can see at a glance where a super-linear total comes from."""
    rows, others, flat, total, (n0, n1) = growth_attribution(by_size, floor_b)
    if not rows and not others and not flat:
        return ""
    s = [f"<h2>Growth attribution (N={n0} &rarr; N={n1})</h2>",
         f"<p class='small'>compileInner grows by <b>{total:.0f} ms</b> across the sweep; "
         f"the mutually-exclusive phase buckets below partition that growth exactly "
         f"(no nested-timer double counting). <b>&times; lin</b> is the same metric as "
         f"the top-level panels, per bucket: the end point vs a linear expectation "
         f"anchored to the bucket's share of the <code>minimal</code> floor and fitted "
         f"on the low-N half — 1.0 = grew exactly linearly, &gt;1 bends up. The "
         f"super-linearity lives where &times; lin (and k) are red.</p>",
         "<table><tr><th>bucket</th>"
         f"<th class=num>t@N={n0}</th><th class=num>t@N={n1}</th>"
         "<th class=num>&Delta; ms</th><th class=num>share</th>"
         "<th class=num>&times; lin</th><th class=num>&prop;N<sup>k</sup></th></tr>"]
    for e in rows:
        share = f"{100 * e['delta'] / total:.0f}%" if total > 0 else "–"
        if e["lin"] is not None:
            lcls = "reg" if e["lin"] >= SUPERLINEAR_K else "flat"
            lcell = f"<td class='num {lcls}'>{e['lin']:.2f}&times;</td>"
        else:
            lcell = "<td class=num>–</td>"
        if e["k"] is not None:
            kcls = "reg" if superlinear(e["k"]) else "flat"
            kcell = f"<td class='num {kcls}'>{e['k']:.2f}</td>"
        else:
            kcell = "<td class=num>–</td>"
        s.append(f"<tr><td>{html.escape(e['name'])}</td>"
                 f"<td class=num>{e['v0']:.0f}</td><td class=num>{e['v1']:.0f}</td>"
                 f"<td class=num>{e['delta']:+.0f}</td><td class=num>{share}</td>"
                 f"{lcell}{kcell}</tr>")
    s.append("</table>")
    if others:
        parts = ", ".join(
            f"{html.escape(e['name'])} (+{e['delta']:.0f} ms, "
            f"{100 * e['delta'] / total:.0f}%)" if total > 0 else
            f"{html.escape(e['name'])} (+{e['delta']:.0f} ms)"
            for e in others)
        s.append(f"<p class='small'><b>Also growing</b> (below top-5): {parts}.</p>")
    if flat:
        parts = ", ".join(f"{html.escape(e['name'])} ({e['v0']:.0f}&rarr;{e['v1']:.0f} ms)"
                          for e in flat)
        s.append(f"<p class='small'><b>Near-constant</b> (&le;2% of growth each): "
                 f"{parts}.</p>")
    return "".join(s)


def render_panels(sweeps, metric, out, floor=0.0, cols=3, link_for=None):
    """One panel per workload: just the compileInner scaling curve vs N (zero-based
    linear y). The panel title links to the workload's own page (full sub-counter
    stacked view + analysis + numbers); a compact ∝N^k flag stays for at-a-glance
    super-linearity. The per-timer lines and fit text block live on those pages."""
    names = canonical_order(sweeps)
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
         f'Complexity sweep — compileInner vs workload size N ({metric} ms, linear)</text>',
         f'<text x="12" y="46" font-size="11" fill="#666">dashed = linear expectation: '
         f'measured floor (the minimal workload) + slope fitted on the low-N half, extrapolated; '
         f'the end-point label is actual/linear at max N (red when ≥{SUPERLINEAR_K}× — super-linear)</text>']

    for k, wl in enumerate(names):
        timers = sweeps[wl]["timers"]
        if "compileInner" not in timers:
            continue
        pts_ci = timers["compileInner"]
        sizes = [sz for sz, _ in pts_ci]
        vmax = max((v for _, v in pts_ci), default=1.0) or 1.0
        nmin, nmax = sizes[0], sizes[-1]

        # Linear expectation, extrapolated from the low-N half. When the
        # measured per-compile floor (the `minimal` workload) is available the
        # intercept is ANCHORED to it and only the slope is fitted
        # (through-origin OLS on t - floor): at small N the floor is 20-35% of
        # the reading, so a free intercept both dilutes the slope estimate and
        # tilts the line when the low half already curves — understating the
        # end-point ratio. Without a measured floor, fall back to free OLS.
        lin_a = lin_b = None
        lin_ratio = None
        if len(pts_ci) >= 3:
            h = max(2, (len(pts_ci) + 1) // 2)
            lo = pts_ci[:h]
            if floor > 0:
                num = sum(x * (v - floor) for x, v in lo)
                den = sum(x * x for x, _ in lo) or 1.0
                lin_a, lin_b = floor, num / den
            else:
                lin_a, lin_b, _ = analyze.linfit([x for x, _ in lo], [v for _, v in lo])
            lin_end = lin_a + lin_b * nmax
            if lin_end > 0:
                lin_ratio = pts_ci[-1][1] / lin_end
        hi = max(vmax, (lin_a + lin_b * nmax) if lin_a is not None else 0.0) * 1.08 or 1.0

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
               f'{html.escape(wl)}</text>')
        if link:
            ttl = f'<a xlink:href="{html.escape(link)}" href="{html.escape(link)}" target="_top">{ttl}</a>'
        s.append(ttl)
        s.append(f'<text x="{ox+ml+pw}" y="{oy+18}" text-anchor="end" font-size="11" '
                 f'fill="{kcl}" font-weight="600">{html.escape(f"∝N^{kk:.2f}")}</text>')

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

        if lin_a is not None:
            y0 = ymap(max(lin_a + lin_b * nmin, 0.0))
            y1 = ymap(max(lin_a + lin_b * nmax, 0.0))
            s.append(f'<line x1="{xmap(nmin):.1f}" y1="{y0:.1f}" x2="{xmap(nmax):.1f}" '
                     f'y2="{y1:.1f}" stroke="#999" stroke-width="1.4" stroke-dasharray="5,4"/>')
        if lin_ratio is not None:
            rcl = "#c0392b" if lin_ratio >= SUPERLINEAR_K else "#888"
            s.append(f'<text x="{xmap(nmax)-4:.1f}" y="{ymap(pts_ci[-1][1])-6:.1f}" '
                     f'text-anchor="end" font-size="11" fill="{rcl}" font-weight="600">'
                     f'{lin_ratio:.2f}× lin</text>')

        pts = [(xmap(x), ymap(v)) for x, v in pts_ci]
        if len(pts) >= 2:
            path = " ".join(f"{x:.1f},{y:.1f}" for x, y in pts)
            s.append(f'<polyline points="{path}" fill="none" stroke="#2171b5" stroke-width="2.6"/>')
        for x, y in pts:
            s.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="2.4" fill="#2171b5"/>')

    s.append("</svg>")
    with analyze.open_output(out) as fh:
        fh.write("\n".join(s))
    return out


def write_sweep_pages(results_dir, label, metric, sweeps, floor, outdir):
    """One detail page per swept workload: the stacked sub-counter view vs N, the
    scaling analysis (floor / k / top-2×), the growth-attribution table (which
    phase buckets the growth lands in), and the raw per-size sweep numbers.
    Returns {workload: href relative to the sweep report} for linking."""
    import inspect
    from lib import manifest
    per_bucket, floor_b = bucket_series(results_dir, label, metric)
    wdir = os.path.join(outdir, "workloads")
    os.makedirs(wdir, exist_ok=True)
    pre = ("background:#fff;border:1px solid #eee;border-radius:6px;padding:8px;overflow:auto")
    links = {}
    for wl in canonical_order(sweeps):
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
        topcls = "reg" if (top and top > SUPERLINEAR_TOP2X) else "flat"
        desc = (inspect.getdoc(spec.gen) if spec and spec.gen else "") or "(no description)"
        flags = " ".join(spec.extra_flags) if spec and spec.extra_flags else "(none)"
        meta = (f"<b>bucket:</b> {html.escape(spec.bucket)} &nbsp;·&nbsp; <b>mode:</b> "
                f"{html.escape(spec.mode)} &nbsp;·&nbsp; <b>flags:</b> <code>{html.escape(flags)}</code>"
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
                + render_growth_table(per_bucket.get(wl, {}), floor_b)
                + f"<h2>Sweep numbers ({metric} ms)</h2>" + "".join(num)
                + "</div>")
        with analyze.open_output(os.path.join(wdir, f"{wl}.html")) as fh:
            fh.write(body)
        links[wl] = f"workloads/{wl}.html"
    return links


def _swept_workload_count(path):
    """Number of workloads with multi-size data in a results.json, or 0.
    An unreadable file (corrupt json, permission change) counts as unswept
    rather than aborting the scan — matching sweep.py's completeness loader."""
    try:
        runs = json.load(open(path))
    except (ValueError, OSError):
        return 0
    per = {}
    for r in runs:
        if r.get("size", 0) > 0 and r.get("timers"):
            per.setdefault(r["workload"], set()).add(r["size"])
    return sum(1 for szs in per.values() if len(szs) >= 2)


def find_swept_labels(results_dir):
    """Every label with swept (multi-size) data, as two lists of
    (label, n_swept_workloads): dailies newest-first (labels are
    `YYYY-MM-DD-<shortsha>`, so reverse-lexicographic is date order) and
    releases in index order (release order) when index.json exists,
    directory order otherwise. Sweeps are opt-in, so these lists stay short."""
    dailies, releases = [], []
    ddir = os.path.join(results_dir, "daily")
    if os.path.isdir(ddir):
        for label in sorted(os.listdir(ddir), reverse=True):
            n = _swept_workload_count(os.path.join(ddir, label, "results.json"))
            if n:
                dailies.append((label, n))
    rdir = os.path.join(results_dir, "releases")
    order = None
    ipath = os.path.join(results_dir, "index.json")
    if os.path.exists(ipath):
        try:
            order = [r["tag"] for r in json.load(open(ipath))]
        except (ValueError, OSError):
            order = None
    if os.path.isdir(rdir):
        tags = order if order is not None else sorted(os.listdir(rdir))
        for tag in tags:
            n = _swept_workload_count(os.path.join(rdir, tag, "results.json"))
            if n:
                releases.append((tag, n))
    return dailies, releases


def find_latest_swept(results_dir):
    """The newest swept daily label — the headline sweep the landing page
    features. Exits when nothing is swept yet (before the first sweep=true run)."""
    dailies, _ = find_swept_labels(results_dir)
    if not dailies:
        raise SystemExit("no swept daily results found; run the nightly with sweep=true first")
    return dailies[0][0]


def render_label(results_dir, label, metric, outdir):
    """Render one label's full sweep page set (index + per-workload pages)
    into `outdir`. Returns the number of swept workloads (0 = nothing swept,
    nothing written)."""
    sweeps = load_sweeps(results_dir, label, metric)
    if not sweeps:
        return 0

    # Fixed per-compile floor = the `minimal` workload's compileInner on this
    # build (the suite's floor canary). Subtracted before the power-law fit so the
    # exponent reflects feature work, not the floor. 0 if minimal wasn't run.
    floor = 0.0
    raw = json.load(open(analyze.results_path(results_dir, label)))
    for r in raw:
        if r["workload"] == "minimal" and r["timers"].get("compileInner"):
            floor = r["timers"]["compileInner"][metric]
            break

    os.makedirs(outdir, exist_ok=True)
    # Per-workload pages (stacked sub-counters + analysis + numbers), then the
    # index's compileInner-only curves linking into them.
    links = write_sweep_pages(results_dir, label, metric, sweeps, floor, outdir)
    svg = render_panels(sweeps, metric, os.path.join(outdir, "sweep_curves.svg"),
                        floor, link_for=links)
    inline = open(svg).read()

    names = canonical_order(sweeps)
    H = ['<!doctype html><meta charset="utf-8">',
         f"<title>Slang compile-perf — complexity sweep ({html.escape(label)})</title>",
         f"<style>{CSS}</style>", '<div class="wrap">',
         "<h1>Slang compile-time performance — complexity sweep</h1>",
         f'<p class="sub">build <b>{html.escape(label)}</b> · metric: {metric} (ms) · '
         f'{len(names)} swept workload(s) · compile time vs workload size <code>N</code>, '
         "each curve a single binary scaled simple→complex.</p>",
         "<h2>Scaling curves (compileInner)</h2>",
         '<p class="small">Each panel is one workload\'s <b>compileInner</b> vs '
         '<code>N</code> on a zero-based linear axis, flagged with the floor-subtracted '
         f'exponent <b>∝N<sup>k</sup></b> (red ⇒ super-linear, k&gt;{SUPERLINEAR_K}). '
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
             f'results/{html.escape(label)}/results.json. Companion to report.py '
             '(cross-release) and ladder_scaling.py (cross-release fit table).</p>')
    H.append("</div>")

    out = os.path.join(outdir, "sweep_report.html")
    with analyze.open_output(out) as fh:
        fh.write("\n".join(H))
    print(f"wrote {out}  ({len(names)} workloads)")
    return len(names)


def publish_all(results_dir, metric, outdir):
    """Render EVERY archived sweep (dailies + releases) under `outdir/<label>/`
    plus a landing page `outdir/index.html` listing them — dailies newest-first
    (the newest is the headline), releases in release order. Sweeps are opt-in
    and therefore few, so rendering all of them per deploy is cheap and makes
    the archive browsable instead of data-only."""
    dailies, releases = find_swept_labels(results_dir)
    os.makedirs(outdir, exist_ok=True)
    rows_d, rows_r = [], []
    for label, _n in dailies:
        n = render_label(results_dir, label, metric, os.path.join(outdir, label))
        if n:
            rows_d.append((label, n))
    for label, _n in releases:
        n = render_label(results_dir, label, metric, os.path.join(outdir, label))
        if n:
            rows_r.append((label, n))

    def table(rows):
        if not rows:
            return "<p class='small'>none yet</p>"
        out = ["<table><tr><th>build</th><th class=num>swept workloads</th></tr>"]
        for label, n in rows:
            out.append(f"<tr><td><a href='{html.escape(label)}/sweep_report.html'>"
                       f"{html.escape(label)}</a></td><td class=num>{n}</td></tr>")
        out.append("</table>")
        return "".join(out)

    headline = (f'<p class="sub">Latest: <a href="{html.escape(rows_d[0][0])}/sweep_report.html">'
                f"<b>{html.escape(rows_d[0][0])}</b></a></p>" if rows_d else
                '<p class="sub">No sweeps recorded yet — dispatch a nightly or release '
                "sweep with <code>sweep=true</code>.</p>")
    H = ['<!doctype html><meta charset="utf-8">',
         "<title>Slang compile-perf — complexity sweeps</title>",
         f"<style>{CSS}</style>", '<div class="wrap">',
         "<h1>Complexity sweeps</h1>", headline,
         '<p class="small">Each sweep compiles every laddered workload at 4 sizes on one '
         "binary and fits the floor-subtracted power law — the scaling view that separates "
         "fixed-cost regressions from per-element and super-linear ones. Sweeps run on "
         "demand (<code>sweep=true</code> on the nightly or release-sweep dispatch); every "
         "archived sweep is listed here.</p>",
         "<h2>Daily tip-of-tree sweeps</h2>", table(rows_d),
         "<h2>Release sweeps</h2>", table(rows_r),
         '<p class="small"><a href="../report_per_workload.html">back to the main report</a></p>',
         "</div>"]
    out = os.path.join(outdir, "index.html")
    with analyze.open_output(out) as fh:
        fh.write("\n".join(H))
    print(f"wrote {out}  ({len(rows_d)} daily + {len(rows_r)} release sweeps)")


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--label", default="dev", help="results/<label>/ to report on")
    ap.add_argument("--latest", action="store_true",
                    help="report on the newest daily/<label> that has swept "
                         "(multi-size) data, ignoring --label")
    ap.add_argument("--publish", default=None, metavar="DIR",
                    help="render EVERY archived sweep + a landing index into DIR "
                         "(what CI deploys as the site's sweep/ section); "
                         "succeeds with an empty index when nothing is swept yet")
    ap.add_argument("--out", default=None,
                    help="output directory (default: <label's results dir>/sweep)")
    ap.add_argument("--metric", default="median", choices=["min", "median", "mean"])
    args = ap.parse_args()

    if args.publish:
        publish_all(args.results, args.metric, args.publish)
        return

    if args.latest:
        args.label = find_latest_swept(args.results)
    outdir = args.out or os.path.join(
        analyze.results_dir_for(args.results, args.label), "sweep")
    if not render_label(args.results, args.label, args.metric, outdir):
        raise SystemExit(f"no multi-size (swept) workloads in results/{args.label}; "
                         "re-run bench.py with --sweep")


if __name__ == "__main__":
    main()

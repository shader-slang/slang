#!/usr/bin/env python3
"""Attribute compile time to the nested phase timers — "where did the time go?".

slangc's -report-perf-benchmark timers are nested:

    compileInner
      frontEndExecute ── parseTranslationUnit, SemanticChecking, generateIR
      generateOutput ─── linkAndOptimizeIR ── specializeModule, simplifyIR, linkIR,
                                               unrollLoopsInModule, legalize*, inlining*
                         emitEntryPointsSourceFromIR

A parent's time is usually larger than the sum of its named children; the gap is
real work with no dedicated timer (e.g. the autodiff IR transform shows up as
linkAndOptimizeIR *self* time, not a named sub-timer). This tool turns each
workload's timers into a set of **mutually-exclusive buckets that sum to
compileInner** — every named leaf, plus a "<parent> (self)" residual per parent —
so a breakdown accounts for 100% of the time and the unnamed hotspots surface.

Modes:
    breakdown.py --label v2026.10                  aggregate across the suite + per-workload table
    breakdown.py --label v2026.10 --workload mdl_dxr   full indented tree for one workload

The metric is the median by default (the suite's reported statistic).
"""
import argparse
import inspect
import json
import os

import analyze
import manifest

HERE = os.path.dirname(os.path.abspath(__file__))

# (timer, [children]) — the nested timer tree. Each parent gets a synthetic
# "<parent> (self)" residual = parent − Σ children, so buckets tile compileInner.
TREE = ("compileInner", [
    ("frontEndExecute", [
        ("parseTranslationUnit", []),
        ("SemanticChecking", []),
        ("generateIR", []),
    ]),
    ("generateOutput", [
        ("linkAndOptimizeIR", [
            ("specializeModule", []),
            ("simplifyIR", []),
            ("linkIR", []),
            ("unrollLoopsInModule", []),
            ("legalizeResourceTypes", []),
            ("legalizeExistentialTypeLayout", []),
            ("performMandatoryEarlyInlining", []),
            ("performForceInlining", []),
        ]),
        ("emitEntryPointsSourceFromIR", []),
    ]),
])


# Canonical bucket order + colors for the stacked view, grouped by stage:
# front-end = greens, linkAndOptimizeIR subtree = blues/purples, emit = oranges,
# residual = grey. Keeping order/colors fixed makes bars comparable across
# workloads at a glance.
BUCKET_ORDER = [
    ("parseTranslationUnit", "#c7e9c0"),
    ("SemanticChecking", "#41ab5d"),
    ("generateIR", "#006d2c"),
    ("frontEndExecute (self)", "#74c476"),
    ("specializeModule", "#6baed6"),
    ("simplifyIR", "#2171b5"),
    ("linkIR", "#08306b"),
    ("unrollLoopsInModule", "#9e9ac8"),
    ("legalizeResourceTypes", "#807dba"),
    ("legalizeExistentialTypeLayout", "#6a51a3"),
    ("performMandatoryEarlyInlining", "#bcbddc"),
    ("performForceInlining", "#dadaeb"),
    ("linkAndOptimizeIR (self)", "#4a1486"),
    ("emitEntryPointsSourceFromIR", "#fd8d3c"),
    ("generateOutput (self)", "#e6550d"),
    ("compileInner (self)", "#969696"),
]
BUCKET_COLOR = dict(BUCKET_ORDER)


def esc(s):
    return str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def _t(timers, name):
    st = timers.get(name)
    return st if isinstance(st, (int, float)) else 0.0


def buckets(timers):
    """Mutually-exclusive {bucket: ms} that sum to compileInner, allocated TOP-DOWN
    from the compileInner budget. Each parent places its measured children within
    its budget; the remainder is '<parent> (self)'.

    Slang's phase timers are not perfectly additive — named sub-timers can sum to
    MORE than their parent (e.g. specializeModule + simplifyIR + … exceed
    linkAndOptimizeIR after the v2026.7 specialization/autodiff work). When that
    happens the children are scaled proportionally to fit the parent's budget, so
    the overshoot stays LOCAL instead of propagating up and zeroing an ancestor's
    self-time (which previously made generateOutput (self) vanish at v2026.7).
    Either way the buckets sum exactly to compileInner."""
    out = {}

    def alloc(node, budget):
        name, children = node
        if budget <= 0:
            return
        if not children:
            out[name] = out.get(name, 0.0) + budget
            return
        cm = [(c, _t(timers, c[0])) for c in children]
        csum = sum(v for _, v in cm)
        if csum > budget and csum > 0:
            scale = budget / csum  # children overshoot parent -> fit proportionally
            for c, v in cm:
                if v > 0:
                    alloc(c, v * scale)
        else:
            for c, v in cm:
                if v > 0:
                    alloc(c, v)
            self_ms = budget - csum
            if self_ms > 0.05:
                out[f"{name} (self)"] = out.get(f"{name} (self)", 0.0) + self_ms

    alloc(TREE, _t(timers, "compileInner"))
    return out


def _runs(results_dir, label, metric):
    path = os.path.join(results_dir, label, "results.json")
    if not os.path.exists(path):
        raise SystemExit(f"no results at {path}")
    out = []
    for r in analyze.canonical_runs(json.load(open(path))):
        timers = {k: v[metric] for k, v in r["timers"].items() if v}
        out.append((r["workload"], r.get("size", 0), timers))
    return out


def _bar(frac, width=28):
    n = int(round(frac * width))
    return "█" * n + "·" * (width - n)


def aggregate(runs):
    """Where the whole benchmark's compile time goes: sum each bucket across all
    workloads (note: weighted by each workload's default size — a deliberately
    large workload contributes more wall-clock, as it does in a real sweep)."""
    agg = {}
    total = 0.0
    for _, _, timers in runs:
        for b, ms in buckets(timers).items():
            agg[b] = agg.get(b, 0.0) + ms
        total += _t(timers, "compileInner")
    print(f"\n=== Where the benchmark spends time (sum of {len(runs)} workloads, "
          f"total compileInner = {total:,.0f} ms) ===")
    print(f"{'phase bucket':34s}{'ms':>10}{'% total':>9}  share")
    print("-" * 88)
    for b, ms in sorted(agg.items(), key=lambda kv: -kv[1]):
        frac = ms / total if total else 0
        print(f"{b:34s}{ms:10.0f}{100*frac:8.1f}%  {_bar(frac)}")


def per_workload(runs):
    """One line per workload: compileInner and its single dominant bucket."""
    print(f"\n=== Per-workload dominant phase ===")
    print(f"{'workload':22s}{'N':>6}{'compileInner':>14}{'dominant bucket':>26}{'%':>7}")
    print("-" * 78)
    rows = sorted(runs, key=lambda r: -_t(r[2], "compileInner"))
    for wl, size, timers in rows:
        ci = _t(timers, "compileInner")
        bk = buckets(timers)
        if not ci or not bk:
            continue
        top, tms = max(bk.items(), key=lambda kv: kv[1])
        print(f"{wl:22s}{size:>6}{ci:12.1f}ms{top:>26}{100*tms/ci:6.1f}%")


def tree_view(runs, workload):
    """Full indented timer tree for one workload, ms + % of compileInner."""
    match = [r for r in runs if r[0] == workload]
    if not match:
        raise SystemExit(f"workload '{workload}' not in this label")
    _, size, timers = match[0]
    ci = _t(timers, "compileInner") or 1.0
    print(f"\n=== {workload} (N={size}) — compileInner = {ci:.1f} ms ===")

    def show(node, depth):
        name, children = node
        total = _t(timers, name)
        if total == 0 and name != "compileInner":
            return
        print(f"{'  ' * depth}{name:30s}{total:9.1f} ms  ({100*total/ci:5.1f}%)")
        child_sum = 0.0
        for c in children:
            child_sum += _t(timers, c[0])
            show(c, depth + 1)
        if children:
            self_ms = max(total, child_sum) - child_sum
            if self_ms > 0.05:
                print(f"{'  ' * (depth + 1)}{'(self / unnamed)':30s}"
                      f"{self_ms:9.1f} ms  ({100*self_ms/ci:5.1f}%)")

    show(TREE, 0)


def render_stacked_svg(runs, label, metric):
    """One horizontal stacked bar per workload, segments = phase buckets, bar
    length proportional to compileInner (so composition AND magnitude both read).
    Sorted by compileInner descending."""
    rows = sorted(((wl, sz, t) for wl, sz, t in runs if _t(t, "compileInner") > 0),
                  key=lambda r: -_t(r[2], "compileInner"))
    max_ci = max((_t(t, "compileInner") for _, _, t in rows), default=1.0)
    ml, mt = 168, 56          # left margin (labels), top margin (title)
    pw = 760                  # max bar pixel width (== max_ci)
    rh, bh = 26, 17           # row pitch, bar height
    legend_cols = 4
    legend_rows = (len(BUCKET_ORDER) + legend_cols - 1) // legend_cols
    H = mt + len(rows) * rh + 30 + legend_rows * 18 + 20
    W = ml + pw + 90
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
         f'viewBox="0 0 {W} {H}" preserveAspectRatio="xMidYMid meet" '
         f'font-family="sans-serif" font-size="11">',
         f'<rect width="{W}" height="{H}" fill="white"/>',
         f'<text x="16" y="26" font-size="17" font-weight="bold">'
         f'Compile-time phase breakdown — {esc(label)} ({esc(metric)})</text>',
         f'<text x="16" y="44" fill="#666">each bar = compileInner, segmented by '
         f'phase; length ∝ time (max {max_ci:,.0f} ms)</text>']
    y = mt
    for wl, sz, t in rows:
        ci = _t(t, "compileInner")
        bk = buckets(t)
        s.append(f'<text x="{ml-6}" y="{y+bh-4}" text-anchor="end" fill="#222" '
                 f'font-weight="600">{esc(wl)}</text>')
        x = ml
        for name, color in BUCKET_ORDER:
            ms = bk.get(name, 0.0)
            if ms <= 0:
                continue
            w = ms / max_ci * pw
            pct = 100 * ms / ci
            s.append(f'<rect x="{x:.1f}" y="{y}" width="{w:.2f}" height="{bh}" '
                     f'fill="{color}"><title>{esc(wl)} — {esc(name)}: '
                     f'{ms:.1f} ms ({pct:.1f}%)</title></rect>')
            if w > 34:  # inline % label only if the segment is wide enough
                tc = "#fff" if color not in ("#c7e9c0", "#dadaeb", "#bcbddc") else "#333"
                s.append(f'<text x="{x+w/2:.1f}" y="{y+bh-5}" text-anchor="middle" '
                         f'fill="{tc}" font-size="9">{pct:.0f}</text>')
            x += w
        s.append(f'<text x="{x+6:.1f}" y="{y+bh-4}" fill="#444">{ci:,.0f} ms</text>')
        y += rh
    # legend
    ly = y + 24
    s.append(f'<text x="16" y="{ly-8}" fill="#666" font-weight="600">phase buckets '
             f'(front-end = greens, optimize = blues/purples, emit = oranges, residual = grey)</text>')
    for i, (name, color) in enumerate(BUCKET_ORDER):
        col, row = i % legend_cols, i // legend_cols
        lx = 16 + col * ((W - 32) // legend_cols)
        yy = ly + row * 18
        s.append(f'<rect x="{lx}" y="{yy}" width="12" height="12" fill="{color}"/>')
        s.append(f'<text x="{lx+16}" y="{yy+10}" fill="#222">{esc(name)}</text>')
    s.append("</svg>")
    return "\n".join(s)


def write_html(results_dir, label, metric):
    runs = _runs(results_dir, label, metric)
    svg = render_stacked_svg(runs, label, metric)
    outdir = os.path.join(results_dir, label, "_breakdown")
    os.makedirs(outdir, exist_ok=True)
    with open(os.path.join(outdir, "breakdown.svg"), "w") as fh:
        fh.write(svg)
    html = (f"<!doctype html><meta charset=utf-8>"
            f"<title>Phase breakdown — {esc(label)}</title>"
            f"<body style='font-family:sans-serif;margin:24px'>"
            f"<p style='color:#555'>Hover a segment for exact ms / %. Buckets are "
            f"mutually exclusive and sum to compileInner; '(self)' = a parent's "
            f"time not covered by a named child (e.g. the autodiff transform sits "
            f"in <code>linkAndOptimizeIR (self)</code>).</p>{svg}</body>")
    out = os.path.join(outdir, "breakdown.html")
    with open(out, "w") as fh:
        fh.write(html)
    print(f"wrote {out}  ({len(runs)} workloads)")
    return out


# Coarse decomposition: just the two top-level children of compileInner, used on
# the index page. Drilling into a workload's own page shows the full BUCKET_ORDER.
FE_GO_ORDER = [
    ("frontEndExecute", "#41ab5d"),
    ("generateOutput", "#e6550d"),
    ("compileInner (self)", "#969696"),
]


def coarse_buckets(timers):
    """Top-level split: frontEndExecute / generateOutput (+ residual), summing to
    compileInner. The high-level view for the per-workload index page."""
    ci = _t(timers, "compileInner")
    fe = _t(timers, "frontEndExecute")
    go = _t(timers, "generateOutput")
    out = {}
    if fe > 0:
        out["frontEndExecute"] = fe
    if go > 0:
        out["generateOutput"] = go
    resid = ci - fe - go
    if resid > 0.05:
        out["compileInner (self)"] = resid
    return out


def _series(results_dir, index_path, metric, bucket_fn):
    """(order_tags, {workload: [bucketdict|None per release]}) using bucket_fn."""
    index = json.load(open(index_path))
    order, per = [], {}
    for rec in index:
        if "slangc" not in rec:
            continue
        tag = rec["tag"]
        path = os.path.join(results_dir, tag, "results.json")
        if not os.path.exists(path):
            continue
        order.append(tag)
        for r in analyze.canonical_runs(json.load(open(path))):
            timers = {k: v[metric] for k, v in r["timers"].items() if v}
            per.setdefault(r["workload"], {})[tag] = bucket_fn(timers)
    return order, {wl: [bytag.get(t) for t in order] for wl, bytag in per.items()}


def render_stacked_multiples(results_dir, index_path, metric, out, bucket_order,
                             bucket_fn, cols=2, names=None, link_for=None,
                             title=None, panel=(620, 300)):
    """Small-multiples stacked-AREA chart: one panel per workload, a filled band
    per bucket across the release history (top edge traces compileInner; own
    zero-based y-axis per panel). `bucket_order`/`bucket_fn` pick the decomposition
    (coarse fe/go vs full sub-counters). If `link_for` maps workload -> href, the
    panel title becomes a link to that page."""
    order, per = _series(results_dir, index_path, metric, bucket_fn)
    nrel = len(order)

    def last_ci(wl):
        return next((sum(bd.values()) for bd in reversed(per[wl]) if bd), 0) or 0

    if names is None:
        names = sorted(per, key=lambda w: -last_ci(w))
    n = len(names)
    rows = (n + cols - 1) // cols
    pw, ph = panel
    ml, mt, mr, mb = 60, 32, 16, 40
    cw, chh = ml + pw + mr, mt + ph + mb
    W = cols * cw + 16
    legend_cols = min(len(bucket_order), 4) or 1
    legend_rows = (len(bucket_order) + legend_cols - 1) // legend_cols
    H = rows * chh + 56 + legend_rows * 18 + 24
    if title is None:
        title = f"Per-benchmark phase composition across releases ({esc(metric)} ms)"

    s = [f'<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" '
         f'width="{W}" height="{H}" viewBox="0 0 {W} {H}" preserveAspectRatio="xMidYMid meet" '
         f'font-family="sans-serif" font-size="13">',
         f'<rect width="{W}" height="{H}" fill="white"/>',
         f'<text x="12" y="30" font-size="20" font-weight="bold">{title}</text>']
    tick_idx = sorted(set([0, nrel // 2, nrel - 1]))

    for k, wl in enumerate(names):
        r, c = divmod(k, cols)
        ox, oy = 8 + c * cw, 44 + r * chh
        series = per.get(wl, [None] * nrel)
        cis = [(sum(bd.values()) if bd else None) for bd in series]
        vmax = max((v for v in cis if v), default=1.0) or 1.0
        hi = vmax * 1.08
        ymap = lambda v: oy + mt + ph - (v / hi) * ph
        xmap = lambda i: ox + ml + (i * pw / (nrel - 1) if nrel > 1 else pw / 2)

        first = next((v for v in cis if v), None)
        lastv = next((v for v in reversed(cis) if v), None)
        ratio = (lastv / first) if (first and lastv) else None
        rtxt = f"  {ratio:.2f}×" if ratio else ""
        link = (link_for or {}).get(wl)
        fill = "#1a5fb4" if link else "#1a1a1a"
        deco = ' text-decoration="underline"' if link else ""
        ttl = (f'<text x="{ox+ml}" y="{oy+16}" font-size="15" font-weight="600" fill="{fill}"{deco}>'
               f'{esc(wl)}<tspan fill="#888" font-weight="400" text-decoration="none">'
               f'{esc(rtxt)}</tspan></text>')
        if link:
            ttl = f'<a xlink:href="{esc(link)}" href="{esc(link)}" target="_top">{ttl}</a>'
        s.append(ttl)

        for frac in (0.0, 0.5, 1.0):
            yv = hi * frac
            y = ymap(yv)
            s.append(f'<line x1="{ox+ml:.1f}" y1="{y:.1f}" x2="{ox+ml+pw:.1f}" y2="{y:.1f}" stroke="#eee"/>')
            lbl = f"{yv:.0f}" if yv >= 10 else f"{yv:.1f}"
            s.append(f'<text x="{ox+ml-4:.1f}" y="{y+3:.1f}" text-anchor="end" fill="#999">{lbl}</text>')
        s.append(f'<line x1="{ox+ml:.1f}" y1="{oy+mt:.1f}" x2="{ox+ml:.1f}" y2="{oy+mt+ph:.1f}" stroke="#333"/>')
        s.append(f'<line x1="{ox+ml:.1f}" y1="{oy+mt+ph:.1f}" x2="{ox+ml+pw:.1f}" y2="{oy+mt+ph:.1f}" stroke="#333"/>')
        for i in tick_idx:
            x = xmap(i)
            t = order[i].replace("v20", "")
            s.append(f'<line x1="{x:.1f}" y1="{oy+mt+ph:.1f}" x2="{x:.1f}" y2="{oy+mt+ph+3:.1f}" stroke="#999"/>')
            s.append(f'<text x="{x:.1f}" y="{oy+mt+ph+16:.1f}" text-anchor="middle" fill="#666">{esc(t)}</text>')

        # Stacked AREA: one filled band per bucket, bottom-to-top in bucket_order;
        # each band spans the releases with data; the topmost top edge = compileInner.
        present = [i for i, bd in enumerate(series) if bd]
        if len(present) >= 2:
            lower = {i: 0.0 for i in present}
            for name, color in bucket_order:
                if all(series[i].get(name, 0.0) <= 0 for i in present):
                    continue
                upper = {i: lower[i] + series[i].get(name, 0.0) for i in present}
                top = [f"{xmap(i):.1f},{ymap(upper[i]):.1f}" for i in present]
                bot = [f"{xmap(i):.1f},{ymap(lower[i]):.1f}" for i in reversed(present)]
                s.append(f'<polygon points="{" ".join(top + bot)}" fill="{color}" '
                         f'fill-opacity="0.9" stroke="#fff" stroke-width="0.4">'
                         f'<title>{esc(wl)} — {esc(name)}</title></polygon>')
                lower = upper

    ly = 44 + rows * chh + 6
    s.append(f'<text x="12" y="{ly}" fill="#666" font-weight="600">phase buckets</text>')
    for i, (name, color) in enumerate(bucket_order):
        col, row = i % legend_cols, i // legend_cols
        lx = 12 + col * ((W - 24) // legend_cols)
        yy = ly + 8 + row * 18
        s.append(f'<rect x="{lx}" y="{yy}" width="12" height="12" fill="{color}"/>')
        s.append(f'<text x="{lx+16}" y="{yy+10}" fill="#222">{esc(name)}</text>')
    s.append("</svg>")
    with open(out, "w") as fh:
        fh.write("\n".join(s))
    return out


def _workload_source(spec, head=40, tail=40, ctx=40):
    """(n, [(filename, display_code)]) — the EXACT compiled Slang at the workload's
    real size. A long file is shown as three windows — its first `head` lines, the
    `±ctx` lines around the `computeMain` entry point (which shows how the generated
    functions are actually invoked), and its last `tail` lines — with the elided
    gaps marked by a comment. Overlapping windows merge. Even a 2000-function shader
    stays readable without dumping thousands of lines."""
    n = spec.default_size
    out = []
    for fn, src in spec.gen(n).items():
        lines = src.splitlines()
        L = len(lines)
        if L <= head + tail:
            out.append((fn, "\n".join(lines)))
            continue
        ranges = [(0, head), (L - tail, L)]
        cm = next((i for i, l in enumerate(lines) if "computeMain" in l), None)
        if cm is not None:
            ranges.append((max(0, cm - ctx), min(L, cm + ctx + 1)))
        ranges.sort()
        merged = []
        for lo, hi in ranges:
            # merge when overlapping or separated by a tiny gap (a 3-line elision
            # marker to hide ≤4 lines is pointless — just show them)
            if merged and lo <= merged[-1][1] + 4:
                merged[-1][1] = max(merged[-1][1], hi)
            else:
                merged.append([lo, hi])
        disp, prev = [], 0
        for idx, (lo, hi) in enumerate(merged):
            if idx > 0:
                disp += ["", f"// … {lo - prev} lines omitted …", ""]
            disp += lines[lo:hi]
            prev = hi
        out.append((fn, "\n".join(disp)))
    return n, out


def write_workload_pages(results_dir, index_path, metric, outdir):
    """One detail page per workload: the FULL sub-counter stacked-area history, the
    workload's description (from its generator's docstring), and the exact compiled
    Slang source. Returns {workload: href relative to outdir} for the index page."""
    _, per = _series(results_dir, index_path, metric, buckets)
    wdir = os.path.join(outdir, "workloads")
    os.makedirs(wdir, exist_ok=True)
    pre = ("background:#f6f8fa;border:1px solid #e3e6ea;border-radius:6px;padding:12px;"
           "overflow:auto;max-height:560px;font:12px/1.45 ui-monospace,Menlo,Consolas,monospace;"
           "white-space:pre;color:#24292f")
    links = {}
    for wl in sorted(per):
        spec = manifest.BY_NAME.get(wl)
        svgp = os.path.join(wdir, f"{wl}.svg")
        render_stacked_multiples(
            results_dir, index_path, metric, svgp, BUCKET_ORDER, buckets,
            cols=1, names=[wl], panel=(1040, 440),
            title=f"{esc(wl)} — full phase breakdown across releases ({esc(metric)} ms)")
        svg = open(svgp).read()

        desc = (inspect.getdoc(spec.gen) if spec and spec.gen else "") or "(no description)"
        flags = " ".join(spec.extra_flags) if spec and spec.extra_flags else "(none)"
        meta = (f"<b>bucket:</b> {esc(spec.bucket)} &nbsp;·&nbsp; <b>compile mode:</b> "
                f"{esc(spec.mode)} &nbsp;·&nbsp; <b>flags:</b> <code>{esc(flags)}</code> "
                f"&nbsp;·&nbsp; <b>default N:</b> {spec.default_size}") if spec else ""

        _, srcfiles = _workload_source(spec) if spec else (0, [])
        tail_txt = ("show the first 40 lines, the area around computeMain (±40), and the last "
                    "40 lines (gaps elided)")
        size_note = ((f"exact compiled source; long files {tail_txt}")
                     if spec and spec.default_size == 0
                     else (f"exact compiled source (N = {spec.default_size}); long files {tail_txt}")
                     if spec else "")
        code_html = ""
        for fn, code in srcfiles:
            code_html += (f"<h3 style='font-size:13px;margin:16px 0 4px;color:#444'>"
                          f"{esc(fn)}</h3><pre style='{pre}'>{esc(code)}</pre>")

        html = (f"<!doctype html><meta charset=utf-8><title>{esc(wl)} — phase breakdown</title>"
                f"<body style='font-family:-apple-system,Segoe UI,Roboto,sans-serif;"
                f"margin:24px;color:#1a1a1a;max-width:1180px'>"
                f"<p><a href='../report_per_workload.html'>&larr; all workloads</a></p>"
                f"<h1 style='font-size:21px;margin:0 0 6px'>{esc(wl)}</h1>"
                f"<p style='color:#444;max-width:900px;white-space:pre-wrap'>{esc(desc)}</p>"
                f"<p style='color:#666;font-size:13px'>{meta}</p>"
                f"<h2 style='font-size:17px;margin:26px 0 8px;border-bottom:2px solid #eee;"
                f"padding-bottom:4px'>Phase composition across releases</h2>"
                f"<p style='color:#666;font-size:13px;max-width:900px'>Full sub-counter "
                f"decomposition of <code>compileInner</code> &mdash; named leaf timers plus "
                f"<code>(self)</code> residuals (a parent's time not covered by a named child, "
                f"e.g. the autodiff transform in <code>linkAndOptimizeIR (self)</code>). Topmost "
                f"band traces compileInner; hover a band for its phase.</p>"
                f"<div style='border:1px solid #eee;border-radius:6px;padding:8px;overflow:auto'>"
                f"{svg}</div>"
                f"<h2 style='font-size:17px;margin:26px 0 8px;border-bottom:2px solid #eee;"
                f"padding-bottom:4px'>Compiled Slang source</h2>"
                f"<p style='color:#666;font-size:13px'>{esc(size_note)}</p>{code_html}"
                f"</body>")
        with open(os.path.join(wdir, f"{wl}.html"), "w") as fh:
            fh.write(html)
        links[wl] = f"workloads/{wl}.html"
    return links


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--label", default="dev", help="results/<label>/ to break down")
    ap.add_argument("--metric", default="median", choices=["min", "median", "mean"])
    ap.add_argument("--workload", default=None, help="show the full tree for one workload")
    ap.add_argument("--html", action="store_true",
                    help="write a stacked-bar phase breakdown to <label>/_breakdown/")
    args = ap.parse_args()

    if args.html:
        write_html(args.results, args.label, args.metric)
        return
    runs = _runs(args.results, args.label, args.metric)
    if args.workload:
        tree_view(runs, args.workload)
    else:
        aggregate(runs)
        per_workload(runs)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Cross-release view of the complexity_ladder scaling curve.

After `sweep.py --only complexity_ladder --sweep`, each release has the ladder
compiled at several complexity sizes. This fits time = floor + slope*N per
release and tabulates floor / slope / R^2 plus the absolute curve endpoints, so
you can see not just *that* compile time rose, but whether the *shape* of the
simple->complex curve changed: a higher floor (heavier per-compile fixed cost),
a steeper slope (each unit of complexity costs more), or worse super-linearity
(the curve bends up harder at high complexity).

    python3 ladder_scaling.py                 # table across releases
    python3 ladder_scaling.py --workload X    # any swept workload
"""
import argparse
import json
import os

from lib import analyze

HERE = os.path.dirname(os.path.abspath(__file__))


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("--index", default=os.path.join(HERE, "releases", "index.json"))
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--workload", default="complexity_ladder")
    ap.add_argument("--metric", default="median", choices=["min", "median", "mean"])
    args = ap.parse_args()

    index = json.load(open(args.index))
    rows = []
    for rec in index:
        tag = rec.get("tag")
        if "slangc" not in rec:
            continue
        p = analyze.results_path(args.results, tag)
        if not os.path.exists(p):
            continue
        pts = []
        for r in json.load(open(p)):
            st = r["timers"].get("compileInner")
            if r["workload"] == args.workload and st and r["size"] > 0:
                pts.append((r["size"], st[args.metric]))
        pts = sorted(set(pts))
        if len(pts) < 2:
            continue
        xs, ys = [x for x, _ in pts], [y for _, y in pts]
        a, b, r2 = analyze._linfit(xs, ys)
        # super-linearity: ratio of the top doubling's time growth vs 2.0
        slin = (ys[-1] / ys[-2]) if (len(ys) >= 2 and ys[-2]) else None
        rows.append((tag, a, b, r2, xs[0], ys[0], xs[-1], ys[-1], slin))

    if not rows:
        raise SystemExit(f"no multi-size data for '{args.workload}'; run "
                         f"sweep.py --only {args.workload} --sweep first")

    nmin, nmax = rows[0][4], rows[0][6]
    print(f"Complexity-scaling curve per release — {args.workload} "
          f"(compileInner {args.metric})\n")
    hdr = (f"{'release':14s}{'floor(ms)':>10}{'slope(ms/u)':>12}{'R^2':>6}"
           f"{f'N={nmin}(ms)':>11}{f'N={nmax}(ms)':>12}{'top2x':>8}{'slopeVs1st':>12}")
    print(hdr)
    print("-" * len(hdr))
    base_slope = rows[0][2]
    for tag, a, b, r2, n0, y0, n1, y1, slin in rows:
        sl = f"{slin:.2f}x" if slin else "-"
        drift = f"{b/base_slope:.2f}x" if base_slope else "?"
        print(f"{tag:14s}{a:10.1f}{b:12.3f}{r2:6.3f}"
              f"{y0:11.1f}{y1:12.1f}{sl:>8}{drift:>12}")

    print(f"\nfloor = fixed cost; slope = ms per complexity unit; "
          f"top2x = t(N={rows[0][6]})/t(N={rows[0][6]//2}) (2.00 = linear, >2 = super-linear).")
    f0, fl = rows[0], rows[-1]
    print(f"\nfirst→last: floor {f0[1]:.0f}→{fl[1]:.0f} ms "
          f"({fl[1]/f0[1] if f0[1] else 0:.2f}x), "
          f"slope {f0[2]:.3f}→{fl[2]:.3f} ms/unit "
          f"({fl[2]/f0[2] if f0[2] else 0:.2f}x)")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Diff two perf-suite runs (base vs head) — "did my change slow compilation?".

Local dev tool. Bench two slangc binaries on the SAME machine back-to-back (your
branch and its baseline), then diff — building both on one box cancels runner
variance, so what's left is your change's actual impact:

    python3 bench.py --slangc <BASE-slangc> --label base [--sweep] [--only ...]
    python3 bench.py --slangc <HEAD-slangc> --label head [--sweep] [--only ...]
    python3 compare.py base head

For every workload's primary timers (+ compileInner) measured at matching sizes,
reports Δ% = head/base − 1 (median metric), sorted by impact, flagging changes past
--threshold (default 15%) and --abs (default 2 ms). Exits non-zero if any timer
regressed (use --no-fail to disable), so it can gate a pre-push hook.

Caveat: the synthetic workloads amplify one compiler pass each — Δ% is a
sensitivity figure, not a user-facing slowdown; mdl_dxr is the realistic signal.
"""
import argparse
import json
import os

import analyze
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def load(results_dir, label):
    p = analyze.results_path(results_dir, label)
    if not os.path.exists(p):
        raise SystemExit(f"no run at {p}; bench.py --label {label} first")
    runs = json.load(open(p))
    return {(r["workload"], r["size"]): r for r in runs}, runs


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("base", help="baseline run label (results/<base>/)")
    ap.add_argument("head", help="changed run label (results/<head>/)")
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--metric", default="median", choices=["min", "median", "mean"])
    ap.add_argument("--threshold", type=float, default=15.0,
                    help="percent change to flag (default 15)")
    ap.add_argument("--abs", type=float, default=2.0, help="min absolute ms delta to flag")
    ap.add_argument("--all-timers", action="store_true",
                    help="compare every timer, not just each workload's primary set")
    ap.add_argument("--no-fail", action="store_true", help="report only; always exit 0")
    args = ap.parse_args()

    base, base_runs = load(args.results, args.base)
    head, head_runs = load(args.results, args.head)
    rel = args.threshold / 100.0

    tty = sys.stdout.isatty()
    RED = "\033[31m" if tty else ""
    GRN = "\033[32m" if tty else ""
    DIM = "\033[2m" if tty else ""
    RST = "\033[0m" if tty else ""

    def slangc_of(runs):
        return runs[0].get("slangc", "?") if runs else "?"
    print(f"base  {args.base:18s} {DIM}{slangc_of(base_runs)}{RST}")
    print(f"head  {args.head:18s} {DIM}{slangc_of(head_runs)}{RST}")
    print(f"Δ% = head/base − 1 ({args.metric}); flag at ±{args.threshold:.0f}% and ≥{args.abs} ms\n")

    rows = []
    broken = []  # head records that failed to compile or dropped a required timer
    for key in sorted(base.keys() & head.keys()):
        wl, size = key
        b, h = base[key], head[key]
        timers = (set(b["timers"]) if args.all_timers
                  else set(b.get("primary_timers", [])) | {"compileInner"})
        # A head build that crashed (ok=false) or lost a timer the base produced is
        # a regression, not something to silently skip.
        missing = [t for t in sorted(timers) if b["timers"].get(t) and not h["timers"].get(t)]
        if not h.get("ok", True) or missing:
            broken.append((wl, size, missing, h.get("error")))
        for t in sorted(timers):
            bt, ht = b["timers"].get(t), h["timers"].get(t)
            if not bt or not ht:
                continue
            bv, hv = bt[args.metric], ht[args.metric]
            if bv <= 0:
                continue
            dpct = (hv / bv - 1.0) * 100.0
            rows.append((wl, size, t, bv, hv, dpct, hv - bv))

    rows.sort(key=lambda r: -r[5])
    regress = [r for r in rows if r[5] >= args.threshold and r[6] >= args.abs]
    improve = [r for r in rows if r[5] <= -args.threshold and -r[6] >= args.abs]

    def classify(dpct, dms):
        if dpct >= args.threshold and dms >= args.abs:
            return "reg"
        if dpct <= -args.threshold and -dms >= args.abs:
            return "imp"
        return "flat"

    style = {"reg": (RED, " ▲"), "imp": (GRN, " ▼"), "flat": (DIM, "")}
    hdr = f"{'workload':18s}{'N':>5}  {'timer':22s}{'base':>10}{'head':>10}{'Δ%':>9}"
    print(hdr)
    print("-" * (len(hdr) + 2))
    for wl, size, t, bv, hv, dpct, dms in rows:
        col, mark = style[classify(dpct, dms)]
        print(f"{wl:18s}{size:>5}  {t:22s}{bv:10.1f}{hv:10.1f}"
              f"{col}{dpct:>+8.1f}%{RST}{mark}")

    # workloads/sizes present in only one run
    only_base = sorted(base.keys() - head.keys())
    only_head = sorted(head.keys() - base.keys())
    if only_base:
        print(f"\n{DIM}only in base (not compared): "
              f"{', '.join(f'{w}@{n}' for w, n in only_base)}{RST}")
    if only_head:
        print(f"{DIM}only in head (not compared): "
              f"{', '.join(f'{w}@{n}' for w, n in only_head)}{RST}")

    # headline: mdl_dxr is the realistic end-to-end signal
    for wl, size, t, bv, hv, dpct, dms in rows:
        if wl == "mdl_dxr" and t == "compileInner":
            print(f"\nmdl_dxr (realistic) compileInner: {bv:.0f} → {hv:.0f} ms "
                  f"({dpct:+.1f}%)")
            break

    if broken:
        print(f"\n{RED}BROKEN on head{RST} — failed compile / missing required timers "
              f"(treated as a regression):")
        for wl, size, miss, err in broken:
            detail = ("missing " + ", ".join(miss)) if miss else (err or "compile failed")
            print(f"  {wl}@{size}: {detail}")

    print(f"\n{len(regress)} regression(s), {len(improve)} improvement(s) "
          f"past ±{args.threshold:.0f}%; {len(broken)} broken head run(s).")
    if regress:
        print(f"{RED}REGRESSED:{RST} " + ", ".join(
            f"{wl}/{t}@{n} {d:+.0f}%" for wl, n, t, _, _, d, _ in regress))
    if (regress or broken) and not args.no_fail:
        sys.exit(1)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""A/B two slangc binaries with the two interleaved at the level of a single timed sample.

bench.py measures one binary over the whole suite, so an A/B comparison built from two of its runs
separates the binaries by about ninety seconds of machine time. That is enough for the machine's own
drift to dominate: comparing a binary against itself across two such runs produces an apparent 0.8%
"improvement" with p < 0.005, which is larger than anything these changes could plausibly do.

This removes the separation. For each workload the corpus is materialised once and both binaries are
run against it in an ABBA order -- A B B A, A B B A, ... -- so within each block of four the mean
position of A equals the mean position of B. Any drift that is linear over the block therefore
cancels exactly, rather than being charged to whichever binary happened to run second.

Emits one JSON with every individual sample, so the analysis can pair them.

Usage: ab.py <slangc-a> <slangc-b> <out.json> [--pairs N] [--warmup N] [--only name,name]
"""

import json
import os
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bench
from lib import corpus, manifest

TIMER = "compileInner"


def main():
    slangc_a, slangc_b, out_path = sys.argv[1], sys.argv[2], sys.argv[3]
    args = sys.argv[4:]
    pairs = int(args[args.index("--pairs") + 1]) if "--pairs" in args else 6
    warmup = int(args[args.index("--warmup") + 1]) if "--warmup" in args else 1
    only = set(args[args.index("--only") + 1].split(",")) if "--only" in args else None

    tmp = tempfile.mkdtemp(prefix="ab-perf-")
    src_root = os.path.join(tmp, "src")
    results = []

    for spec in manifest.WORKLOADS:
        if spec.mode == "api":
            continue  # needs the api-driver, which is not what is being compared here
        if spec.gen is None:
            continue  # needs a checked-in corpus (mdl_dxr); skipped when absent
        if only and spec.name not in only:
            continue
        size = spec.default_size
        src_dir = os.path.join(src_root, corpus.dir_name(spec, size))
        try:
            files = corpus.materialize(spec, size, src_dir)
        except Exception as e:  # a workload that cannot be generated is not a measurement
            print(f"[skip] {spec.name}: {e}")
            continue

        # Separate output directories: the two binaries must not race on, or reuse, each other's
        # artifacts, which would make one of them look faster for the wrong reason.
        cmds = {}
        ok = True
        for tag, slangc in (("a", slangc_a), ("b", slangc_b)):
            out_dir = os.path.join(tmp, f"out-{tag}", corpus.dir_name(spec, size))
            os.makedirs(out_dir, exist_ok=True)
            c = bench.build_commands(slangc, spec, src_dir, files, out_dir, size=size)
            for setup in c["setup"]:
                import subprocess
                if subprocess.run(setup, stdout=subprocess.DEVNULL,
                                  stderr=subprocess.DEVNULL, timeout=600).returncode != 0:
                    ok = False
            cmds[tag] = c["timed"]
        if not ok:
            print(f"[skip] {spec.name}: setup failed")
            continue

        for tag in ("a", "b"):
            for _ in range(warmup):
                bench.run_once(cmds[tag])

        samples = {"a": [], "b": []}
        failed = False
        for block in range(pairs):
            # ABBA: the mean position of each binary within the block is identical, so a drift
            # that is linear across the block contributes equally to both.
            order = ("a", "b", "b", "a") if block % 2 == 0 else ("b", "a", "a", "b")
            for tag in order:
                rc, wall, text, rss = bench.run_once(cmds[tag])
                if rc != 0 and rc != 1:
                    failed = True
                    break
                ms = bench.parse_timers(text).get(TIMER)
                if ms is None:
                    failed = True
                    break
                samples[tag].append(ms)
            if failed:
                break
        if failed or not samples["a"] or not samples["b"]:
            print(f"[skip] {spec.name}: no timers")
            continue

        import statistics
        ra, rb = statistics.median(samples["a"]), statistics.median(samples["b"])
        print(f"[ok] {spec.name:<24} n={len(samples['a'])}  a={ra:8.2f}ms  b={rb:8.2f}ms  "
              f"{rb / ra:.3f}x")
        results.append({"workload": spec.name, "size": size,
                        "a": samples["a"], "b": samples["b"]})

    json.dump({"slangc_a": slangc_a, "slangc_b": slangc_b, "timer": TIMER,
               "results": results}, open(out_path, "w"), indent=1)
    print(f"\nwrote {out_path}  ({len(results)} workloads)")
    shutil.rmtree(tmp, ignore_errors=True)


main()

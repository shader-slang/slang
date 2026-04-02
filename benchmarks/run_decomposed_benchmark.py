#!/usr/bin/env python3
"""
Decomposed backward pass benchmark: measures each operation individually
and the full chained backward, comparing Slang vs Tin2.

Uses pre-built per-operation kernels — no source modification or rebuild needed.

Slang kernels:
  - benchmark_single_layer_transpose.slang  (compute_single_layer_transpose)
  - benchmark_single_layer_outer_product.slang (compute_outer_product)
  - benchmark_single_layer_bias_reduce.slang (compute_bias_reduce)
  - benchmark_single_layer_backward.slang (compute_backward)

Tin2 binaries:
  - bench_tin2_single_layer_transpose
  - bench_tin2_single_layer_outer_product
  - bench_tin2_single_layer_bias_reduce
  - bench_tin2_single_layer_backward
"""

import subprocess
import sys
import os
import re
import json

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)

SLANGC = os.path.join(REPO_ROOT, "build/Release/bin/slangc")
INCLUDE_DIR = os.path.join(REPO_ROOT, "build/Release/lib/slang-standard-module-2026.3.1")
NCU_LAUNCHER = os.path.join(REPO_ROOT, "benchmarks/ncu_launcher_new_mma")
TIN2_DIR = os.path.join(REPO_ROOT, "tin2/benchmarks/mlp_perf/build")

WARPS = 2
RUNS_PER_CONFIG = 5

NETWORK_CONFIGS = {
    "tiny":   {"input": 32,  "output": 16},
    "small":  {"input": 64,  "output": 16},
    "medium": {"input": 128, "output": 32},
    "large":  {"input": 256, "output": 64},
    "xlarge": {"input": 128, "output": 128},
}

OPERATIONS = [
    {
        "name": "mma_transpose",
        "slang_file": "benchmark_single_layer_transpose.slang",
        "entry": "compute_single_layer_transpose",
        "mode": "transpose",
        "tin2_bin": "bench_tin2_single_layer_transpose",
    },
    {
        "name": "outer_product",
        "slang_file": "benchmark_single_layer_outer_product.slang",
        "entry": "compute_outer_product",
        "mode": "outer_product",
        "tin2_bin": "bench_tin2_single_layer_outer_product",
    },
    {
        "name": "bias_reduce",
        "slang_file": "benchmark_single_layer_bias_reduce.slang",
        "entry": "compute_bias_reduce",
        "mode": "bias_reduce",
        "tin2_bin": "bench_tin2_single_layer_bias_reduce",
    },
    {
        "name": "full_backward",
        "slang_file": "benchmark_single_layer_backward.slang",
        "entry": "compute_backward",
        "mode": "backward",
        "tin2_bin": "bench_tin2_single_layer_backward",
    },
]


def compile_ptx(op, input_size, output_size):
    ptx_path = f"/tmp/decomposed_{op['name']}_{input_size}x{output_size}.ptx"
    slang_path = os.path.join(SCRIPT_DIR, op["slang_file"])
    cmd = [
        SLANGC, slang_path,
        "-target", "ptx", "-entry", op["entry"], "-stage", "compute",
        "-o", ptx_path,
        "-I", INCLUDE_DIR,
        f"-DINPUT_SIZE={input_size}",
        f"-DOUTPUT_SIZE={output_size}",
        f"-DSUBGROUP_COUNT={WARPS}",
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if result.returncode != 0:
        return None
    return ptx_path


def run_slang(ptx_path, op, input_size, output_size, batch_size):
    cmd = [
        NCU_LAUNCHER, ptx_path,
        "--input-size", str(input_size),
        "--output-size", str(output_size),
        "--batch-size", str(batch_size),
        "--warps", str(WARPS),
        "--mode", op["mode"],
    ]
    times = []
    for _ in range(RUNS_PER_CONFIG):
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        match = re.search(r'Avg time:\s+([\d.]+)\s*ms', result.stdout)
        if match:
            times.append(float(match.group(1)))
    return sum(times) / len(times) if times else None


def run_tin2(op, size_name, batch_size):
    tin2_path = os.path.join(TIN2_DIR, op["tin2_bin"])
    if not os.path.exists(tin2_path):
        return None

    cmd = [tin2_path, "--batch-size", str(batch_size), "--warps", str(WARPS), "--size", size_name]

    times = []
    for _ in range(RUNS_PER_CONFIG):
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)

        if op["name"] == "full_backward":
            for line in result.stdout.split('\n'):
                if line.strip().startswith(size_name):
                    match = re.search(r'(\d+\.\d+)\s+\d+\s+samples/s', line)
                    if match:
                        times.append(float(match.group(1)))
        else:
            for match in re.finditer(r'Avg time:\s+([\d.]+)\s*ms', result.stdout):
                times.append(float(match.group(1)))
    return sum(times) / len(times) if times else None


ALL_BATCH_SIZES = [256, 512, 1024, 2048, 4096, 8192]


def main():
    import argparse
    parser = argparse.ArgumentParser(description="Decomposed backward benchmark: Slang vs Tin2")
    parser.add_argument("--batch-size", type=int, default=8192,
                        help="Single batch size to run (default: 8192)")
    parser.add_argument("--all-batches", action="store_true",
                        help="Run all batch sizes: " + ", ".join(str(b) for b in ALL_BATCH_SIZES))
    parser.add_argument("--size", default=None, choices=list(NETWORK_CONFIGS.keys()),
                        help="Run only one network size (default: all)")
    parser.add_argument("--op", default=None, choices=[o["name"] for o in OPERATIONS],
                        help="Run only one operation (default: all)")
    parser.add_argument("--skip-compile", action="store_true",
                        help="Skip PTX compilation, reuse previously compiled PTX files")
    args = parser.parse_args()

    batch_sizes = ALL_BATCH_SIZES if args.all_batches else [args.batch_size]
    sizes = [args.size] if args.size else list(NETWORK_CONFIGS.keys())
    ops = [o for o in OPERATIONS if args.op is None or o["name"] == args.op]

    print("=" * 90)
    print("Decomposed Backward Benchmark: Slang vs Tin2")
    print(f"Batch sizes: {batch_sizes}, Warps: {WARPS}, Runs per config: {RUNS_PER_CONFIG}")
    print("=" * 90)

    # Step 1: Compile all PTX (once, shared across batch sizes)
    ptx_cache = {}
    if args.skip_compile:
        print("\n---- Skipping PTX compilation (--skip-compile) ----")
        for op in ops:
            for size_name in sizes:
                cfg = NETWORK_CONFIGS[size_name]
                key = (op["name"], size_name)
                ptx = f"/tmp/decomposed_{op['name']}_{cfg['input']}x{cfg['output']}.ptx"
                if os.path.exists(ptx):
                    ptx_cache[key] = ptx
                else:
                    print(f"  WARNING: {ptx} not found, will skip {op['name']} {size_name}")
    else:
        print("\n---- Compiling PTX ----")
        for op in ops:
            for size_name in sizes:
                cfg = NETWORK_CONFIGS[size_name]
                key = (op["name"], size_name)
                sys.stdout.write(f"  {op['name']} {size_name} ({cfg['input']}x{cfg['output']})... ")
                sys.stdout.flush()
                ptx = compile_ptx(op, cfg["input"], cfg["output"])
                if ptx:
                    ptx_cache[key] = ptx
                    print("OK")
                else:
                    print("FAIL")

    # Step 2: Run benchmarks for each batch size
    # results[op_name][batch_size][size_name] = {"slang": ms, "tin2": ms}
    all_results = {}
    for batch_size in batch_sizes:
        print(f"\n---- Running benchmarks (batch={batch_size}) ----")
        for op in ops:
            if op["name"] not in all_results:
                all_results[op["name"]] = {}
            if batch_size not in all_results[op["name"]]:
                all_results[op["name"]][batch_size] = {}

            for size_name in sizes:
                cfg = NETWORK_CONFIGS[size_name]
                key = (op["name"], size_name)
                ptx = ptx_cache.get(key)

                slang_ms = run_slang(ptx, op, cfg["input"], cfg["output"], batch_size) if ptx else None
                tin2_ms = run_tin2(op, size_name, batch_size)

                all_results[op["name"]][batch_size][size_name] = {"slang": slang_ms, "tin2": tin2_ms}

                gap_str = ""
                if slang_ms and tin2_ms:
                    gap = (slang_ms / tin2_ms - 1) * 100
                    gap_str = f"  gap={gap:+.1f}%"
                s_str = f"{slang_ms:.4f}" if slang_ms else "N/A"
                t_str = f"{tin2_ms:.4f}" if tin2_ms else "N/A"
                print(f"  {op['name']:16s} {size_name:8s}  Slang={s_str:>10}  Tin2={t_str:>10}{gap_str}")

    # Step 3: Summary tables
    for batch_size in batch_sizes:
        print("\n" + "=" * 90)
        print(f"Summary: Slang vs Tin2 (batch={batch_size}, {WARPS} warps, avg of {RUNS_PER_CONFIG})")
        print("=" * 90)

        header = f"{'Operation':<18}"
        for size_name in sizes:
            cfg = NETWORK_CONFIGS[size_name]
            header += f"  {size_name:>8} ({cfg['input']}x{cfg['output']})"
        print(header)
        print("-" * len(header))

        for op in ops:
            row_s = f"  {'Slang':<16}"
            row_t = f"  {'Tin2':<16}"
            row_g = f"  {'Gap':<16}"
            for size_name in sizes:
                r = all_results[op["name"]].get(batch_size, {}).get(size_name, {})
                s, t = r.get("slang"), r.get("tin2")
                row_s += f"  {s:>16.4f}" if s else f"  {'N/A':>16}"
                row_t += f"  {t:>16.4f}" if t else f"  {'N/A':>16}"
                if s and t:
                    gap = (s / t - 1) * 100
                    row_g += f"  {gap:>+15.1f}%"
                else:
                    row_g += f"  {'N/A':>16}"

            print(f"\n{op['name']}:")
            print(row_s)
            print(row_t)
            print(row_g)

    # Step 4: Cross-batch summary for each (op, size) pair
    if len(batch_sizes) > 1:
        print("\n" + "=" * 90)
        print(f"Cross-batch summary: Gap% (Slang vs Tin2, negative = Slang faster)")
        print("=" * 90)

        for op in ops:
            print(f"\n{op['name']}:")
            header = f"  {'Size':<10}"
            for bs in batch_sizes:
                header += f"  {'b=' + str(bs):>10}"
            print(header)
            print("  " + "-" * (len(header) - 2))
            for size_name in sizes:
                row = f"  {size_name:<10}"
                for bs in batch_sizes:
                    r = all_results[op["name"]].get(bs, {}).get(size_name, {})
                    s, t = r.get("slang"), r.get("tin2")
                    if s and t:
                        gap = (s / t - 1) * 100
                        row += f"  {gap:>+9.1f}%"
                    else:
                        row += f"  {'N/A':>10}"
                print(row)

    # Save raw results
    output_path = "/tmp/decomposed_benchmark_results.json"
    with open(output_path, 'w') as f:
        json.dump(all_results, f, indent=2)
    print(f"\nRaw results saved to {output_path}")


if __name__ == "__main__":
    main()

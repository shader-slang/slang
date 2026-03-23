#!/usr/bin/env python3
"""
Run Slang vs Tin2 benchmark comparison across batch sizes and network sizes.
Supports both forward MMA and transpose MMA (W^T * dOut) modes.

Usage:
  python run_comparison.py                   # forward (default)
  python run_comparison.py --mode forward
  python run_comparison.py --mode transpose
"""

import argparse
import subprocess
import re
import sys
import os
import numpy as np

BATCH_SIZES = [256, 512, 1024, 2048, 4096, 8192]

NETWORK_CONFIGS = {
    "tiny": {"input": 32, "output": 16},
    "small": {"input": 64, "output": 16},
    "medium": {"input": 128, "output": 32},
    "large": {"input": 256, "output": 64},
    "xlarge": {"input": 128, "output": 128},
}

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)

MODE_CONFIG = {
    "forward": {
        "slang_script": "benchmark_single_layer_forward_new_mma.py",
        "tin2_bin": "bench_tin2_single_layer_forward",
        "title": "Forward MMA",
        "net_label": lambda cfg: f"{cfg['input']}->{cfg['output']}",
        "chart_file": "slang_vs_tin2_forward.png",
    },
    "transpose": {
        "slang_script": "benchmark_single_layer_transpose.py",
        "tin2_bin": "bench_tin2_single_layer_transpose",
        "title": "Transpose MMA (W^T * dOut)",
        "net_label": lambda cfg: f"{cfg['output']}->{cfg['input']}",
        "chart_file": "slang_vs_tin2_transpose.png",
    },
    "bias_reduce": {
        "slang_script": "benchmark_single_layer_bias_reduce.py",
        "tin2_bin": "bench_tin2_single_layer_bias_reduce",
        "title": "Bias Gradient Reduction",
        "net_label": lambda cfg: f"output={cfg['output']}",
        "chart_file": "slang_vs_tin2_bias_reduce.png",
    },
    "outer_product": {
        "slang_script": "benchmark_single_layer_outer_product.py",
        "tin2_bin": "bench_tin2_single_layer_outer_product",
        "title": "Outer Product (dW = dOut * input^T)",
        "net_label": lambda cfg: f"{cfg['output']}x{cfg['input']}",
        "chart_file": "slang_vs_tin2_outer_product.png",
    },
}

def run_slang(mode, size, batch_size):
    """Run Slang benchmark and return avg time in ms."""
    script = os.path.join(SCRIPT_DIR, MODE_CONFIG[mode]["slang_script"])
    cmd = [
        sys.executable, script,
        "--size", size,
        "--warps", "2",
        "--batch-size", str(batch_size),
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
        match = re.search(r'Avg time:\s+([\d.]+)\s*ms', result.stdout)
        if match:
            return float(match.group(1))
    except Exception as e:
        print(f"  Slang {size} batch={batch_size} FAILED: {e}")
    return None

def run_tin2(mode, size, batch_size):
    """Run Tin2 benchmark and return avg time in ms."""
    tin2_bin = os.path.join(REPO_ROOT, "tin2/benchmarks/mlp_perf/build", MODE_CONFIG[mode]["tin2_bin"])
    cmd = [
        tin2_bin,
        "--size", size,
        "--warps", "2",
        "--batch-size", str(batch_size),
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
        match = re.search(r'Avg time:\s+([\d.]+)\s*ms', result.stdout)
        if match:
            return float(match.group(1))
    except Exception as e:
        print(f"  Tin2 {size} batch={batch_size} FAILED: {e}")
    return None

def main():
    parser = argparse.ArgumentParser(description="Slang vs Tin2 MMA Benchmark Comparison")
    parser.add_argument("--mode", default="forward", choices=["forward", "transpose", "bias_reduce", "outer_product"],
                        help="Benchmark mode: forward (W*x), transpose (W^T*dOut), bias_reduce, or outer_product")
    args = parser.parse_args()

    mode = args.mode
    mcfg = MODE_CONFIG[mode]

    print("=" * 70)
    print(f"Slang vs Tin2 Comparison: {mcfg['title']}")
    print(f"All batch sizes x all network sizes")
    print("=" * 70)

    results = {}
    for size in NETWORK_CONFIGS:
        results[size] = {}
        for bs in BATCH_SIZES:
            net = mcfg["net_label"](NETWORK_CONFIGS[size])
            print(f"\nRunning {size} ({net}) batch={bs}...")
            slang_ms = run_slang(mode, size, bs)
            tin2_ms = run_tin2(mode, size, bs)
            results[size][bs] = {"slang": slang_ms, "tin2": tin2_ms}
            if slang_ms and tin2_ms:
                ratio = slang_ms / tin2_ms
                print(f"  Slang={slang_ms:.4f}ms  Tin2={tin2_ms:.4f}ms  Ratio={ratio:.2f}x")
            else:
                print(f"  Slang={slang_ms}  Tin2={tin2_ms}")

    # Print summary table
    print("\n" + "=" * 90)
    print(f"Summary Table ({mcfg['title']}): Slang time / Tin2 time (lower = Slang faster)")
    print("=" * 90)
    header = f"{'Size':<10}"
    for bs in BATCH_SIZES:
        header += f"{'batch=' + str(bs):>14}"
    print(header)
    print("-" * 90)

    for size in NETWORK_CONFIGS:
        row = f"{size:<10}"
        for bs in BATCH_SIZES:
            r = results[size][bs]
            if r["slang"] and r["tin2"]:
                ratio = r["slang"] / r["tin2"]
                row += f"{ratio:>14.2f}x"
            else:
                row += f"{'N/A':>14}"
        print(row)

    # Print raw times
    print("\n" + "=" * 90)
    print(f"Raw Times (ms) — {mcfg['title']}: Slang / Tin2")
    print("=" * 90)
    for size in NETWORK_CONFIGS:
        cfg = NETWORK_CONFIGS[size]
        net = mcfg["net_label"](cfg)
        print(f"\n{size} ({net}):")
        row_s = "  Slang: "
        row_t = "  Tin2:  "
        for bs in BATCH_SIZES:
            r = results[size][bs]
            row_s += f"{r['slang']:>12.4f}" if r["slang"] else f"{'N/A':>12}"
            row_t += f"{r['tin2']:>12.4f}" if r["tin2"] else f"{'N/A':>12}"
        print(f"  Batch:  " + "".join(f"{bs:>12}" for bs in BATCH_SIZES))
        print(row_s)
        print(row_t)

    # Generate plot
    try:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt

        fig, axes = plt.subplots(2, 3, figsize=(18, 10))
        fig.suptitle(f"Slang MMAHelperNew vs Tin2: {mcfg['title']} Benchmark\n(2 warps/block, checksum output)", fontsize=14)

        sizes = list(NETWORK_CONFIGS.keys())
        for idx, size in enumerate(sizes):
            ax = axes[idx // 3][idx % 3]
            cfg = NETWORK_CONFIGS[size]
            net = mcfg["net_label"](cfg)

            slang_times = []
            tin2_times = []
            bs_list = []

            for bs in BATCH_SIZES:
                r = results[size][bs]
                if r["slang"] is not None and r["tin2"] is not None:
                    bs_list.append(bs)
                    slang_times.append(r["slang"])
                    tin2_times.append(r["tin2"])

            if bs_list:
                ax.plot(bs_list, slang_times, 'o-', label='Slang', color='#2196F3', linewidth=2, markersize=6)
                ax.plot(bs_list, tin2_times, 's-', label='Tin2', color='#FF5722', linewidth=2, markersize=6)
                ax.set_xlabel('Batch Size')
                ax.set_ylabel('Time (ms)')
                ax.set_title(f"{size}: {net}")
                ax.legend()
                ax.grid(True, alpha=0.3)
                ax.set_xscale('log', base=2)

        axes[1][2].set_visible(False)

        ax = fig.add_subplot(2, 3, 6)
        for size in sizes:
            speedups = []
            bs_list = []
            for bs in BATCH_SIZES:
                r = results[size][bs]
                if r["slang"] is not None and r["tin2"] is not None:
                    bs_list.append(bs)
                    speedups.append(r["tin2"] / r["slang"])
            if bs_list:
                ax.plot(bs_list, speedups, 'o-', label=size, linewidth=2, markersize=5)

        ax.axhline(y=1.0, color='gray', linestyle='--', alpha=0.5)
        ax.set_xlabel('Batch Size')
        ax.set_ylabel('Speedup (Tin2 / Slang)')
        ax.set_title('Slang Speedup over Tin2')
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)
        ax.set_xscale('log', base=2)

        plt.tight_layout()
        out_path = os.path.join(SCRIPT_DIR, mcfg["chart_file"])
        plt.savefig(out_path, dpi=150)
        print(f"\nChart saved to: {out_path}")

    except ImportError:
        print("\nmatplotlib not available — skipping chart generation")

if __name__ == "__main__":
    main()

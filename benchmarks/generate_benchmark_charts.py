#!/usr/bin/env python3
"""
Generate comparison charts for Slang vs Tin2 backward pass benchmarks.

Reads the unified JSON from run_decomposed_benchmark.py:
  results[op_name][batch_size][size_name] = {"slang": ms, "tin2": ms}

Charts:
1. Decomposed: MMA Transpose, Outer Product, Bias Reduce (bar charts)
2. Full Backward: bar chart + ratio line
3. Component ratio overlay (xlarge)
"""

import json
import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
matplotlib.rcParams['font.size'] = 11

RESULTS_PATH = "/tmp/decomposed_benchmark_results.json"

NETWORK_SIZES = ["small", "medium", "large", "xlarge"]
NETWORK_LABELS = {
    "small": "64->16", "medium": "128->32",
    "large": "256->64", "xlarge": "128->128"
}

COMPONENT_CONFIGS = {
    "MMA Transpose": "mma_transpose",
    "Outer Product": "outer_product",
    "Bias Reduce": "bias_reduce",
    "Full Backward": "full_backward",
}


def load_results(path):
    with open(path) as f:
        return json.load(f)


def get_batch_sizes(data):
    for op in data.values():
        return sorted(int(b) for b in op.keys())
    return []


def create_grouped_bar_chart(fig, data, config_key, title, batch_sizes):
    n_batches = len(batch_sizes)
    n_networks = len(NETWORK_SIZES)
    ncols = min(n_batches, 3)
    nrows = (n_batches + ncols - 1) // ncols
    axes_grid = fig.subplots(nrows, ncols, sharey=False)
    if n_batches == 1:
        axes = [axes_grid]
    else:
        axes = axes_grid.flatten() if hasattr(axes_grid, 'flatten') else [axes_grid]
    for i in range(n_batches, len(axes)):
        axes[i].set_visible(False)
    fig.suptitle(title, fontsize=14, fontweight='bold')

    bar_width = 0.35
    x = np.arange(n_networks)

    op_data = data.get(config_key, {})

    for bi, (ax, batch) in enumerate(zip(axes, batch_sizes)):
        batch_str = str(batch)
        slang_vals = []
        tin2_vals = []

        for net in NETWORK_SIZES:
            r = op_data.get(batch_str, {}).get(net, {})
            slang_vals.append(r.get("slang") or 0)
            tin2_vals.append(r.get("tin2") or 0)

        slang_arr = np.array(slang_vals)
        tin2_arr = np.array(tin2_vals)

        ax.bar(x - bar_width/2, slang_arr, bar_width, label='Slang', color='#4C72B0', alpha=0.85)
        ax.bar(x + bar_width/2, tin2_arr, bar_width, label='Tin2', color='#DD8452', alpha=0.85)

        ax.set_title(f'batch={batch}', fontsize=10, fontweight='bold')
        ax.set_xticks(x)
        ax.set_xticklabels([NETWORK_LABELS[n] for n in NETWORK_SIZES], rotation=45, ha='right', fontsize=8)
        ax.set_ylabel('Time (ms)' if bi == 0 else '')
        ax.grid(axis='y', alpha=0.3)

        if bi == 0:
            ax.legend(fontsize=8)

        for i in range(n_networks):
            if tin2_arr[i] > 0 and slang_arr[i] > 0:
                ratio = slang_arr[i] / tin2_arr[i]
                max_h = max(slang_arr[i], tin2_arr[i])
                color = '#c0392b' if ratio > 1.1 else '#27ae60' if ratio < 0.95 else '#7f8c8d'
                ax.text(i, max_h * 1.01, f'{ratio:.2f}x', ha='center', va='bottom',
                        fontsize=7, color=color, fontweight='bold')
            ax.set_ylim(top=ax.get_ylim()[1] * 1.12)


def create_ratio_chart(ax, data, config_key, title, batch_sizes):
    colors = ['#4C72B0', '#DD8452', '#55A868', '#C44E52', '#8172B3']
    op_data = data.get(config_key, {})

    for ni, net in enumerate(NETWORK_SIZES):
        ratios = []
        bs_valid = []
        for batch in batch_sizes:
            batch_str = str(batch)
            r = op_data.get(batch_str, {}).get(net, {})
            sv, tv = r.get("slang"), r.get("tin2")
            if sv and tv and tv > 0:
                ratios.append(sv / tv)
                bs_valid.append(batch)
        if bs_valid:
            ax.plot(bs_valid, ratios, 'o-', label=f'{NETWORK_LABELS[net]}',
                    color=colors[ni], linewidth=2, markersize=6)

    ax.axhline(y=1.0, color='gray', linestyle='--', alpha=0.5, label='Parity')
    ax.set_xlabel('Batch Size')
    ax.set_ylabel('Slang / Tin2 Ratio')
    ax.set_title(title, fontsize=13, fontweight='bold')
    ax.legend(fontsize=8, ncol=2)
    ax.set_xscale('log', base=2)
    ax.set_xticks(batch_sizes)
    ax.set_xticklabels([str(b) for b in batch_sizes])
    ax.grid(alpha=0.3)
    ax.set_ylim(0.5, 2.0)


def main():
    results_path = sys.argv[1] if len(sys.argv) > 1 else RESULTS_PATH
    if not os.path.exists(results_path):
        print(f"Results file not found: {results_path}")
        print(f"Run: python3 benchmarks/run_decomposed_benchmark.py --all-batches")
        sys.exit(1)

    data = load_results(results_path)
    batch_sizes = get_batch_sizes(data)
    print(f"Loaded results: {list(data.keys())}, batch sizes: {batch_sizes}")

    # Chart 1: Decomposed components
    for comp_name, config_key in [
        ("MMA Transpose", "mma_transpose"),
        ("Outer Product", "outer_product"),
        ("Bias Reduce", "bias_reduce"),
    ]:
        if config_key not in data:
            continue
        ncols = min(len(batch_sizes), 3)
        nrows = (len(batch_sizes) + ncols - 1) // ncols
        fig = plt.figure(figsize=(6 * ncols, 5 * nrows))
        create_grouped_bar_chart(fig, data, config_key,
                                 f'Slang vs Tin2: {comp_name} (2-warp)', batch_sizes)
        fig.tight_layout(rect=[0, 0, 1, 0.95])
        fname = f'/tmp/backward_decomposed_{config_key}.png'
        fig.savefig(fname, dpi=150, bbox_inches='tight')
        print(f"Saved: {fname}")
        plt.close(fig)

    # Chart 2: Full backward bar chart
    if "full_backward" in data:
        ncols = min(len(batch_sizes), 3)
        nrows = (len(batch_sizes) + ncols - 1) // ncols
        fig2 = plt.figure(figsize=(6 * ncols, 5 * nrows))
        create_grouped_bar_chart(fig2, data, "full_backward",
                                 'Slang vs Tin2: Full Backward Pass (2-warp)', batch_sizes)
        fig2.tight_layout(rect=[0, 0, 1, 0.95])
        fig2.savefig('/tmp/backward_overall_comparison.png', dpi=150, bbox_inches='tight')
        print("Saved: /tmp/backward_overall_comparison.png")
        plt.close(fig2)

        # Chart 2b: Full backward ratio
        fig2b, ax2b = plt.subplots(figsize=(10, 6))
        create_ratio_chart(ax2b, data, "full_backward",
                          "Full Backward — Slang/Tin2 Ratio", batch_sizes)
        fig2b.tight_layout()
        fig2b.savefig('/tmp/backward_overall_ratio.png', dpi=150, bbox_inches='tight')
        print("Saved: /tmp/backward_overall_ratio.png")
        plt.close(fig2b)

    # Chart 3: Component ratio overlay (xlarge)
    fig3, ax3 = plt.subplots(figsize=(10, 6))
    fig3.suptitle('Slang/Tin2 Ratio: xlarge (128->128) by Component', fontsize=14, fontweight='bold')

    comp_colors = {'MMA Transpose': '#4C72B0', 'Outer Product': '#DD8452',
                   'Bias Reduce': '#55A868', 'Full Backward': '#C44E52'}

    for comp_name, config_key in COMPONENT_CONFIGS.items():
        if config_key not in data:
            continue
        op_data = data[config_key]
        ratios = []
        bs_valid = []
        for batch in batch_sizes:
            batch_str = str(batch)
            r = op_data.get(batch_str, {}).get("xlarge", {})
            sv, tv = r.get("slang"), r.get("tin2")
            if sv and tv and tv > 0:
                ratios.append(sv / tv)
                bs_valid.append(batch)
        if bs_valid:
            ax3.plot(bs_valid, ratios, 'o-', label=comp_name,
                     color=comp_colors[comp_name], linewidth=2.5, markersize=7)

    ax3.axhline(y=1.0, color='gray', linestyle='--', alpha=0.5, label='Parity')
    ax3.set_xlabel('Batch Size')
    ax3.set_ylabel('Slang / Tin2 Ratio')
    ax3.legend(fontsize=10)
    ax3.set_xscale('log', base=2)
    ax3.set_xticks(batch_sizes)
    ax3.set_xticklabels([str(b) for b in batch_sizes])
    ax3.grid(alpha=0.3)
    ax3.set_ylim(0.5, 2.0)

    fig3.tight_layout()
    fig3.savefig('/tmp/backward_ratio_by_component.png', dpi=150, bbox_inches='tight')
    print("Saved: /tmp/backward_ratio_by_component.png")

    # Chart 4: Stacked decomposed + full backward, one subfigure per network size
    # For each network, show grouped bars: (Slang decomposed, Tin2 decomposed, Slang full, Tin2 full)
    # Uses the largest batch size available.
    largest_batch = str(max(batch_sizes))
    op_names = ["mma_transpose", "outer_product", "bias_reduce", "full_backward"]
    op_labels = ["Transpose", "Outer Product", "Bias Reduce", "Full Backward"]

    n_nets = len(NETWORK_SIZES)
    fig4, axes4 = plt.subplots(1, n_nets, figsize=(5 * n_nets, 6), sharey=False)
    if n_nets == 1:
        axes4 = [axes4]
    fig4.suptitle(f'Decomposed + Full Backward (batch={largest_batch}, 2-warp)', fontsize=14, fontweight='bold')

    bar_width = 0.35
    x = np.arange(len(op_names))

    for ni, (ax, net) in enumerate(zip(axes4, NETWORK_SIZES)):
        slang_vals = []
        tin2_vals = []
        for op_key in op_names:
            r = data.get(op_key, {}).get(largest_batch, {}).get(net, {})
            slang_vals.append(r.get("slang") or 0)
            tin2_vals.append(r.get("tin2") or 0)

        slang_arr = np.array(slang_vals)
        tin2_arr = np.array(tin2_vals)

        ax.bar(x - bar_width/2, slang_arr, bar_width, label='Slang', color='#4C72B0', alpha=0.85)
        ax.bar(x + bar_width/2, tin2_arr, bar_width, label='Tin2', color='#DD8452', alpha=0.85)

        ax.set_title(f'{net} ({NETWORK_LABELS[net]})', fontsize=11, fontweight='bold')
        ax.set_xticks(x)
        ax.set_xticklabels(op_labels, rotation=30, ha='right', fontsize=9)
        ax.set_ylabel('Time (ms)' if ni == 0 else '')
        ax.grid(axis='y', alpha=0.3)

        if ni == 0:
            ax.legend(fontsize=9)

        for i in range(len(op_names)):
            if tin2_arr[i] > 0 and slang_arr[i] > 0:
                ratio = slang_arr[i] / tin2_arr[i]
                max_h = max(slang_arr[i], tin2_arr[i])
                color = '#c0392b' if ratio > 1.1 else '#27ae60' if ratio < 0.95 else '#7f8c8d'
                ax.text(i, max_h * 1.01, f'{ratio:.2f}x', ha='center', va='bottom',
                        fontsize=8, color=color, fontweight='bold')
        ax.set_ylim(top=ax.get_ylim()[1] * 1.12)

    fig4.tight_layout(rect=[0, 0, 1, 0.94])
    fig4.savefig('/tmp/backward_decomposed_and_full.png', dpi=150, bbox_inches='tight')
    print("Saved: /tmp/backward_decomposed_and_full.png")

    # Chart 5: Decomposed + full backward across ALL batch sizes
    # One subfigure per batch size, bars = operations, grouped by Slang/Tin2, for xlarge only
    op_names = ["mma_transpose", "outer_product", "bias_reduce", "full_backward"]
    op_labels = ["Transpose", "Outer Prod", "Bias Red", "Full Bwd"]
    ncols = min(len(batch_sizes), 3)
    nrows = (len(batch_sizes) + ncols - 1) // ncols
    fig5, axes5 = plt.subplots(nrows, ncols, figsize=(6 * ncols, 5 * nrows), sharey=False)
    axes_flat = axes5.flatten() if hasattr(axes5, 'flatten') else [axes5]
    for i in range(len(batch_sizes), len(axes_flat)):
        axes_flat[i].set_visible(False)
    fig5.suptitle('Decomposed + Full Backward: xlarge (128->128), Slang vs Tin2', fontsize=14, fontweight='bold')

    bar_width = 0.35
    x = np.arange(len(op_names))

    for bi, (ax, bs) in enumerate(zip(axes_flat, batch_sizes)):
        bs_str = str(bs)
        slang_vals = []
        tin2_vals = []
        for op_key in op_names:
            r = data.get(op_key, {}).get(bs_str, {}).get("xlarge", {})
            slang_vals.append(r.get("slang") or 0)
            tin2_vals.append(r.get("tin2") or 0)

        slang_arr = np.array(slang_vals)
        tin2_arr = np.array(tin2_vals)

        ax.bar(x - bar_width/2, slang_arr, bar_width, label='Slang', color='#4C72B0', alpha=0.85)
        ax.bar(x + bar_width/2, tin2_arr, bar_width, label='Tin2', color='#DD8452', alpha=0.85)

        ax.set_title(f'batch={bs}', fontsize=11, fontweight='bold')
        ax.set_xticks(x)
        ax.set_xticklabels(op_labels, rotation=30, ha='right', fontsize=9)
        ax.set_ylabel('Time (ms)' if bi % ncols == 0 else '')
        ax.grid(axis='y', alpha=0.3)
        if bi == 0:
            ax.legend(fontsize=9)

        for i in range(len(op_names)):
            if tin2_arr[i] > 0 and slang_arr[i] > 0:
                ratio = slang_arr[i] / tin2_arr[i]
                max_h = max(slang_arr[i], tin2_arr[i])
                color = '#c0392b' if ratio > 1.1 else '#27ae60' if ratio < 0.95 else '#7f8c8d'
                ax.text(i, max_h * 1.01, f'{ratio:.2f}x', ha='center', va='bottom',
                        fontsize=8, color=color, fontweight='bold')
        ax.set_ylim(top=ax.get_ylim()[1] * 1.12)

    fig5.tight_layout(rect=[0, 0, 1, 0.95])
    fig5.savefig('/tmp/backward_decomposed_by_batch.png', dpi=150, bbox_inches='tight')
    print("Saved: /tmp/backward_decomposed_by_batch.png")

    print("\nAll charts generated successfully!")


if __name__ == "__main__":
    main()

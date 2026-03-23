#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Bias reduce Phase 3 benchmark (all-warps path only).

The neural module now uses the all-warps bias reduce path only. This script
still runs twice per size (legacy "first warp" / "all warps" labels); both
columns use the same all-warps kernel. Useful for timing stability (median,
min, std) across sizes.
"""

import argparse
import math
import numpy as np
from pathlib import Path

try:
    import slangpy as spy
    from slangpy.core.calldata import SLANG_PATH
except ImportError:
    print("Error: slangpy not found")
    exit(1)

NETWORK_CONFIGS = {
    "tiny": {"input": 32, "output": 16},
    "small": {"input": 64, "output": 16},
    "medium": {"input": 128, "output": 32},
    "large": {"input": 256, "output": 64},
}

BATCH_SIZE = 256
WARMUP = 50
# Many iterations per run so total time is 10–100 ms (avoids timestamp quantization).
ITERATIONS_PER_RUN = 10000
RUNS = 30  # Runs per (size, strategy); min/median are usually more stable than mean on GPU.


def find_neural_module_dir():
    """Find the neural.slang-module directory."""
    slang_root = Path(__file__).resolve().parents[5]
    candidates = [
        slang_root / "build" / "Release" / "lib" / "slang-standard-module-2026.3.1",
        slang_root / "build" / "Debug" / "lib" / "slang-standard-module-2026.3.1",
    ]
    for candidate in candidates:
        neural_module = candidate / "slang" / "neural.slang-module"
        if neural_module.exists():
            return candidate
    return None


def linear_to_tiled(weights_linear: np.ndarray, output_size: int, input_size: int) -> np.ndarray:
    """Convert a weight matrix from linear (row-major) to tiled layout."""
    tile_size = 16
    n_tile_rows = math.ceil(output_size / tile_size)
    n_tile_cols = math.ceil(input_size / tile_size)
    total_elements = n_tile_rows * n_tile_cols * tile_size * tile_size

    w = weights_linear[: output_size * input_size].reshape(output_size, input_size)

    padded_m = n_tile_rows * tile_size
    padded_k = n_tile_cols * tile_size
    w_padded = np.zeros((padded_m, padded_k), dtype=w.dtype)
    w_padded[:output_size, :input_size] = w

    tiled = np.zeros(total_elements, dtype=w.dtype)
    for tr in range(n_tile_rows):
        for tc in range(n_tile_cols):
            tile_index = tr * n_tile_cols + tc
            tile_data = w_padded[
                tr * tile_size : (tr + 1) * tile_size,
                tc * tile_size : (tc + 1) * tile_size,
            ]
            tiled[tile_index * 256 : (tile_index + 1) * 256] = tile_data.ravel()

    return tiled


def run_one_size_one_strategy(
    device_type_str: str,
    size: str,
    use_first_warp_only: int,
    runs: int = RUNS,
    iterations_per_run: int = ITERATIONS_PER_RUN,
    warmup: int = WARMUP,
    verbose: bool = True,
):
    """
    Run benchmark for one network size and one strategy.
    Returns (mean_ms, std_ms) or (None, None) on failure.
    """
    config = NETWORK_CONFIGS[size]
    input_size = config["input"]
    output_size = config["output"]

    tile_size = 16
    n_tile_rows = math.ceil(output_size / tile_size)
    n_tile_cols = math.ceil(input_size / tile_size)
    tiled_weight_count = n_tile_rows * n_tile_cols * 256
    total_params = tiled_weight_count + output_size

    neural_module_dir = find_neural_module_dir()
    if not neural_module_dir:
        if verbose:
            print("ERROR: neural.slang-module not found")
        return None, None

    device_type_map = {
        "vulkan": spy.DeviceType.vulkan,
        "cuda": spy.DeviceType.cuda,
    }
    device_type = device_type_map[device_type_str.lower()]
    test_dir = Path(__file__).resolve().parent

    defines = {
        "INPUT_SIZE": str(input_size),
        "OUTPUT_SIZE": str(output_size),
    }

    include_paths = [str(test_dir), str(neural_module_dir), SLANG_PATH]

    compiler_options = spy.SlangCompilerOptions({
        "include_paths": include_paths,
        "debug_info": spy.SlangDebugInfoLevel.standard,
        "defines": defines,
    })

    try:
        device = spy.Device(
            type=device_type,
            enable_debug_layers=True,
            compiler_options=compiler_options,
            bindless_options=spy.BindlessDesc(buffer_count=1000),
        )

        module = device.load_module("benchmark_single_layer_backward_tiled.slang")

        program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_single_layer_backward_tiled")],
        )
        kernel = device.create_compute_kernel(program)

        rng = np.random.default_rng(42)

        linear_weight_count = input_size * output_size
        weights_linear = rng.standard_normal(linear_weight_count).astype(np.float16) * 0.1
        bias_data = rng.standard_normal(output_size).astype(np.float16) * 0.1

        weights_tiled = linear_to_tiled(weights_linear, output_size, input_size)

        params_data = np.concatenate([weights_tiled, bias_data])
        inputs_data = rng.standard_normal(BATCH_SIZE * input_size).astype(np.float16) * 0.1
        grad_outputs_data = rng.standard_normal(BATCH_SIZE * output_size).astype(np.float16) * 0.1

        params_buf = device.create_buffer(
            data=params_data,
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )
        inputs_buf = device.create_buffer(
            data=inputs_data,
            usage=spy.BufferUsage.shader_resource,
        )
        grad_outputs_buf = device.create_buffer(
            data=grad_outputs_data,
            usage=spy.BufferUsage.shader_resource,
        )
        grad_params_buf = device.create_buffer(
            data=np.zeros(total_params, dtype=np.float16),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )
        grad_inputs_buf = device.create_buffer(
            data=np.zeros(BATCH_SIZE * input_size, dtype=np.float16),
            usage=spy.BufferUsage.unordered_access,
        )

        thread_count = [BATCH_SIZE * 32, 1, 1]

        if device_type == spy.DeviceType.vulkan:
            params_arg = params_buf.descriptor_handle_rw
            grad_params_arg = grad_params_buf.descriptor_handle_rw
        else:
            params_arg = params_buf
            grad_params_arg = grad_params_buf

        # Warmup
        for _ in range(warmup):
            kernel.dispatch(
                thread_count=thread_count,
                params=params_arg,
                inputs=inputs_buf,
                grad_outputs=grad_outputs_buf,
                grad_params=grad_params_arg,
                grad_inputs=grad_inputs_buf,
                batch_size=BATCH_SIZE,
            )
        device.wait()

        # Multiple runs, each with its own timestamp range
        run_times_ms = []
        query_pool = device.create_query_pool(type=spy.QueryType.timestamp, count=2)
        frequency = float(device.info.timestamp_frequency)

        for run in range(runs):
            command_encoder = device.create_command_encoder()
            command_encoder.write_timestamp(query_pool, 0)

            for _ in range(iterations_per_run):
                kernel.dispatch(
                    thread_count=thread_count,
                    command_encoder=command_encoder,
                    params=params_arg,
                    inputs=inputs_buf,
                    grad_outputs=grad_outputs_buf,
                    grad_params=grad_params_arg,
                    grad_inputs=grad_inputs_buf,
                    batch_size=BATCH_SIZE,
                )

            command_encoder.write_timestamp(query_pool, 1)
            device.submit_command_buffer(command_encoder.finish())
            device.wait()

            timestamps = np.array(query_pool.get_results(0, 2))
            elapsed_ticks = timestamps[1] - timestamps[0]
            total_time_ms = (elapsed_ticks / frequency) * 1000
            avg_time_ms = total_time_ms / iterations_per_run
            run_times_ms.append(avg_time_ms)

        device.close()

        run_times_ms = np.array(run_times_ms)
        return {
            "min": float(np.min(run_times_ms)),
            "median": float(np.median(run_times_ms)),
            "mean": float(np.mean(run_times_ms)),
            "std": float(np.std(run_times_ms)),
            "freq_hz": frequency,
        }

    except Exception as e:
        if verbose:
            print(f"FAILED: {e}")
            import traceback
            traceback.print_exc()
        return None


def main():
    parser = argparse.ArgumentParser(
        description="Compare bias reduce strategies (first-warp-only vs all-warps)"
    )
    parser.add_argument("--device", default="cuda", choices=["cuda", "vulkan"])
    parser.add_argument("--size", default=None, choices=list(NETWORK_CONFIGS.keys()) + [None],
                        help="Single size or omit for all")
    parser.add_argument("--runs", type=int, default=RUNS, help=f"Runs per (size, strategy), default {RUNS}")
    parser.add_argument("--iterations", type=int, default=ITERATIONS_PER_RUN,
                        help=f"Iterations per run, default {ITERATIONS_PER_RUN}")
    parser.add_argument("--quiet", action="store_true", help="Less per-size output")
    args = parser.parse_args()

    sizes = [args.size] if args.size else list(NETWORK_CONFIGS.keys())

    print("=" * 72)
    print("Bias reduce Phase 3 strategy benchmark (first-warp-only vs all-warps)")
    print("=" * 72)
    print(f"Batch size: {BATCH_SIZE}")
    print(f"Runs: {args.runs} x {args.iterations} iterations per (size, strategy)")
    print(f"Backend: {args.device}")
    print()

    results = {}  # (size, strategy) -> dict with min, median, mean, std, freq_hz

    for size in sizes:
        config = NETWORK_CONFIGS[size]
        net_str = f"{config['input']} -> {config['output']}"
        if not args.quiet:
            print(f"  {size}: {net_str} ... ", end="", flush=True)

        r1 = run_one_size_one_strategy(
            args.device, size, use_first_warp_only=1,
            runs=args.runs, iterations_per_run=args.iterations, warmup=WARMUP, verbose=not args.quiet
        )
        r0 = run_one_size_one_strategy(
            args.device, size, use_first_warp_only=0,
            runs=args.runs, iterations_per_run=args.iterations, warmup=WARMUP, verbose=not args.quiet
        )

        results[(size, 1)] = r1
        results[(size, 0)] = r0

        if r1 and r0 and not args.quiet:
            med1, med0 = r1["median"], r0["median"]
            winner = "first_warp" if med1 < med0 else "all_warps"
            diff_pct = 100.0 * (med1 - med0) / med0 if med0 else 0
            print(f"first_warp med={med1:.4f} min={r1['min']:.4f} ms, all_warps med={med0:.4f} min={r0['min']:.4f} ms -> {winner} ({diff_pct:+.1f}%)")

    # Summary table (median is more stable than mean on GPU; min = best run)
    print()
    print("=" * 90)
    print("Summary — median (min) ms, mean ± std; winner by median")
    print("=" * 90)
    print(f"{'Size':<8} {'Net':<8} {'First warp':<24} {'All warps':<24} {'Faster':<10} {'Diff %':<8}")
    print("-" * 90)

    for size in sizes:
        config = NETWORK_CONFIGS[size]
        net_str = f"{config['input']}->{config['output']}"
        r1 = results.get((size, 1))
        r0 = results.get((size, 0))

        if r1 is None or r0 is None:
            print(f"{size:<8} {net_str:<8} {'N/A':<24} {'N/A':<24} FAIL")
            continue

        str1 = f"{r1['median']:.4f}({r1['min']:.4f}) {r1['mean']:.4f}±{r1['std']:.4f}"
        str0 = f"{r0['median']:.4f}({r0['min']:.4f}) {r0['mean']:.4f}±{r0['std']:.4f}"
        med1, med0 = r1["median"], r0["median"]
        faster = "first_warp" if med1 < med0 else "all_warps"
        diff_pct = 100.0 * (med1 - med0) / med0 if med0 else 0
        print(f"{size:<8} {net_str:<8} {str1:<24} {str0:<24} {faster:<10} {diff_pct:+.1f}%")

    # Print timestamp resolution hint once (from last successful run)
    for size in sizes:
        r = results.get((size, 1)) or results.get((size, 0))
        if r and "freq_hz" in r:
            res_us = 1e6 / r["freq_hz"]
            print()
            print(f"Timestamp frequency: {r['freq_hz']:.0f} Hz -> ~{res_us:.2f} µs resolution.")
            print("If std is still 0 or differences tiny, try: --iterations 50000 --runs 20")
            break

    print()
    print("Use first_warp_only for large M when it wins; branch by size in sumReduceRows if needed.")
    print("=" * 90)


if __name__ == "__main__":
    main()

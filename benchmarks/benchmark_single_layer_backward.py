#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Single Layer Backward Benchmark (chained: transpose MMA + outer product + bias reduce)

Measures the full backward pass of a single linear layer.
Uses MMAHelperNew with TransposeA for dIn, outerProductAccumulate for dW,
sumReduceRowsV2 for dBias.
"""

import argparse
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
    "xlarge": {"input": 128, "output": 128},
}

DEFAULT_BATCH_SIZE = 8192
WARMUP = 100
ITERATIONS = 1000


def find_neural_module_dir():
    script_dir = Path(__file__).resolve().parent
    for parent in [script_dir] + list(script_dir.parents):
        build_dir = parent / "build"
        if build_dir.is_dir():
            for candidate in sorted(build_dir.rglob("neural.slang-module")):
                return candidate.parent.parent
            break
    return None


def run_benchmark(device_type_str: str, size: str, num_warps: int = 2, batch_size: int = DEFAULT_BATCH_SIZE):
    config = NETWORK_CONFIGS[size]
    input_size = config["input"]
    output_size = config["output"]

    n_tile_rows = (output_size + 15) // 16
    n_tile_cols = (input_size + 15) // 16
    tiled_weight_count = n_tile_rows * n_tile_cols * 256

    print(f"\n{'='*70}")
    print(f"Backward: {input_size} -> {output_size}, batch={batch_size}")
    print(f"{'='*70}")

    neural_module_dir = find_neural_module_dir()
    if not neural_module_dir:
        print("ERROR: neural.slang-module not found")
        return None

    device_type_map = {"cuda": spy.DeviceType.cuda}
    if device_type_str.lower() not in device_type_map:
        print(f"ERROR: Only CUDA supported, got '{device_type_str}'")
        return None
    device_type = device_type_map[device_type_str.lower()]
    test_dir = Path(__file__).resolve().parent

    defines = {
        "INPUT_SIZE": str(input_size),
        "OUTPUT_SIZE": str(output_size),
        "SUBGROUP_COUNT": str(num_warps),
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

        module = device.load_module("benchmark_single_layer_backward.slang")
        program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_backward")]
        )
        kernel = device.create_compute_kernel(program)

        threads_per_group = 32 * num_warps
        num_groups = batch_size // num_warps
        thread_count = [num_groups * threads_per_group, 1, 1]

        rng = np.random.default_rng(42)

        params_data = (rng.standard_normal(tiled_weight_count) * 0.1).astype(np.float16)
        params_buf = device.create_buffer(
            data=params_data,
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )
        outputs_buf = device.create_buffer(
            data=np.zeros(batch_size * 32, dtype=np.float16),
            usage=spy.BufferUsage.unordered_access,
        )
        weight_grad_buf = device.create_buffer(
            data=np.zeros(tiled_weight_count, dtype=np.float16),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )
        bias_grad_buf = device.create_buffer(
            data=np.zeros(output_size, dtype=np.float16),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )

        print(f"Warming up ({WARMUP} iterations)...")
        for _ in range(WARMUP):
            kernel.dispatch(
                thread_count=thread_count,
                params=params_buf,
                outputs=outputs_buf,
                weightGrad=weight_grad_buf,
                biasGrad=bias_grad_buf,
                batch_size=batch_size,
            )
        device.wait()

        print(f"Benchmarking ({ITERATIONS} iterations)...")
        query_pool = device.create_query_pool(type=spy.QueryType.timestamp, count=2)
        command_encoder = device.create_command_encoder()
        command_encoder.write_timestamp(query_pool, 0)

        for _ in range(ITERATIONS):
            kernel.dispatch(
                thread_count=thread_count,
                command_encoder=command_encoder,
                params=params_buf,
                outputs=outputs_buf,
                weightGrad=weight_grad_buf,
                biasGrad=bias_grad_buf,
                batch_size=batch_size,
            )

        command_encoder.write_timestamp(query_pool, 1)
        device.submit_command_buffer(command_encoder.finish())
        device.wait()

        timestamps = np.array(query_pool.get_results(0, 2))
        frequency = float(device.info.timestamp_frequency)
        elapsed_ticks = timestamps[1] - timestamps[0]
        total_time_ms = (elapsed_ticks / frequency) * 1000
        avg_time_ms = total_time_ms / ITERATIONS

        print(f"\nResults:")
        print(f"  Backend: {device_type_str}")
        print(f"  Network: {input_size} -> {output_size}")
        print(f"  Avg time: {avg_time_ms:.4f} ms")

        device.close()
        return avg_time_ms

    except Exception as e:
        print(f"FAILED: {e}")
        import traceback
        traceback.print_exc()
        return None


def main():
    parser = argparse.ArgumentParser(description="Single Layer Backward Benchmark (chained)")
    parser.add_argument("--device", default="cuda", choices=["cuda"])
    parser.add_argument("--size", default="xlarge", choices=list(NETWORK_CONFIGS.keys()))
    parser.add_argument("--all", action="store_true", help="Run all sizes")
    parser.add_argument("--warps", type=int, default=2, choices=[1, 2])
    parser.add_argument("--batch-size", type=int, default=DEFAULT_BATCH_SIZE)
    args = parser.parse_args()

    print("=" * 70)
    print("Single Layer Backward Benchmark (transpose MMA + outer product + bias reduce)")
    print("=" * 70)
    print(f"Batch size: {args.batch_size}")
    print(f"Warmup: {WARMUP}, Iterations: {ITERATIONS}")
    print(f"Warps per workgroup: {args.warps}")

    if args.all:
        results = {}
        for size in NETWORK_CONFIGS.keys():
            time_ms = run_benchmark(args.device, size, args.warps, args.batch_size)
            if time_ms:
                results[size] = time_ms

        print("\n" + "=" * 70)
        print("Summary: Single Layer Backward")
        print("=" * 70)
        print(f"{'Size':<10} {'Network':<15} {'Time (ms)':<12} {'Status'}")
        print("-" * 50)
        for size, config in NETWORK_CONFIGS.items():
            net = f"{config['input']}->{config['output']}"
            if size in results:
                print(f"{size:<10} {net:<15} {results[size]:<12.4f} OK")
            else:
                print(f"{size:<10} {net:<15} {'N/A':<12} FAILED")
    else:
        run_benchmark(args.device, args.size, args.warps, args.batch_size)


if __name__ == "__main__":
    main()

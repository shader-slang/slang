#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Single Layer Outer Product Benchmark

Measures outerProductAccumulate: dW[m][k] = sum_{all lanes} dOut[m] * input[k]
Uses MMAHelperNew.outerProductAccumulate (FromLocalColumn on MatA + FromLocalRow on MatB).
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
    "xlarge": {"input": 128, "output": 128},
}

DEFAULT_BATCH_SIZE = 256
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


def run_benchmark(device_type_str: str, size: str, num_warps: int = 1, batch_size: int = DEFAULT_BATCH_SIZE, validate: bool = False):
    config = NETWORK_CONFIGS[size]
    input_size = config["input"]
    output_size = config["output"]

    tile_size = 16
    n_tile_rows = math.ceil(output_size / tile_size)
    n_tile_cols = math.ceil(input_size / tile_size)
    tiled_weight_count = n_tile_rows * n_tile_cols * 256

    print(f"\n{'='*70}")
    print(f"Outer Product (MMAHelperNew): {output_size}x{input_size}, batch={batch_size}")
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
    if validate:
        defines["RES_VALIDATION"] = "1"

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

        module = device.load_module("benchmark_single_layer_outer_product.slang")

        # Timing kernel
        program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_outer_product")]
        )
        kernel = device.create_compute_kernel(program)

        threads_per_group = 32 * num_warps
        num_groups = batch_size // num_warps
        thread_count = [num_groups * threads_per_group, 1, 1]

        total_tiles = n_tile_rows * n_tile_cols

        if validate:
            # Layout conversion kernel
            convert_program = device.link_program(
                modules=[module],
                entry_points=[module.entry_point("convert_native_to_rowmajor")]
            )
            convert_kernel = device.create_compute_kernel(convert_program)

            val_batch_simple = num_warps
            val_tc_simple = [threads_per_group, 1, 1]
            native_wgrad_buf = device.create_buffer(
                data=np.zeros(tiled_weight_count, dtype=np.float16),
                usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
            )
            rowmajor_buf = device.create_buffer(
                data=np.zeros(total_tiles * 256, dtype=np.float16),
                usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
            )

            kernel.dispatch(
                thread_count=val_tc_simple,
                weightGradBuffer=native_wgrad_buf,
                batch_size=val_batch_simple,
            )
            device.wait()

            convert_kernel.dispatch(
                thread_count=[total_tiles * 32, 1, 1],
                nativeBuffer=native_wgrad_buf,
                rowMajorBuffer=rowmajor_buf,
            )
            device.wait()

            gpu_rowmajor = rowmajor_buf.to_numpy().view(np.float16).astype(np.float32)

            subgroup_size = 32
            thread_sum_sq = sum(((t + 1) / subgroup_size) ** 2 for t in range(subgroup_size))
            dout_scale = 16.0 / output_size
            input_scale = 16.0 / input_size
            factor = num_warps * thread_sum_sq * dout_scale * input_scale

            expected_flat = np.zeros(total_tiles * 256, dtype=np.float32)
            tile_idx = 0
            for tr in range(n_tile_rows):
                for tc in range(n_tile_cols):
                    for r in range(16):
                        for c in range(16):
                            m = tr * 16 + r
                            k = tc * 16 + c
                            if m < output_size and k < input_size:
                                expected_flat[tile_idx * 256 + r * 16 + c] = float(m + 1) * float(k + 1) * factor
                    tile_idx += 1

            max_err = np.max(np.abs(gpu_rowmajor - expected_flat))
            nonzero_mask = expected_flat != 0
            max_rel_err = np.max(np.abs((gpu_rowmajor[nonzero_mask] - expected_flat[nonzero_mask]) / expected_flat[nonzero_mask])) if np.any(nonzero_mask) else 0

            first_tile = gpu_rowmajor[:256].reshape(16, 16)
            expected_tile = expected_flat[:256].reshape(16, 16)

            print(f"Validation:")
            print(f"  GPU     tile row 0: {first_tile[0]}")
            print(f"  Expected tile row 0: {expected_tile[0]}")
            print(f"  Max absolute error: {max_err:.4f}")
            print(f"  Max relative error: {max_rel_err:.6f}")
            if max_rel_err < 0.01:
                print(f"  PASS")
            else:
                print(f"  FAIL")

        # Timing
        weight_grad_buf_timing = device.create_buffer(
            data=np.zeros(tiled_weight_count, dtype=np.float16),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )

        print(f"Warming up ({WARMUP} iterations)...")
        for _ in range(WARMUP):
            kernel.dispatch(
                thread_count=thread_count,
                weightGradBuffer=weight_grad_buf_timing,
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
                weightGradBuffer=weight_grad_buf_timing,
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
        print(f"  Outer product: {output_size}x{input_size}")
        print(f"  Avg time: {avg_time_ms:.4f} ms")

        device.close()
        return avg_time_ms

    except Exception as e:
        print(f"FAILED: {e}")
        import traceback
        traceback.print_exc()
        return None


def main():
    parser = argparse.ArgumentParser(description="Outer Product Benchmark (MMAHelperNew)")
    parser.add_argument("--device", default="cuda", choices=["cuda"])
    parser.add_argument("--size", default="small", choices=list(NETWORK_CONFIGS.keys()))
    parser.add_argument("--all", action="store_true", help="Run all sizes")
    parser.add_argument("--warps", type=int, default=1, choices=[1, 2])
    parser.add_argument("--batch-size", type=int, default=DEFAULT_BATCH_SIZE)
    parser.add_argument("--val", action="store_true", help="Run result validation")
    args = parser.parse_args()

    batch_size = args.batch_size

    print("="*70)
    print("Outer Product Benchmark (MMAHelperNew / outerProductAccumulate)")
    print("="*70)
    print(f"Batch size: {batch_size}")
    print(f"Warmup: {WARMUP}, Iterations: {ITERATIONS}")
    print(f"Warps per workgroup: {args.warps}")
    if args.val:
        print("Validation: ENABLED")

    if args.all:
        results = {}
        for size in NETWORK_CONFIGS.keys():
            time_ms = run_benchmark(args.device, size, args.warps, batch_size, validate=args.val)
            if time_ms:
                results[size] = time_ms

        print("\n" + "="*70)
        print("Summary: Outer Product (MMAHelperNew)")
        print("="*70)
        print(f"{'Size':<10} {'Dims':<15} {'Time (ms)':<12} {'Status'}")
        print("-"*50)
        for size, config in NETWORK_CONFIGS.items():
            dims = f"{config['output']}x{config['input']}"
            if size in results:
                print(f"{size:<10} {dims:<15} {results[size]:<12.4f} OK")
            else:
                print(f"{size:<10} {dims:<15} {'N/A':<12} FAILED")
    else:
        run_benchmark(args.device, args.size, args.warps, batch_size, validate=args.val)


if __name__ == "__main__":
    main()

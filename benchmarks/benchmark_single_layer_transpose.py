#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Single Layer Transpose MMA Benchmark — dIn = W^T * dOut

Uses MMAHelperNew with TransposeA=true (zero shared memory, warp shuffles).
Weight matrix is OUTPUT_SIZE x INPUT_SIZE (same layout as forward).

Network sizes (same as forward, but operation is transposed):
- tiny:  32 -> 16  (transpose: 16 -> 32)
- small: 64 -> 16  (transpose: 16 -> 64)
- medium: 128 -> 32  (transpose: 32 -> 128)
- large: 256 -> 64  (transpose: 64 -> 256)
- xlarge: 128 -> 128 (transpose: 128 -> 128)
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
    """Find the directory to add to include paths so that 'import slang.neural' resolves."""
    script_dir = Path(__file__).resolve().parent
    for parent in [script_dir] + list(script_dir.parents):
        build_dir = parent / "build"
        if build_dir.is_dir():
            for candidate in sorted(build_dir.rglob("neural.slang-module")):
                return candidate.parent.parent
            break
    return None


def linear_to_tiled(weights_linear: np.ndarray, output_size: int, input_size: int) -> np.ndarray:
    """Convert a weight matrix from linear (row-major) to tiled layout."""
    tile_size = 16
    n_tile_rows = math.ceil(output_size / tile_size)
    n_tile_cols = math.ceil(input_size / tile_size)
    total_elements = n_tile_rows * n_tile_cols * tile_size * tile_size

    w = weights_linear[:output_size * input_size].reshape(output_size, input_size)

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


def run_benchmark(device_type_str: str, size: str, num_warps: int = 1, batch_size: int = DEFAULT_BATCH_SIZE, val: bool = False):
    """Run single-layer transpose MMA benchmark."""
    config = NETWORK_CONFIGS[size]
    input_size = config["input"]
    output_size = config["output"]

    tile_size = 16
    n_tile_rows = math.ceil(output_size / tile_size)
    n_tile_cols = math.ceil(input_size / tile_size)
    tiled_weight_count = n_tile_rows * n_tile_cols * 256

    print(f"\n{'='*70}")
    print(f"Single Layer Transpose (MMAHelperNew): W^T * dOut, {output_size} -> {input_size}, batch={batch_size}")
    print(f"Tiled weights: {tiled_weight_count} (weight matrix: {output_size}x{input_size})")
    print(f"{'='*70}")

    neural_module_dir = find_neural_module_dir()
    if not neural_module_dir:
        print("ERROR: neural.slang-module not found")
        return None
    print(f"Neural module include path: {neural_module_dir}")

    device_type_map = {"cuda": spy.DeviceType.cuda}
    if device_type_str.lower() not in device_type_map:
        print(f"ERROR: This benchmark only supports CUDA, got '{device_type_str}'")
        return None
    device_type = device_type_map[device_type_str.lower()]
    test_dir = Path(__file__).resolve().parent

    defines = {
        "INPUT_SIZE": str(input_size),
        "OUTPUT_SIZE": str(output_size),
        "SUBGROUP_COUNT": str(num_warps),
    }
    if val:
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

        module = device.load_module("benchmark_single_layer_transpose.slang")

        program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_single_layer_transpose")]
        )
        kernel = device.create_compute_kernel(program)

        convert_program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("convert_tiled_to_native")]
        )
        convert_kernel = device.create_compute_kernel(convert_program)

        rng = np.random.default_rng(42)

        linear_weight_count = input_size * output_size
        weights_linear = rng.standard_normal(linear_weight_count).astype(np.float16) * 0.1

        weights_tiled = linear_to_tiled(weights_linear, output_size, input_size)

        tiled_weights_buf = device.create_buffer(
            data=weights_tiled,
            usage=spy.BufferUsage.shader_resource,
        )

        native_params = np.zeros(tiled_weight_count, dtype=np.float16)
        native_params_buf = device.create_buffer(
            data=native_params,
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )

        num_tiles = n_tile_rows * n_tile_cols
        print(f"Converting {num_tiles} tiles from tiled to native layout...")
        convert_kernel.dispatch(
            thread_count=[num_tiles * 32, 1, 1],
            tiledWeights=tiled_weights_buf,
            nativeWeights=native_params_buf,
        )
        device.wait()

        outputs_buf = device.create_buffer(
            data=np.zeros(batch_size * 32, dtype=np.float16),
            usage=spy.BufferUsage.unordered_access,
        )

        threads_per_group = 32 * num_warps
        num_groups = batch_size // num_warps
        thread_count = [num_groups * threads_per_group, 1, 1]

        params_arg = native_params_buf

        # CPU reference: dIn = W^T * dOut
        # W is output_size x input_size, dOut = [1, 2, ..., output_size]
        W = weights_linear[:output_size * input_size].reshape(output_size, input_size).astype(np.float16)
        dOut = np.arange(1, output_size + 1, dtype=np.float16)
        cpu_dIn = (W.T.astype(np.float32) @ dOut.astype(np.float32)).astype(np.float16)
        cpu_checksum = np.sum(cpu_dIn.astype(np.float32)).astype(np.float16)

        kernel.dispatch(
            thread_count=thread_count,
            params=params_arg,
            outputs=outputs_buf,
            batch_size=batch_size,
        )
        device.wait()

        if val:
            gpu_dIn = outputs_buf.to_numpy().view(np.float16)[:input_size]
            max_err = 0.0
            fail_count = 0
            for i in range(input_size):
                g = float(gpu_dIn[i])
                c = float(cpu_dIn[i])
                e = abs(g - c)
                max_err = max(max_err, e)
                mark = "  <-- FAIL" if e > max(abs(c) * 0.05, 0.01) else ""
                if mark:
                    fail_count += 1
                if input_size <= 64 or mark:
                    print(f"  [{i:3d}] GPU={g:10.4f}  CPU={c:10.4f}  err={e:.4f}{mark}")
            gpu_checksum = np.sum(gpu_dIn.astype(np.float32)).astype(np.float16)
            print(f"  GPU checksum: {float(gpu_checksum):.4f}  CPU checksum: {float(cpu_checksum):.4f}")
            print(f"  Max element error: {max_err:.4f}, Failed elements: {fail_count}/{input_size}")
            if fail_count == 0:
                print(f"  PASS")
            else:
                print(f"  FAIL")
        else:
            gpu_checksums = outputs_buf.to_numpy().view(np.float16)[:32]
            gpu_checksum = gpu_checksums[0]
            err = abs(float(gpu_checksum) - float(cpu_checksum))
            print(f"Validation (sample 0, thread 0 checksum):")
            print(f"  GPU checksum: {float(gpu_checksum):.4f}")
            print(f"  CPU checksum: {float(cpu_checksum):.4f}")
            print(f"  Error: {err:.4f}")
            if err > abs(float(cpu_checksum)) * 0.1 + 1.0:
                print(f"  FAIL")
            else:
                print(f"  PASS")

        print(f"Warming up ({WARMUP} iterations)...")
        for _ in range(WARMUP):
            kernel.dispatch(
                thread_count=thread_count,
                params=params_arg,
                outputs=outputs_buf,
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
                params=params_arg,
                outputs=outputs_buf,
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
        throughput = batch_size / (avg_time_ms / 1000)

        print(f"\nResults:")
        print(f"  Backend: {device_type_str}")
        print(f"  Transpose: {output_size} -> {input_size} (W^T * dOut)")
        print(f"  Avg time: {avg_time_ms:.4f} ms")
        print(f"  Throughput: {throughput:.0f} samples/s")

        device.close()
        return avg_time_ms

    except Exception as e:
        print(f"FAILED: {e}")
        import traceback
        traceback.print_exc()
        return None


def main():
    parser = argparse.ArgumentParser(description="Single Layer Transpose MMA Benchmark")
    parser.add_argument("--device", default="cuda", choices=["cuda"])
    parser.add_argument("--size", default="small", choices=list(NETWORK_CONFIGS.keys()))
    parser.add_argument("--all", action="store_true", help="Run all sizes")
    parser.add_argument("--warps", type=int, default=1, choices=[1, 2], help="Number of warps per workgroup (1 or 2)")
    parser.add_argument("--batch-size", type=int, default=DEFAULT_BATCH_SIZE, help="Batch size (number of samples)")
    parser.add_argument("--val", action="store_true", help="Enable per-element validation against CPU reference")
    args = parser.parse_args()

    batch_size = args.batch_size

    print("="*70)
    print("Single Layer Transpose MMA Benchmark (MMAHelperNew / TransposeA)")
    print("="*70)
    print(f"Batch size: {batch_size}")
    print(f"Warmup: {WARMUP}, Iterations: {ITERATIONS}")
    print(f"Backend: {args.device}")
    print(f"Warps per workgroup: {args.warps}")

    if args.all:
        results = {}
        for size in NETWORK_CONFIGS.keys():
            time_ms = run_benchmark(args.device, size, args.warps, batch_size, args.val)
            if time_ms:
                results[size] = time_ms

        print("\n" + "="*70)
        print("Summary: Single Layer Transpose (MMAHelperNew / TransposeA)")
        print("="*70)
        print(f"{'Size':<10} {'Transpose':<15} {'Time (ms)':<12} {'Status'}")
        print("-"*50)
        for size, config in NETWORK_CONFIGS.items():
            net = f"{config['output']}->{config['input']}"
            if size in results:
                print(f"{size:<10} {net:<15} {results[size]:<12.4f} OK")
            else:
                print(f"{size:<10} {net:<15} {'N/A':<12} FAILED")
    else:
        run_benchmark(args.device, args.size, args.warps, batch_size, args.val)


if __name__ == "__main__":
    main()

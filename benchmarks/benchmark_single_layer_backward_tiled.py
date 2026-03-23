#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Single Layer Backward Pass Benchmark — Tiled (OptimalLayout) Weights

Uses neural_unittest.testTiledBackward which calls TiledMMAHelper directly:
  1. dIn = W^T * dOut         (transpose MMA)
  2. dBias += sum(dOut)       (bias reduce)
  3. dW += dOut * input^T     (outer product accumulate)

Supports both CUDA and Vulkan backends.
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


def linear_to_tiled(weights_linear: np.ndarray, output_size: int, input_size: int) -> np.ndarray:
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


def run_benchmark(device_type_str: str, size: str, num_warps: int = 1, batch_size: int = DEFAULT_BATCH_SIZE):
    config = NETWORK_CONFIGS[size]
    input_size = config["input"]
    output_size = config["output"]

    tile_size = 16
    n_tile_rows = math.ceil(output_size / tile_size)
    n_tile_cols = math.ceil(input_size / tile_size)
    tiled_weight_count = n_tile_rows * n_tile_cols * 256

    print(f"\n{'='*70}")
    print(f"Single Layer Backward (Tiled): {input_size} -> {output_size}, batch={batch_size}")
    print(f"Tiled weights: {tiled_weight_count}, Bias: {output_size}")
    print(f"{'='*70}")

    neural_module_dir = find_neural_module_dir()
    if not neural_module_dir:
        print("ERROR: neural.slang-module not found")
        return None

    device_type_map = {
        "vulkan": spy.DeviceType.vulkan,
        "cuda": spy.DeviceType.cuda,
    }
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

        module = device.load_module("benchmark_single_layer_backward_tiled.slang")
        program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_single_layer_backward_tiled")]
        )
        kernel = device.create_compute_kernel(program)

        rng = np.random.default_rng(42)

        # Use float32: Vulkan doesn't support half atomicAdd needed by
        # outer product and bias reduce gradient accumulation.
        dtype = np.float32

        linear_weight_count = input_size * output_size
        weights_linear = rng.standard_normal(linear_weight_count).astype(dtype) * 0.1
        weights_tiled = linear_to_tiled(weights_linear, output_size, input_size)

        inputs_data = rng.standard_normal(batch_size * input_size).astype(dtype) * 0.1
        grad_outputs_data = rng.standard_normal(batch_size * output_size).astype(dtype) * 0.1

        weights_buf = device.create_buffer(
            data=weights_tiled,
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
        grad_weights_buf = device.create_buffer(
            data=np.zeros(tiled_weight_count, dtype=dtype),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )
        grad_bias_buf = device.create_buffer(
            data=np.zeros(output_size, dtype=dtype),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )
        grad_inputs_buf = device.create_buffer(
            data=np.zeros(batch_size * input_size, dtype=dtype),
            usage=spy.BufferUsage.unordered_access,
        )

        threads_per_group = 32 * num_warps
        num_groups = batch_size // num_warps
        thread_count = [num_groups * threads_per_group, 1, 1]

        if device_type == spy.DeviceType.vulkan:
            weights_arg = weights_buf.descriptor_handle_rw
            grad_weights_arg = grad_weights_buf.descriptor_handle_rw
            grad_bias_arg = grad_bias_buf.descriptor_handle_rw
        else:
            weights_arg = weights_buf
            grad_weights_arg = grad_weights_buf
            grad_bias_arg = grad_bias_buf

        print(f"Warming up ({WARMUP} iterations)...")
        for _ in range(WARMUP):
            kernel.dispatch(
                thread_count=thread_count,
                weights=weights_arg,
                inputs=inputs_buf,
                grad_outputs=grad_outputs_buf,
                grad_weights=grad_weights_arg,
                grad_bias=grad_bias_arg,
                grad_inputs=grad_inputs_buf,
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
                weights=weights_arg,
                inputs=inputs_buf,
                grad_outputs=grad_outputs_buf,
                grad_weights=grad_weights_arg,
                grad_bias=grad_bias_arg,
                grad_inputs=grad_inputs_buf,
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
        print(f"  Network: {input_size} -> {output_size}")
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
    parser = argparse.ArgumentParser(description="Single Layer Backward Benchmark (Tiled Layout)")
    parser.add_argument("--device", default="cuda", choices=["cuda", "vulkan"])
    parser.add_argument("--size", default="small", choices=list(NETWORK_CONFIGS.keys()))
    parser.add_argument("--all", action="store_true", help="Run all sizes")
    parser.add_argument("--warps", type=int, default=1, choices=[1, 2])
    parser.add_argument("--batch-size", type=int, default=DEFAULT_BATCH_SIZE)
    args = parser.parse_args()

    print("="*70)
    print("Single Layer Backward Pass Benchmark (Tiled / OptimalLayout)")
    print("="*70)
    print(f"Batch size: {args.batch_size}")
    print(f"Warmup: {WARMUP}, Iterations: {ITERATIONS}")
    print(f"Backend: {args.device}")
    print(f"Warps per workgroup: {args.warps}")

    if args.all:
        results = {}
        for size in NETWORK_CONFIGS.keys():
            time_ms = run_benchmark(args.device, size, args.warps, args.batch_size)
            if time_ms:
                results[size] = time_ms

        print("\n" + "="*70)
        print("Summary: Single Layer Backward (Tiled / OptimalLayout)")
        print("="*70)
        print(f"{'Size':<10} {'Network':<15} {'Time (ms)':<12} {'Status'}")
        print("-"*50)
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

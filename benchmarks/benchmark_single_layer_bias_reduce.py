#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Single Layer Bias Gradient Reduction Benchmark

Measures sumReduceRows: bias_grad[i] = sum_{all lanes} dOut[i]
Uses MMAHelperNew.sumReduceRows (MMA-based, FromLocalRow, zero-shmem MMA phase).

Network sizes (OUTPUT_SIZE is the bias dimension):
- tiny:  output=16
- small: output=16
- medium: output=32
- large: output=64
- xlarge: output=128
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


def run_benchmark(device_type_str: str, size: str, num_warps: int = 1, batch_size: int = DEFAULT_BATCH_SIZE):
    """Run bias reduce benchmark."""
    config = NETWORK_CONFIGS[size]
    input_size = config["input"]
    output_size = config["output"]

    print(f"\n{'='*70}")
    print(f"Bias Reduce (MMAHelperNew): output_size={output_size}, batch={batch_size}")
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

        module = device.load_module("benchmark_single_layer_bias_reduce.slang")

        # Timing kernel (constant dOut)
        program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_bias_reduce")]
        )
        kernel = device.create_compute_kernel(program)

        # Validation kernel (reads dOut from buffer)
        val_program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_bias_reduce_validate")]
        )
        val_kernel = device.create_compute_kernel(val_program)

        threads_per_group = 32 * num_warps
        num_groups = batch_size // num_warps
        thread_count = [num_groups * threads_per_group, 1, 1]

        # Thorough validation with random per-thread dOut data.
        # Use a small validation batch to keep values in float16 range.
        val_batch = min(batch_size, 64)
        val_num_groups = val_batch // num_warps
        val_thread_count = [val_num_groups * threads_per_group, 1, 1]
        total_threads = val_batch * 32

        rng = np.random.default_rng(42)
        dout_data = (rng.standard_normal(total_threads * output_size) * 0.1).astype(np.float16)

        dout_buf = device.create_buffer(
            data=dout_data,
            usage=spy.BufferUsage.shader_resource,
        )
        val_bias_buf = device.create_buffer(
            data=np.zeros(output_size, dtype=np.float16),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )

        val_kernel.dispatch(
            thread_count=val_thread_count,
            biasBuffer=val_bias_buf,
            dOutInput=dout_buf,
            batch_size=val_batch,
        )
        device.wait()

        # CPU reference: for each element i, sum dOut[thread][i] across all threads.
        gpu_bias = val_bias_buf.to_numpy().view(np.float16)[:output_size].astype(np.float32)
        dout_matrix = dout_data.reshape(total_threads, output_size).astype(np.float32)
        cpu_bias = dout_matrix.sum(axis=0)

        per_elem_err = np.abs(gpu_bias - cpu_bias)
        max_err = np.max(per_elem_err)
        max_ref = np.max(np.abs(cpu_bias)) + 1e-6
        rel_err = max_err / max_ref
        print(f"Validation ({val_batch} samples, {total_threads} threads, random dOut):")
        print(f"  GPU bias[0..3]: {gpu_bias[:4]}")
        print(f"  CPU bias[0..3]: {cpu_bias[:4]}")
        print(f"  Max abs error: {max_err:.6f}")
        print(f"  Rel error: {rel_err:.6f}")
        if rel_err >= 0.05:
            print(f"  FAIL — per-element dump:")
            for i in range(output_size):
                flag = " <<<" if per_elem_err[i] > 0.05 * (abs(cpu_bias[i]) + 1e-6) else ""
                print(f"    [{i:3d}] GPU={gpu_bias[i]:12.4f}  CPU={cpu_bias[i]:12.4f}  err={per_elem_err[i]:10.4f}{flag}")
        else:
            print(f"  PASS")


        # Create bias buffer for timing runs
        bias_buf = device.create_buffer(
            data=np.zeros(output_size, dtype=np.float16),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )

        print(f"Warming up ({WARMUP} iterations)...")
        for _ in range(WARMUP):
            kernel.dispatch(
                thread_count=thread_count,
                biasBuffer=bias_buf,
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
                biasBuffer=bias_buf,
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
        print(f"  Bias reduce: {output_size} elements")
        print(f"  Avg time: {avg_time_ms:.4f} ms")

        device.close()
        return avg_time_ms

    except Exception as e:
        print(f"FAILED: {e}")
        import traceback
        traceback.print_exc()
        return None


def main():
    parser = argparse.ArgumentParser(description="Bias Gradient Reduction Benchmark (MMAHelperNew)")
    parser.add_argument("--device", default="cuda", choices=["cuda"])
    parser.add_argument("--size", default="small", choices=list(NETWORK_CONFIGS.keys()))
    parser.add_argument("--all", action="store_true", help="Run all sizes")
    parser.add_argument("--warps", type=int, default=1, choices=[1, 2])
    parser.add_argument("--batch-size", type=int, default=DEFAULT_BATCH_SIZE)
    args = parser.parse_args()

    batch_size = args.batch_size

    print("="*70)
    print("Bias Gradient Reduction Benchmark (MMAHelperNew / sumReduceRows)")
    print("="*70)
    print(f"Batch size: {batch_size}")
    print(f"Warmup: {WARMUP}, Iterations: {ITERATIONS}")
    print(f"Warps per workgroup: {args.warps}")

    if args.all:
        results = {}
        for size in NETWORK_CONFIGS.keys():
            time_ms = run_benchmark(args.device, size, args.warps, batch_size)
            if time_ms:
                results[size] = time_ms

        print("\n" + "="*70)
        print("Summary: Bias Reduce (MMAHelperNew / sumReduceRows)")
        print("="*70)
        print(f"{'Size':<10} {'Output':<10} {'Time (ms)':<12} {'Status'}")
        print("-"*45)
        for size, config in NETWORK_CONFIGS.items():
            if size in results:
                print(f"{size:<10} {config['output']:<10} {results[size]:<12.4f} OK")
            else:
                print(f"{size:<10} {config['output']:<10} {'N/A':<12} FAILED")
    else:
        run_benchmark(args.device, args.size, args.warps, batch_size)


if __name__ == "__main__":
    main()

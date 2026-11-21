#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
MLP Backward Pass Benchmark - GPU Timestamps

Fair comparison of backward pass (training) performance:
- CoopMat (neural.slang) backward pass
- Tin2 backward pass (run separately via CUDA)

Same conditions as forward pass:
- 256 CTAs, 32 threads each
- fp16 data type
- GPU timestamps
"""

import argparse
import numpy as np
from pathlib import Path
from dataclasses import dataclass
from typing import Optional

try:
    import slangpy as spy
    from slangpy.core.calldata import SLANG_PATH
except ImportError:
    print("Error: slangpy not found. Please install slangpy first.")
    exit(1)

# Network configurations (same as forward)
NETWORK_CONFIGS = {
    "small": {"input": 64, "hidden": 64, "output": 16},
    "medium": {"input": 128, "hidden": 128, "output": 32},
    "large": {"input": 256, "hidden": 256, "output": 64},
    "xlarge": {"input": 512, "hidden": 512, "output": 128},
}

BATCH_SIZE = 256
WARMUP_ITERATIONS = 50
BENCHMARK_ITERATIONS = 200


@dataclass
class BenchmarkResult:
    """Results from a benchmark run."""
    name: str
    backend: str
    network: str
    time_ms: float
    throughput: float
    success: bool
    error: Optional[str] = None


def get_network_config(size_name: str) -> dict:
    """Get network configuration for a given size."""
    config = NETWORK_CONFIGS[size_name]
    input_size = config["input"]
    hidden_size = config["hidden"]
    output_size = config["output"]
    
    layer1_weights = input_size * hidden_size
    layer1_biases = hidden_size
    layer2_weights = hidden_size * output_size
    layer2_biases = output_size
    total_params = layer1_weights + layer1_biases + layer2_weights + layer2_biases
    
    return {
        "input_size": input_size,
        "hidden_size": hidden_size,
        "output_size": output_size,
        "layer1_weights": layer1_weights,
        "layer1_biases": layer1_biases,
        "layer2_weights": layer2_weights,
        "layer2_biases": layer2_biases,
        "total_params": total_params,
    }


def find_neural_module_dir():
    """Find the neural.slang-module directory."""
    slang_root = Path(__file__).resolve().parents[5]
    candidates = [
        slang_root / "build" / "Release" / "lib" / "slang-standard-module-2026.1",
        slang_root / "build" / "Debug" / "lib" / "slang-standard-module-2026.1",
    ]
    for candidate in candidates:
        neural_module = candidate / "neural.slang-module"
        if neural_module.exists():
            return candidate
    return None


def run_benchmark_with_gpu_timing(
    device,
    kernel,
    thread_count: list,
    dispatch_args: dict,
    warmup_iterations: int,
    benchmark_iterations: int,
    batch_size: int,
) -> tuple:
    """Run benchmark using GPU timestamps. Returns (avg_time_ms, throughput)."""
    
    # Warmup
    for _ in range(warmup_iterations):
        kernel.dispatch(thread_count=thread_count, **dispatch_args)
    device.wait_for_idle()
    
    # Create query pool for GPU timestamps
    query_pool = device.create_query_pool(
        type=spy.QueryType.timestamp,
        count=2,
    )
    
    # Benchmark with GPU timestamps
    query_pool.reset()
    command_encoder = device.create_command_encoder()
    
    # Record start timestamp
    command_encoder.write_timestamp(query_pool, 0)
    
    # Dispatch all iterations (no sync between iterations)
    for _ in range(benchmark_iterations):
        kernel.dispatch(
            thread_count=thread_count,
            command_encoder=command_encoder,
            **dispatch_args,
        )
    
    # Record end timestamp
    command_encoder.write_timestamp(query_pool, 1)
    
    # Submit and wait
    device.submit_command_buffer(command_encoder.finish())
    device.wait_for_idle()
    
    # Get timestamp results
    timestamps = query_pool.get_results(0, 2)
    frequency = float(device.info.timestamp_frequency)
    
    elapsed_ticks = timestamps[1] - timestamps[0]
    total_time_ms = (elapsed_ticks / frequency) * 1000
    avg_time_ms = total_time_ms / benchmark_iterations
    throughput = batch_size / (avg_time_ms / 1000)
    
    return avg_time_ms, throughput


def run_coopmat_backward_benchmark(device_type_str: str, network_size: str) -> BenchmarkResult:
    """Run CoopMat backward pass benchmark with GPU timestamps."""
    config = get_network_config(network_size)
    network_str = f"{config['input_size']}->{config['hidden_size']}->{config['output_size']}"
    
    try:
        device_type_map = {
            "vulkan": spy.DeviceType.vulkan,
            "cuda": spy.DeviceType.cuda,
        }
        device_type = device_type_map[device_type_str.lower()]
        test_dir = Path(__file__).resolve().parent
        
        defines = {
            "INPUT_SIZE": str(config['input_size']),
            "HIDDEN_SIZE": str(config['hidden_size']),
            "OUTPUT_SIZE": str(config['output_size']),
        }
        
        neural_module_dir = find_neural_module_dir()
        include_paths = [str(test_dir)]
        if neural_module_dir:
            include_paths.append(str(neural_module_dir))
        
        compiler_options = spy.SlangCompilerOptions({
            "include_paths": include_paths,
            "enable_experimental_features": True,
            "defines": defines,
        })
        
        device = spy.Device(
            type=device_type,
            enable_debug_layers=False,
            compiler_options=compiler_options,
        )
        
        try:
            # Load backward module
            module = device.load_module("benchmark_neural_mlp_coopmat_backward_fp16.slang")
            dtype = np.float16
            
            backward_program = device.link_program(
                modules=[module],
                entry_points=[module.entry_point("compute_batch_backward")]
            )
            backward_kernel = device.create_compute_kernel(backward_program)
            
            # Initialize parameters
            rng = np.random.default_rng(42)
            params_data = np.zeros(config['total_params'], dtype=dtype)
            scale1 = np.sqrt(2.0 / config['input_size'])
            params_data[:config['layer1_weights']] = (rng.standard_normal(config['layer1_weights']) * scale1).astype(dtype)
            offset = config['layer1_weights'] + config['layer1_biases']
            scale2 = np.sqrt(2.0 / config['hidden_size'])
            params_data[offset:offset + config['layer2_weights']] = (rng.standard_normal(config['layer2_weights']) * scale2).astype(dtype)
            
            inputs_data = rng.standard_normal(BATCH_SIZE * config['input_size']).astype(dtype)
            grad_outputs_data = rng.standard_normal(BATCH_SIZE * config['output_size']).astype(dtype)
            
            # Create buffers
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
                data=np.zeros(config['total_params'], dtype=dtype),
                usage=spy.BufferUsage.unordered_access,
            )
            grad_inputs_buf = device.create_buffer(
                data=np.zeros(BATCH_SIZE * config['input_size'], dtype=dtype),
                usage=spy.BufferUsage.unordered_access,
            )
            
            # 32 threads per group (1 warp)
            thread_count = [BATCH_SIZE * 32, 1, 1]
            
            dispatch_args = {
                "params": params_buf,
                "inputs": inputs_buf,
                "grad_outputs": grad_outputs_buf,
                "grad_params": grad_params_buf,
                "grad_inputs": grad_inputs_buf,
                "batch_size": BATCH_SIZE,
            }
            
            avg_time_ms, throughput = run_benchmark_with_gpu_timing(
                device, backward_kernel, thread_count, dispatch_args,
                WARMUP_ITERATIONS, BENCHMARK_ITERATIONS, BATCH_SIZE
            )
            
            return BenchmarkResult(
                name="CoopMat-Backward",
                backend=device_type_str,
                network=network_str,
                time_ms=avg_time_ms,
                throughput=throughput,
                success=True,
            )
        finally:
            device.close()
            
    except Exception as e:
        import traceback
        return BenchmarkResult(
            name="CoopMat-Backward",
            backend=device_type_str,
            network=network_str,
            time_ms=0,
            throughput=0,
            success=False,
            error=f"{str(e)}\n{traceback.format_exc()}",
        )


def main():
    parser = argparse.ArgumentParser(description="MLP Backward Pass Benchmark with GPU Timestamps")
    parser.add_argument("--device", choices=["cuda", "vulkan"], default="cuda", help="Device type")
    parser.add_argument("--size", choices=list(NETWORK_CONFIGS.keys()), help="Network size (or --sweep for all)")
    parser.add_argument("--sweep", action="store_true", help="Run all network sizes")
    args = parser.parse_args()
    
    print("=" * 100)
    print("MLP BACKWARD Pass Benchmark - GPU Timestamps")
    print("=" * 100)
    print(f"Device: {args.device.upper()}")
    print(f"Timing: GPU timestamps (equivalent to CUDA events)")
    print(f"Batch size: {BATCH_SIZE} samples")
    print(f"Warmup: {WARMUP_ITERATIONS}, Benchmark: {BENCHMARK_ITERATIONS} iterations")
    print(f"Pass: BACKWARD (computes weight gradients)")
    print()
    
    sizes = list(NETWORK_CONFIGS.keys()) if args.sweep else [args.size or "small"]
    
    print(f"{'Size':<10} {'Network':<18} {'Impl':<18} {'Backend':<8} {'Time (ms)':<12} {'Throughput':<15} {'Status'}")
    print("-" * 110)
    
    for size_name in sizes:
        config = NETWORK_CONFIGS[size_name]
        network_str = f"{config['input']}->{config['hidden']}->{config['output']}"
        
        result = run_coopmat_backward_benchmark(args.device, size_name)
        if result.success:
            print(f"{size_name:<10} {network_str:<18} {result.name:<18} {args.device:<8} {result.time_ms:<12.4f} {result.throughput:<15.0f} OK")
        else:
            error_short = result.error.split('\n')[0][:40] if result.error else "Unknown"
            print(f"{size_name:<10} {network_str:<18} {result.name:<18} {args.device:<8} {'FAILED':<12} {'':<15} {error_short}")
    
    print()
    print("=" * 100)
    print("\nTo compare with Tin backward pass, run:")
    print("  cd Tin-repo/benchmarks/mlp_perf/comparison/build")
    print("  ./bench_tin2_256cta_backward_fp16")
    print("=" * 100)


if __name__ == "__main__":
    main()

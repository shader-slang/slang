#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Single Layer Backward Pass Benchmark

Simplified backward pass comparison: single layer (Input -> Output)
This requires much less shared memory than 2-layer MLP.

Network sizes:
- tiny:  32 -> 16 (528 params)
- small: 64 -> 16 (1,040 params)
- medium: 128 -> 32 (4,128 params)
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

# Single-layer network configurations
NETWORK_CONFIGS = {
    "tiny": {"input": 32, "output": 16},
    "small": {"input": 64, "output": 16},
    "medium": {"input": 128, "output": 32},
    "large": {"input": 256, "output": 64},
}

BATCH_SIZE = 256
WARMUP = 50
ITERATIONS = 200


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


def run_single_layer_backward(device_type_str: str, size: str):
    """Run single-layer backward benchmark."""
    config = NETWORK_CONFIGS[size]
    input_size = config["input"]
    output_size = config["output"]
    
    # Single layer: weights [input x output] + biases [output]
    total_params = input_size * output_size + output_size
    
    print(f"\n{'='*70}")
    print(f"Single Layer Backward: {input_size} -> {output_size}")
    print(f"Parameters: {total_params}")
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
    
    # Defines for shader
    defines = {
        "INPUT_SIZE": str(input_size),
        "OUTPUT_SIZE": str(output_size),
    }
    
    include_paths = [str(test_dir), str(neural_module_dir), SLANG_PATH]
    
    compiler_options = spy.SlangCompilerOptions({
        "include_paths": include_paths,
        "debug_info": spy.SlangDebugInfoLevel.standard,
        "enable_experimental_features": True,
        "defines": defines,
    })
    
    try:
        device = spy.Device(
            type=device_type,
            enable_debug_layers=True,
            compiler_options=compiler_options,
        )
        
        module = device.load_module("benchmark_single_layer_backward.slang")
        
        # Link and create kernel
        program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_single_layer_backward")]
        )
        kernel = device.create_compute_kernel(program)
        
        # Allocate buffers
        rng = np.random.default_rng(42)
        
        params_data = rng.standard_normal(total_params).astype(np.float16) * 0.1
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
            usage=spy.BufferUsage.unordered_access,
        )
        grad_inputs_buf = device.create_buffer(
            data=np.zeros(BATCH_SIZE * input_size, dtype=np.float16),
            usage=spy.BufferUsage.unordered_access,
        )
        
        thread_count = [BATCH_SIZE, 1, 1]
        
        # Warmup
        print(f"Warming up ({WARMUP} iterations)...")
        for _ in range(WARMUP):
            kernel.dispatch(
                thread_count=thread_count,
                params=params_buf,
                inputs=inputs_buf,
                grad_outputs=grad_outputs_buf,
                grad_params=grad_params_buf,
                grad_inputs=grad_inputs_buf,
                batch_size=BATCH_SIZE,
            )
        device.wait()
        
        # Benchmark with GPU timestamps
        print(f"Benchmarking ({ITERATIONS} iterations)...")
        query_pool = device.create_query_pool(type=spy.QueryType.timestamp, count=2)
        
        command_encoder = device.create_command_encoder()
        command_encoder.write_timestamp(query_pool, 0)
        
        for _ in range(ITERATIONS):
            kernel.dispatch(
                thread_count=thread_count,
                command_encoder=command_encoder,
                params=params_buf,
                inputs=inputs_buf,
                grad_outputs=grad_outputs_buf,
                grad_params=grad_params_buf,
                grad_inputs=grad_inputs_buf,
                batch_size=BATCH_SIZE,
            )
        
        command_encoder.write_timestamp(query_pool, 1)
        device.submit_command_buffer(command_encoder.finish())
        device.wait()
        
        # Calculate results
        timestamps = np.array(query_pool.get_results(0, 2))
        frequency = float(device.info.timestamp_frequency)
        elapsed_ticks = timestamps[1] - timestamps[0]
        total_time_ms = (elapsed_ticks / frequency) * 1000
        avg_time_ms = total_time_ms / ITERATIONS
        throughput = BATCH_SIZE / (avg_time_ms / 1000)
        
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
    parser = argparse.ArgumentParser(description="Single Layer Backward Benchmark")
    parser.add_argument("--device", default="cuda", choices=["cuda", "vulkan"])
    parser.add_argument("--size", default="small", choices=list(NETWORK_CONFIGS.keys()))
    parser.add_argument("--all", action="store_true", help="Run all sizes")
    args = parser.parse_args()
    
    print("="*70)
    print("Single Layer Backward Pass Benchmark")
    print("="*70)
    print(f"Batch size: {BATCH_SIZE}")
    print(f"Warmup: {WARMUP}, Iterations: {ITERATIONS}")
    print(f"Backend: {args.device}")
    
    if args.all:
        results = {}
        for size in NETWORK_CONFIGS.keys():
            time_ms = run_single_layer_backward(args.device, size)
            if time_ms:
                results[size] = time_ms
        
        print("\n" + "="*70)
        print("Summary: Single Layer Backward (CoopMat)")
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
        run_single_layer_backward(args.device, args.size)


if __name__ == "__main__":
    main()

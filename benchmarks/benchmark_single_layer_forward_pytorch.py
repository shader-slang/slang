#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Single Layer Forward Pass Benchmark — PyTorch Baseline

Profiles a single nn.Linear (MatVecMul + bias) in half precision on CUDA.
Uses the same network configs, batch size, warmup, and iteration counts
as the Slang benchmarks for direct comparison.

Network sizes:
- tiny:  32 -> 16
- small: 64 -> 16
- medium: 128 -> 32
- large: 256 -> 64
"""

import argparse
import torch
import torch.nn as nn

NETWORK_CONFIGS = {
    "tiny": {"input": 32, "output": 16},
    "small": {"input": 64, "output": 16},
    "medium": {"input": 128, "output": 32},
    "large": {"input": 256, "output": 64},
    "xlarge": {"input": 128, "output": 128},
}

BATCH_SIZE = 256
WARMUP = 50
ITERATIONS = 200


def run_benchmark(size: str):
    """Run single-layer forward benchmark with PyTorch."""
    config = NETWORK_CONFIGS[size]
    input_size = config["input"]
    output_size = config["output"]

    total_params = input_size * output_size + output_size

    print(f"\n{'='*70}")
    print(f"Single Layer Forward (PyTorch): {input_size} -> {output_size}")
    print(f"Parameters: {total_params}")
    print(f"{'='*70}")

    device = torch.device("cuda")

    # Single linear layer in half precision
    layer = nn.Linear(input_size, output_size, bias=True).half().to(device)

    # Fixed seed for reproducibility
    torch.manual_seed(42)
    with torch.no_grad():
        layer.weight.normal_(0, 0.1)
        layer.bias.normal_(0, 0.1)

    # Input data: same shape as Slang benchmark
    # Slang dispatches one thread group (32 threads) per sample,
    # each thread carries its own input vector → effectively batch_size samples.
    inputs = torch.randn(BATCH_SIZE, input_size, dtype=torch.float16, device=device) * 0.1

    # Warmup
    print(f"Warming up ({WARMUP} iterations)...")
    with torch.no_grad():
        for _ in range(WARMUP):
            _ = layer(inputs)
    torch.cuda.synchronize()

    # Benchmark with CUDA events
    print(f"Benchmarking ({ITERATIONS} iterations)...")
    start_event = torch.cuda.Event(enable_timing=True)
    end_event = torch.cuda.Event(enable_timing=True)

    with torch.no_grad():
        start_event.record()
        for _ in range(ITERATIONS):
            _ = layer(inputs)
        end_event.record()

    torch.cuda.synchronize()
    total_time_ms = start_event.elapsed_time(end_event)
    avg_time_ms = total_time_ms / ITERATIONS
    throughput = BATCH_SIZE / (avg_time_ms / 1000)

    print(f"\nResults:")
    print(f"  Backend: PyTorch CUDA (half)")
    print(f"  Network: {input_size} -> {output_size}")
    print(f"  Avg time: {avg_time_ms:.4f} ms")
    print(f"  Throughput: {throughput:.0f} samples/s")

    return avg_time_ms


def main():
    parser = argparse.ArgumentParser(description="Single Layer Forward Benchmark (PyTorch)")
    parser.add_argument("--size", default="small", choices=list(NETWORK_CONFIGS.keys()))
    parser.add_argument("--all", action="store_true", help="Run all sizes")
    args = parser.parse_args()

    if not torch.cuda.is_available():
        print("ERROR: CUDA not available")
        return

    print("="*70)
    print("Single Layer Forward Pass Benchmark (PyTorch)")
    print("="*70)
    print(f"Batch size: {BATCH_SIZE}")
    print(f"Warmup: {WARMUP}, Iterations: {ITERATIONS}")
    print(f"GPU: {torch.cuda.get_device_name(0)}")

    if args.all:
        results = {}
        for size in NETWORK_CONFIGS.keys():
            time_ms = run_benchmark(size)
            if time_ms:
                results[size] = time_ms

        print("\n" + "="*70)
        print("Summary: Single Layer Forward (PyTorch)")
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
        run_benchmark(args.size)


if __name__ == "__main__":
    main()

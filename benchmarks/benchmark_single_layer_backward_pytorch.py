"""
PyTorch single-layer backward benchmark.

Measures the three backward operations of a single linear layer:
  1. grad_input  = grad_output @ weight          (input gradient)
  2. grad_weight = grad_output^T @ input          (weight gradient)  
  3. grad_bias   = sum(grad_output, dim=0)        (bias gradient)

All tensors are pre-allocated in GPU memory. Inputs are constant (all 1.0)
to match the Slang/Tin2 benchmark.
"""

import torch
import argparse


# Custom CUDA kernel that writes ones into a tensor (simulates a shader
# generating input data and writing to global memory)
fill_ones_kernel = None

def get_fill_kernel():
    global fill_ones_kernel
    if fill_ones_kernel is not None:
        return fill_ones_kernel

    src = r"""
    extern "C" __global__ void fill_ones_f16(half* out, int n) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx < n) out[idx] = __float2half(1.0f);
    }
    """
    from torch.utils.cpp_extension import load_inline
    # Use raw cubin approach instead
    fill_ones_kernel = torch.cuda.jiterator._create_jit_fn("template<typename T> T fill_fn(T x) { return T(1); }")
    return fill_ones_kernel


def benchmark_backward(input_size, output_size, batch_size, warmup=200, iterations=1000):
    device = torch.device("cuda")

    weight = torch.randn(output_size, input_size, dtype=torch.float16, device=device) * 0.1
    input_data = torch.empty(batch_size, input_size, dtype=torch.float16, device=device)
    grad_output = torch.empty(batch_size, output_size, dtype=torch.float16, device=device)

    grad_weight = torch.zeros_like(weight)
    grad_bias = torch.zeros(output_size, dtype=torch.float16, device=device)
    grad_input = torch.empty(batch_size, input_size, dtype=torch.float16, device=device)

    # Warmup
    for _ in range(warmup):
        input_data.fill_(1.0)
        grad_output.fill_(1.0)
        torch.mm(grad_output, weight, out=grad_input)
        torch.mm(grad_output.t(), input_data, out=grad_weight)
        torch.sum(grad_output, dim=0, out=grad_bias)

    torch.cuda.synchronize()

    # Benchmark: fill (simulates shader writing to gmem) + backward ops
    start = torch.cuda.Event(enable_timing=True)
    end = torch.cuda.Event(enable_timing=True)

    start.record()
    for _ in range(iterations):
        input_data.fill_(1.0)
        grad_output.fill_(1.0)
        torch.mm(grad_output, weight, out=grad_input)
        torch.mm(grad_output.t(), input_data, out=grad_weight)
        torch.sum(grad_output, dim=0, out=grad_bias)
    end.record()
    torch.cuda.synchronize()
    total_ms = start.elapsed_time(end) / iterations

    # Measure fill-only overhead
    start2 = torch.cuda.Event(enable_timing=True)
    end2 = torch.cuda.Event(enable_timing=True)

    start2.record()
    for _ in range(iterations):
        input_data.fill_(1.0)
        grad_output.fill_(1.0)
    end2.record()
    torch.cuda.synchronize()
    fill_ms = start2.elapsed_time(end2) / iterations

    return total_ms, fill_ms


def main():
    parser = argparse.ArgumentParser(description="PyTorch single-layer backward benchmark")
    parser.add_argument("--batch-size", type=int, default=8192)
    parser.add_argument("--warmup", type=int, default=200)
    parser.add_argument("--iterations", type=int, default=1000)
    args = parser.parse_args()

    print("=" * 60)
    print("PyTorch Single Layer Backward Benchmark")
    print("=" * 60)
    print(f"Device: {torch.cuda.get_device_name()}")
    print(f"PyTorch: {torch.__version__}")
    print(f"Batch size: {args.batch_size}")
    print(f"Data type: fp16")
    print(f"Operations: mm(grad_out, W) + mm(grad_out^T, input) + sum(grad_out)")
    print()

    configs = [
        ("tiny",   32,  16),
        ("small",  64,  16),
        ("medium", 128, 32),
        ("large",  256, 64),
        ("xlarge", 128, 128),
    ]

    print(f"{'Size':<10} {'Network':<12} {'Total':>10} {'Fill':>10} {'Backward':>10} {'Throughput':>18}")
    print("-" * 75)

    for name, ip, op in configs:
        total_ms, fill_ms = benchmark_backward(ip, op, args.batch_size, args.warmup, args.iterations)
        bwd_ms = total_ms - fill_ms
        throughput = args.batch_size / (total_ms / 1000.0)
        print(f"{name:<10} {ip:>3} -> {op:<3}   {total_ms:>8.4f}ms {fill_ms:>8.4f}ms {bwd_ms:>8.4f}ms   {throughput:>12.0f} samples/s")

    print()


if __name__ == "__main__":
    main()

# Neural MLP Performance Benchmark

Compares identical 2-layer MLP network architecture across three GPU implementations:

| Backend      | Description                                                         |
| ------------ | ------------------------------------------------------------------- |
| **For-loop** | Scalar GPU loops (original SlangPy LinearLayer style)               |
| **CoopVec**  | `coopVecMatMulAdd` intrinsics (original SlangPy CoopVecLinearLayer) |
| **CoopMat**  | WaveTangledVector + FFLayer (current neural.slang module)           |

## Purpose

This benchmark tracks performance and guides future CoopMat optimizations by comparing against the original implementations.

## Usage

### Forward Pass (GPU Timing - Recommended)

```bash
# Run all implementations with GPU timestamps
python benchmark_all_gpu_timing.py --device cuda --sweep

# Vulkan backend
python benchmark_all_gpu_timing.py --device vulkan --sweep

# Specific implementation
python benchmark_all_gpu_timing.py --device cuda --sweep --impl coopmat
```

### Backward Pass (Training)

```bash
# Run backward pass benchmark
python benchmark_backward_gpu_timing.py --device cuda --sweep

# Specific network size
python benchmark_backward_gpu_timing.py --device cuda --size small
```

### CPU Timing (Original)

```bash
# Basic benchmark (small network, Vulkan)
python benchmark_neural_mlp.py

# Specific network size
python benchmark_neural_mlp.py --size medium

# Sweep all network sizes
python benchmark_neural_mlp.py --sweep

# JSON output for CI integration
python benchmark_neural_mlp.py --json

# Different device
python benchmark_neural_mlp.py --device cuda
```

## Timing Methods

| Method             | Script                        | Measures                     | Fair Comparison |
| ------------------ | ----------------------------- | ---------------------------- | --------------- |
| **GPU Timestamps** | `benchmark_all_gpu_timing.py` | Pure GPU kernel time         | ✅ Yes          |
| CPU Timer          | `benchmark_neural_mlp.py`     | GPU + driver + sync overhead | ❌ No           |

**Use GPU timing** for accurate performance comparison. CPU timing can inflate results by 3-5x due to Python/driver overhead.

## Options

| Option           | Default | Description                                |
| ---------------- | ------- | ------------------------------------------ |
| `--iterations N` | 100     | Number of measured iterations              |
| `--warmup N`     | 10      | Number of warmup iterations                |
| `--batch-size N` | 256     | Batch size for forward pass                |
| `--device TYPE`  | vulkan  | Device type: vulkan, d3d12, cuda           |
| `--size SIZE`    | small   | Network size: small, medium, large, xlarge |
| `--sweep`        | -       | Run benchmark across all network sizes     |
| `--json`         | -       | Output results in JSON format              |
| `--forloop-only` | -       | Run only for-loop benchmark                |
| `--coopvec-only` | -       | Run only CoopVec benchmark                 |
| `--coopmat-only` | -       | Run only CoopMat benchmark                 |

## Network Sizes

| Size   | Architecture    | Parameters |
| ------ | --------------- | ---------- |
| small  | 64 → 64 → 16    | 5,200      |
| medium | 128 → 128 → 32  | 20,640     |
| large  | 256 → 256 → 64  | 82,240     |
| xlarge | 512 → 512 → 128 | 328,320    |

## Requirements

- slangpy (`pip install slangpy`)
- numpy
- GPU with Vulkan (or CUDA/D3D12) support
- For CoopVec: Cooperative vector extension support
- For CoopMat: Cooperative matrix extension support

## Files

### Forward Pass

| File                                      | Description                                            |
| ----------------------------------------- | ------------------------------------------------------ |
| `benchmark_all_gpu_timing.py`             | **GPU timing benchmark (recommended)**                 |
| `benchmark_neural_mlp.py`                 | CPU timing benchmark (original)                        |
| `benchmark_neural_mlp_forloop.slang`      | Scalar for-loop implementation (fp32)                  |
| `benchmark_neural_mlp_coopvec.slang`      | CoopVec implementation (fp16)                          |
| `benchmark_neural_mlp_coopmat.slang`      | CoopMat implementation (fp32)                          |
| `benchmark_neural_mlp_coopmat_fp16.slang` | **CoopMat implementation (fp16, for fair comparison)** |

### Backward Pass (Training)

| File                                               | Description                              |
| -------------------------------------------------- | ---------------------------------------- |
| `benchmark_backward_gpu_timing.py`                 | **Backward pass benchmark (GPU timing)** |
| `benchmark_neural_mlp_coopmat_backward_fp16.slang` | CoopMat backward using autodiff (fp16)   |

# Tiny Backward Hand-Off

## What the current problem is

We are investigating the **tiny** case of single-layer backward (`INPUT_SIZE=32`, `OUTPUT_SIZE=16`, `SUBGROUP_COUNT=2`, `batch=8192`, `2 warps`).

The key mystery is:

- **Tin2 full backward is much faster than its isolated pieces would suggest**.
- **Slang does not get that same composition benefit**.

Current launcher-based timings that matter:

| Mode | Slang | Tin2 |
|---|---:|---:|
| `transpose_only` | `0.0048 ms` | `0.0040 ms` |
| `transpose_bias` | `0.0073 ms` | `0.0074 ms` |
| `transpose_outer` | `0.0513 ms` | `0.0236 ms` |
| `full` | `0.0523 ms` | `0.0274 ms` |

The divergence starts immediately at **`transpose_outer`**.

This means the important problem is **not**:
- transpose MMA by itself
- bias reduce by itself
- final result extraction by itself
- standalone outer-product by itself

It is the **fused `transpose + outer` path**.

## Strong conclusions we already established

### 1. Do not use Python/slangpy timings for performance conclusions
Use only:
- `slangc -> PTX -> benchmarks/ncu_launcher_new_mma`
- `ncu`
- or direct `nvcc`-compiled CUDA binaries

### 2. The `slangc -> PTX -> launcher` path is not the source of the gap
We compiled the generated CUDA directly with:

```bash
/usr/local/cuda/bin/nvcc -O3 --expt-relaxed-constexpr -arch=sm_89 -DSLANG_CUDA_ENABLE_HALF -w ...
```

and got essentially the same timing as the PTX path.

So:
- **NVRTC/PTX vs nvcc is not the issue**.
- The generated CUDA itself already contains the problematic structure.

### 3. Outer-product in isolation is basically tied with Tin2
Standalone outer-product timing is close enough that the outer-product tail **alone** is not the mystery.
The mystery is the **fused transpose+outer composition**.

### 4. Final extraction is not the main bottleneck
We compared:
- `mmaTranspose + toArrayPacked + checksum`
- `mmaTranspose + fragmentRead checksum`

The runtime barely moved, so final extraction is not the dominant source of the tiny gap.

### 5. The main remaining diagnostic target is the **generated CUDA**, not the high-level Slang source
We copied the current generated CUDA into a standalone benchmark source:

- `benchmarks/bench_slang_generated_transpose_outer_tiny.cu`

This file is the current ground-truth diagnostic artifact.

## Important files

### Slang benchmark source with phase switches
- `benchmarks/benchmark_single_layer_backward.slang`

Useful macros already in the file:
- `FRAGMENT_CHECKSUM_MODE`
- `ENABLE_BIAS_REDUCE`
- `ENABLE_OUTER_PRODUCT`
- `ENABLE_OUTER_REDUCE`
- `ENABLE_OUTER_STORE`
- `EARLY_RESULT_CHECKSUM`

### Slang implementation under investigation
- `source/standard-modules/neural/mma-new-helper-v2.slang`

### Standalone generated CUDA benchmark
- `benchmarks/bench_slang_generated_transpose_outer_tiny.cu`

This file was copied from a generated CUDA snapshot and then given a standalone host-side benchmark harness.

### Tin2 mode-enabled comparison benchmark
- `tin2/benchmarks/mlp_perf/bench_tin2_single_layer_backward.cu`

This file was modified to accept:
- `--mode full`
- `--mode transpose_only`
- `--mode transpose_bias`
- `--mode transpose_outer`

## What to experiment next

The next experiment should stay focused on **generated CUDA vs Tin2 CUDA**, specifically on the **fused `transpose_outer` path**.

Recommended next step:

1. Work directly in:
   - `benchmarks/bench_slang_generated_transpose_outer_tiny.cu`
2. Keep the benchmark harness unchanged.
3. Patch only the **kernel body** to make the fused `transpose_outer` region look more like Tin2.
4. Recompile with `nvcc`, rerun timing, then rerun `ncu`.

### What to patch in the generated CUDA first

The generated Slang CUDA currently expands the fused path into many explicit temporaries such as:
- `_S23`, `_S24`, `_S25`, `_S26`
- `inputB_0`
- `matC_1`
- explicit `FragmentRead/FragmentWrite`
- explicit `mmaChangeMajor`
- explicit subgroup offset arithmetic

The best next patch target is the **early fused `transpose_outer` block** before the final shared-memory/global-store tail.

This is where the first clear source-level structural divergence vs Tin2 appears.

## How to regenerate the current generated CUDA

If the high-level Slang source changes and you want to refresh the generated CUDA snapshot:

```bash
./build/Release/bin/slangc benchmarks/benchmark_single_layer_backward.slang \
  -target cuda \
  -o /tmp/slang_transpose_outer_generated.cu \
  -DINPUT_SIZE=32 \
  -DOUTPUT_SIZE=16 \
  -DSUBGROUP_COUNT=2 \
  -DFRAGMENT_CHECKSUM_MODE=1 \
  -DENABLE_BIAS_REDUCE=0 \
  -DENABLE_OUTER_PRODUCT=1 \
  -DENABLE_OUTER_REDUCE=1 \
  -DENABLE_OUTER_STORE=1 \
  -experimental-feature
```

Then copy it back into:

```bash
cp /tmp/slang_transpose_outer_generated.cu benchmarks/bench_slang_generated_transpose_outer_tiny.cu
```

## How to build and run the standalone generated CUDA benchmark

Use the same `nvcc` setup that matched the PTX path:

```bash
/usr/local/cuda/bin/nvcc -O3 --expt-relaxed-constexpr -arch=sm_89 -DSLANG_CUDA_ENABLE_HALF -w \
  benchmarks/bench_slang_generated_transpose_outer_tiny.cu \
  -o /tmp/bench_slang_generated_transpose_outer_tiny

/tmp/bench_slang_generated_transpose_outer_tiny --batch-size 8192
```

Current rough result:
- generated CUDA standalone: about `0.0464 ms`

## How to build and run the Tin2 phase benchmark

```bash
/usr/local/cuda/bin/nvcc -O3 --expt-relaxed-constexpr -arch=sm_89 -I tin2/include -w \
  tin2/benchmarks/mlp_perf/bench_tin2_single_layer_backward.cu \
  -o /tmp/bench_tin2_backward_modes

/tmp/bench_tin2_backward_modes --batch-size 8192 --warps 2 --size tiny --mode transpose_outer
/tmp/bench_tin2_backward_modes --batch-size 8192 --warps 2 --size tiny --mode full
/tmp/bench_tin2_backward_modes --batch-size 8192 --warps 2 --size tiny --mode transpose_only
/tmp/bench_tin2_backward_modes --batch-size 8192 --warps 2 --size tiny --mode transpose_bias
```

## How to run the Slang phase-matrix benchmark through PTX + launcher

### Compile PTX for a given mode

Example: tiny `transpose_outer`

```bash
./build/Release/bin/slangc benchmarks/benchmark_single_layer_backward.slang \
  -target ptx -entry compute_backward -stage compute \
  -o /tmp/ref_transpose_outer.ptx \
  -I build/Release/lib/slang-standard-module-2026.3.1 \
  -DINPUT_SIZE=32 \
  -DOUTPUT_SIZE=16 \
  -DSUBGROUP_COUNT=2 \
  -DFRAGMENT_CHECKSUM_MODE=1 \
  -DENABLE_BIAS_REDUCE=0 \
  -DENABLE_OUTER_PRODUCT=1 \
  -DENABLE_OUTER_REDUCE=1 \
  -DENABLE_OUTER_STORE=1 \
  -experimental-feature
```

Example: tiny `full`

```bash
./build/Release/bin/slangc benchmarks/benchmark_single_layer_backward.slang \
  -target ptx -entry compute_backward -stage compute \
  -o /tmp/ref_full.ptx \
  -I build/Release/lib/slang-standard-module-2026.3.1 \
  -DINPUT_SIZE=32 \
  -DOUTPUT_SIZE=16 \
  -DSUBGROUP_COUNT=2 \
  -DFRAGMENT_CHECKSUM_MODE=1 \
  -DENABLE_BIAS_REDUCE=1 \
  -DENABLE_OUTER_PRODUCT=1 \
  -DENABLE_OUTER_REDUCE=1 \
  -DENABLE_OUTER_STORE=1 \
  -experimental-feature
```

### Run timing with launcher

```bash
./benchmarks/ncu_launcher_new_mma /tmp/ref_transpose_outer.ptx \
  --input-size 32 --output-size 16 --batch-size 8192 --warps 2 --mode backward

./benchmarks/ncu_launcher_new_mma /tmp/ref_full.ptx \
  --input-size 32 --output-size 16 --batch-size 8192 --warps 2 --mode backward
```

## How to extract metrics with `ncu`

Lock clocks first if needed:

```bash
sudo nvidia-smi -pm 1
sudo nvidia-smi --lock-gpu-clocks=2520,2520
```

Then profile the launcher:

```bash
sudo env PATH=$PATH /usr/local/cuda/bin/ncu \
  --metrics gpu__time_duration.sum,smsp__average_warp_latency_issue_stalled_mio_throttle,smsp__average_warp_latency_issue_stalled_long_scoreboard,smsp__average_warp_latency_issue_stalled_barrier \
  --launch-skip 100 --launch-count 1 \
  ./benchmarks/ncu_launcher_new_mma /tmp/ref_transpose_outer.ptx \
  --input-size 32 --output-size 16 --batch-size 8192 --warps 2 --mode backward
```

Do the same for Tin2:

```bash
sudo env PATH=$PATH /usr/local/cuda/bin/ncu \
  --metrics gpu__time_duration.sum,smsp__average_warp_latency_issue_stalled_mio_throttle,smsp__average_warp_latency_issue_stalled_long_scoreboard,smsp__average_warp_latency_issue_stalled_barrier \
  --launch-skip 100 --launch-count 1 \
  /tmp/bench_tin2_backward_modes --batch-size 8192 --warps 2 --size tiny --mode transpose_outer
```

## Current phase-matrix result to remember

### Slang
- `transpose_only`: `0.0048 ms`
- `transpose_bias`: `0.0073 ms`
- `transpose_outer`: `0.0513 ms`
- `full`: `0.0523 ms`

### Tin2
- `transpose_only`: `0.0040 ms`
- `transpose_bias`: `0.0074 ms`
- `transpose_outer`: `0.0236 ms`
- `full`: `0.0274 ms`

The gap begins at **`transpose_outer`**. That is the core fact to preserve.

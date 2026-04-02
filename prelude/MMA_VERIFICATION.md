# MMA Verification Guide

Three tests must pass after any change to the MMA pipeline (prelude, mma-new-helper.slang,
shared-memory-pool.slang, hlsl.meta.slang CoopMat operations):

1. **Result validation (outer product)** — correctness of outer product computation
2. **Result validation (transpose MMA)** — correctness of W^T * dOut computation
3. **ncu benchmark** — performance regression check

## Prerequisites

```bash
# Enter Python virtual environment (required for slangpy)
source ~/pip_venv/bin/activate

# Build (from repo root)
cmake --build --preset release -j12

# If neural module .slang files changed, force rebuild:
touch source/standard-modules/neural/mma-new-helper.slang
cmake --build --preset release -j12 --target slang-neural-module
```

## Test 1: Result Validation

Runs the outer product with per-thread unique data and compares against expected
values. Tests all 5 network sizes with 2 warps.

```bash
source ~/pip_venv/bin/activate
cd benchmarks

# Test multiple batch sizes to verify atomicAdd accumulation
for bs in 32 256 2048 8192; do
  echo "=== batch=$bs ==="
  python3 benchmark_single_layer_outer_product.py --all --warps 2 --batch-size $bs --val
done
```

**Expected output**: All 5 sizes × all batch sizes show `PASS`:
```
tiny       16x32    PASS
small      16x64    PASS
medium     32x128   PASS
large      64x256   PASS
xlarge     128x128  PASS
```

The `--val` flag enables:
- `RES_VALIDATION` macro in the shader (per-thread unique data instead of all-ones)
- Native-to-row-major conversion kernel
- Comparison against CPU-computed expected values
- Pass threshold: max relative error < 1% (half-precision tolerance)

## Test 2: Transpose MMA Validation

Runs the transpose MMA (dIn = W^T * dOut) and compares per-element GPU results
against CPU reference. Tests all 5 network sizes.

```bash
source ~/pip_venv/bin/activate
cd benchmarks

python3 benchmark_single_layer_transpose.py --all --warps 2 --batch-size 8192 --val
```

**Expected output**: All 5 sizes show `PASS`:
```
tiny       16->32    PASS
small      16->64    PASS
medium     32->128   PASS
large      64->256   PASS
xlarge     128->128  PASS
```

The `--val` flag enables:
- `RES_VALIDATION` macro in the shader (thread 0 dumps full dIn vector instead of checksum)
- Per-element comparison against CPU-computed W^T * dOut
- Pass threshold: max element error < 5% of absolute value or 0.01

Without `--val`, a simpler checksum comparison is used (faster, suitable for benchmarking).

## Test 3: ncu Benchmark

Measures kernel duration, spilling, and warp stall cycles for the 128x128 outer product.

### Step 1: Compile PTX
```bash
./build/Release/bin/slangc benchmarks/benchmark_single_layer_outer_product.slang \
  -target ptx -entry compute_outer_product -stage compute \
  -o /tmp/outer_ncu.ptx \
  -I build/Release/lib/slang-standard-module-2026.3.1 \
  -DINPUT_SIZE=128 -DOUTPUT_SIZE=128 -DSUBGROUP_COUNT=2
```

### Step 2: Check ptxas spill stats
```bash
head -c -1 /tmp/outer_ncu.ptx > /tmp/outer_ncu_fixed.ptx
/usr/local/cuda/bin/ptxas -v --gpu-name sm_89 /tmp/outer_ncu_fixed.ptx -o /dev/null
```

**Baseline** (as of commit `1639526f4`):
```
spill stores: 192 bytes
spill loads:  240 bytes
registers:    255
```

### Step 3: Run ncu (requires sudo)
```bash
sudo nvidia-smi -pm 1
sudo nvidia-smi --lock-gpu-clocks=2520,2520

sudo env PATH=$PATH /usr/local/cuda/bin/ncu --set full --launch-skip 100 --launch-count 1 \
  -o /tmp/outer_ncu_report \
  ./benchmarks/ncu_launcher_new_mma /tmp/outer_ncu.ptx \
  --input-size 128 --output-size 128 --batch-size 8192 --warps 2 --mode outer_product

# Don't reset clocks if running more benchmarks
# sudo nvidia-smi --reset-gpu-clocks
```

### Step 4: Extract metrics
```bash
/usr/local/cuda/bin/ncu --import /tmp/outer_ncu_report.ncu-rep --print-summary per-kernel 2>&1 | \
  grep -iE 'Duration|Spilling Req|Warp Cycles Per Issued'
```

**Baseline** (128x128, batch=8192, 2 warps, locked clocks at 2520MHz):

| Metric | Slang | Tin2 |
|--------|-------|------|
| Duration (us) | 149.41 | 157.12 |
| Spilling Requests | 499,712 | 507,904 |
| Warp Cycles/Issued | 17.47 | 20.73 |

### Transpose MMA ncu

Same process for the transpose MMA kernel (128x128, 2 warps):

#### Step 1: Compile PTX
```bash
./build/Release/bin/slangc benchmarks/benchmark_single_layer_transpose.slang \
  -target ptx -entry compute_single_layer_transpose -stage compute \
  -o /tmp/transpose_ncu.ptx \
  -I build/Release/lib/slang-standard-module-2026.3.1 \
  -DINPUT_SIZE=128 -DOUTPUT_SIZE=128 -DSUBGROUP_COUNT=2
```

#### Step 2: Check ptxas spill stats
```bash
head -c -1 /tmp/transpose_ncu.ptx > /tmp/transpose_ncu_fixed.ptx
/usr/local/cuda/bin/ptxas -v --gpu-name sm_89 /tmp/transpose_ncu_fixed.ptx -o /dev/null
```

**Baseline** (as of commit `8673d5f0b`):
```
spill stores: 0 bytes
spill loads:  0 bytes
registers:    42
```

#### Step 3: Run ncu (requires sudo, clocks should already be locked)
```bash
sudo env PATH=$PATH /usr/local/cuda/bin/ncu --set full --launch-skip 100 --launch-count 1 \
  -o /tmp/transpose_ncu_report \
  ./benchmarks/ncu_launcher_new_mma /tmp/transpose_ncu.ptx \
  --input-size 128 --output-size 128 --batch-size 8192 --warps 2 --mode transpose
```

#### Step 4: Extract metrics
```bash
/usr/local/cuda/bin/ncu --import /tmp/transpose_ncu_report.ncu-rep --print-summary per-kernel 2>&1 | \
  grep -iE 'Duration|Spilling Req|Warp Cycles Per Issued'
```

**Baseline** (128x128, batch=8192, 2 warps, locked clocks at 2520MHz):

| Metric | Slang | Tin2 |
|--------|-------|------|
| Duration (us) | 31.55 | 39.80 |
| Spilling Requests | 0 | 0 |
| Warp Cycles/Issued | 30.59 | — |

## Quick Smoke Test

For a fast check without ncu (no sudo needed):

```bash
# Compile and run timing only (all-ones data, no validation)
./build/Release/bin/slangc benchmarks/benchmark_single_layer_outer_product.slang \
  -target ptx -entry compute_outer_product -stage compute \
  -o /tmp/outer_smoke.ptx \
  -I build/Release/lib/slang-standard-module-2026.3.1 \
  -DINPUT_SIZE=128 -DOUTPUT_SIZE=128 -DSUBGROUP_COUNT=2

./benchmarks/ncu_launcher_new_mma /tmp/outer_smoke.ptx \
  --input-size 128 --output-size 128 --batch-size 8192 --warps 2 --mode outer_product
```

**Baseline**: ~0.15ms average time (without locked clocks, varies with GPU boost).

### Transpose Smoke Test

```bash
./build/Release/bin/slangc benchmarks/benchmark_single_layer_transpose.slang \
  -target ptx -entry compute_single_layer_transpose -stage compute \
  -o /tmp/transpose_smoke.ptx \
  -I build/Release/lib/slang-standard-module-2026.3.1 \
  -DINPUT_SIZE=128 -DOUTPUT_SIZE=128 -DSUBGROUP_COUNT=2

./benchmarks/ncu_launcher_new_mma /tmp/transpose_smoke.ptx \
  --input-size 128 --output-size 128 --batch-size 8192 --warps 2 --mode transpose
```

**Baseline**: ~0.031ms average time (without locked clocks, varies with GPU boost).

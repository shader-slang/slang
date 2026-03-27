# m16n16k16 CoopMat Refactor — Handoff Document

## What Was Done

Replaced user-facing m16n8k16 CoopMat shape with m16n16k16 as the only exposed MMA shape on CUDA.
Internally, m16n16k16 is implemented via 2x `mma.sync.aligned.m16n8k16` PTX instructions.
MatA stays 16x16 (unchanged). MatB and MatC are now 16x16 (wrapping two physical 16x8 halves).

### Changes

| File | Change |
|------|--------|
| `prelude/slang-cuda-prelude.h` | Combined `_16x16` helpers for FromLocalColumn/Row, ToLocalColumn/Row; `coopMatMulAdd` for m16n16k16 via 2x `Mma16n8k16Helper::eval` with reg pointers; `FromMatA` for MatA→MatB conversion; `clear()` for zero-init using integer zero; `RegisterCount` for MatB = 4 regs; `ChangeMajor` for 16x16 MatB; `__restrict__` and `reinterpret_cast` optimizations |
| `source/slang/hlsl.meta.slang` | `FromMatA` intrinsic; `clear()` intrinsic (CUDA: integer zero, SPIR-V: fill(0)); `FromLocalColumnPackOffsetInto`/`PackInto` for direct array writes; `FromLocalRowPackOffsetInto`/`PackInto` |
| `source/slang/slang-emit-cuda.cpp` | MatA shape maps to `{16,16,16}`; removed stale m16n8k16 entries from `computeShapeCombination` |
| `source/standard-modules/neural/mma-new-helper.slang` | `COLUMN_B/C=16`; `NCoopMatColumn` halves (4→2 for N=32); `outerProductCompute` uses `MatB.FromMatA` + single `coopMatMulAdd`; `clear()` instead of `fill(T(0.0f))` for matC init; `[ForceInline]` on outerProductCompute/Reduce/Store |

### Performance (batch=8192, 2 warps, locked GPU clocks)

| Size | Dims | Slang (ms) | Tin2 (ms) | Ratio |
|------|------|-----------|-----------|-------|
| tiny | 16x32 | 0.0437 | 0.0453 | 0.96x |
| small | 16x64 | 0.0460 | 0.0475 | 0.97x |
| medium | 32x128 | 0.0465 | 0.0471 | 0.99x |
| large | 64x256 | 0.1556 | 0.1572 | 0.99x |
| xlarge | 128x128 | 0.1538 | 0.1527 | 1.01x |

At parity with Tin2 across all sizes.

## Open Issue: Extra b32 Virtual Registers in Build Phase

### Problem

Slang's outer product kernel uses 6515 b32 virtual PTX registers vs Tin2's 5840 (+675).
After the `clear()` fix, the MMA+Store sections match Tin2 exactly.
The remaining +176 unique b32 regs are in the **build phase** (fragment construction via `mmaBatchFromArray_MatA_16x16`).

### Root Cause

`mmaBatchFromArray_MatA_16x16` writes to `fragments[frag_idx].regs[reg_idx]` where `frag_idx` and `reg_idx` are computed at runtime from loop variables. Even though these are constant after `#pragma unroll`, neither nvrtc nor nvcc fully constant-folds the index arithmetic through the pointer indirection.

Tin2's equivalent `from_array` writes to `this->m_mma_mat[mma_idx].m_mma_reg[reg_idx].m_reg` — deeply nested struct members that the compiler resolves to constant offsets.

This generates 98 extra `add.s32`, 98 extra `setp.eq.s32`, 70 extra `and.b32`, and 28 extra `shr.u32` in the build phase.

### Impact

For 128x128 (the only config that spills): OLD and NEW Slang both have STL=43-44, LDL=60-62 vs Tin2's STL=42, LDL=57. The spill difference is small (3-5 instructions) and the perf impact is <1%.

The extra registers don't affect physical register pressure during the MMA loop because they're dead by then. The spill pattern (10 HMMAs before first spill vs Tin2's 131) is from the m16n16k16 tile writing 4 matC regs per MMA call vs 2 in m16n8k16.

### Potential Fix

Rewrite `mmaBatchFromArray_MatA_16x16` to use struct member access instead of pointer+index, matching Tin2's pattern. This requires either:
1. Making the function a member of the fragment type (like Tin2's `from_array` is a member of `Vector`)
2. Using template metaprogramming to resolve indices at compile time

## Profiling Commands

### Compile benchmark to PTX
```bash
./build/Release/bin/slangc benchmarks/benchmark_single_layer_outer_product.slang \
  -target ptx -entry compute_outer_product -stage compute \
  -o /tmp/outer_product_profile.ptx \
  -I build/Release/lib/slang-standard-module-2026.3.1 \
  -DINPUT_SIZE=128 -DOUTPUT_SIZE=128 -DSUBGROUP_COUNT=2
```

### Compile to CUDA (for inspecting generated C++)
```bash
./build/Release/bin/slangc benchmarks/benchmark_single_layer_outer_product.slang \
  -target cuda -entry compute_outer_product -stage compute \
  -o /tmp/outer_product.cu \
  -I build/Release/lib/slang-standard-module-2026.3.1 \
  -DINPUT_SIZE=128 -DOUTPUT_SIZE=128 -DSUBGROUP_COUNT=2
```

### Compile PTX to SASS (for spill analysis)
```bash
truncate -s -1 /tmp/outer_product_profile.ptx  # remove trailing null
/usr/local/cuda/bin/ptxas -arch=sm_89 -o /tmp/slang.cubin /tmp/outer_product_profile.ptx
/usr/local/cuda/bin/cuobjdump --dump-sass /tmp/slang.cubin > /tmp/slang_sass.txt
```

### Compile Tin2 benchmark
```bash
/usr/local/cuda/bin/nvcc -o /tmp/bench_tin2_outer \
  tin2/benchmarks/mlp_perf/bench_tin2_single_layer_outer_product.cu \
  -I tin2/include -arch=sm_89
```

### Extract Tin2 PTX for 128x128 kernel
```bash
/usr/local/cuda/bin/cuobjdump --dump-ptx /tmp/bench_tin2_outer 2>&1 | \
  sed -n '26346,35511p' > /tmp/tin2_128x128_kernel.ptx
```

### Extract Tin2 SASS
```bash
/usr/local/cuda/bin/cuobjdump --dump-sass /tmp/bench_tin2_outer 2>&1 | \
  sed -n '3532,/^$/p' > /tmp/tin2_sass.txt
```

### Run ncu profiling
```bash
# Lock GPU clocks first
sudo nvidia-smi -pm 1
sudo nvidia-smi --lock-gpu-clocks=2520,2520

# Build ncu launcher if needed
/usr/local/cuda/bin/nvcc -o benchmarks/ncu_launcher_new_mma \
  benchmarks/ncu_launcher_new_mma.cu -lcuda -arch=sm_89

# Profile Slang outer product
sudo env PATH=$PATH /usr/local/cuda/bin/ncu --set full --launch-skip 100 --launch-count 1 \
  -o /tmp/slang_outer_128x128 \
  ./benchmarks/ncu_launcher_new_mma /tmp/outer_product_profile.ptx \
  --input-size 128 --output-size 128 --batch-size 8192 --warps 2 --mode outer_product

# Profile Tin2 outer product (xlarge = 5th size, skip 4*101+100=504 launches)
sudo env PATH=$PATH /usr/local/cuda/bin/ncu --set full --launch-skip 504 --launch-count 1 \
  -o /tmp/tin2_outer_128x128 \
  /tmp/bench_tin2_outer --batch-size 8192 --warps 2

# Unlock clocks
sudo nvidia-smi --reset-gpu-clocks
```

### Extract key metrics from ncu reports
```bash
for report in /tmp/slang_outer_128x128.ncu-rep /tmp/tin2_outer_128x128.ncu-rep; do
    echo "=== $(basename $report) ==="
    /usr/local/cuda/bin/ncu --import $report --print-summary per-kernel 2>&1 | \
      grep -E 'Duration|Registers Per Thread|Executed Instructions |Waves Per SM|Spilling Requests|SM Busy|Warp Cycles Per Issued'
    echo ""
done
```

### PTX register analysis
```bash
# Register declarations
grep '\.reg' /tmp/outer_product_profile.ptx | head -6

# Instruction counts
echo -n "mma.sync: "; grep -c 'mma\.sync' /tmp/outer_product_profile.ptx
echo -n "shfl.sync: "; grep -c 'shfl\.sync' /tmp/outer_product_profile.ptx
echo -n "movmatrix: "; grep -c 'movmatrix' /tmp/outer_product_profile.ptx

# Unique b32 regs per section
fs=$(grep -n 'mma\.sync' /tmp/outer_product_profile.ptx | head -1 | cut -d: -f1)
echo -n "Build phase: "; head -$fs /tmp/outer_product_profile.ptx | grep -oP '%r\d+' | sort -u | wc -l
echo -n "Compute+Store: "; tail -n +$fs /tmp/outer_product_profile.ptx | grep -oP '%r\d+' | sort -u | wc -l

# MMA accumulator reuse check
grep 'mma\.sync' /tmp/outer_product_profile.ptx | awk -F'[{}]' '{print $8}' | sort -u | wc -l
echo "unique accumulator pairs (should be ~129, matching Tin2)"
```

### SASS spill analysis
```bash
echo -n "STL="; grep -c 'STL' /tmp/slang_sass.txt
echo -n "LDL="; grep -c 'LDL' /tmp/slang_sass.txt
echo -n "HMMA="; grep -c 'HMMA' /tmp/slang_sass.txt

# HMMAs before first spill cluster
second_stl=$(grep -n 'STL' /tmp/slang_sass.txt | sed -n '2p' | cut -d: -f1)
echo -n "HMMAs before spill: "; head -$second_stl /tmp/slang_sass.txt | grep -c 'HMMA'
```

### Run benchmarks (with locked clocks)
```bash
source ~/pip_venv/bin/activate
python benchmarks/benchmark_single_layer_outer_product.py --all --batch-size 8192 --warps 2 --device cuda
/tmp/bench_tin2_outer --batch-size 8192 --warps 2
```

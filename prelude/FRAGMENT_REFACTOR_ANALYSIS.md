# Fragment Refactor Analysis — Outer Product 128x128

## Summary

Replaced CUDA-only `FromLocalRowPack*` intrinsics with `buildAllTilesFromArray` implemented
in pure Slang using `fragmentWrite` + `WaveMaskReadLaneAt`. After 32-bit shared memory
addressing optimization and correctness fixes, Slang now beats Tin2 by 5%.

## ncu Metrics (128x128, batch=8192, 2 warps, locked clocks)

| Metric | Original Slang | Optimized Slang | Tin2 |
|--------|---------------|-----------------|------|
| Duration (us) | 156.06 | **149.41** | 157.12 |
| Executed Instructions | 10,334,208 | 13,701,120 | 12,095,488 |
| Registers Per Thread | 255 | 255 | 255 |
| Spilling Requests | 557,056 | **499,712** | 507,904 |
| Warp Cycles/Issued | 24.17 | **17.47** | 20.73 |
| SM Busy | 18.81% | 19.13% | 18.69% |
| Theoretical Occupancy | 12.50% | 12.50% | 12.50% |
| Achieved Occupancy | 12.10% | 12.06% | 12.05% |

## ptxas -v Spill Report (verified, sm_89)

| Metric | Original Slang | Optimized Slang | Tin2 |
|--------|---------------|-----------------|------|
| Spill stores | 244 bytes | **192 bytes** | 220 bytes |
| Spill loads | 292 bytes | **240 bytes** | 228 bytes |
| Registers | 255 | 255 | 255 |
| Shared memory | 32768 bytes | 32768 bytes | 32768 bytes |

## SASS Spill Analysis (verified via cuobjdump)

| Instruction | Slang | Tin2 |
|-------------|-------|------|
| HMMA | 256 | 256 |
| STL (spill stores) | 44 | 42 |
| LDL (spill loads) | 62 | 57 |
| STS (shared stores) | 128 | 128 |
| LDS (shared loads) | 320 | 320 |

Spill timing relative to HMMA instructions:
- **Slang**: 1 STL before first HMMA (R252, thread index spill), then 9 STL + 1 LDL burst
  after HMMA #10. HMMA results are evicted early to make room for more MMA outputs.
- **Tin2**: 0 STL before any HMMA. First STL at HMMA #131. Runs 131 HMMAs spill-free.

## PTX Register Declarations

| Type | Slang | Tin2 |
|------|-------|------|
| .b32 | %r<6331> | %r<5840> |
| .b16 | %rs<257> | %rs<1793> |
| .b64 | %rd<529> | %rd<515> |
| .f64/.f32 | %fd<257> (f64) | %f<257> (f32) |
| .pred | %p<142> | %p<152> |

Notable: Tin2 declares 7x more b16 registers (1792 vs 256). Tin2 unpacks f16 halves into
individual b16 registers, while Slang keeps halves packed as b32.

## Root Cause and Fix

The 4% gap was caused by **64-bit shared memory addressing** in Slang's generated PTX.

### The problem

Slang emits groupshared array references as `((&data_0))->m_data` in CUDA C++. The `&`
operator on a `__shared__` variable produces a generic 64-bit pointer, and all downstream
pointer arithmetic inherits 64-bit addressing. nvrtc does not recover the shared-memory
provenance even after inlining, so all `ld.shared` and `st.shared` instructions use 64-bit
`%rd` registers instead of 32-bit `%r` registers. Each b64 register consumes 2 physical
register slots, increasing register pressure and causing ptxas to spill earlier.

Tin2 accesses `__shared__` arrays directly within the kernel scope. nvrtc can prove the
pointers are in shared memory and uses 32-bit `.shared` addressing throughout.

### The fix (commit `774fb28ec`)

Three shared memory access paths were converted to 32-bit addressing:

1. **smemLoad/smemStore**: New `Slang_SharedMemLoad`/`Slang_SharedMemStore` prelude helpers
   use `__cvta_generic_to_shared()` to extract the 32-bit shared offset, then explicit
   `ld.shared.u32`/`st.shared.u32` PTX inline asm.

2. **storeNative/loadNative**: Compute 32-bit shared offset via `__cvta_generic_to_shared()`,
   then convert back to a valid generic pointer via `cvta.shared.u64` for vector
   `uint4`/`uint2` stores and loads.

### Experiments that did NOT help

- **b16 register mix**: Matching Tin2's unpacked `add.f16` pattern (1792 b16 registers)
  had no effect on spilling. The b16/b32 register mix does not influence ptxas allocation.
- **f64 vs f32 constant init**: Slang emits `cvt.rn.f16.f64` while Tin2 uses
  `cvt.rn.f16.f32`. Patching to f32 had no effect (benchmark-specific, not production).
- **Post-MMA stripping**: Removing reduce/store phases reduced spills for both kernels
  equally (~50%), confirming ptxas does global register allocation. But 32-bit addressing
  was the actionable fix.

## Benchmark Results (all sizes)

| Size | Dims | Slang (ms) | Tin2 (ms) | Ratio |
|------|------|-----------|-----------|-------|
| tiny | 16x32 | 0.0404 | 0.0429 | 0.94x |
| small | 16x64 | 0.0433 | 0.0440 | 0.98x |
| medium | 32x128 | 0.0441 | 0.0461 | 0.96x |
| large | 64x256 | 0.1490 | 0.1454 | 1.02x |
| xlarge | 128x128 | 0.1455 | 0.1461 | 1.00x |

## Files Changed

| File | Change |
|------|--------|
| `prelude/slang-cuda-prelude.h` | Added `FragmentWrite`, `FragmentRead`, `GetPackedFragmentCount`; added `Slang_SharedMemLoad`/`Store` with 32-bit shmem addressing; converted `storeNative`/`loadNative` to 32-bit shmem via `cvta.shared.u64` |
| `source/slang/hlsl.meta.slang` | Added `fragmentWrite`, `fragmentRead`, `getPackedFragmentCount` to `CoopMat` (CUDA + SPIR-V) |
| `source/standard-modules/neural/mma-new-helper.slang` | Added `buildAllTilesFromArray`, replaced `FromLocalRowPack*` in outer product |
| `source/standard-modules/neural/shared-memory-pool.slang` | Routed `smemLoad`/`smemStore` CUDA path through `Slang_SharedMemLoad`/`Store` via `__intrinsic_asm` |

## Profiling Commands

## Correctness Fixes

The outer product had correctness bugs that were invisible with uniform (all-ones) data
but produced wrong results with per-thread varying data.

### Bug 1: MatA RegisterCount was 8 instead of 4

The m16n8k16 MatA operand needs only 4 registers per thread (4 × f16x2), but Slang
declared 8. This wasted registers and contributed to spilling. The MMA only used
regs[0-3]; regs[4-7] were allocated but unused.

### Bug 2: mmaChangeMajor had a built-in register swap

The `mmaChangeMajor` function for MatA combined movmatrix AND a register 1↔2 swap
in a single operation. This meant `ChangeMajor()` was equivalent to Tin2's
`transpose() + change_major_axis()` combined — making it impossible to use them
separately. Fixed to only apply movmatrix (pure `change_major_axis` equivalent).

### Bug 3: Missing dOut transpose in outer product

Tin2's outer product does `bt = change_major_axis(dout.transpose())` — transpose
THEN change_major_axis. The transpose (reg 1↔2 swap) converts the dOut layout from
"rows=threads, cols=elements" to "rows=elements, cols=threads" so the MMA's K-sum
corresponds to the thread sum. Added `transposeTilesA` (reg 1↔2 swap) on dOut tiles
before `ChangeMajor()`.

### No matC transpose needed (unlike Tin2)

Tin2 transposes the MMA result (`w.transpose()`) before storing because Tin2's
forward pass loads weights as MatrixB (col-major). Slang's forward pass loads
weights as MatA (row-major), so no result transpose is needed — the outer product's
native layout directly matches the forward weight layout.

### Validation

Result validation uses `--val` flag:
```bash
source ~/pip_venv/bin/activate
python3 benchmarks/benchmark_single_layer_outer_product.py --all --warps 2 --val
```

## Profiling Commands

### Compile Slang PTX
```bash
./build/Release/bin/slangc benchmarks/benchmark_single_layer_outer_product.slang \
  -target ptx -entry compute_outer_product -stage compute \
  -o /tmp/xlarge_ncu.ptx \
  -I build/Release/lib/slang-standard-module-2026.3.1 \
  -DINPUT_SIZE=128 -DOUTPUT_SIZE=128 -DSUBGROUP_COUNT=2
```

### Compile Tin2
```bash
/usr/local/cuda/bin/nvcc -o /tmp/bench_tin2_outer \
  tin2/benchmarks/mlp_perf/bench_tin2_single_layer_outer_product.cu \
  -I tin2/include -arch=sm_89
```

### Run ncu
```bash
sudo nvidia-smi -pm 1
sudo nvidia-smi --lock-gpu-clocks=2520,2520

# Slang
sudo env PATH=$PATH /usr/local/cuda/bin/ncu --set full --launch-skip 100 --launch-count 1 \
  -o /tmp/xlarge_current_ncu \
  ./benchmarks/ncu_launcher_new_mma /tmp/xlarge_ncu.ptx \
  --input-size 128 --output-size 128 --batch-size 8192 --warps 2 --mode outer_product

# Tin2 (xlarge only)
sudo env PATH=$PATH /usr/local/cuda/bin/ncu --set full --launch-skip 100 --launch-count 1 \
  -o /tmp/tin2_outer_128x128_new \
  /tmp/bench_tin2_outer --batch-size 8192 --warps 2 --size xlarge

sudo nvidia-smi --reset-gpu-clocks
```

### Assemble Slang PTX and dump SASS
```bash
# Strip trailing null byte from slangc output, assemble with verbose output
head -c -1 /tmp/xlarge_ncu.ptx > /tmp/xlarge_ncu_fixed.ptx
/usr/local/cuda/bin/ptxas -v --gpu-name sm_89 /tmp/xlarge_ncu_fixed.ptx -o /tmp/slang_outer.cubin
/usr/local/cuda/bin/cuobjdump --dump-sass /tmp/slang_outer.cubin > /tmp/slang_sass.txt
```

### Compile Tin2 with ptxas verbose and dump SASS
```bash
/usr/local/cuda/bin/nvcc -o /tmp/bench_tin2_outer \
  tin2/benchmarks/mlp_perf/bench_tin2_single_layer_outer_product.cu \
  -I tin2/include -arch=sm_89 -Xptxas -v --keep --keep-dir /tmp/tin2_keep
/usr/local/cuda/bin/cuobjdump --dump-sass \
  /tmp/tin2_keep/bench_tin2_single_layer_outer_product.sm_89.cubin > /tmp/tin2_sass.txt
```

### Extract metrics
```bash
for report in /tmp/xlarge_current_ncu.ncu-rep /tmp/tin2_outer_128x128_new.ncu-rep; do
    echo "=== $(basename $report) ==="
    /usr/local/cuda/bin/ncu --import $report --print-summary per-kernel 2>&1 | \
      grep -E 'Duration|Registers Per Thread|Executed Instructions |Spilling|Warp Cycles|Occupancy'
done
```

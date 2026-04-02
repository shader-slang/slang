# MMAHelperNew: Zero-Shared-Memory MMA Implementation

## Summary

MMAHelperNew performs tiled matrix-vector multiply with **zero shared memory** for the MMA phase. Weight tiles are loaded directly from global memory in native fragment layout (`LoadRaw`), input vectors are assembled via batched warp shuffles (`buildAllTilesFromArray`), and fragment conversion uses `ChangeMajor` + `copyFrom` (register ops only).

The backward pass chains three operations: transpose MMA (`dIn = W^T * dOut`), outer product (`dW = dOut * input^T`), and bias gradient reduction (`dBias = sum(dOut)`).

## Current API Surface

### CoopMat intrinsics (hlsl.meta.slang + slang-cuda-prelude.h)

These are the remaining intrinsic operations that still require C++ prelude support because they cannot be expressed in pure Slang:

| Method | Why intrinsic | Description |
|--------|--------------|-------------|
| `LoadRaw(RWStructuredBuffer<T>, uint)` | Vectorized LDG.E.128 from native layout | Buffer load, no shuffles |
| `LoadRaw(StructuredBuffer<T>, uint)` | Same, readonly variant | |
| `LoadRaw(T*, uint)` | Same, pointer variant | |
| `StoreRaw(RWStructuredBuffer<T>, uint)` | Vectorized store to buffer | |
| `StoreRaw(T*, uint)` | Vectorized store to pointer | |
| `ChangeMajor()` | Uses `movmatrix` PTX instruction | In-register transpose (MatrixA only) |
| `copyFrom(CoopMat other)` | `reinterpret_cast<uint4*>` register copy | Cross-MatrixUse vectorized copy |
| `fragmentRead(int)` | Direct register access | Read one 32-bit fragment register |
| `fragmentWrite(int, uint)` | Direct register access | Write one 32-bit fragment register |
| `fill(T value)` | Packed broadcast | Fill all registers |
| `clear()` | Integer zero | Zero all registers |
| `coopMatMulAdd(A, B, C)` | `mma.sync.aligned.m16n8k16` PTX | Tensor core MMA |

### Operations moved from intrinsics to neural.slang

These were previously C++ intrinsics in `slang-cuda-prelude.h` but are now implemented in pure Slang using only `fragmentRead`/`fragmentWrite` + warp shuffles + `smemStore`/`smemLoad`:

| Removed intrinsic | Slang replacement | Location |
|-------------------|-------------------|----------|
| `FromLocalRow` / `FromLocalColumn` | `buildAllTilesFromArray` (batched shuffles) | `mma-new-helper.slang` |
| `ToLocalRow` / `ToLocalColumn` | Direct `fragmentRead` → shmem/output | `mma-new-helper.slang` |
| `FromMatA` (MatA → MatB copy) | `copyFrom` (vectorized, generic) | `hlsl.meta.slang` |
| `ExtractMatBLo/Hi` | Unused, removed | — |
| `storeShMem` / `loadShMem` / `loadShMemSingle` | Unused, removed | — |
| Groupshared `StoreRaw` / `LoadRaw` | `storeTileToShmem` / `loadTileFromShmem` | `mma-new-helper.slang`, `accelerate-vector-coopmat.slang` |
| `FromLocalRowPack*` variants | `buildAllTilesFromArray` | `mma-new-helper.slang` |
| 26+ C++ shuffle helpers (`mmaFromRow*`, etc.) | All removed | `slang-cuda-prelude.h` |

The benefit: fragment construction and shmem tile store/load are now portable Slang code that can target SPIR-V in the future, with only the low-level MMA instruction and register access remaining as CUDA-specific intrinsics.

### Shared Memory Pool (shared-memory-pool.slang)

| Feature | Description |
|---------|-------------|
| `ShmemAddressMode.Shared` | Direct `st.shared.v4.u32` asm (fewer instructions, constrains ptxas) |
| `ShmemAddressMode.Generic` | `cvta.shared.u64` → generic ptr → dereference (ptxas can reschedule) |
| `smemStore<U, Mode>(offset, index, value)` | Mode-aware store (default: Shared) |
| `smemLoad<U, Mode>(offset, index)` | Mode-aware load (default: Shared) |
| `storeTileToShmem(tile, offset)` | Store MatC fragment via Generic mode (in MMAHelperNew & TiledMMAHelper) |
| `loadTileFromShmem(offset)` | Load MatC fragment via Generic mode |

### MMAHelperNew operations (mma-new-helper.slang)

| Operation | Method | Description |
|-----------|--------|-------------|
| Forward MMA | `mma<...>()` | W × input, zero shmem |
| Transpose MMA | `mma<..., TransposeA=true>()` | W^T × dOut, zero shmem |
| Outer product | `outerProductAccumulate<...>()` | dW = dOut × input^T, shmem for cross-warp reduction |
| Bias reduce (MMA) | `sumReduceRows<...>()` | MMA-based: ones × B, then direct register read |
| Bias reduce (shuffle) | `sumReduceRowsV2<...>()` | Shuffle-based tree reduction (matches Tin2 perf) |
| In-warp reduce helper | `reduceMatATilesToShmem<...>()` | Register adds + shfl_down, writes to shmem |
| Cross-warp reduce | `crossWarpReduceAndStore<...>()` | Shmem reduction + atomicAdd to global |
| Tile construction | `buildAllTilesFromArray<...>()` | Batched warp shuffles for MatA tiles |

## Performance Results (128×128, 2 warps, batch=8192, locked 2520MHz)

### Decomposed Operations (ncu profiler)

| Operation | Slang (us) | Tin2 (us) | Registers | Spilling |
|-----------|-----------|----------|-----------|----------|
| Bias reduce | 12.06 | 12.06 | 19 vs 16 | 0 vs 0 |
| Outer product | 152 | 157 | 255 vs 255 | 499K vs 508K |
| Transpose MMA | 31.1 | 39.8 | 54 vs — | 0 vs 0 |

### Chained Backward (ncu profiler, constant data)

| Metric | Slang | Tin2 |
|--------|-------|------|
| Duration | 206 us | 221 us |
| Spilling Requests | 581K | 1.3M |
| Executed Instructions | 22.4M | 24.0M |
| DRAM Throughput | 3.9% | 7.9% |

Slang is ~7% faster due to 2.2x less spilling from better ptxas register allocation.

## Address Mode Analysis

We discovered that shared memory access patterns significantly impact register allocation:

- **`st.shared.v4.u32`** (inline asm) generates `STS.128` in SASS. All 4 data registers must be live simultaneously. For 255-register kernels, this forces extra spill-reloads (37→44 STL in our measurements).

- **`cvta.shared.u64` → `*ptr = value`** generates `ST.E.128` in SASS via generic addressing. ptxas can decompose and reschedule freely. Costs 3 extra SASS instructions per store (`IADD3` + `IADD3` + `IMAD.X` for 64-bit address), but eliminates the spill-reloads.

Both produce identical throughput (128-bit vectorized store to shared memory). The `ShmemAddressMode` enum lets callers choose based on register pressure.

## Files

| File | Purpose |
|------|---------|
| `prelude/slang-cuda-prelude.h` | `LoadRaw` (buffer), `copyFrom`, `ChangeMajor`, `mmaBatchFromArray`, `mmaChangeMajor`, `mmaExtractMatB*`, `coopMatMulAdd` |
| `source/slang/hlsl.meta.slang` | `LoadRaw`, `StoreRaw` (buffer/pointer), `ChangeMajor`, `copyFrom`, `fragmentRead/Write`, `fill`, `clear` |
| `source/standard-modules/neural/shared-memory-pool.slang` | `ShmemAddressMode`, `smemStore/Load<U, Mode>`, `smemStoreViaGenericPtr`, `smemLoadViaGenericPtr` |
| `source/standard-modules/neural/mma-new-helper.slang` | `MMAHelperNew`: `mma`, `outerProductAccumulate`, `sumReduceRows/V2`, `reduceMatATilesToShmem`, `crossWarpReduceAndStore`, `buildAllTilesFromArray`, `storeTileToShmem`, `loadTileFromShmem` |
| `source/standard-modules/neural/accelerate-vector-coopmat.slang` | `WaveTangledVector` backward: `linearTransformBwdOnTarget`, `storeTileToShmem`, `loadTileFromShmem` |
| `benchmarks/benchmark_single_layer_backward.slang` | Chained backward kernel (transpose + outer product + bias reduce) |
| `benchmarks/run_regression.sh` | Automated PTX compile + validation + ncu + timing comparison |
| `benchmarks/run_decomposed_benchmark.py` | Decomposed benchmark across all sizes/batch sizes |

## How to Run

```bash
# Build
cmake --build --preset release -j12

# Quick regression (validation + timing)
./benchmarks/run_regression.sh --quick

# Full regression (includes ncu profiling, needs sudo)
sudo nvidia-smi --lock-gpu-clocks=2520,2520
./benchmarks/run_regression.sh

# Decomposed benchmark (all operations, all sizes)
python3 benchmarks/run_decomposed_benchmark.py --batch-size 8192

# All batch sizes
python3 benchmarks/run_decomposed_benchmark.py --all-batches

# ncu profiling for specific kernel
./build/Release/bin/slangc benchmarks/benchmark_single_layer_backward.slang \
  -target ptx -entry compute_backward -stage compute \
  -o /tmp/backward.ptx -I build/Release/lib/slang-standard-module-2026.3.1 \
  -DINPUT_SIZE=128 -DOUTPUT_SIZE=128 -DSUBGROUP_COUNT=2
sudo ncu --set full --launch-skip 100 --launch-count 1 \
  ./benchmarks/ncu_launcher_new_mma /tmp/backward.ptx \
  --input-size 128 --output-size 128 --batch-size 8192 --warps 2 --mode backward
```

## References

- `prelude/MMA_VERIFICATION.md` — validation + ncu baselines for outer product and transpose
- `prelude/CLEANUP_VOID_DIAGNOSTIC_NOTE.md` — compiler diagnostic for zero-sized groupshared arrays
- PTX ISA: [mma.sync.aligned.m16n8k16](https://docs.nvidia.com/cuda/parallel-thread-execution/#warp-level-matrix-fragment-mma-16816-float)

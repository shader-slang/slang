# MMAHelperNew Backward Pass: TransposeA, sumReduceRows, outerProductAccumulate

## Summary

Extends MMAHelperNew with backward pass operations: transpose MMA, bias gradient reduction, and outer product weight gradient accumulation. All operations use zero shared memory for the MMA phase (FromLocal intrinsics + ChangeMajor), with shmem only for cross-warp reduction and store phases.

## New Operations

### 1. Transpose MMA (`TransposeA=true`)

`MMAHelperNew<T, InputSize, OutputSize, SubgroupSize, Target, TransposeA=true>`

Computes `dIn = W^T × dOut` for the backward pass input gradient.

- `LoadNative` loads weight tiles, `ChangeMajor()` transposes the fragment registers
- Tile addressing swaps row/col indices for the physical M×K layout
- `SharedDimAligned`/`OutputDim` adjust for the transposed dimensions
- Forward path (`TransposeA=false`) unchanged

**Performance**: Competitive with Tin2 across most configs. Slang's lower register count (62 vs 255) gives 2.76x higher occupancy, enabling better latency hiding at large batch sizes.

### 2. Bias Gradient Reduction (`sumReduceRows`)

`MMAHelperNew.sumReduceRows<U, Address, InArrayType, ShMemSize, SubgroupCount>(...)`

Computes `bias_grad[i] = sum_{all lanes} dOut[i]` using MMA.

- MatA = all-ones (16×16), MatB = dOut via `FromLocalRow` (thread identity in K)
- `NRowGroups=2` iterations cover all 32 warp threads (16 per FromLocalRow group)
- Per-warp results written to shmem, cross-warp reduction via half2 atomicAdd
- 8 output elements per tile (COLUMN_B=8), `NBiasTiles = (M+7)/8` tiles

**Known limitation**: `FromLocalRow` on MatB fights the column-major hardware layout, generating predicated shuffle loops. For large M, this creates register pressure. Future optimization: WaveActiveSum or shmem-based loading.

### 3. Outer Product (`outerProductAccumulate`)

`MMAHelperNew.outerProductAccumulate<U, Address, TypeA, InArrayTypeA, TypeB, InArrayTypeB, ShMemSize, SubgroupCount>(...)`

Computes `dW[m][k] = sum_{all threads} dOut[m] × input[k]` for weight gradients.

Split into 3 non-inlined functions for register scope isolation:

1. `**outerProductCompute`**: MMA phase with `FromLocalRow + ChangeMajor` on MatA and `FromLocalColumn + ChangeMajor` on MatB. Thread identity in K (contraction axis). All matC tiles computed.
2. `**outerProductReduce**`: Cross-warp tree-reduce via `storeNative`/`loadNative` on shmem. Same pattern as `MMAHelper.sumReduceTilesAllAtOnce`.
3. `**outerProductStore**`: `storeNative` all matC tiles to shmem, then atomicAdd from shmem to global weight buffer in native fragment layout.

**Performance**: ~1.27x slower than Tin2 on large config. Root cause: 3-4x more register spills due to FromLocal shuffle overhead vs Tin2's zero-shuffle `change_major_axis` on fragment-native vectors. The 128 matC tiles (256 registers) already max out the register file, leaving no room for shuffle temporaries.

## Prelude Changes (`slang-cuda-prelude.h`)

### Register Spill Fixes (predicated assignment pattern)

All `mmaFromRow`* and `mmaFromCol*` functions were optimized to eliminate runtime-indexed intermediate arrays that caused stack spills:


| Function                      | Old pattern                             | Fix                                |
| ----------------------------- | --------------------------------------- | ---------------------------------- |
| `mmaFromRowMatrixA_16x16`     | `s[8][2]` indexed by `tid`              | `if (tid == j)` predicate          |
| `mmaFromRowMatrixB_16x8`      | `s[4][4]` indexed by `k`                | `if (k == ki)` predicate           |
| `mmaFromRowMatrixCD_f32_16x8` | `s[8][2]` indexed by `tid*2`            | `if (tid*2 == j)` predicate        |
| `mmaFromRowMatrixCD_f16_16x8` | `s[4][2]` indexed by `tid`              | `if (tid == j)` predicate          |
| `mmaFromColMatrixA_16x16`     | `packedCol[pk]` indexed by runtime `pk` | `if (k == p) val = packedCol[...]` |
| `mmaFromColMatrixCD_f32_16x8` | `s0[8]` indexed by `gid`                | `if (gid == g)` predicate          |
| `mmaFromColMatrixCD_f16_16x8` | `s[8][2]` indexed by `k`/`k2`           | `if (k == p)` predicate            |


### ChangeMajor for MatB

Added `mmaChangeMajor` support for `MatrixUse::MatrixB` (2 `movmatrix.sync` instructions). Previously only MatA was supported.

### FromLocalRowOffset

Added `CoopMat.FromLocalRowOffset<int ArraySize>(T data[ArraySize], int offset, int matIndex)` — loads from a sub-array without copying. Uses `&(data[offset])` pointer arithmetic in the inline asm.

## Shared Memory Budget (`shared-memory-pool.slang`)

Updated `SharedMemorySize0.Bytes` calculation:

- Removed `BytesForOuterProduct` (old: staged ALL vectors in shmem, caused 64KB overflow for large configs)
- Added `BytesForOuterProductStore` (new: only one row of MatC tiles for store phase)
- `BytesForCrossWarpReduction` unchanged (tree-reduce still needs shmem)

For large (INPUT=256, OUTPUT=64, Training, SubgroupCount=2):

- Old: 64KB (OVERFLOW) → New: 16KB (fits easily)

## WaveTangledVector Integration (`accelerate-vector-coopmat.slang`)

Forward path (`linearTransformOnTarget`): CUDA + OptimalLayout routes through `MMAHelperNew` via `__target_switch`. SPIR-V keeps `TiledMMAHelper`.

## Files Changed


| File                                                             | Changes                                           |
| ---------------------------------------------------------------- | ------------------------------------------------- |
| `prelude/slang-cuda-prelude.h`                                   | Predicated shuffle fixes, ChangeMajor for MatB    |
| `source/slang/hlsl.meta.slang`                                   | `FromLocalRowOffset` intrinsic                    |
| `source/standard-modules/neural/mma-new-helper.slang`            | TransposeA, sumReduceRows, outerProductAccumulate |
| `source/standard-modules/neural/shared-memory-pool.slang`        | Updated shmem budget                              |
| `source/standard-modules/neural/accelerate-vector-coopmat.slang` | CUDA OptimalLayout routing                        |


## Known Issues & Future Work

1. **Outer product spills**: 3-4x more spills than Tin2 due to FromLocal shuffle overhead. Solution: make WaveTangledVector fragment-native (like Tin2's HVector).
2. **Bias reduce**: ~1.4x slower than Tin2. Consider WaveActiveSum or shmem-based MMA loading.
3. **Validation**: Outer product validation with random data has a Python-side bug (constant-value validation passes).
4. **Store addressing**: The outer product store uses native MatC layout for the weight gradient buffer. Verify compatibility with the forward LoadNative (MatA layout).


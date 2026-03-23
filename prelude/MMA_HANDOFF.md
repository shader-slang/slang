# MMA m16n8k16 Implementation Hand-off

## What Was Done

This work adds PTX `mma.sync.aligned.m16n8k16` instruction support to Slang's CUDA cooperative matrix backend, replacing the simpler WMMA instruction path for the m16n8k16 shape. The MMA instructions expose explicit matrix element-to-thread mapping, enabling hand-optimized vectorized loads with warp shuffle redistribution.

## Files Changed

| File | Purpose |
|------|---------|
| `prelude/slang-cuda-prelude.h` | Core MMA load/store/multiply/transpose implementation |
| `prelude/WMMA_IMPLEMENTATION_PLAN.md` | Design documentation with ASCII fragment layout diagrams |
| `source/slang/hlsl.meta.slang` | `ChangeMajor()` method on `CoopMat` type |
| `source/slang/slang-emit-cuda.cpp` | Compiler shape mapping and SM version for m16n8k16 |
| `tests/cooperative-matrix/mma-m16n8k16.slang` | Basic row-major load + MMA test |
| `tests/cooperative-matrix/mma-m16n8k16-colmajor.slang` | Column-major A load test |
| `tests/cooperative-matrix/mma-m16n8k16-all-layouts.slang` | All 8 layout combinations (A x B x C) |
| `tests/cooperative-matrix/mma-m16n8k16-change-major.slang` | In-register transpose via `ChangeMajor()` |

## Architecture

### Load Path (128-bit vectorized + warp shuffle)

Each matrix load uses this pattern:
1. Each thread does a single `uint4` (128-bit) aligned load from global/shared memory
2. Warp shuffles redistribute data so each thread holds the correct MMA fragment registers
3. Layout-specific shuffle patterns handle row-major vs column-major

| Matrix | Row-major | Column-major |
|--------|-----------|-------------|
| A (16x16 f16) | 16 shuffles | 32 shuffles + half-extraction |
| B (16x8 f16) | 16 shuffles + extraction | 8 shuffles |
| C/D (16x8 f32) | 16 shuffles | 16 shuffles |
| C/D (16x8 f16) | 8 shuffles | 16 shuffles + extraction |

### Store Path (128-bit vectorized)

Row-major stores use reverse shuffle to collect contiguous data and write via `uint4` stores. Column-major stores also use shuffles + 128-bit writes.

### ChangeMajor (in-register transpose)

Uses 4x `movmatrix.sync.aligned.m8n8.trans.b16` PTX instructions to transpose a 16x16 f16 matrix in-register. The 16x16 matrix is decomposed into 2x2 blocks of 8x8, each transposed by `movmatrix`, with off-diagonal blocks swapped.

### MMA Instruction

The `mma.sync.aligned.m16n8k16.row.col.f32.f16.f16.f32` PTX instruction performs the multiply-accumulate. Layout is always `row.col` -- the load/store paths handle memory layout conversion.

## Known Limitations / TODO

### Compiler Shape Ambiguity (Prototype Limitation)

The `computeShapeCombination` function in `slang-emit-cuda.cpp` has ambiguous mappings:
- Matrix A (16x16) collides between m16n16k16 (WMMA) and m16n8k16 (MMA)
- Matrix B (16x8) collides between m32n8k16 (WMMA) and m16n8k16 (MMA)

Currently m16n8k16 takes priority, which **breaks** existing m16n16k16 and m32n8k16 WMMA shapes. A proper disambiguation mechanism (e.g., capability attribute, explicit shape annotation, or inference from MulAdd context) is needed before merging to main.

### Features Not Yet Implemented

1. **BF16 input support**: The MMA instruction supports `bf16` inputs but only `f16` is implemented in the load/store paths.
2. **Integer (s8/u8) input support**: m16n8k16 supports integer MMA but is not implemented.
3. **Mixed CType/DType accumulator**: Only `CType == DType` is supported (matching Slang's interface).
4. **ChangeMajor for Matrix B**: Only Matrix A (16x16) is supported. Matrix B (16x8) would need different handling.
5. **ChangeMajor for f32 types**: `movmatrix` only supports `.b16`, so f32 matrices need an alternative approach.
6. **Non-tight-packed strides**: The 128-bit load assumes stride equals the natural matrix width. Arbitrary strides work but may not be optimal.

### SM Version Requirements

- MMA m16n8k16: requires sm_80 (Ampere) or higher
- `movmatrix` (ChangeMajor): requires sm_75 (Turing) or higher (already covered by sm_80 requirement)

## How to Test

```bash
# Build
cmake --build --preset release --target slangc slang-test

# Run all MMA tests
./build/Release/bin/slang-test tests/cooperative-matrix/mma-m16n8k16.slang
./build/Release/bin/slang-test tests/cooperative-matrix/mma-m16n8k16-colmajor.slang
./build/Release/bin/slang-test tests/cooperative-matrix/mma-m16n8k16-all-layouts.slang
./build/Release/bin/slang-test tests/cooperative-matrix/mma-m16n8k16-change-major.slang
```

Requires an Ampere (RTX 3000 / A100) or newer GPU.

## References

- [PTX ISA: mma.sync.aligned.m16n8k16](https://docs.nvidia.com/cuda/parallel-thread-execution/#warp-level-matrix-fragment-mma-16816-float)
- [PTX ISA: movmatrix](https://docs.nvidia.com/cuda/parallel-thread-execution/#warp-level-matrix-instructions-movmatrix)
- `prelude/WMMA_IMPLEMENTATION_PLAN.md` for detailed fragment layout diagrams and shuffle step-by-step figures

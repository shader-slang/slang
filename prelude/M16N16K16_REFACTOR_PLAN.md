# m16n16k16 CoopMat at Intrinsic Level

## Status

- [x] Step 0: Revert wrapper structs from mma-new-helper.slang
- [ ] Step 1: Prelude — Replace m16n16k16 coopMatMulAdd with 2x Mma16n8k16Helper
- [ ] Step 2: Prelude — Add combined _16x16 helper functions + update WmmaFragment methods
- [ ] Step 3: hlsl.meta.slang — Shape routing for m16n16k16
- [ ] Step 4: Simplify mma-new-helper.slang (COLUMN_B/C=16)
- [ ] Step 5: Build and verify benchmarks

## Core Idea

From the user's perspective, the only MMA shape is **m16n16k16**. There is no m16n8k16 exposed. Internally, each m16n16k16 operation runs 2x `mma.sync.aligned.m16n8k16` PTX instructions, but this is entirely hidden inside the CUDA prelude.

This replaces the old WMMA-based m16n16k16 implementation with our new paired-m16n8k16 approach.

## Register Layout (half)

- **MatA 16x16**: 8 regs (unchanged — same physical layout as the m16n8k16 MatA)
- **MatB 16x16**: lo(regs[0:1]) + hi(regs[2:3]) = 4 regs used (of 8 allocated)
- **MatC f16 16x16**: lo(regs[0:1]) + hi(regs[2:3]) = 4 regs (exact fit)
- **MatC f32 16x16**: lo(regs[0:3]) + hi(regs[4:7]) = 8 regs (exact fit)

## Step 1: Prelude — Replace m16n16k16 coopMatMulAdd

In `prelude/slang-cuda-prelude.h` `coopMatMulAdd` (~line 10111):

Replace the old WMMA m16n16k16 path. The 16x16 fragment's `regs` array directly holds both halves — no temporaries needed. Just call the m16n8k16 PTX helper twice with pointer offsets:

```cpp
if constexpr (M == 16 && N == 16 && K == 16)
{
    // m16n16k16 via 2x m16n8k16
    // matB.regs[0:1] = lo, matB.regs[2:3] = hi
    // matC/matD: [0:1]/[2:3] for f16, [0:3]/[4:7] for f32
    Mma16n8k16Helper<AType, BType, CType>::eval(matD.regs,     matA.regs, matB.regs,     matC.regs);     // lo
    Mma16n8k16Helper<AType, BType, CType>::eval(matD.regs + 2, matA.regs, matB.regs + 2, matC.regs + 2); // hi
}
```

Note: `Mma16n8k16Helper::eval` currently takes WmmaFragment references. Need to change it to accept raw `unsigned*` pointers (or add an overload).

Remove the old `m16n8k16` user-facing branch — only m16n16k16 exists now.

## Step 2: Prelude — Combined _16x16 helpers + WmmaFragment methods

Add combined 16x16 versions of the helper functions that do both halves in one call:

**`mmaFromColMatrixB_16x16`**: Builds a 16x16 MatB from local columns (4 regs output):
```cpp
__device__ inline void mmaFromColMatrixB_16x16(
    uint32_t* regs, const void* localCol, int matIndex, unsigned gid, unsigned tid)
{
    uint32_t packedCol[8];
    memcpy(packedCol, localCol, 32);
    unsigned base = matIndex * 16;
    const uint32_t mask = 0xFFFFFFFF;
    uint32_t tmp;
    #pragma unroll
    for (int t = 0; t < 4; t++)
    {
        tmp = __shfl_sync(mask, packedCol[t],     gid + base);      if (tid == t) regs[0] = tmp;
        tmp = __shfl_sync(mask, packedCol[t + 4], gid + base);      if (tid == t) regs[1] = tmp;
        tmp = __shfl_sync(mask, packedCol[t],     gid + base + 8);  if (tid == t) regs[2] = tmp;
        tmp = __shfl_sync(mask, packedCol[t + 4], gid + base + 8);  if (tid == t) regs[3] = tmp;
    }
}
```

Same pattern for: `mmaToColMatrixCD_f16_16x16`, `mmaFromRowMatrixB_16x16`, `mmaToRowMatrixCD_f16_16x16`.

Update WmmaFragment methods:
- Change `static_assert(M == 16 && N == 8 && K == 16)` to `static_assert(M == 16 && (N == 8 || N == 16) && K == 16)`
- For `N == 16`: dispatch to the combined `_16x16` helpers
- For `N == 8`: keep existing `_16x8` helpers

**`FromMatA`** (new): Convert a 16x16 MatA to a 16x16 MatB (register copy, no shuffles):
```cpp
static This FromMatA(const WmmaFragment<T, 16, 16, 16, MatrixUse::MatrixA>& matA)
{
    This result;
    memcpy(result.regs, matA.regs, 4 * sizeof(unsigned));
    return result;
}
```

Generic methods (`fill`, `storeNative`/`loadNative`, arithmetic) already work for any RegsCount.

## Step 3: hlsl.meta.slang — Shape routing

- Ensure `CoopMat<half, ..., 16, 16, MatrixB>` is valid on CUDA
- Ensure `coopMatMulAdd<T, false, half, half, T, ..., 16, 16, 16>` routes correctly
- Add `FromMatA` intrinsic declaration
- Check for shape guards that reject N=16 for MatB/MatC and relax them

## Step 4: Simplify mma-new-helper.slang

```slang
static const int COLUMN_B = 16;  // was 8
static const int COLUMN_C = 16;  // was 8

typealias MatB = CoopMat<half, ..., 16, 16, MatrixB>;
typealias MatC = CoopMat<T,    ..., 16, 16, Accumulator>;
```

Everything simplifies naturally:
- `NCoopMatColumn = (32 + 16 - 1) / 16 = 2` (was 4)
- Forward `mma`: `MatB.FromLocalColumn(localColB, j)` with j=0..1, single `coopMatMulAdd`
- `outerProductCompute`: `MatB.FromMatA(inputTile)` + single `coopMatMulAdd` — no more `ExtractMatBLo/Hi`
- `OPA_CoopMatCColumns` halves, `OPA_TotalTiles` halves
- `outerProductReduce`/`outerProductStore` work unchanged (storeNative/loadNative are generic)

## Files to Modify

| File | Change |
|------|--------|
| `prelude/slang-cuda-prelude.h` | Combined _16x16 helpers, WmmaFragment dispatch, coopMatMulAdd, FromMatA |
| `source/slang/hlsl.meta.slang` | Shape validation, FromMatA intrinsic |
| `source/standard-modules/neural/mma-new-helper.slang` | COLUMN_B/C=16, remove ExtractMatBLo/Hi |

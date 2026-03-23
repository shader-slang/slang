# Fragment-Based Internal Storage for WaveTangledVector

## Motivation

Currently, `WaveTangledVector` stores its data as `T[Uint4AlignedInputSize]` (a plain half array). Every MMA operation rebuilds CoopMat fragments from scratch via expensive shuffle operations:

- **Forward `mma`**: For each of K/16 tiles, builds 4 MatB fragments via `FromLocalColumn` (8 shuffles each) = K/16 x 4 x 8 = **K*2 shuffles**. For K=128: **256 shuffles**.
- **Backward `outerProductCompute`**: Builds all MatA fragments from both input vectors via `FromLocalRowPackOffset` = **(M/16 + K/16) x 8 shuffles**. For M=64, K=128: **96 shuffles**.

By pre-building fragments at construction time and storing them as the internal representation, all subsequent MMA operations can skip the shuffle-heavy fragment construction entirely.

**For K=128 forward**: Construction costs 64 shuffles once (via `FromLocalRowPackOffset`), then MMA uses `ChangeMajor` (movmatrix, not a shuffle) + `ExtractMatBLo/Hi` (register copy) = **0 shuffles in MMA loop**. Net savings: ~75%.

## Scope

- **CUDA + OptimalLayout only** (MMAHelperNew path)
- SPIR-V and non-Optimal paths continue using `T[]` arrays through existing `IArrayAccessor` interface

## Data Layout

For a vector of N elements with SubgroupSize=32 and 16x16 tiles:

```
N=128 -> 128x32 warp matrix -> 8 row-tiles x 2 thread-groups = 16 MatA tiles
Each MatA tile = 4 uint32 per thread = 1 uint4 per thread
Storage: uint4[16] per thread (= 256 bytes/thread for N=128)

General formula: TotalTiles = ceil(N/16) * 2
```

### Current vs Proposed Flow

**Current: Shuffle at MMA Time**
```
__init(T[] arr) --copy elements--> T[N] data
    data --FromLocalColumn per K-tile--> mma: 256 shuffles
    data --FromLocalRowPackOffset--> outerProduct: 96 shuffles
```

**Proposed: Shuffle at Init Time**
```
__init(T[] arr) --FromLocalRowPackOffset--> Fragment data: 64 shuffles (once)
    data --ChangeMajor + ExtractMatB--> mma: 0 shuffles
    data --direct use--> outerProduct: 0 shuffles
```

## Step 1: Define `Fragment` struct

Location: `source/standard-modules/neural/accelerate-vector-coopmat.slang` (or a new file)

```slang
[Differentiable]
public struct Fragment<int NumElements, int SubgroupSize = 32> : IDifferentiable
{
    static const int NTileRows = (NumElements + 15) / 16;
    static const int NTileGroups = (SubgroupSize + 15) / 16;  // 2 for 32 threads
    static const int TotalTiles = NTileRows * NTileGroups;

    // Each 16x16 MatA tile = 4 uint32 = 1 uint4 per thread.
    // uint4 is NOT differentiable, so Fragment implements IDifferentiable manually.
    uint4 tiles[TotalTiles];

    typealias Differential = Fragment<NumElements, SubgroupSize>;

    static Differential dzero() { /* zero-init all tiles */ }
    static Differential dadd(Differential a, Differential b) {
        // bit_cast halves, add pairwise, bit_cast back
    }
}
```

The `Differential` of a Fragment is another Fragment -- the derivative of each half element is stored in the same uint4 bit layout in the differential Fragment.

## Step 2: Modify `WaveTangledVector` internal storage

In `source/standard-modules/neural/accelerate-vector-coopmat.slang` ~line 1191:

**Before:**
```slang
[DerivativeMember(Differential.data)]
internal T[Uint4AlignedInputSize] data;
```

**After:**
```slang
[DerivativeMember(Differential.data)]
internal Fragment<N, SubgroupSize> data;
```

## Step 3: Update constructors (`__init`)

- **`__init(T[Size] inData)`**: Call `FromLocalRowPackOffset` in a loop over N/16 chunks to populate `data.tiles[]`. This is where the shuffles happen (once, at construction time).
- **`__init(T value)`**: Broadcast value then build fragments.
- **`__init()`**: Zero-initialize all tiles.
- Provide bulk conversion helpers: `static Fragment fromArray(T[N] arr)` and `T[N] toArray()`.

## Step 4: Custom subscript with backward

```slang
[ForwardDerivative(subscript_fwd)]
[BackwardDerivative(subscript_bwd)]
public __subscript(int index) -> T
{
    get {
        // Determine which tile and register position holds data[index]
        // Unpack the half value from the uint4 tile (bit manipulation, not shuffles)
    }
    set {
        // Pack the half value into the correct uint4 position
    }
}
```

For the custom backward of `get(index)`:
- Given `d_output`, the backward writes: `d_fragment.set(index, d_output)`

Subscript will require pack/unpack since Fragment wraps uint types internally. This is acceptable since element-by-element access is rare in hot MMA paths.

## Step 5: Update `MMAHelperNew.mma` to accept Fragment input

In `source/standard-modules/neural/mma-new-helper.slang`, add an `mma` overload (or modify existing) that takes `Fragment` instead of `IArrayAccessor`.

**Current** (line 146-156, per K-tile, per column group):
```slang
half localColB[ROW_B];
for (uint r = 0; r < ROW_B; r++)
    localColB[r] = inputVector[srcIdx + r];  // element access
let matB = MatB.FromLocalColumn(localColB, j);  // 8 shuffles
```

**Proposed** (per K-tile, per thread-group):
```slang
let inputTile = inputFragment.tiles[tileColIdx * NTileGroups + group];
var matA_input = MatA.FromNative(inputTile);  // register load, 0 shuffles
matA_input.ChangeMajor();                      // movmatrix, 0 shuffles
let matB_lo = MatA.ExtractMatBLo(matA_input);  // register copy
let matB_hi = MatA.ExtractMatBHi(matA_input);  // register copy
// MMA with matB_lo (covers columns 0-7) and matB_hi (covers columns 8-15)
```

Loop iterates over `NTileGroups` (2) instead of `NCoopMatColumn` (4), each producing 2 MatB tiles via lo/hi split.

## Step 6: Update `outerProductCompute` to accept Fragment input

In `source/standard-modules/neural/mma-new-helper.slang`, the current `outerProductCompute` has 3 phases:
- Phase 1: Build dOut as MatA tiles (shuffles) -- **ELIMINATED**
- Phase 2: Build input as MatA tiles (shuffles) -- **ELIMINATED**
- Phase 3: MMA loop -- **unchanged, reads tiles directly from Fragment**

## Step 7: Update backward path

In `source/standard-modules/neural/accelerate-vector-coopmat.slang` `linearTransformBwdOnTarget` (line 1341-1345):
- `dthis.p.data` becomes a `Fragment<N>`, passed directly to `outerProductCompute`
- `doutput.data` (from `OutputVector.Differential`) is also a Fragment, usable directly

## Step 8: Non-CUDA fallback

The SPIR-V path and non-Optimal layout path still need sequential element access. Provide:
- `readUint4(int uint4Index)` on WaveTangledVector that unpacks fragments back to sequential layout
- Or keep a dual-path: `__target_switch` in `readUint4` to handle both

## Files to modify

| File | Change |
|------|--------|
| `source/standard-modules/neural/accelerate-vector-coopmat.slang` | Fragment struct, WaveTangledVector storage/constructors/subscript, linearTransform paths |
| `source/standard-modules/neural/mma-new-helper.slang` | mma overload for Fragment, outerProductCompute for Fragment |
| `source/slang/hlsl.meta.slang` | Possibly add `FromNative`/`LoadFromUint4` intrinsic on CoopMat |
| `prelude/slang-cuda-prelude.h` | Possibly add C++ helper for loading MatA from raw uint4 registers |

## Risks / Open Questions

- **Subscript performance**: Element access requires unpacking from fragment registers. Acceptable since MMA and outer product use fragments directly, and subscript is rare in hot paths.
- **Register pressure**: Fragment layout uses same register count as flat array, but may affect ptxas allocation differently. Needs benchmarking.
- **Non-CUDA fallback**: SPIR-V path needs old `T[]` representation. May need `readUint4` to unpack, or a target-switch dual-path.
- **IDifferentiable on Fragment**: Need to verify Slang's autodiff handles custom `dzero`/`dadd` on a struct wrapping non-differentiable `uint4` correctly.

## TODO Checklist

- [ ] Define `Fragment<NumElements, SubgroupSize>` struct with `IDifferentiable`, `dzero`, `dadd`
- [ ] Change `WaveTangledVector.data` from `T[Uint4AlignedInputSize]` to `Fragment<N>`
- [ ] Update `__init` methods to build fragments via `FromLocalRowPackOffset`
- [ ] Implement subscript get/set with pack/unpack, write custom forward/backward derivatives
- [ ] Add `MMAHelperNew.mma` overload accepting Fragment (`ChangeMajor` + `ExtractMatB`, 0 shuffles)
- [ ] Update `outerProductCompute` to accept Fragment inputs (skip Phase 1/2)
- [ ] Update `linearTransformBwdOnTarget` to pass Fragment data to updated MMA/OPA
- [ ] Ensure SPIR-V and non-Optimal paths still work (`readUint4` unpack or `target_switch`)
- [ ] Benchmark Fragment vs current approach at large sizes (64x256, 128x128) batch=8192

# WaveMatrix Vulkan Extension Plan

## Goal

Extend `WaveMatrix` and `MMAHelperNewV2` to work on Vulkan (SPIR-V), enabling
the same high-level backward API on both CUDA and Vulkan. Currently the
`MMAHelperNewV2.backward()` path is CUDA-only; Vulkan uses the lower-level
`TiledMMAHelper` path instead.

## Baseline Performance (Vulkan, TiledMMAHelper, float, batch=8192)

These numbers come from `benchmark_single_layer_backward_tiled.py --device vulkan --batch-size 8192`:

| Size   | Network    | Time (ms) |
|--------|------------|-----------|
| tiny   | 32 → 16   | 0.1090    |
| small  | 64 → 16   | 0.1500    |
| medium | 128 → 32  | 0.6053    |
| large  | 256 → 64  | 4.3109    |
| xlarge | 128 → 128 | 2.1810    |

The goal is to match or improve these numbers by routing Vulkan through
`MMAHelperNewV2`, which uses `WaveMatrix` for cleaner code and potentially
better register utilization.

## CUDA Backward Strategy: MMA Construction via WaveMatrix

This section documents how `MMAHelperNewV2.backward()` constructs MMA
operations on CUDA using WaveMatrix register-level transforms. Understanding
this is essential for designing the Vulkan alternative.

### Type Mapping

```
M = OutputSize      K = InputSize       N = 32 (warp size)

DOutMatrix   = MatA(N=32, M)     — each lane's dOut (M elements) is one row
InputMatrix  = MatA(N=32, K)     — each lane's input (K elements) is one row
WeightMatrix = MatB(M, K)        — weight matrix from global memory
ResultMatrix = MatC(N=32, K)     — each lane's dInput (K elements) is one row
OPResult     = MatC(M, K)        — accumulated weight gradient
```

### Step 1: Transpose MMA — `dInput = W^T × dOut`

Goal: for each lane i, compute `dInput[i] = W^T × dOut[i]`.

In row-vector form: `dInput[i]^T = dOut[i]^T × W`. So stacking all 32 lanes:
`Result(32×K) = DOut(32×M) × W(M×K)`.

Construction:

1. **Pack dOut → MatA(32×M)**: each lane's M-element dOut vector becomes a row
   via `fromArrayPacked` (warp shuffles distribute elements across fragment
   registers).

2. **Load weight W → MatB(M×K)**: `loadWeightRaw` reads tiles directly from
   global memory into MatB fragment registers (no shared memory).

3. **Transpose weight**: the weight was loaded for the forward direction
   (B matrix in A×B). For the backward we need the transposed content. The
   chain `copyFrom(MatB→MatA) + transposeSubMatrix + toMatrixB` rearranges
   the data within fragment registers:
   - `copyFrom`: reinterpret MatB tile bits as MatA tile bits
   - `transposeSubMatrix()`: swap reg[1] ↔ reg[2] within each 16×16 tile
     (this transposes the 8×8 sub-blocks that make up the fragment layout)
   - `toMatrixB()`: `copyFrom(MatA→MatB) + ChangeMajor` per tile + tile grid
     remap (column-major → row-major tile order)
   - Net effect: MatB(M×K) now contains W^T in the correct MMA layout

4. **MMA**: `matMul<T, N, M, K>(dOut, wT)` computes
   `Result(32×K) = DOut(32×M) × WT(M×K)`. Each lane reads its K-element
   dInput from its row in the result.

All register-level: **zero shared memory, zero barriers**.

### Step 2: Bias Reduce — `dBias += Σ_lanes dOut`

Goal: `dBias[m] = Σ_{lane=0}^{31} dOut[lane][m]` (sum each output-gradient
component across all lanes in the warp).

Reuses the dOut MatA(32×M) from step 1 (it has not been mutated yet).

Construction (exploits CUDA fragment register layout):

1. **Fold across tile rows**: MatA(32×M) has `N/16 = 2` tile rows. For each
   column of tiles, sum fragment registers pairwise across the two tile rows.

2. **Fold within a tile**: each tile's 4 fragment registers (reg[0..3])
   hold `half2` values. Sum `reg[0]+reg[1]` and `reg[2]+reg[3]`, then use
   `WaveMaskReadLaneAt` shuffles at strides 4, 8, 16 to reduce across the
   8 groups of 4 lanes.

3. **Write to shared memory**: first lane-group of each warp writes the
   per-warp partial sums.

4. **Cross-warp reduction + atomicAdd**: sum partial sums across warps, then
   atomicAdd to the global bias gradient buffer.

This is CUDA-specific because it reads fragment registers by index and knows
exactly which lanes hold which matrix elements.

### Step 3: Outer Product — `dW += Σ_lanes dOut[lane] ⊗ input[lane]`

Goal: `dW[m][k] = Σ_{lane=0}^{31} dOut[lane][m] × input[lane][k]`

This sum of 32 rank-1 outer products is expressed as a single matrix multiply:
`dW(M×K) = DOut^T(M×32) × Input(32×K)`.

Construction:

1. **Transform input MatA(32×K) → MatB(32×K)**:
   - `input.changeMajor()`: ChangeMajor on each MatA tile in-place (one PTX
     `movmatrix.trans` per tile)
   - `copyFrom(MatA→MatB)`: reinterpret bits as MatB tiles
   - Net effect: each lane's K-element row vector is now in the column-based
     MatB layout, ready as the right operand of the MMA

2. **Transpose dOut MatA(32×M) → MatA(M×32)**:
   - `dOut.transposeSubMatrix()`: swap reg[1] ↔ reg[2] per tile
   - `dOut.changeMajor()`: ChangeMajor per tile
   - Tile grid remap: reassign tiles from the (32/16 × M/16) grid to the
     (M/16 × 32/16) grid
   - Net effect: what was row i's M-element vector becomes column i across
     the M rows — a full matrix transpose

3. **MMA**: `matMul<T, M, N, K>(dOutT, inputB)` computes
   `dW(M×K) = DOutT(M×32) × InputB(32×K)`.

4. **Cross-warp reduction**: `storeRaw`/`loadRaw` + `add` via shared memory
   (tree reduction across warps).

5. **Store to global**: `storeRaw` to shared memory, then cooperative
   `atomicAdd` to the weight gradient buffer.

Steps 1-3 are all register-level (changeMajor = 1 PTX insn, transposeSubMatrix
= register swap, copyFrom = bit reinterpret). **Zero shared memory until the
cross-warp reduction in step 4.**

### Summary: what Vulkan needs to replace

| Step | CUDA (register ops)                         | Vulkan equivalent needed           |
|------|---------------------------------------------|------------------------------------|
| 1.1  | `fromArrayPacked` (shuffles)                | shmem write + CoopMat.Load         |
| 1.2  | `loadWeightRaw` (raw ptr)                   | CoopMat.Load from global           |
| 1.3  | copyFrom + transposeSubMatrix + toMatrixB   | shmem store/load with layout swap  |
| 1.4  | `matMul` (standard MMA)                     | same (already portable)            |
| 2    | fragment register reduction + shuffles       | MMA-based reduction (sumReduceRows)|
| 3.1  | changeMajor + copyFrom (1 PTX per tile)     | shmem store/load with layout swap  |
| 3.2  | transposeSubMatrix + changeMajor + remap    | shmem store/load with layout swap  |
| 3.3  | `matMul` (standard MMA)                     | same (already portable)            |
| 3.4  | `storeRaw`/`loadRaw`/`add` (shmem)          | same (already portable)            |
| 4    | `toArrayPacked` (shuffles)                  | CoopMat.Store + shmem read         |

## Current State

### WaveMatrix (`WaveMatrix.slang`)

- `[require(cuda)]` on the struct — needs broadening to `[require(cooperative_matrix)]`
- Stores array of `CoopMat` tiles in column-major tile order

#### Methods — already portable (no changes needed)

| Method          | Why it works                                        |
|-----------------|-----------------------------------------------------|
| `tileIdx()`     | Pure computation                                    |
| `add()`         | Standard `CoopMat` addition                         |
| `storeRaw()`    | `fragmentRead` + shared memory (opaque save/restore)|
| `loadRaw()`     | `fragmentWrite` + shared memory                     |

#### Methods — CUDA-only, stay CUDA-only

| Method                | Reason                                   |
|-----------------------|------------------------------------------|
| `changeMajor()`       | `movmatrix.trans` PTX instruction        |
| `transposeSubMatrix()`| Relies on CUDA fragment register layout  |

#### Methods — CUDA-only, need Vulkan alternatives

| Method            | CUDA impl                  | Vulkan plan                                         |
|-------------------|----------------------------|-----------------------------------------------------|
| `fromArrayPacked` | Warp shuffles (register)   | **`fromArrayPackedWithShMem`**: write to shmem → `CoopMat.Load` |
| `toArrayPacked`   | Warp shuffles (register)   | **`toArrayPackedWithShMem`**: `CoopMat.Store` to shmem → read  |
| `loadWeightRaw`   | Raw pointer loads          | **`loadWeightVulkan`**: reuse `loadTileDirect` (CoopMat.Load from global, with float→half conversion via MatrixAccumulator) |

### Free functions (`WaveMatrix.slang`)

| Function     | Status        | Vulkan plan                                                 |
|--------------|---------------|-------------------------------------------------------------|
| `toMatrixB`  | CUDA-only     | Portable via shmem: store MatA tiles → load as MatB tiles (or use CoopMat conversion). Not needed for initial milestone. |
| `matMad`     | Works on both | Change `[require(cuda)]` → `[require(cooperative_matrix)]`  |
| `matMul`     | Works on both | Same as `matMad`                                            |

### MMAHelperNewV2 (`mma-new-helper-v2.slang`)

Currently `[require(cuda)]`. Uses WaveMatrix throughout. Each operation needs
its CUDA-only WaveMatrix calls replaced with target-switched alternatives:

#### `backward()` — the main entry point

1. **`fromArrayPacked`** (vector → MatA) — needs `fromArrayPackedWithShMem` on Vulkan
2. **`mma`** (transpose MMA: `dIn = W^T × dOut`)
   - `loadWeightRaw` → `loadWeightVulkan` on Vulkan
   - `transposeSubMatrix` + `changeMajor` + `toMatrixB` → **different approach needed on Vulkan** (see below)
   - `matMul` → already portable
3. **`outerProduct`** (dW accumulation)
   - `changeMajor` on input/dOut → not available on Vulkan
   - `transposeSubMatrix` on dOut → not available on Vulkan
   - Alternative: load vectors to shmem in correct layout directly (like TiledMMAHelper does)
4. **`biasReduce`** — uses `fragmentRead` with CUDA register layout knowledge → needs shmem-based alternative
5. **`toArrayPacked`** (MatC → vector) — needs `toArrayPackedWithShMem` on Vulkan

## Implementation Plan

### Phase 1: WaveMatrix Vulkan extensions

1. **Relax struct requirement**: `[require(cuda)]` → `[require(cooperative_matrix)]`
2. **Add `[require(cuda)]` to CUDA-only methods**: `changeMajor`, `transposeSubMatrix`, `loadTileRawFromPointer`, `loadWeightRaw`
3. **Implement `fromArrayPackedWithShMem`**:
   ```
   fromArrayPackedWithShMem<ShMemSize, int PackedSize>(uint packedData[PackedSize], uint shMemOffset)
   ```
   - Each lane writes its packed data to shared memory
   - `CoopMat.Load` reads tiles from shared memory
   - Works for both CUDA and Vulkan (single implementation)

4. **Implement `toArrayPackedWithShMem`**:
   ```
   toArrayPackedWithShMem<ShMemSize, int PackedSize>(inout uint packedOut[PackedSize], uint shMemOffset)
   ```
   - `CoopMat.Store` writes tiles to shared memory
   - Each lane reads its packed data from shared memory

5. **Implement `loadWeightVulkan`**:
   - Reuse `loadTileDirect` from `mma-tiled-layout-helper.slang`
   - CoopMat.Load from global memory; float→half via MatrixAccumulator conversion

6. **Relax free function requirements**:
   - `matMad`, `matMul`: `[require(cuda)]` → `[require(cooperative_matrix)]`

### Phase 2: Unified TiledMMAHelper with backend dispatch

**Decision**: We will unify the architecture under `TiledMMAHelper`, which
internally dispatches to backend-specific implementations:

```
TiledMMAHelper
  ├── CUDA:   MMAHelperCuda   (= current MMAHelperNewV2, WaveMatrix + register ops)
  └── Vulkan: MMAHelperVulkan (= evolved TiledMMAHelper, CoopMat.Load + shmem)
```

Both backends share the same WaveMatrix type system and public API. The
difference is how data enters/leaves WaveMatrix tiles.

#### Vulkan backward — the new plan

**Key principle**: avoid `changeMajor`/`transposeSubMatrix` entirely. Instead,
load data directly into the correct WaveMatrix type (MatA or MatB) from shared
memory or global memory. Each backward step loads what it needs fresh — no
in-register transforms.

##### MMA (dInput = W^T × dOut)

Same structure as current TiledMMAHelper Vulkan path:

1. **Preload all weight tiles at the beginning** (before the k-loop).
   Weight → MatA via `loadTileDirect` (CoopMat.Load from global; float → half
   via MatrixAccumulator conversion when needed).
2. **Input vector → MatB** via shmem: each lane writes its vector to shared
   memory, then `CoopMat.Load` reads it as MatB tiles.
   This is the same as the current Vulkan version.
3. **Transpose case**: load dOut vector → MatB (same shmem path as input vector).
   The weight is already loaded as MatA, and the MMA computes
   `result = W × dOut` which gives `dInput` (since MatA holds W in transposed
   tile order).

No `changeMajor` or `transposeSubMatrix` needed — data enters in the right
layout from the start.

##### Bias reduce (dBias += Σ_lanes dOut)

Nothing changes from the current implementation.

**Optimization**: reuse the MatB tiles of dOut that were already loaded into
shared memory during the MMA step. The sumReduceRows MMA-based reduction can
read directly from the same shmem region — no need to reload dOut.

##### Outer product (dW += dOut ⊗ input)

This is the step that changes most from the CUDA path:

1. **Reload dOut as MatA**: the outer product computes
   `dW(M×K) = dOut^T(M×32) × input(32×K)`. On CUDA this is done by
   transposing the existing MatA(32×M) dOut via register ops. On Vulkan,
   we simply **reload dOut from shared memory into MatA(M×32)** — write each
   lane's dOut as a column, then `CoopMat.Load` with the appropriate layout.
2. **Load input vector as MatB**: same shmem path as in the MMA step — each
   lane writes its input vector, `CoopMat.Load` reads as MatB(32×K).
3. **MMA + cross-warp reduction + atomic store**: identical to current path.

No `changeMajor` or `transposeSubMatrix` — all data enters via shmem loads
with the correct matrix layout specified at load time.

##### Summary: Vulkan backward data flow

```
backward() {
    // Pack vectors to shmem (once)
    write dOut to shmem
    write input to shmem

    // 1. Transpose MMA: dInput = W^T × dOut
    weight tiles → MatA  (CoopMat.Load from global, all tiles preloaded)
    dOut         → MatB  (CoopMat.Load from shmem)
    MMA → dInput in MatC
    extract dInput via toArrayPackedWithShMem

    // 2. Bias reduce: dBias += sum(dOut)
    reuse dOut MatB from shmem (already loaded in step 1)
    sumReduceRows → atomicAdd to bias gradient

    // 3. Outer product: dW += dOut ⊗ input
    reload dOut  → MatA  (CoopMat.Load from shmem, column layout for M×32)
    reload input → MatB  (CoopMat.Load from shmem)
    MMA → dW in MatC
    cross-warp reduction → atomicAdd to weight gradient
}
```

**Shmem round-trips**: 1 write (dOut+input to shmem) + 3-4 CoopMat.Loads.
Compared to the CUDA path which has 0 shmem round-trips for the register
transforms but then needs shmem for cross-warp reduction anyway.

#### Step 2 (deferred): Extract shared reduction code

**Do this later**, after the migration is confirmed working and benchmarked.

Extract the cross-warp reduction and atomic store logic (currently duplicated
between `MMAHelper.sumReduceTilesAllAtOnce`, `TiledMMAHelper.outerProductAccumulate`
store phase, and `MMAHelperNewV2.outerProduct` reduction) into shared utilities.
Both backends can reuse:

- `sumReduceTilesAllAtOnce` (cross-warp tile reduction via shmem)
- `storeRaw` / `loadRaw` (already portable in WaveMatrix)
- `storeTileToGlobal` (atomic tile writeback)

### Phase 3: Benchmark and compare

1. Vulkan TiledMMAHelper baseline — **done** (see table above)
2. Run `benchmark_single_layer_backward_tiled.py --device cuda` (CUDA TiledMMAHelper)
3. Run `benchmark_single_layer_backward.py --device cuda` (CUDA MMAHelperNewV2)
4. After Phase 2: Vulkan via new unified path vs baseline
5. Compare CUDA MMAHelperNewV2 vs CUDA TiledMMAHelper (register ops vs CoopMat.Load)

## Performance Concern: shmem round-trips

The fundamental perf difference between CUDA and Vulkan WaveMatrix paths:

| Operation            | CUDA (register)             | Vulkan (shmem/global)                |
|----------------------|-----------------------------|--------------------------------------|
| `fromArrayPacked`    | Warp shuffles (0 shmem)     | shmem write + CoopMat.Load (1 trip)  |
| `toArrayPacked`      | Warp shuffles (0 shmem)     | CoopMat.Store + shmem read (1 trip)  |
| `loadWeightRaw`      | Raw pointer load (0 shmem)  | CoopMat.Load from global (0 shmem)   |
| `changeMajor`        | 1 PTX insn per tile         | **avoided** — load in correct layout |
| `transposeSubMatrix` | Register swap               | **avoided** — load in correct layout |
| MatA↔MatB conversion | changeMajor + copyFrom      | **avoided** — load as correct type   |

The new Vulkan plan avoids `changeMajor`/`transposeSubMatrix` entirely by
loading data in the right layout from the start. The cost is shmem round-trips
for vector↔CoopMat conversion, but we avoid the multi-step transform chains.

## Files to modify

| File | Changes |
|------|---------|
| `WaveMatrix.slang` | Relax require, add `fromArrayPackedWithShMem`, `toArrayPackedWithShMem`, `loadWeightVulkan` |
| `mma-new-helper-v2.slang` | Relax require, add `__target_switch` dispatch; Vulkan path uses CoopMat.Load instead of register transforms |
| `mma-tiled-layout-helper.slang` | Refactor to share code with the new unified path |
| `shared-memory-pool.slang` | Update shmem sizing for WaveMatrix shmem usage |
| `accelerate-vector-coopmat.slang` | Route both backends through the unified TiledMMAHelper |









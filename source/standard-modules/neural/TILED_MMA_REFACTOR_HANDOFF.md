# Tiled MMA Refactor Handoff

## Architecture

```
TiledMMAHelper (dispatcher, mma-tiled-layout-helper.slang)
  ├── forward()  → __target_switch { cuda: TiledMMACuda, spirv: TiledMMAVulkan }
  ├── backward() → __target_switch { cuda: TiledMMACuda, spirv: TiledMMAVulkan }
  │
  ├── TiledMMACuda (mma-tiled-cuda.slang)
  │     ├── forward():      InputMatrix(MatA) + WeightMatrix(MatB) via loadWeightRaw + matMul
  │     ├── backward():     fromArrayPacked + mmaTranspose + biasReduce + outerProduct
  │     ├── mma():          forward MMA (input MatA × weight MatB → result MatC)
  │     └── mmaTranspose(): backward MMA (dOut MatA × W^T MatB → dInput MatC)
  │
  └── TiledMMAVulkan (mma-tiled-vulkan.slang)
        ├── forward():      loadWeightTiled + fromVectorViaShMem + matMul + toVectorViaShMem
        ├── backward():     mmaTranspose + biasReduce + outerProductAccumulate
        ├── mmaTranspose(): transpose MMA (W^T × input), consistent naming with CUDA
        ├── biasReduce():   MMA-based (ones × dOutB → column sums), reuses pre-loaded dOutB
        └── outerProductAccumulate(): fromVectorViaShMem + matMul + cross-warp reduce + atomic store
```

## What passes (all 37 tests)

### Tiled layout forward + transpose tests: 36/36

All 8 test files pass on both CUDA and Vulkan, float and half, single-warp and multi-warp, aligned and arbitrary-size:

```bash
./build/Release/bin/slang-test tests/neural/mma-tiled-layout-test-*.slang -use-test-server -server-count 8
# 100% of tests passed (36/36)
```

### Backward test: 1/1

```bash
./build/Release/bin/slang-test tests/neural/mma-tiled-backward-test.slang
# 100% of tests passed (1/1)
```

| Size   | Network    | Slang (ms) | Tin2 (ms) | Ratio |
|--------|------------|-----------|-----------|-------|
| tiny   | 32 → 16   | 0.0562    | 0.0554    | 1.01  |
| small  | 64 → 16   | 0.0562    | 0.0544    | 1.03  |
| medium | 128 → 32  | 0.0627    | 0.0600    | 1.05  |
| large  | 256 → 64  | 0.2779    | 0.2827    | 0.98  |
| xlarge | 128 → 128 | 0.2289    | 0.2211    | 1.04  |

Register pressure identical to Tin2:

| Size   | Slang regs | Slang spill S/L | Tin2 regs | Tin2 spill S/L |
|--------|-----------|----------------|-----------|----------------|
| tiny   | 40        | 0 / 0          | 40        | 0 / 0          |
| small  | 72        | 0 / 0          | 64        | 0 / 0          |
| medium | 166       | 0 / 0          | 166       | 0 / 0          |
| large  | 255       | 932 / 852      | 255       | 952 / 796      |
| xlarge | 255       | 576 / 560      | 255       | 548 / 496      |

## What was fixed in this session

### 1. `toArrayPacked` for float MatC (`WaveMatrix.slang`)

The register indexing and column ordering were hardcoded for half's 2-reg-per-sub-tile layout. Float MatC has 4 regs per sub-tile, requiring:
- **Register indexing**: `base_reg = (r_col / ColsPerRowPerSubTile) * RegsPerSubTile + (r_col % ColsPerRowPerSubTile)` with `RowGroupStride = 2` for float (vs 1 for half).
- **Column ordering**: output index interleaves even/odd columns for float since each register holds 1 value (vs 2 packed halves).

All new constants are `static const` and fold to the original expressions at compile time for half — zero perf impact on the backward path.

### 2. `LanesAreRows` heuristic (`WaveMatrix.slang`)

`fromVectorViaShMem` and `toVectorViaShMem` had `LanesAreRows = (Rows == 32)` which fails when both Rows and Cols equal 32. Replaced with an explicit `bool LanesAreRows` template parameter — no default, caller always specifies:
- `false` for mma/mmaTranspose input and result (lanes = columns = N dimension)
- `true` for outer product input (lanes = rows = N dimension)

### 3. Tile-row-at-a-time shmem (`WaveMatrix.slang`)

`fromVectorViaShMem` and `toVectorViaShMem` now process one tile-group (16 elements per lane) at a time instead of writing the entire vector. Shmem per operation reduced from `VecSize × 32 × sizeof(element)` to constant `16 × 32 × sizeof(element)`.

### 4. Vulkan `mma` renamed to `mmaTranspose` (`mma-tiled-vulkan.slang`)

For consistency with `TiledMMACuda`. Both backends now have `mma` (forward) and `mmaTranspose` (transpose).

### 5. Unit test refactored (`unittest-mat-vec-mul.slang`)

`testTiledMatVecMulImpl` now calls `mma`/`mmaTranspose` directly on each backend via `__target_switch`, instead of routing `TransposeA` through `forward()`.

### 6. Test infrastructure (`common.slang` + all test files)

- `convertTileRowMajorToNativeB`: per-tile row-major → native MatB register layout conversion
- `convertTiledToNativeB`: applies the per-tile conversion to all tiles (CUDA only, no-op on Vulkan)
- `fillTransposeWeightTiled<T, Rows, Cols, WorkgroupSize, Clear>`: shared fill function for transpose tests, handles CUDA K×M vs Vulkan M×K weight storage
- Forward arbitrary-size tests: weight fill corrected from M×K to K×M for MatB

## What was fixed in the outer product / backward session

### 7. Outer product test routing (`unittest-outerproduct-reduce.slang`)

Rewrote `testOuterProductAccumulateTiled` to dispatch to `TiledMMACuda.outerProduct` (CUDA) and `TiledMMAVulkan.outerProductAccumulate` (Vulkan) via `__target_switch`, instead of the old `MMAHelper.outerProductAccumulate` path.

### 8. Vulkan shmem race in `outerProductAccumulate` (`mma-tiled-vulkan.slang`)

`fromVectorViaShMem` was called by all warps with the same shmem offset. Warps with different input data clobbered each other. Fixed by giving each warp its own shmem region via `perWarpBase = shBase + waveId * ShmemPerWarp`.

### 9. Float store path in `TiledMMACuda.outerProduct` (`mma-tiled-cuda.slang`)

Added `else if (sizeof(U) == 4)` branch for float atomicAdd in the outer product store section. Previously only half was handled.

### 10. `fromVectorViaShMem` type-aware stride (`WaveMatrix.slang`)

The function always read 8 elements per uint4 (assuming half). For float input, each uint4 holds only 4 floats. Fixed with `sizeof(U)`-aware branching: half reads 8 elements per uint4, float reads 4 per sub-group (two sub-groups per uint4). Compile-time `if (s < VecSize)` guards prevent dead-code array bounds errors (E30029).

### 11. `toVectorViaShMem` bounds guard (`WaveMatrix.slang`)

Added `if (s < VecSize)` guard for small VecSize (e.g. OutputSize=2) to prevent out-of-bounds writes that triggered E30029.

### 12. Vulkan biasReduce rewritten (`mma-tiled-vulkan.slang`)

Old: `ones(16,16) × dOutB(M,N)` — summed within each lane (wrong for M < 16).
New: `dOutA(M,N) × onesB(N,16)` — sums across lanes (correct). Takes `MatrixA<M,N>` so dOutA is reused by `outerProductAccumulate`.

### 13. `outerProductAccumulate` takes pre-loaded MatA (`mma-tiled-vulkan.slang`)

Changed from raw `T dOutArr[]` to `MatrixA<M,N> dOutA`. Eliminates redundant shmem reload — `backward()` loads dOutA once and passes to both biasReduce and outerProduct.

### 14. `Bias` flag on backward chain

`linearTransformBwdOnTarget` in `accelerate-vector-coopmat.slang` passed an undefined `biasGradAddress` when `Bias=false`, causing biasReduce to corrupt the weight buffer. Fixed by adding `bool Bias` template parameter through `TiledMMAHelper.backward()` → `TiledMMACuda/Vulkan.backward()`, guarding biasReduce with `if (Bias)`.

### 15. Uint4-aligned packed sizes (`mma-tiled-cuda.slang`)

`backward()` and `forward()` used packed sizes that could be < `RegColsPacked=4` for small InputSize/OutputSize, triggering E30029. Fixed by deriving packed sizes from `WaveTangledVector.Uint4AlignedInputSize`.

### 16. Vulkan forward/backward pass `.data` directly (`mma-tiled-vulkan.slang`)

Callsites pass `WaveTangledVector.data` (Uint4Aligned) instead of building local `T arr[K]` arrays. Eliminates array bounds errors for sub-tile sizes.

### 17. CUDA native layout test helpers (`common.slang`)

- `rowMajorToNativeMatB(row, col)`: maps (K_row, M_col) to CUDA MatB fragment index
- `rowMajorToNativeMatC(row, col)`: maps (M_row, K_col) to CUDA MatC fragment index
- `tiledWeightOffset(row, col)`: CUDA uses `rowMajorToNativeMatB(col, row)` (swapped: weight is MatB<K,M>), VK uses `row*16+col`
- `tiledDWeightOffset(row, col)`: CUDA uses `rowMajorToNativeMatC(row, col)`, VK uses `row*16+col`
- `convertNativeToRowMajorTiled`: register-based (no scratch buffer) conversion for outer product output

## What fails (remaining 4 tests)

### CUDA float backward

```
tests/neural/basic-coopmat-vector-tiled-layout-test.slang.2 (cuda) — float bindless
tests/neural/basic-coopmat-vector-tiled-layout-test.slang.4 (cuda) — float pointer
tests/neural/fflayer-wavetangled-vector-tiled-test.slang.1 (cuda)  — float bindless
tests/neural/fflayer-wavetangled-vector-tiled-test.slang.2 (cuda)  — float pointer
```

CUDA half backward passes. CUDA float forward passes. CUDA float backward fails.

Root cause: `TiledMMACuda.backward()` does `bit_cast<uint[]>(dOut.data)` and passes to `fromArrayPacked`. For T=float, each uint contains a float bit pattern, but MatA is half-typed — `fromArrayPacked` interprets each uint as 2 halves. The forward path handles this with explicit float→half conversion before packing; the backward path needs `fromArrayPacked` to be made type-aware for float (similar to how `toArrayPacked` already has `ElemsPerReg = sizeof(uint) / sizeof(T)`).

## Files modified

| File | Changes |
|------|---------|
| `WaveMatrix.slang` | `toArrayPacked` float fix, `LanesAreRows` param, tile-row-at-a-time shmem, `fromVectorViaShMem` float stride + bounds, `toVectorViaShMem` bounds guard |
| `mma-tiled-cuda.slang` | Float store in outerProduct, Uint4Aligned packed sizes, `Bias` flag |
| `mma-tiled-vulkan.slang` | Renamed `mma`→`mmaTranspose`, per-warp shmem, pass `.data`, biasReduce `dOutA×onesB`, reuse dOutA, `Bias` flag |
| `mma-tiled-layout-helper.slang` | `Bias` template param on backward |
| `accelerate-vector-coopmat.slang` | Pass `Bias` from `linearTransformBwdOnTarget` |
| `unit-test/unittest-outerproduct-reduce.slang` | Route to TiledMMACuda/Vulkan, load dOutA for VK |
| `unit-test/unittest-mat-vec-mul.slang` | Direct mma/mmaTranspose dispatch, per-warp shmem offset |
| `unit-test/unittest-mma-backward.slang` | Explicit `Bias=true` |
| `tests/neural/common.slang` | `convertTileRowMajorToNativeB`, `convertTiledToNativeB`, `fillTransposeWeightTiled`, `rowMajorToNativeMatB/MatC`, `tiledWeightOffset/tiledDWeightOffset`, `convertNativeToRowMajorTiled` |
| `tests/neural/basic-coopmat-vector-tiled-layout-test.slang` | Use tiled offset helpers |
| `tests/neural/fflayer-wavetangled-vector-tiled-test.slang` | Use tiled offset helpers |
| `tests/neural/outerproduct-accumulate-tiled-test*.slang` | Add conversion call, fix expected values |
| `tests/neural/mma-tiled-layout-test-*.slang` (8 files) | Native MatB conversion, weight fill fixes |

## How to run tests

```bash
# Build
cmake --build --preset release -j12
touch source/standard-modules/neural/*.slang
cmake --build --preset release -j12 --target slang-neural-module

# All tiled layout tests (CUDA + Vulkan)
./build/Release/bin/slang-test tests/neural/mma-tiled-layout-test-*.slang -use-test-server -server-count 8

# Outer product tests
./build/Release/bin/slang-test tests/neural/outerproduct-accumulate-tiled-test*.slang -use-test-server -server-count 4

# basic-coopmat + fflayer tests
./build/Release/bin/slang-test tests/neural/basic-coopmat-vector-tiled-layout-test.slang tests/neural/fflayer-wavetangled-vector-tiled-test.slang -use-test-server -server-count 4

# Register pressure check
for cfg in "32 16 tiny" "64 16 small" "128 32 medium" "256 64 large" "128 128 xlarge"; do
  set -- $cfg
  ./build/Release/bin/slangc benchmarks/benchmark_single_layer_backward.slang \
    -target ptx -entry compute_backward -stage compute \
    -o /tmp/backward_${3}.ptx \
    -I build/Release/lib/slang-standard-module-2026.3.1 \
    -DINPUT_SIZE=$1 -DOUTPUT_SIZE=$2 -DSUBGROUP_COUNT=2 -experimental-feature
  head -c -1 /tmp/backward_${3}.ptx > /tmp/backward_${3}_fixed.ptx
  printf "%-7s " "$3:"
  /usr/local/cuda/bin/ptxas -v --gpu-name sm_89 /tmp/backward_${3}_fixed.ptx -o /dev/null 2>&1 | grep -oE "Used [0-9]+ registers|[0-9]+ bytes spill (stores|loads)"
done

# Runtime benchmark (uses ncu_launcher, run 5 times per size for stable averages)
# Build launcher if needed:
#   /usr/local/cuda/bin/nvcc -o benchmarks/ncu_launcher_new_mma \
#     benchmarks/ncu_launcher_new_mma.cu -lcuda -arch=sm_89
for cfg in "32 16 tiny" "64 16 small" "128 32 medium" "256 64 large" "128 128 xlarge"; do
  set -- $cfg
  ./build/Release/bin/slangc benchmarks/benchmark_single_layer_backward.slang \
    -target ptx -entry compute_backward -stage compute \
    -o /tmp/backward_${3}.ptx \
    -I build/Release/lib/slang-standard-module-2026.3.1 \
    -DINPUT_SIZE=$1 -DOUTPUT_SIZE=$2 -DSUBGROUP_COUNT=2 -experimental-feature
  head -c -1 /tmp/backward_${3}.ptx > /tmp/backward_${3}_fixed.ptx
  echo "=== $3 (${1}x${2}) ==="
  for run in 1 2 3 4 5; do
    ./benchmarks/ncu_launcher_new_mma /tmp/backward_${3}_fixed.ptx \
      --input-size $1 --output-size $2 --batch-size 8192 --warps 2 --mode backward
  done
done
```

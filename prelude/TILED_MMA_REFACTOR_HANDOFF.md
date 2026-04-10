# Tiled MMA Refactor Handoff

## What was done

Refactored the tiled MMA architecture into a clean dispatcher + backend split:

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
        ├── backward():     mma (transpose) + biasReduce + outerProductAccumulate
        ├── mma():          transpose MMA using WaveMatrix types
        ├── biasReduce():   MMA-based (ones × dOutB → column sums), reuses pre-loaded dOutB
        └── outerProductAccumulate(): fromVectorViaShMem + matMul + cross-warp reduce + atomic store
```

### Key changes

- **Weight is MatB(K, M)** matching Tin2's `HMatrixB<Z0=InputSize, Z1=OutputSize>`
- **WaveMatrix.transpose()**: new method — sub-tile transpose + ChangeMajor + tile grid swap in one pass
- **WaveMatrix.fromVectorViaShMem**: takes any type U, converts to half internally via `__realCast<half>`
- **WaveMatrix.toVectorViaShMem**: handles sizeof(T)==2 and sizeof(T)==4 correctly
- **WaveMatrix.loadWeightTiled**: supports `Transpose` parameter for backward weight loading
- **accelerate-vector-coopmat.slang**: OptimalLayout backward now uses `TiledMMAHelper.backward()` with `bit_cast` for Differential types

## What passes

### Backward (all sizes, CUDA)

Tested via `benchmark_single_layer_backward.slang` → `testTiledBackward` → `TiledMMAHelper.backward()`.

Locked clocks (2520MHz), batch=8192, 2 warps, 5 runs averaged:

| Size   | Network    | Slang (ms) | Tin2 (ms) | Ratio |
|--------|------------|-----------|-----------|-------|
| tiny   | 32 → 16   | 0.0562    | 0.0554    | 1.01  |
| small  | 64 → 16   | 0.0562    | 0.0544    | 1.03  |
| medium | 128 → 32  | 0.0627    | 0.0600    | 1.05  |
| large  | 256 → 64  | 0.2779    | 0.2827    | 0.98  |
| xlarge | 128 → 128 | 0.2289    | 0.2211    | 1.04  |

Register pressure (ptxas, sm_89):

| Size   | Slang regs | Slang spill S/L | Tin2 regs | Tin2 spill S/L |
|--------|-----------|----------------|-----------|----------------|
| tiny   | 40        | 0 / 0          | 40        | 0 / 0          |
| small  | 72        | 0 / 0          | 64        | 0 / 0          |
| medium | 166       | 0 / 0          | 166       | 0 / 0          |
| large  | 255       | 932 / 852      | 255       | 952 / 796      |
| xlarge | 255       | 576 / 560      | 255       | 548 / 496      |

### Forward (half only, CUDA)

`tests/neural/mma-tiled-layout-test-single-warp.slang` — half tests (TEST_HALF=1) pass on CUDA with native MatB layout conversion.

## What fails

### Forward (float, CUDA)

`tests/neural/mma-tiled-layout-test-single-warp.slang` tests .4 and .6 (TEST_HALF=0) fail.

The float forward path has been partially fixed:
- Input: float→half conversion before `fromArrayPacked` ✓
- Weight: native MatB layout conversion in test ✓
- Result: `toArrayPacked` with correct packed size for float MatC ✓

But the float tests still fail. Remaining suspects:
- The `toArrayPacked` shuffle logic may not handle float MatC correctly (it was designed for half MatA and half/float MatC with specific register packing assumptions)
- The `fromArrayPacked` with `PackedSizeFwdInputHalf = K/2` may not match the internal shuffle loop's expectations when the original data was float

### Forward (Vulkan)

Not yet tested. The Vulkan forward in `TiledMMAVulkan.forward()` is implemented but needs test coverage.

### Tests disabled

- `tests/neural/mma-tiled-layout-test-single-warp.slang` CUDA float tests: still active but failing
- Other tiled layout tests (multi-warp, transpose, arbitrary size): CUDA tests may need similar native layout conversion

## How to run perf tests

### Prerequisites

```bash
# Build release
cmake --build --preset release -j12

# If neural module .slang files changed, force rebuild:
touch source/standard-modules/neural/*.slang
cmake --build --preset release -j12 --target slang-neural-module
```

### Quick backward perf check (no sudo)

```bash
# Compile PTX for all sizes
for cfg in "32 16 tiny" "64 16 small" "128 32 medium" "256 64 large" "128 128 xlarge"; do
  set -- $cfg
  ./build/Release/bin/slangc benchmarks/benchmark_single_layer_backward.slang \
    -target ptx -entry compute_backward -stage compute \
    -o /tmp/backward_${3}.ptx \
    -I build/Release/lib/slang-standard-module-2026.3.1 \
    -DINPUT_SIZE=$1 -DOUTPUT_SIZE=$2 -DSUBGROUP_COUNT=2 \
    -experimental-feature
done

# Run all sizes
for cfg in "32 16 tiny" "64 16 small" "128 32 medium" "256 64 large" "128 128 xlarge"; do
  set -- $cfg
  ./benchmarks/ncu_launcher_new_mma /tmp/backward_${3}.ptx \
    --input-size $1 --output-size $2 --batch-size 8192 --warps 2 --mode backward
done
```

### Locked-clock benchmark (requires sudo)

```bash
sudo nvidia-smi -pm 1
sudo nvidia-smi --lock-gpu-clocks=2520,2520

# Run Slang (5 times for averaging)
for run in 1 2 3 4 5; do
  for cfg in "32 16 tiny" "64 16 small" "128 32 medium" "256 64 large" "128 128 xlarge"; do
    set -- $cfg
    ./benchmarks/ncu_launcher_new_mma /tmp/backward_${3}.ptx \
      --input-size $1 --output-size $2 --batch-size 8192 --warps 2 --mode backward 2>&1 | grep "Avg time"
  done
done

# Run Tin2 (5 times for averaging)
for run in 1 2 3 4 5; do
  tin2/benchmarks/mlp_perf/build/bench_tin2_single_layer_backward --batch-size 8192 --warps 2
done

# Reset clocks when done
sudo nvidia-smi --reset-gpu-clocks
```

### Register pressure check

```bash
# Slang
for cfg in "32 16 tiny" "64 16 small" "128 32 medium" "256 64 large" "128 128 xlarge"; do
  set -- $cfg
  head -c -1 /tmp/backward_${3}.ptx > /tmp/backward_${3}_fixed.ptx
  echo -n "$3: "
  /usr/local/cuda/bin/ptxas -v --gpu-name sm_89 /tmp/backward_${3}_fixed.ptx -o /dev/null 2>&1 | grep -E "spill|registers"
done

# Tin2
/usr/local/cuda/bin/nvcc -O3 --expt-relaxed-constexpr -arch=sm_89 \
  -Itin2/include -w --ptxas-options=-v \
  -c tin2/benchmarks/mlp_perf/bench_tin2_single_layer_backward.cu -o /dev/null 2>&1 | \
  grep -E "spill|registers|Function"
```

### Unit tests

```bash
# Backward test (CUDA + Vulkan)
./build/Release/bin/slang-test tests/neural/mma-tiled-backward-test.slang

# Forward test (CUDA — half passes, float fails)
./build/Release/bin/slang-test tests/neural/mma-tiled-layout-test-single-warp.slang -api cuda
```

## Files modified

| File | Role |
|------|------|
| `mma-tiled-layout-helper.slang` | Pure dispatcher (~100 lines): forward() + backward() |
| `mma-tiled-cuda.slang` | CUDA backend: WaveMatrix register ops (from MMAHelperNewV2) |
| `mma-tiled-vulkan.slang` | Vulkan backend: WaveMatrix + shmem (CoopMat.Load) |
| `WaveMatrix.slang` | Added transpose(), fromVectorViaShMem, toVectorViaShMem, loadWeightTiled |
| `mma-new-helper-v2.slang` | Emptied (content moved to mma-tiled-cuda.slang) |
| `accelerate-vector-coopmat.slang` | OptimalLayout uses TiledMMAHelper.forward()/backward() |
| `neural.slang` | Include order updated |
| `unit-test/unittest-mma-backward.slang` | Uses TiledMMAHelper.backward() |
| `unit-test/unittest-mat-vec-mul.slang` | Uses TiledMMAHelper.forward() |
| `benchmarks/benchmark_single_layer_backward.slang` | Uses testTiledBackward |

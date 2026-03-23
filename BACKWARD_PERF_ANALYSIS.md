# Backward Pass Performance Analysis

All benchmarks: locked clocks (nvidia-smi -lgc 3105 -lmc 10501), batch=256, half precision, tiled (OptimalLayout) weights.

## 1. OuterProduct Shmem Doubling Improvement

The `outerProduct` function was changed to load ALL 32 lanes' vectors simultaneously (2x shmem) instead of 16 lanes per k-iteration. This eliminates idle lanes during vector loading.

### OPA-only (2-warp)


| Size   | Network | Before (ms) | After (ms) | Change   |
| ------ | ------- | ----------- | ---------- | -------- |
| tiny   | 32→16   | 0.0033      | 0.0035     | +6%      |
| small  | 64→16   | 0.0037      | 0.0039     | +5%      |
| medium | 128→32  | 0.0082      | 0.0061     | **-26%** |
| large  | 256→64  | 0.0277      | 0.0223     | **-19%** |
| xlarge | 128→128 | 0.0224      | 0.0185     | **-17%** |


Medium/large/xlarge benefit from more tiles to amortize the cost. Tiny/small regress slightly due to 2x shmem reducing occupancy for trivial workloads.

## 2. Decomposed Backward Pass Performance

### OuterProductAccumulate (tiled, 2-warp)


| Size   | Network | Slang (ms) | Tin2 (ms) | Difference |
| ------ | ------- | ---------- | --------- | ---------- |
| tiny   | 32→16   | 0.0031     | 0.0030    | +3%        |
| small  | 64→16   | 0.0033     | 0.0030    | +10%       |
| medium | 128→32  | 0.0050     | 0.0047    | +6%        |
| large  | 256→64  | 0.0141     | 0.0161    | **-12%**   |
| xlarge | 128→128 | 0.0115     | 0.0128    | **-10%**   |


### OuterProductAccumulate (tiled, 1-warp)


| Size   | Network | Slang (ms) | Tin2 (ms) | Difference |
| ------ | ------- | ---------- | --------- | ---------- |
| tiny   | 32→16   | 0.0042     | 0.0028    | +50%       |
| small  | 64→16   | 0.0045     | 0.0040    | +13%       |
| medium | 128→32  | 0.0067     | 0.0066    | +2%        |
| large  | 256→64  | 0.0176     | 0.0188    | **-6%**    |
| xlarge | 128→128 | 0.0157     | 0.0147    | +7%        |


### MMA Transpose + Bias Reduce (tiled, 2-warp)


| Size   | Network | Slang (ms) | Tin2 (ms) | Difference |
| ------ | ------- | ---------- | --------- | ---------- |
| tiny   | 32→16   | 0.0024     | 0.0022    | +9%        |
| small  | 64→16   | 0.0030     | 0.0026    | +15%       |
| medium | 128→32  | 0.0040     | 0.0033    | +21%       |
| large  | 256→64  | 0.0076     | 0.0060    | +27%       |
| xlarge | 128→128 | 0.0072     | 0.0076    | **-5%**    |


### MMA Transpose + Bias Reduce (tiled, 1-warp)


| Size   | Network | Slang (ms) | Tin2 (ms) | Difference |
| ------ | ------- | ---------- | --------- | ---------- |
| tiny   | 32→16   | 0.0022     | 0.0022    | 0%         |
| small  | 64→16   | 0.0024     | 0.0037    | **-35%**   |
| medium | 128→32  | 0.0034     | 0.0031    | +10%       |
| large  | 256→64  | 0.0067     | 0.0058    | +16%       |
| xlarge | 128→128 | 0.0070     | 0.0095    | **-26%**   |


## 3. Sum of Decomposed Parts vs Overall Backward

### 1-warp


| Size   | Slang OPA | Slang MMA+bias | Slang Sum | Slang Full | Slang Overhead | Tin2 OPA | Tin2 MMA+bias | Tin2 Sum | Tin2 Full | Tin2 Overhead |
| ------ | --------- | -------------- | --------- | ---------- | -------------- | -------- | ------------- | -------- | --------- | ------------- |
| tiny   | 0.0042    | 0.0022         | 0.0064    | 0.0046     | -28%           | 0.0028   | 0.0022        | 0.0050   | 0.0035    | -30%          |
| small  | 0.0045    | 0.0024         | 0.0069    | 0.0056     | -19%           | 0.0040   | 0.0037        | 0.0077   | 0.0049    | -36%          |
| medium | 0.0067    | 0.0034         | 0.0101    | 0.0073     | -28%           | 0.0066   | 0.0031        | 0.0097   | 0.0077    | -21%          |
| large  | 0.0176    | 0.0067         | 0.0243    | 0.0249     | +2%            | 0.0188   | 0.0058        | 0.0246   | 0.0290    | +18%          |
| xlarge | 0.0157    | 0.0070         | 0.0227    | 0.0199     | -12%           | 0.0147   | 0.0095        | 0.0242   | 0.0192    | -21%          |


### 2-warp


| Size   | Slang OPA | Slang MMA+bias | Slang Sum | Slang Full | Slang Overhead | Tin2 OPA | Tin2 MMA+bias | Tin2 Sum | Tin2 Full | Tin2 Overhead |
| ------ | --------- | -------------- | --------- | ---------- | -------------- | -------- | ------------- | -------- | --------- | ------------- |
| tiny   | 0.0031    | 0.0024         | 0.0055    | 0.0032     | -42%           | 0.0030   | 0.0022        | 0.0052   | 0.0036    | -31%          |
| small  | 0.0033    | 0.0030         | 0.0063    | 0.0038     | -40%           | 0.0030   | 0.0026        | 0.0056   | 0.0033    | -41%          |
| medium | 0.0050    | 0.0040         | 0.0090    | 0.0071     | -21%           | 0.0047   | 0.0033        | 0.0080   | 0.0045    | -44%          |
| large  | 0.0141    | 0.0076         | 0.0217    | 0.0255     | +18%           | 0.0161   | 0.0060        | 0.0221   | 0.0227    | +3%           |
| xlarge | 0.0115    | 0.0072         | 0.0187    | 0.0188     | +1%            | 0.0128   | 0.0076        | 0.0204   | 0.0152    | -25%          |


Negative overhead = full backward faster than sum-of-parts (data locality / cache reuse benefit).
Positive overhead = full backward slower than sum-of-parts (register pressure / code bloat).

## 4. Full Backward: Slang vs Tin2

### 1-warp


| Size   | Network | Slang (ms) | Tin2 (ms) | Difference |
| ------ | ------- | ---------- | --------- | ---------- |
| tiny   | 32→16   | 0.0046     | 0.0035    | +31%       |
| small  | 64→16   | 0.0056     | 0.0049    | +14%       |
| medium | 128→32  | 0.0073     | 0.0077    | **-5%**    |
| large  | 256→64  | 0.0249     | 0.0290    | **-14%**   |
| xlarge | 128→128 | 0.0199     | 0.0192    | +4%        |


### 2-warp


| Size   | Network | Slang (ms) | Tin2 (ms) | Difference |
| ------ | ------- | ---------- | --------- | ---------- |
| tiny   | 32→16   | 0.0032     | 0.0036    | **-11%**   |
| small  | 64→16   | 0.0038     | 0.0033    | +15%       |
| medium | 128→32  | 0.0071     | 0.0045    | +58%       |
| large  | 256→64  | 0.0255     | 0.0227    | +12%       |
| xlarge | 128→128 | 0.0188     | 0.0152    | +24%       |


## PTX Analysis: Why Slang Uses More Registers

PTX comparison for 128→128 2-warp full backward:


| Metric               | Slang  | Tin2   | Ratio |
| -------------------- | ------ | ------ | ----- |
| b32 registers        | 19,515 | 10,322 | 1.9x  |
| b16 registers        | 4,315  | 1,667  | 2.6x  |
| PTX lines            | 22,818 | 13,138 | 1.7x  |
| shuffle instructions | 640    | 240    | 2.7x  |
| mov instructions     | 2,868  | 958    | 3.0x  |
| cvt instructions     | 87     | 3      | 29x   |
| atomics (red)        | 257    | 257    | 1.0x  |


### Root cause: bias reduce implementation

The extra movs and shuffles are concentrated in the bias reduce (sumReduceRows) region:

- **Slang**: Treats dOut as a flat per-thread array. Does element-by-element `WaveMaskSum` (5-stage butterfly shuffle) for each element pair. Each stage requires `mov.b32 {%rs, %rs}` to broadcast half to u32, shuffle, extract, add. Total: 640 shuffles for 128 elements.
- **Tin2**: Operates directly on MMA register layout. The dOut vector is already distributed across lanes in MMA format. Reduction is done as register-level `half2` adds on MMA tiles, needing only ~16 shuffles (`__shfl_down_sync` on packed half2) for the final intra-tile reduction.

This is a fundamental architectural difference: Tin2 keeps data in the MMA register layout throughout the backward pass and reduces in that domain. Slang materializes the MMA output into a per-thread array (WaveTangledVector) then reduces element-by-element, losing the MMA layout structure.

## 5. MMA-based Bias Reduce (sumReduceRowsMMA)

Replaces per-element WaveMaskSum with MMA: A=all-ones x B=dOut_data, so C=column-sums across lanes. Eliminates all shuffles from bias reduce.

### Performance (full backward, 2-warp)


| Size   | Network  | WaveMaskSum (ms) | MMA bias (ms) | Change | Tin2 (ms) | vs Tin2 |
| ------ | -------- | ---------------- | ------------- | ------ | --------- | ------- |
| tiny   | 32->16   | 0.0032           | 0.0031        | -3%    | 0.0036    | -14%    |
| small  | 64->16   | 0.0038           | 0.0043        | +13%   | 0.0033    | +30%    |
| medium | 128->32  | 0.0071           | 0.0058        | -18%   | 0.0045    | +29%    |
| large  | 256->64  | 0.0255           | 0.0220        | -14%   | 0.0227    | -3%     |
| xlarge | 128->128 | 0.0188           | 0.0162        | -14%   | 0.0152    | +7%     |


### Performance (full backward, 1-warp)


| Size   | Network  | WaveMaskSum (ms) | MMA bias (ms) | Change | Tin2 (ms) | vs Tin2 |
| ------ | -------- | ---------------- | ------------- | ------ | --------- | ------- |
| tiny   | 32->16   | 0.0046           | 0.0042        | -9%    | 0.0035    | +20%    |
| small  | 64->16   | 0.0056           | 0.0048        | -14%   | 0.0049    | -2%     |
| medium | 128->32  | 0.0073           | 0.0073        | 0%     | 0.0077    | -5%     |
| large  | 256->64  | 0.0249           | 0.0262        | +5%    | 0.0290    | -10%    |
| xlarge | 128->128 | 0.0199           | 0.0182        | -9%    | 0.0192    | -5%     |


### PTX comparison (128->128, 2-warp)


| Metric               | WaveMaskSum | MMA bias | Tin2   | MMA vs Original | MMA vs Tin2 |
| -------------------- | ----------- | -------- | ------ | --------------- | ----------- |
| b32 registers        | 19,515      | 13,987   | 10,322 | -28%            | +35%        |
| b16 registers        | 4,315       | 485      | 1,667  | -89%            | -71%        |
| PTX lines            | 22,818      | 7,775    | 13,138 | -66%            | -41%        |
| mov instructions     | 2,807       | 273      | 958    | -90%            | -71%        |
| shuffle instructions | 640         | 0        | 240    | -100%           | -100%       |
| wmma/mma ops         | 256         | 272      | 512    | +6%             | —           |
| st.shared            | 114         | 136      | 144    | +19%            | -6%         |
| ld.shared            | 338         | 346      | 322    | +2%             | +7%         |
| local spills         | 132         | 0        | 0      | eliminated      | same        |
| atomics (red)        | 257         | 257      | 257    | same            | same        |


The MMA bias reduce eliminates all 640 shuffles and 2,500+ mov instructions from the bias reduce path.

### Latest PTX comparison (128->128, 2-warp, with readUint4 + MMA bias)

| Metric              | Slang (latest) | Tin2   | Slang/Tin2 |
|---------------------|----------------|--------|------------|
| b32 registers       | 7,982          | 10,322 | **0.77x (23% fewer)** |
| b16 registers       | 577            | 1,667  | 0.35x      |
| PTX lines           | 6,548          | 13,138 | **0.50x (half)** |
| MMA ops (total)     | 256            | 512    | 0.50x (wmma m16n16k16 vs mma m16n8k16) |
| shuffle             | 0              | 240    | eliminated |
| mov                 | 215            | 958    | 0.22x      |
| cvt                 | **193**        | 3      | **64x more** |
| st.shared           | 96             | 144    | 0.67x      |
| ld.shared           | 320            | 322    | ~same      |
| ld.global/ld.param  | **261**        | 199    | **1.3x more** |
| local spills        | 0              | 0      | same       |
| atomics (red)       | 256            | 257    | same       |

Slang now uses 23% fewer b32 registers than Tin2 and generates a kernel half the size. Two areas where Slang is still heavier:

- **cvt instructions (193 vs 3)**: Type conversions from the autodiff machinery and generic type handling. Tin2 stays in half throughout with no conversions.
- **ld.global (261 vs 199)**: Slang loads more from global memory, likely from `DifferentialPair.primal` accessing the parameter buffer extra times rather than reusing register-resident data.

## 6. Updated Decomposed + Full Backward (with MMA bias reduce + OPA shmem doubling)

### Decomposed: MMA Transpose + Bias Reduce (MMA-based)


| Size   | Network | Slang 1w (ms) | Slang 2w (ms) | Tin2 1w (ms) | Tin2 2w (ms) |
| ------ | ------- | ------------- | ------------- | ------------ | ------------ |
| tiny   | 32→16   | 0.0023        | 0.0023        | 0.0022       | 0.0022       |
| small  | 64→16   | 0.0024        | 0.0024        | 0.0037       | 0.0026       |
| medium | 128→32  | 0.0033        | 0.0033        | 0.0031       | 0.0033       |
| large  | 256→64  | 0.0064        | 0.0062        | 0.0058       | 0.0060       |
| xlarge | 128→128 | 0.0062        | 0.0061        | 0.0095       | 0.0076       |


### Decomposed: OuterProductAccumulate (tiled, shmem doubled)


| Size   | Network | Slang 1w (ms) | Slang 2w (ms) | Tin2 1w (ms) | Tin2 2w (ms) |
| ------ | ------- | ------------- | ------------- | ------------ | ------------ |
| tiny   | 32→16   | 0.0038        | 0.0027        | 0.0028       | 0.0030       |
| small  | 64→16   | 0.0046        | 0.0038        | 0.0040       | 0.0030       |
| medium | 128→32  | 0.0065        | 0.0050        | 0.0066       | 0.0047       |
| large  | 256→64  | 0.0169        | 0.0144        | 0.0188       | 0.0161       |
| xlarge | 128→128 | 0.0146        | 0.0121        | 0.0147       | 0.0128       |


### Sum of Parts vs Full Backward (1-warp)


| Size   | Slang OPA | Slang MMA+bias | Slang Sum | Slang Full | Overhead | Tin2 Sum | Tin2 Full | Tin2 Overhead |
| ------ | --------- | -------------- | --------- | ---------- | -------- | -------- | --------- | ------------- |
| tiny   | 0.0038    | 0.0023         | 0.0061    | 0.0042     | -31%     | 0.0050   | 0.0035    | -30%          |
| small  | 0.0046    | 0.0024         | 0.0070    | 0.0048     | -31%     | 0.0077   | 0.0049    | -36%          |
| medium | 0.0065    | 0.0033         | 0.0098    | 0.0073     | -26%     | 0.0097   | 0.0077    | -21%          |
| large  | 0.0169    | 0.0064         | 0.0233    | 0.0262     | +12%     | 0.0246   | 0.0290    | +18%          |
| xlarge | 0.0146    | 0.0062         | 0.0208    | 0.0182     | -13%     | 0.0242   | 0.0192    | -21%          |


### Sum of Parts vs Full Backward (2-warp)


| Size   | Slang OPA | Slang MMA+bias | Slang Sum | Slang Full | Overhead | Tin2 Sum | Tin2 Full | Tin2 Overhead |
| ------ | --------- | -------------- | --------- | ---------- | -------- | -------- | --------- | ------------- |
| tiny   | 0.0027    | 0.0023         | 0.0050    | 0.0031     | -38%     | 0.0052   | 0.0036    | -31%          |
| small  | 0.0038    | 0.0024         | 0.0062    | 0.0038     | -39%     | 0.0056   | 0.0033    | -41%          |
| medium | 0.0050    | 0.0033         | 0.0083    | 0.0059     | -29%     | 0.0080   | 0.0045    | -44%          |
| large  | 0.0144    | 0.0062         | 0.0206    | 0.0207     | +0%      | 0.0221   | 0.0227    | +3%           |
| xlarge | 0.0121    | 0.0061         | 0.0182    | 0.0161     | -12%     | 0.0204   | 0.0152    | -25%          |


### Full Backward: Slang vs Tin2

**1-warp:**


| Size   | Network | Slang (ms) | Tin2 (ms) | Difference |
| ------ | ------- | ---------- | --------- | ---------- |
| tiny   | 32→16   | 0.0042     | 0.0035    | +20%       |
| small  | 64→16   | 0.0048     | 0.0049    | -2%        |
| medium | 128→32  | 0.0073     | 0.0077    | -5%        |
| large  | 256→64  | 0.0262     | 0.0290    | -10%       |
| xlarge | 128→128 | 0.0182     | 0.0192    | -5%        |


**2-warp:**


| Size   | Network | Slang (ms) | Tin2 (ms) | Difference |
| ------ | ------- | ---------- | --------- | ---------- |
| tiny   | 32→16   | 0.0031     | 0.0036    | -14%       |
| small  | 64→16   | 0.0038     | 0.0033    | +15%       |
| medium | 128→32  | 0.0059     | 0.0045    | +31%       |
| large  | 256→64  | 0.0207     | 0.0227    | -9%        |
| xlarge | 128→128 | 0.0161     | 0.0152    | +6%        |


## 7. Scaling Analysis: Decomposed Backward (batch 512–8192)

Comprehensive benchmarking across batch sizes reveals how each component scales.
All data: 2-warp, tiled (OptimalLayout) weights.

Benchmark scripts: `slang/tests/integration/slangpy/neural/benchmarks/run_decomposed_benchmark.py`
and `tin2/benchmarks/mlp_perf/run_tin2_decomposed_benchmark.py`.
Charts: `slang/tests/integration/slangpy/neural/benchmarks/benchmark_results/`.

### MMA Transpose Scaling

| Batch | Slang xlarge (ms) | Tin2 xlarge (ms) | Ratio |
|-------|-------------------|------------------|-------|
| 512   | 0.0057            | 0.0066           | 0.86x |
| 1024  | 0.0102            | 0.0076           | 1.34x |
| 2048  | 0.0195            | 0.0135           | 1.44x |
| 4096  | 0.0369            | 0.0260           | 1.42x |
| 8192  | 0.0706            | 0.0505           | 1.40x |

At scale (≥1024 batch), Slang's MMA transpose is consistently ~1.4x slower.

### Bias Reduce Scaling

| Batch | Slang xlarge (ms) | Tin2 xlarge (ms) | Ratio |
|-------|-------------------|------------------|-------|
| 512   | 0.0037            | 0.0037           | 1.00x |
| 1024  | 0.0062            | 0.0069           | 0.90x |
| 2048  | 0.0102            | 0.0099           | 1.03x |
| 4096  | 0.0172            | 0.0173           | 0.99x |
| 8192  | 0.0315            | 0.0328           | 0.96x |

Bias reduce: **perfect parity** across all batch sizes.

### OPA Scaling

| Batch | Slang xlarge (ms) | Tin2 xlarge (ms) | Ratio |
|-------|-------------------|------------------|-------|
| 512   | 0.0178            | 0.0219           | 0.81x |
| 1024  | 0.0424            | 0.0384           | 1.10x |
| 2048  | 0.0591            | 0.0628           | 0.94x |
| 4096  | 0.1095            | 0.1218           | 0.90x |
| 8192  | 0.1952            | 0.2303           | 0.85x |

OPA: **Slang 10-19% faster** at large batch sizes.

### Full Backward Scaling

| Batch | Slang xlarge (ms) | Tin2 xlarge (ms) | Ratio |
|-------|-------------------|------------------|-------|
| 512   | 0.0257            | 0.0244           | 1.05x |
| 1024  | 0.0484            | 0.0455           | 1.06x |
| 2048  | 0.0882            | 0.0777           | 1.14x |
| 4096  | 0.1650            | 0.1533           | 1.08x |
| 8192  | 0.3045            | 0.2671           | 1.14x |

Full backward: **Slang 5-14% slower** overall. OPA gains partially offset MMA transpose losses.

### Consistency Check (xlarge, batch 4096)

| Component       | Slang (ms) | Tin2 (ms) |
|-----------------|-----------|----------|
| MMA Transpose   | 0.0369    | 0.0260   |
| Bias Reduce     | 0.0172    | 0.0173   |
| OPA             | 0.1095    | 0.1218   |
| **Sum of parts**| **0.1636**| **0.1651**|
| **Full backward**| **0.1650**| **0.1533**|
| Fusion overhead | +0.9%     | -7.2%    |

Tin2 benefits from ~7% fusion speedup (shared register state across phases),
while Slang's sum-of-parts matches full backward almost exactly.

## 8. Nsight Compute Profiling: MMA Transpose Bottleneck

Profiled the Slang backward kernel (xlarge 128→128, 2-warp, batch 4096) using Nsight Compute.
Slang was profiled via a standalone C++ launcher loading the slangc-compiled cubin.

### Hardware Metrics Comparison

| Metric | Slang | Tin2 | Ratio |
|--------|-------|------|-------|
| Tensor pipe instructions | 1,048,576 | 1,048,576 | **1.00x** (identical) |
| LSU pipe instructions | 3,817,472 | 2,637,824 | 1.45x |
| ALU pipe instructions | 997,376 | 4,358,144 | 0.23x |
| **Global load sectors** | **8,912,896** | **4,456,448** | **2.00x** |
| Global store sectors | 262,144 | 524,288 | 0.50x |
| Shared memory wavefronts | 3,426,304 | 1,589,661 | 2.16x |
| Warps active (occupancy) | 3.99% | 15.55% | 0.26x |

### Warp Stall Reasons

| Stall Reason | Slang | Tin2 |
|-------------|-------|------|
| Wait (instruction latency) | **36.13%** | 23.37% |
| Math pipe throttle | 20.29% | **24.53%** |
| Long scoreboard (memory) | 14.98% | 14.87% |
| Short scoreboard (shmem) | 1.82% | 5.88% |
| Barrier | 0.41% | 0% |
| Not selected | 0% | 3.35% |

Tin2 is **compute-bound** (48% math throttle + wait), while Slang spends more time
waiting for results (36% wait) due to lower occupancy and higher memory latency.

### SASS Register & Shared Memory (ptxas sm_89)

| | Slang | Tin2 |
|---|-------|------|
| SASS registers | 190 | 182 |
| Static shared memory | **40,960 bytes** | **0 bytes** |
| Spills | 0 | 0 |

Tin2 uses zero static shared memory because it communicates via shuffles.
Slang's 40KB shmem limits occupancy to ~2 blocks/SM.

### Occupancy Difference

Slang's 40KB static shared memory is sized for the full training kernel (MMA + bias + OPA).
For the MMA-transpose-only benchmark, computed minimum is only ~2-6KB depending on network size.
However, in real MLP training all three phases share the same pool, so 40KB is required.

## 9. Root Cause: `wmma.load.a` 2x Global Memory Overhead

### Discovery

The 2.00x global load sector difference was traced to the `wmma.load.a` (cooperative matrix load)
instruction generating 2x the expected sector requests.

### Isolated CUDA Verification

A minimal CUDA test loading the same 8 tiles (32KB) with both methods:

```
test_wmma_load_only:    1,048,576 sectors  (2x expected)
test_regular_load_only:   524,288 sectors  (1x expected)
```

Confirmed on sm_89 (Ada) and sm_90 (Hopper) — consistent across architectures.

### SASS-Level Root Cause

SASS disassembly reveals the mechanism:

**Regular `ld.global.v4` (Tin2-style):** 8 × `LDG.E.128.CONSTANT` per warp
- Each lane loads 16 contiguous bytes; 32 lanes cover one full 512-byte tile
- Perfectly coalesced: **16 sectors per tile**

**`wmma.load.a` (Slang):** 32 × `LDG.E` (4-byte loads) per warp
- Each tile generates 4 × `LDG.E` per thread, at scattered offsets:
  - `+0x000` (row 0, col 0-1)
  - `+0x010` (row 0, col 8-9)
  - `+0x100` (row 8, col 0-1)
  - `+0x110` (row 8, col 8-9)
- The WMMA fragment layout distributes each thread's registers across 4 quadrants
  of the 16×16 tile, requiring scattered reads: **32 sectors per tile (2x)**

### Why This Is Fundamental

The WMMA API defines an opaque fragment layout optimized for MMA computation, not memory access.
Each thread's 4 fragment registers hold data from 4 non-adjacent quadrants of the tile.
The hardware must load from these scattered positions, doubling sector requests.

Tin2 avoids this by:
1. Loading tiles linearly with `LDG.E.128` (perfectly coalesced, 1x sectors)
2. Using **shuffles** (~30 per tile) to redistribute data into the MMA register layout

This is a tradeoff: WMMA trades memory efficiency for API simplicity and portability.

### Layout vs Row/Column Major: No Effect

Tested both `wmma.load.a` with RowMajor and ColumnMajor layouts — **identical** 8,912,896 sectors.
The 2x overhead is from the fragment register distribution, not the access pattern direction.

### Alternative Approaches Tested

| Approach | Global sectors | Performance | Notes |
|----------|---------------|-------------|-------|
| `wmma.load.a` (original) | 8,912,896 (2x) | **Baseline** | Best overall |
| `readUint4` + `operator[]` fill | 4,784,128 (1.07x) | -11% faster | Wrong results (no transpose) |
| `readUint4` → shared → `wmma.load.a` from shared | ~4.8M (1x) | +53% slower | GroupSync kills perf |
| Per-warp shmem staging (WaveSync only) | ~4.8M (1x) | +18% slower | Extra shmem write + load |
| `BindlessAddress.readUint4` scalar path | 34M (8x) | Much slower | Generates `ld.global.u16` per element |

**Conclusion**: `wmma.load.a` from global memory at 2x sectors is the best available option
within the WMMA API. The only way to achieve 1x would be to bypass WMMA and use
`ldmatrix` + `mma.sync` with manual register layout (architecture-specific, not portable).

### `BindlessAddress.readUint4` Fast Path

During investigation, we discovered that `BindlessAddress<half>.readUint4()` generates
512 scalar `ld.global.u16` loads (2 bytes each) instead of vectorized `ld.global.v4`.
This is because `accessUint4` reads through `RWStructuredBuffer<half>` element-by-element.

A CUDA fast path was prototyped that casts to `uint4*` for vectorized reads:
```slang
let bufferPtr = __getStructuredBufferPtr(*handle);
let basePtr = reinterpret<T**>(bufferPtr)[0];
let elemPtr = basePtr + baseIndex + offsetIndex;
let vecPtr = reinterpret<uint4*>(elemPtr);
return *vecPtr;
```
This reduced `ld.global.u16` from 512 to 0, replacing them with 63 `ld.global.v4.u32`.
However, this optimization was not committed as the `wmma.load.a` path (used for weight
loading) is more efficient overall despite its 2x sector overhead.

## 10. Optimization Attempts That Did Not Help

### Pre-loading full input vector to shared memory
Loaded all tile iterations' input vectors to shared memory before the tile loop,
eliminating per-tile WaveSync barriers. **Result: 38% regression.** The compiler
(`ptxas`) generated worse SASS code for the large unrolled block — fewer registers
(132 vs 190) but more address computation overhead (21 extra `mul.wide`).

### Double buffering
Overlapped `st.shared` for next tile's vector with current tile's MMA.
**Result: 18% regression.** The ILP gain (~40-60 cycles) was negligible in a
~45,000 cycle kernel, and the extra addressing complexity hurt ptxas optimization.

### 2-row C extraction
Stored 2 rows of matC per Phase 4 iteration instead of 1, halving Phase 4 barriers.
**Result: 9-11% regression.** Extra address math offset the barrier savings.

### `CoopMat.clear()` optimization
Added a `clear()` method using direct `0U` register assignment to avoid `f64→f16`
conversions in `fill(T(0))`. Reduced `cvt` instructions from 96 to 8.
**Result: No measurable improvement** — the conversions were not on the critical path.

### Key Lesson
The original per-tile pattern (`st.shared → WaveSync → MMA`, one row at a time for C
extraction) is a sweet spot that `ptxas` optimizes very well. Any deviation triggers
different register allocation decisions that result in worse performance, even when
the PTX looks better on paper.



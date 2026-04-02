// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Single-layer backward benchmark: chains transpose MMA + outer product + bias reduce.
// Matches Tin2's HLinear::backward() for direct performance comparison.

import slang.neural;

#ifndef INPUT_SIZE
#define INPUT_SIZE 64
#endif
#ifndef OUTPUT_SIZE
#define OUTPUT_SIZE 16
#endif

static const int SubgroupSize = 32;
#ifndef SUBGROUP_COUNT
#define SUBGROUP_COUNT 2
#endif
static const int SubgroupCount = SUBGROUP_COUNT;

typealias Address = BindlessAddress<half>;

typealias ShMemSize = SharedMemorySize<half, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, SubgroupCount>;
typealias ShMemSizeLayer = ShMemSize.OfLayer1<INPUT_SIZE, OUTPUT_SIZE>;

typealias MMATranspose = MMAHelperNew<half, INPUT_SIZE, OUTPUT_SIZE, SubgroupSize, TargetEnum.CUDA, true>;
typealias MMAForward = MMAHelperNew<half, INPUT_SIZE, OUTPUT_SIZE, SubgroupSize, TargetEnum.CUDA, false>;

static const int NTileRows = (OUTPUT_SIZE + 15) / 16;
static const int NTileCols = (INPUT_SIZE + 15) / 16;
static const int TILED_WEIGHT_COUNT = NTileRows * NTileCols * 256;

static const uint ElementCountPerVector = sizeof(uint4) / sizeof(half);
static const int Uint4AlignedInputSize = ((INPUT_SIZE + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;

public int get_input_size() { return INPUT_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }
public int get_tiled_weight_count() { return TILED_WEIGHT_COUNT; }

[shader("compute")]
#if SUBGROUP_COUNT == 2
[numthreads(64, 1, 1)]
#else
[numthreads(32, 1, 1)]
#endif
void compute_backward(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform RWStructuredBuffer<half>.Handle params,
    uniform RWStructuredBuffer<half> outputs,
    uniform RWStructuredBuffer<half>.Handle weightGrad,
    uniform RWStructuredBuffer<half>.Handle biasGrad,
    uniform int batch_size)
{
#if SUBGROUP_COUNT == 2
    int sampleIdx = int(gid.x) * 2 + int(gtid / 32);
#else
    int sampleIdx = int(gid.x);
#endif
    if (sampleIdx >= batch_size)
        return;

    int subgroupId = int(gtid / SubgroupSize);
    uint laneId = gtid % SubgroupSize;

    let weightAddr = Address(params).getOffset(0);
    let weightGradAddr = Address(weightGrad).getOffset(0);
    let biasGradAddr = Address(biasGrad).getOffset(0);

    half dOutArr[OUTPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < OUTPUT_SIZE; i++)
        dOutArr[i] = half(1.0h);

    half inputArr[INPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
        inputArr[i] = half(1.0h);

    // 1. Transpose MMA: dIn = W^T * dOut
    let dInArr = MMATranspose.mma<half, Address, half[OUTPUT_SIZE], half[Uint4AlignedInputSize], half, false>(
        dOutArr, weightAddr, none);

    // Store dIn checksum to output buffer
    half sum = half(0.0);
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
        sum = sum + dInArr[i];
    outputs[sampleIdx * SubgroupSize + int(laneId)] = sum;

    // 2. Outer product: dW = dOut * input^T
    MMAForward.outerProductAccumulate<half, Address, half, half[OUTPUT_SIZE], half, half[INPUT_SIZE],
                                      ShMemSizeLayer, SubgroupCount>(
        0u, dOutArr, inputArr, weightGradAddr);

    GroupMemoryBarrierWithGroupSync();

    // 3. Bias reduce: dBias = sum(dOut)
    MMAForward.sumReduceRowsV2<half, Address, half[OUTPUT_SIZE], ShMemSizeLayer, SubgroupCount>(
        0u, dOutArr, subgroupId, biasGradAddr);
}

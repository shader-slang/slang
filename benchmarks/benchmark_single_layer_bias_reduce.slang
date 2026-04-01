// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Single-layer bias gradient reduction benchmark: bias_grad[i] = sum_{lanes} dOut[i]
// Uses MMAHelperNew.sumReduceRowsV2 (shuffle-based tree reduction).

import slang.neural;

#ifndef INPUT_SIZE
#define INPUT_SIZE 64
#endif
#ifndef OUTPUT_SIZE
#define OUTPUT_SIZE 16
#endif

static const int SubgroupSize = 32;
#ifndef SUBGROUP_COUNT
#define SUBGROUP_COUNT 1
#endif
static const int SubgroupCount = SUBGROUP_COUNT;

typealias Address = BindlessAddress<half>;

// Minimal shmem for sumReduceRows: SubgroupCount * PairsPerWarp * sizeof(uint) bytes.
// PairsPerWarp = ((OUTPUT_SIZE + 1) & ~1) / 2.
static const uint BiasReduceShMemBytes =
    (SUBGROUP_COUNT * (((OUTPUT_SIZE + 1) & ~1) / 2) * 4 + 15) & ~15;
public struct BiasReduceShMem : ISharedMemorySize { public static const uint Bytes = BiasReduceShMemBytes; }
typealias ShMemSizeLayer = BiasReduceShMem;

typealias MMA = MMAHelperNew<half, INPUT_SIZE, OUTPUT_SIZE, SubgroupSize, TargetEnum.CUDA>;

public int get_output_size() { return OUTPUT_SIZE; }

// Validation kernel: reads per-thread dOut from input buffer.
// Input layout: batch_size * SubgroupSize * SubgroupCount threads,
// each has OUTPUT_SIZE halfs at offset (sampleIdx * SubgroupSize + laneId) * OUTPUT_SIZE.
[shader("compute")]
#if SUBGROUP_COUNT == 2
[numthreads(64, 1, 1)]
#else
[numthreads(32, 1, 1)]
#endif
void compute_bias_reduce_validate(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform RWStructuredBuffer<half>.Handle biasBuffer,
    uniform StructuredBuffer<half> dOutInput,
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

    let biasAddr = Address(biasBuffer).getOffset(0);

    int threadOffset = (sampleIdx * SubgroupSize + int(laneId)) * OUTPUT_SIZE;
    half dOutArr[OUTPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < OUTPUT_SIZE; i++)
        dOutArr[i] = dOutInput[threadOffset + i];

    MMA.sumReduceRowsV2<half, Address, half[OUTPUT_SIZE], ShMemSizeLayer, SubgroupCount>(
        0u, dOutArr, subgroupId, biasAddr);
}

// Timing kernel: uses constant dOut to avoid input buffer overhead.
[shader("compute")]
#if SUBGROUP_COUNT == 2
[numthreads(64, 1, 1)]
#else
[numthreads(32, 1, 1)]
#endif
void compute_bias_reduce(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform RWStructuredBuffer<half>.Handle biasBuffer,
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

    let biasAddr = Address(biasBuffer).getOffset(0);

    half dOutArr[OUTPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < OUTPUT_SIZE; i++)
        dOutArr[i] = half(1.0f);

    MMA.sumReduceRowsV2<half, Address, half[OUTPUT_SIZE], ShMemSizeLayer, SubgroupCount>(
        0u, dOutArr, subgroupId, biasAddr);
}

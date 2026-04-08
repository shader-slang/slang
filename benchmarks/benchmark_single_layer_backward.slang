// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Single-layer backward benchmark using MMAHelperNewV2 (WaveMatrix-based).

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

typealias Helper = MMAHelperNewV2<half, INPUT_SIZE, OUTPUT_SIZE, SubgroupSize, TargetEnum.CUDA>;

static const int NTileRows = (OUTPUT_SIZE + 15) / 16;
static const int NTileCols = (INPUT_SIZE + 15) / 16;
static const int TILED_WEIGHT_COUNT = NTileRows * NTileCols * 256;

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
    uniform RWStructuredBuffer<half>.Handle params,
    uniform RWStructuredBuffer<uint> outputs,
    uniform RWStructuredBuffer<half>.Handle weightGrad,
    uniform RWStructuredBuffer<half>.Handle biasGrad,
    uniform int batch_size)
{
    uint tidx = cudaThreadIdx().x;
    uint bidx = cudaBlockIdx().x;

    int subgroupId = int(tidx / 32u);
    uint laneId = tidx % 32u;
    int sampleIdx = int(bidx) * SubgroupCount + subgroupId;

    if (sampleIdx >= batch_size)
        return;

    let weightAddr = Address(params).getOffset(0);
    let weightGradAddr = Address(weightGrad).getOffset(0);
    let biasGradAddr = Address(biasGrad).getOffset(0);

    static const int DOutPacked = OUTPUT_SIZE / 2;
    static const int InputPacked = INPUT_SIZE / 2;

    uint dOutPacked[DOutPacked];
    [ForceUnroll]
    for (int i = 0; i < DOutPacked; i++)
        dOutPacked[i] = 0x3C003C00u;

    uint inputPacked[InputPacked];
    [ForceUnroll]
    for (int i = 0; i < InputPacked; i++)
        inputPacked[i] = 0x3C003C00u;

    Helper.DOutMatrix dOut;
    dOut.fromArrayPacked<DOutPacked>(dOutPacked);

    Helper.InputMatrix input;
    input.fromArrayPacked<InputPacked>(inputPacked);

    // 1. Transpose MMA: dIn = W^T * dOut
    Helper.ResultMatrix result;
    Helper.mmaTranspose<Address>(dOut, weightAddr, result);

    // 2. Bias reduce
    Helper.biasReduce<half, Address, ShMemSizeLayer, SubgroupCount>(
        0u, dOut, subgroupId, biasGradAddr);

    // 3. Outer product
    Helper.outerProduct<half, Address, ShMemSizeLayer, SubgroupCount>(
        0u, dOut, input, weightGradAddr);

    // 4. Extract result to local array and compute checksum
    static const int ResultPacked = INPUT_SIZE / 2;
    uint resultPacked[ResultPacked];
    result.toArrayPacked<ResultPacked>(resultPacked);

    uint checksum = 0;
    [ForceUnroll]
    for (int i = 0; i < ResultPacked; i++)
        checksum = checksum + resultPacked[i];

    outputs[sampleIdx * SubgroupSize + int(laneId)] = checksum;
}

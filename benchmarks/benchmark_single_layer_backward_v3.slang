// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Single-layer backward benchmark using MMAHelperNewV3 (WaveMatrix-based).

import slang.neural;

#ifndef INPUT_SIZE
#define INPUT_SIZE 64
#endif
#ifndef OUTPUT_SIZE
#define OUTPUT_SIZE 16
#endif

#ifndef FRAGMENT_CHECKSUM_MODE
#define FRAGMENT_CHECKSUM_MODE 0
#endif

#ifndef ENABLE_BIAS_REDUCE
#define ENABLE_BIAS_REDUCE 1
#endif

#ifndef ENABLE_OUTER_PRODUCT
#define ENABLE_OUTER_PRODUCT 1
#endif

#ifndef ENABLE_OUTER_REDUCE
#define ENABLE_OUTER_REDUCE 1
#endif

#ifndef ENABLE_OUTER_STORE
#define ENABLE_OUTER_STORE 1
#endif

#ifndef EARLY_RESULT_CHECKSUM
#define EARLY_RESULT_CHECKSUM 0
#endif

static const int SubgroupSize = 32;
#ifndef SUBGROUP_COUNT
#define SUBGROUP_COUNT 2
#endif
static const int SubgroupCount = SUBGROUP_COUNT;

typealias Address = BindlessAddress<half>;

typealias ShMemSize = SharedMemorySize<half, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, SubgroupCount>;
typealias ShMemSizeLayer = ShMemSize.OfLayer1<INPUT_SIZE, OUTPUT_SIZE>;

typealias Helper = MMAHelperNewV3<half, INPUT_SIZE, OUTPUT_SIZE, SubgroupSize, TargetEnum.CUDA>;

// Use V2 just for fromArrayPacked/toArrayPacked (same shuffle logic)
typealias HelperV2 = MMAHelperNewV2<half, INPUT_SIZE, OUTPUT_SIZE, SubgroupSize, TargetEnum.CUDA>;

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
    static const int ResultPacked = INPUT_SIZE / 2;

    uint dOutPacked[DOutPacked];
    [ForceUnroll]
    for (int i = 0; i < DOutPacked; i++)
        dOutPacked[i] = 0x3C003C00u;

    uint inputPacked[InputPacked];
    [ForceUnroll]
    for (int i = 0; i < InputPacked; i++)
        inputPacked[i] = 0x3C003C00u;

    // Build WaveMatrix tiles via V2's fromArrayPacked
    Helper.DOutMatrix dOut;
    HelperV2.fromArrayPacked(dOutPacked, dOut.tiles);

    Helper.InputMatrix input;
    HelperV2.fromArrayPacked(inputPacked, input.tiles);

    // 1. Transpose MMA: dIn = W^T * dOut
    Helper.ResultMatrix result;
    Helper.mmaTranspose<Address>(dOut, weightAddr, result);

    uint checksum = 0;
#if FRAGMENT_CHECKSUM_MODE && EARLY_RESULT_CHECKSUM
    [ForceUnroll]
    for (int t = 0; t < Helper.InputTileCount; ++t)
    {
        checksum += result.tiles[t].fragmentRead(0);
        checksum += result.tiles[t].fragmentRead(1);
        checksum += result.tiles[t].fragmentRead(2);
        checksum += result.tiles[t].fragmentRead(3);
    }
#endif

    // 2. Bias reduce
#if ENABLE_BIAS_REDUCE
    Helper.biasReduce<half, Address, ShMemSizeLayer, SubgroupCount>(
        0u, dOut, subgroupId, biasGradAddr);
#endif

    // 3. Outer product
    uint outerChecksum = 0;
#if ENABLE_OUTER_PRODUCT
#if ENABLE_OUTER_STORE
    Helper.outerProduct<half, Address, ShMemSizeLayer, SubgroupCount>(
        0u, dOut, input, weightGradAddr);
#endif
#endif

    // 4. Compute checksum
#if !(FRAGMENT_CHECKSUM_MODE && EARLY_RESULT_CHECKSUM)
    checksum = 0;
#if FRAGMENT_CHECKSUM_MODE
    [ForceUnroll]
    for (int t = 0; t < Helper.InputTileCount; ++t)
    {
        checksum += result.tiles[t].fragmentRead(0);
        checksum += result.tiles[t].fragmentRead(1);
        checksum += result.tiles[t].fragmentRead(2);
        checksum += result.tiles[t].fragmentRead(3);
    }
#else
    uint resultPacked[ResultPacked];
    HelperV2.toArrayPacked<ResultPacked, Helper.InputTileCount>(result.tiles, resultPacked);

    [ForceUnroll]
    for (int i = 0; i < ResultPacked; i++)
        checksum = checksum + resultPacked[i];
#endif
#endif
    checksum += outerChecksum;
    outputs[sampleIdx * SubgroupSize + int(laneId)] = checksum;
}

// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Single-layer backward benchmark using MMAHelperNewV2.backward().

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
typealias InputVec = WaveTangledVector<half, ShMemSizeLayer, INPUT_SIZE, SubgroupSize, SubgroupCount>;
typealias OutputVec = WaveTangledVector<half, ShMemSizeLayer, OUTPUT_SIZE, SubgroupSize, SubgroupCount>;

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

    InputVec input = InputVec(half(1.0));
    OutputVec dOut = OutputVec(half(1.0));

    InputVec dInput = Helper.backward<Address, ShMemSizeLayer, SubgroupCount>(
        input, dOut, weightAddr, weightGradAddr, biasGradAddr);

    // Checksum
    uint checksum = 0;
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
        checksum += bit_cast<uint>(half2(dInput[i], half(0)));

    outputs[sampleIdx * SubgroupSize + int(laneId)] = checksum;
}

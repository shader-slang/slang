// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Single-layer backward benchmark using TiledMMAHelper.backward() dispatcher.
// On CUDA, dispatches to TiledMMACuda (WaveMatrix register ops).

#define UNIT_TEST
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

static const int NTileRows = (OUTPUT_SIZE + 15) / 16;
static const int NTileCols = (INPUT_SIZE + 15) / 16;
static const int TILED_WEIGHT_COUNT = NTileRows * NTileCols * 256;

public int get_input_size() { return INPUT_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }
public int get_tiled_weight_count() { return TILED_WEIGHT_COUNT; }

public struct BenchShMem : ISharedMemorySize { public static const uint Bytes = 40 * 1024; }

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

    half inputArr[INPUT_SIZE];
    half dOutArr[OUTPUT_SIZE];
    for (int i = 0; i < INPUT_SIZE; i++) inputArr[i] = half(1.0);
    for (int i = 0; i < OUTPUT_SIZE; i++) dOutArr[i] = half(1.0);

    let dInput = neural_unittest.testTiledBackward<half, INPUT_SIZE, OUTPUT_SIZE, BenchShMem, SubgroupCount>(
        inputArr, dOutArr, params, weightGrad, biasGrad);

    // Checksum to prevent dead code elimination
    uint checksum = 0;
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
        checksum += bit_cast<uint>(half2(dInput[i], half(0)));

    outputs[sampleIdx * SubgroupSize + int(laneId)] = checksum;
}

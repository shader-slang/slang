// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Single-layer forward pass benchmark using WaveTangledVector + OptimalLayout.
// On CUDA, this routes through MMAHelperNew (m16n8k16, zero shared memory).

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

static const int NTileRows = (OUTPUT_SIZE + 15) / 16;
static const int NTileCols = (INPUT_SIZE + 15) / 16;
static const int TILED_WEIGHT_COUNT = NTileRows * NTileCols * 256;

static const int BIAS_OFFSET = TILED_WEIGHT_COUNT;
static const int PARAM_COUNT = TILED_WEIGHT_COUNT + OUTPUT_SIZE;

public int get_input_size() { return INPUT_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }
public int get_total_params() { return PARAM_COUNT; }
public int get_tiled_weight_count() { return TILED_WEIGHT_COUNT; }

// Shared memory size (unused on CUDA with MMAHelperNew, but required by WaveTangledVector type).
typealias ShMemSize = SharedMemorySize<half, TargetEnum.CUDA, ExecutionMode.Inference, SubgroupSize, SubgroupCount>;
typealias ShMemSizeLayer = ShMemSize.OfLayer1<INPUT_SIZE, OUTPUT_SIZE>;

typealias VInput = WaveTangledVector<half, ShMemSizeLayer, INPUT_SIZE, SubgroupSize, SubgroupCount>;
typealias VOutput = WaveTangledVector<half, ShMemSizeLayer, OUTPUT_SIZE, SubgroupSize, SubgroupCount>;

static const int NUM_TILES = NTileRows * NTileCols;

// Convert one 16x16 tile from row-major to native MMA fragment register layout.
[shader("compute")]
[numthreads(32, 1, 1)]
void convert_tiled_to_native(
    uint3 gid_dispatch : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform StructuredBuffer<half> tiledWeights,
    uniform RWStructuredBuffer<half> nativeWeights)
{
    int tileIdx = int(gid_dispatch.x);
    if (tileIdx >= NUM_TILES)
        return;

    uint laneid = gtid;
    uint gid = laneid >> 2;
    uint tid = laneid & 3;

    int tileBase = tileIdx * 256;

    half h0 = tiledWeights[tileBase + gid * 16 + tid * 2];
    half h1 = tiledWeights[tileBase + gid * 16 + tid * 2 + 1];
    half h2 = tiledWeights[tileBase + (gid + 8) * 16 + tid * 2];
    half h3 = tiledWeights[tileBase + (gid + 8) * 16 + tid * 2 + 1];
    half h4 = tiledWeights[tileBase + gid * 16 + tid * 2 + 8];
    half h5 = tiledWeights[tileBase + gid * 16 + tid * 2 + 9];
    half h6 = tiledWeights[tileBase + (gid + 8) * 16 + tid * 2 + 8];
    half h7 = tiledWeights[tileBase + (gid + 8) * 16 + tid * 2 + 9];

    int outBase = tileBase + int(laneid) * 8;
    nativeWeights[outBase + 0] = h0;
    nativeWeights[outBase + 1] = h1;
    nativeWeights[outBase + 2] = h2;
    nativeWeights[outBase + 3] = h3;
    nativeWeights[outBase + 4] = h4;
    nativeWeights[outBase + 5] = h5;
    nativeWeights[outBase + 6] = h6;
    nativeWeights[outBase + 7] = h7;
}

[shader("compute")]
#if SUBGROUP_COUNT == 2
[numthreads(64, 1, 1)]
#else
[numthreads(32, 1, 1)]
#endif
void compute_single_layer_forward_new_mma(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform RWStructuredBuffer<half>.Handle params,
    uniform RWStructuredBuffer<half> outputs,
    uniform int batch_size)
{
#if SUBGROUP_COUNT == 2
    int sampleIdx = int(gid.x) * 2 + int(gtid / 32);
#else
    int sampleIdx = int(gid.x);
#endif
    if (sampleIdx >= batch_size)
        return;

    let baseAddr = Address(params);
    let weightAddr = baseAddr.getOffset(0);
    let biasAddr = baseAddr.getOffset(BIAS_OFFSET);

    half inputArr[INPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
        inputArr[i] = half(float(i + 1));

    let input = VInput(inputArr);
    let output = input.linearTransform<Address, OptimalLayout, VOutput>(weightAddr, biasAddr);

    half sum = half(0.0);
    [ForceUnroll]
    for (int i = 0; i < OUTPUT_SIZE; i++)
        sum = sum + output[i];

    uint laneId = gtid % SubgroupSize;
    outputs[sampleIdx * SubgroupSize + int(laneId)] = sum;
}

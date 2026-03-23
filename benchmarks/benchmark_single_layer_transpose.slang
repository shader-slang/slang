// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Single-layer transpose MMA benchmark: dIn = W^T * dOut
// Uses MMAHelperNew with TransposeA=true (zero shared memory, warp shuffles).
// Weight matrix is OUTPUT_SIZE x INPUT_SIZE, same layout as the forward pass.

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

// Weight tiling: OUTPUT_SIZE x INPUT_SIZE in 16x16 tiles (same as forward)
static const int NTileRows = (OUTPUT_SIZE + 15) / 16;
static const int NTileCols = (INPUT_SIZE + 15) / 16;
static const int TILED_WEIGHT_COUNT = NTileRows * NTileCols * 256;
static const int PARAM_COUNT = TILED_WEIGHT_COUNT;

public int get_input_size() { return INPUT_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }
public int get_total_params() { return PARAM_COUNT; }
public int get_tiled_weight_count() { return TILED_WEIGHT_COUNT; }

// MMAHelperNew with TransposeA=true:
//   M = OutputSize, K = InputSize (physical weight dimensions)
//   Shared dimension = M (iterate over OutputSize)
//   Output dimension = K (result has InputSize elements)
typealias MMA = MMAHelperNew<half, INPUT_SIZE, OUTPUT_SIZE, SubgroupSize, TargetEnum.CUDA, true>;

static const int NUM_TILES = NTileRows * NTileCols;

static const uint ElementCountPerVector = sizeof(uint4) / sizeof(half);
static const int Uint4AlignedInputSize = ((INPUT_SIZE + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;

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
void compute_single_layer_transpose(
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

    // Synthetic dOut gradient: [1, 2, ..., OUTPUT_SIZE]
    half dOutArr[OUTPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < OUTPUT_SIZE; i++)
        dOutArr[i] = half(float(i + 1));

    // dIn = W^T * dOut (TransposeA=true, no bias)
    let dInArr = MMA.mma<half, Address, half[OUTPUT_SIZE], half[Uint4AlignedInputSize], half, false>(
        dOutArr, weightAddr, none);

    uint laneId = gtid % SubgroupSize;
#ifdef RES_VALIDATION
    if (sampleIdx == 0 && laneId == 0)
    {
        [ForceUnroll]
        for (int i = 0; i < INPUT_SIZE; i++)
            outputs[i] = dInArr[i];
    }
#else
    half sum = half(0.0);
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
        sum = sum + dInArr[i];
    outputs[sampleIdx * SubgroupSize + int(laneId)] = sum;
#endif
}

// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Single-layer outer product benchmark: dW[m][k] = sum_{lanes} dOut[m] * input[k]
// Uses MMAHelperNew.outerProductAccumulate (buildAllTilesFromArray + ChangeMajor + copyFrom).

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

typealias ShMemSize = SharedMemorySize<half, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, SubgroupCount>;
typealias ShMemSizeLayer = ShMemSize.OfLayer1<INPUT_SIZE, OUTPUT_SIZE>;

typealias MMA = MMAHelperNew<half, INPUT_SIZE, OUTPUT_SIZE, SubgroupSize, TargetEnum.CUDA>;

// Weight tiling: OUTPUT_SIZE x INPUT_SIZE in 16x16 tiles
static const int NTileRows = (OUTPUT_SIZE + 15) / 16;
static const int NTileCols = (INPUT_SIZE + 15) / 16;
static const int TILED_WEIGHT_COUNT = NTileRows * NTileCols * 256;

public int get_input_size() { return INPUT_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }
public int get_tiled_weight_count() { return TILED_WEIGHT_COUNT; }

// Validation kernel: reads dOut and input from buffers.
[shader("compute")]
#if SUBGROUP_COUNT == 2
[numthreads(64, 1, 1)]
#else
[numthreads(32, 1, 1)]
#endif
void compute_outer_product_validate(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform RWStructuredBuffer<half>.Handle weightGradBuffer,
    uniform StructuredBuffer<half> dOutInput,
    uniform StructuredBuffer<half> inputInput,
    uniform int batch_size)
{
#if SUBGROUP_COUNT == 2
    int sampleIdx = int(gid.x) * 2 + int(gtid / 32);
#else
    int sampleIdx = int(gid.x);
#endif
    if (sampleIdx >= batch_size)
        return;

    uint laneId = gtid % SubgroupSize;
    int threadOffset = (sampleIdx * SubgroupSize + int(laneId));

    half dOutArr[OUTPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < OUTPUT_SIZE; i++)
        dOutArr[i] = dOutInput[threadOffset * OUTPUT_SIZE + i];

    half inputArr[INPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
        inputArr[i] = inputInput[threadOffset * INPUT_SIZE + i];

    let weightAddr = Address(weightGradBuffer).getOffset(0);

    MMA.outerProductAccumulate<half, Address, half, half[OUTPUT_SIZE], half, half[INPUT_SIZE],
                               ShMemSizeLayer, SubgroupCount>(
        0u, dOutArr, inputArr, weightAddr);
}

// Timing kernel: uses constant values.
[shader("compute")]
#if SUBGROUP_COUNT == 2
[numthreads(64, 1, 1)]
#else
[numthreads(32, 1, 1)]
#endif
void compute_outer_product(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform RWStructuredBuffer<half>.Handle weightGradBuffer,
    uniform int batch_size)
{
#if SUBGROUP_COUNT == 2
    int sampleIdx = int(gid.x) * 2 + int(gtid / 32);
#else
    int sampleIdx = int(gid.x);
#endif
    if (sampleIdx >= batch_size)
        return;

#ifdef RES_VALIDATION
    uint laneId = WaveGetLaneIndex();
    half threadVal = half(laneId + 1) / half(SubgroupSize);
    half dOutScale = half(16.0h) / half(OUTPUT_SIZE);
    half inputScale = half(16.0h) / half(INPUT_SIZE);

    half dOutArr[OUTPUT_SIZE];
    for (int i = 0; i < OUTPUT_SIZE; i++)
        dOutArr[i] = half(i + 1) * threadVal * dOutScale;

    half inputArr[INPUT_SIZE];
    for (int i = 0; i < INPUT_SIZE; i++)
        inputArr[i] = half(i + 1) * threadVal * inputScale;
#else
    half dOutArr[OUTPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < OUTPUT_SIZE; i++)
        dOutArr[i] = half(1.0h);

    half inputArr[INPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
        inputArr[i] = half(1.0h);
#endif

    let weightAddr = Address(weightGradBuffer).getOffset(0);

    MMA.outerProductAccumulate<half, Address, half, half[OUTPUT_SIZE], half, half[INPUT_SIZE],
                               ShMemSizeLayer, SubgroupCount>(
        0u, dOutArr, inputArr, weightAddr);
}

// =========================================================================
// Layout conversion: native tiled -> row-major per tile
// Each workgroup (1 warp) converts one 16x16 tile.
// =========================================================================
typealias MatC = linalg.CoopMat<half, MemoryScope.Subgroup, 16, 16, linalg.CoopMatMatrixUse.MatrixAccumulator>;

static const int OPA_ElementsPerLane = (16 * 16) / 32;
static const int OPA_RegsPerLane = (OPA_ElementsPerLane * sizeof(half) + 3) / 4;
static const int OPA_TileStride = (OPA_RegsPerLane * 32 * 4 + sizeof(uint4) - 1) / sizeof(uint4);
static const int TotalTiles = NTileRows * NTileCols;
static const int NativeHalvesPerTile = OPA_TileStride * (int(sizeof(uint4)) / int(sizeof(half)));

public int get_native_halves_per_tile() { return NativeHalvesPerTile; }
public int get_total_tiles() { return TotalTiles; }

[shader("compute")]
[numthreads(32, 1, 1)]
void convert_native_to_rowmajor(
    uint3 gid : SV_GroupID,
    uniform RWStructuredBuffer<half>.Handle nativeBuffer,
    uniform RWStructuredBuffer<half> rowMajorBuffer)
{
    int tileIdx = int(gid.x);
    if (tileIdx >= TotalTiles)
        return;

    static groupshared uint4 shmem[OPA_TileStride];

    uint laneId = WaveGetLaneIndex();
    int base = tileIdx * OPA_TileStride;
    for (uint i = laneId; i < OPA_TileStride; i += 32)
        shmem[i] = reinterpret<RWStructuredBuffer<uint4>>(nativeBuffer)[base + int(i)];

    GroupMemoryBarrierWithGroupSync();

    let tile = MatC.LoadRaw(shmem, 0u);

    tile.Store<linalg.CoopMatMatrixLayout.RowMajor>(rowMajorBuffer, uint(tileIdx * 256), 16u);
}

// Debug kernel: dumps dOut fragment registers after buildAllTilesFromArray + transposeTilesA + ChangeMajor.
// Compare with Tin2's Stage 3: change_major_axis(dout.transpose()).
typealias MatADbg = linalg.CoopMat<half, MemoryScope.Subgroup, 16, 16, linalg.CoopMatMatrixUse.MatrixA>;

[shader("compute")]
[numthreads(32, 1, 1)]
void dump_dout_transformed(
    uint gtid : SV_GroupIndex,
    uniform RWStructuredBuffer<uint> output)
{
    uint laneId = WaveGetLaneIndex();

    half arr[16];
    for (int i = 0; i < 16; i++)
        arr[i] = half(laneId + 1) * half(i + 1) / half(32);

    MatADbg tiles[2];
    tiles[0].clear();
    tiles[1].clear();
    MMA.buildAllTilesFromArray(arr, tiles);

    // Dump 3 stages: before swap, after swap, after ChangeMajor
    // Stage 0: original (offset 0)
    for (int t = 0; t < 2; t++)
        for (int r = 0; r < 4; r++)
            output[0 * 256 + laneId * 8 + t * 4 + r] = tiles[t].fragmentRead(r);

    // Apply transpose (reg swap 1↔2, 5↔6)
    MMA.transposeTilesA(tiles);

    // Stage 1: after swap (offset 256)
    for (int t = 0; t < 2; t++)
        for (int r = 0; r < 4; r++)
            output[1 * 256 + laneId * 8 + t * 4 + r] = tiles[t].fragmentRead(r);

    // Apply ChangeMajor (movmatrix)
    [ForceUnroll]
    for (int i = 0; i < 2; i++)
        tiles[i].ChangeMajor();

    // Stage 2: after ChangeMajor (offset 512)
    for (int t = 0; t < 2; t++)
        for (int r = 0; r < 4; r++)
            output[2 * 256 + laneId * 8 + t * 4 + r] = tiles[t].fragmentRead(r);
}

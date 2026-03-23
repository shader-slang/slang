// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Simple SINGLE LAYER forward pass benchmark with tiled (OptimalLayout) weights.
// Profiles just the linearTransform (MatVecMul), no activation.

import slang.neural;

// Network configuration - single layer: INPUT_SIZE -> OUTPUT_SIZE
#ifndef INPUT_SIZE
#define INPUT_SIZE 64
#endif
#ifndef OUTPUT_SIZE
#define OUTPUT_SIZE 16
#endif

// CoopMat configuration
static const int SubgroupSize = 32;
static const int BatchSize = 32;

// Use bindless address
typealias Address = BindlessAddress<half>;

// Tiled layout: weight matrix is OUTPUT_SIZE x INPUT_SIZE, stored as
// ceil(OUTPUT_SIZE/16) * ceil(INPUT_SIZE/16) tiles of 16x16 (256 elements each).
static const int NTileRows = (OUTPUT_SIZE + 15) / 16;
static const int NTileCols = (INPUT_SIZE + 15) / 16;
static const int TILED_WEIGHT_COUNT = NTileRows * NTileCols * 256;

// Bias is stored after the tiled weight data.
static const int WEIGHT_OFFSET = 0;
static const int BIAS_OFFSET = TILED_WEIGHT_COUNT;
static const int PARAM_COUNT = TILED_WEIGHT_COUNT + OUTPUT_SIZE;

// Query functions
public int get_input_size() { return INPUT_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }
public int get_total_params() { return PARAM_COUNT; }
public int get_tiled_weight_count() { return TILED_WEIGHT_COUNT; }

// Shared memory size for single layer
typealias ShMemSize = SharedMemorySize<half, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, BatchSize / SubgroupSize>;
typealias ShMemSizeLayer = ShMemSize.OfLayer1<INPUT_SIZE, OUTPUT_SIZE>;

// Define vector types (use logical sizes, not padded)
typealias VInput = WaveTangledVector<half, ShMemSizeLayer, INPUT_SIZE, SubgroupSize, 1>;
typealias VOutput = WaveTangledVector<half, ShMemSizeLayer, OUTPUT_SIZE, SubgroupSize, 1>;

// Compute shader entry point - one thread group per sample
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_single_layer_forward_tiled(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform RWStructuredBuffer<half>.Handle params,
    uniform StructuredBuffer<half> inputs,
    uniform RWStructuredBuffer<half> outputs,
    uniform int batch_size)
{
    int sampleIdx = int(gid.x);
    if (sampleIdx >= batch_size)
        return;

    let baseAddr = Address(params);
    let weightAddr = baseAddr.getOffset(WEIGHT_OFFSET);
    let biasAddr = baseAddr.getOffset(BIAS_OFFSET);

    // Load input
    half inputArr[INPUT_SIZE];
    int inBaseIdx = sampleIdx * INPUT_SIZE;
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
    {
        inputArr[i] = inputs[inBaseIdx + i];
    }
    let input = VInput(inputArr);

    // Forward: linearTransform with OptimalLayout (tiled weight matrix)
    let output = input.linearTransform<Address, OptimalLayout, VOutput>(weightAddr, biasAddr);

    // Write output
    if (gtid == 0)
    {
        int outBaseIdx = sampleIdx * OUTPUT_SIZE;
        [ForceUnroll]
        for (int i = 0; i < OUTPUT_SIZE; i++)
        {
            outputs[outBaseIdx + i] = output[i];
        }
    }
}

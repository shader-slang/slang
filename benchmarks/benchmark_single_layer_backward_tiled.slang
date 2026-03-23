// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Single-layer backward benchmark using TiledMMAHelper via neural_unittest.testTiledBackward.
// Calls the three backward operations explicitly (no autodiff):
//   1. dIn = W^T * dOut         (transpose MMA)
//   2. dBias += sum(dOut)       (bias reduce via MMA)
//   3. dW += dOut * input^T     (outer product accumulate, tiled store)

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
#define SUBGROUP_COUNT 1
#endif
static const int SubgroupCount = SUBGROUP_COUNT;

// Use float for all buffers: Vulkan doesn't support half atomicAdd,
// and the outer product / bias reduce accumulate via atomics.
// MMA internally still uses half for MatA/MatB (converts from float).
typealias DType = float;

static const int NTileRows = (OUTPUT_SIZE + 15) / 16;
static const int NTileCols = (INPUT_SIZE + 15) / 16;
static const int TILED_WEIGHT_COUNT = NTileRows * NTileCols * 256;

public int get_input_size() { return INPUT_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }
public int get_tiled_weight_count() { return TILED_WEIGHT_COUNT; }

public struct LargeShMem : ISharedMemorySize { public static const uint Bytes = 40 * 1024; }

[shader("compute")]
#if SUBGROUP_COUNT == 2
[numthreads(64, 1, 1)]
#else
[numthreads(32, 1, 1)]
#endif
void compute_single_layer_backward_tiled(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform RWStructuredBuffer<DType>.Handle weights,
    uniform StructuredBuffer<DType> inputs,
    uniform StructuredBuffer<DType> grad_outputs,
    uniform RWStructuredBuffer<DType>.Handle grad_weights,
    uniform RWStructuredBuffer<DType>.Handle grad_bias,
    uniform RWStructuredBuffer<DType> grad_inputs,
    uniform int batch_size)
{
#if SUBGROUP_COUNT == 2
    int sampleIdx = int(gid.x) * 2 + int(gtid / 32);
#else
    int sampleIdx = int(gid.x);
#endif
    if (sampleIdx >= batch_size)
        return;

    DType inputArr[INPUT_SIZE];
    int inBase = sampleIdx * INPUT_SIZE;
    for (int i = 0; i < INPUT_SIZE; i++)
        inputArr[i] = inputs[inBase + i];

    DType dOutArr[OUTPUT_SIZE];
    int gradOutBase = sampleIdx * OUTPUT_SIZE;
    for (int i = 0; i < OUTPUT_SIZE; i++)
        dOutArr[i] = grad_outputs[gradOutBase + i];

    let dInput = neural_unittest.testTiledBackward<DType, INPUT_SIZE, OUTPUT_SIZE, LargeShMem, SubgroupCount>(
        inputArr, dOutArr, weights, grad_weights, grad_bias);

    if (gtid == 0)
    {
        int gradInBase = sampleIdx * INPUT_SIZE;
        for (int i = 0; i < INPUT_SIZE; i++)
            grad_inputs[gradInBase + i] = dInput[i];
    }
}

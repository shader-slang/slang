// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Simple SINGLE LAYER backward pass benchmark.
// Input -> Output directly (no hidden layer)
// This should require much less shared memory than 2-layer MLP.

import neural;

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

// Use half precision storage
typealias Storage = StructuredBufferStorage<half>;

// Single layer parameter layout:
// weights [INPUT_SIZE * OUTPUT_SIZE], biases [OUTPUT_SIZE]
static const int LAYER_WEIGHTS = INPUT_SIZE * OUTPUT_SIZE;
static const int LAYER_BIASES = OUTPUT_SIZE;
static const int PARAM_COUNT = LAYER_WEIGHTS + LAYER_BIASES;

static const uint WEIGHT_OFFSET = 0u;
static const uint BIAS_OFFSET = uint(LAYER_WEIGHTS);

// Query functions
public int get_input_size() { return INPUT_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }
public int get_total_params() { return PARAM_COUNT; }

// Shared memory size for single layer (much smaller than 2-layer!)
typealias ShMemSize = SharedMemorySize<half, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, BatchSize / SubgroupSize>;
typealias ShMemSizeLayer = ShMemSize.OfLayer1<INPUT_SIZE, OUTPUT_SIZE>;

// Define vector types
typealias ShMemPool = SharedMemoryPool<ShMemSizeLayer>;
typealias VInput = WaveTangledVector<half, ShMemPool, INPUT_SIZE, SubgroupSize>;
typealias VOutput = WaveTangledVector<half, ShMemPool, OUTPUT_SIZE, SubgroupSize>;
typealias Act = ReLU<half>;
typealias Layer = FFLayer<half, VInput, VOutput, Storage, LinearLayout, Act, true>;

// Simple differentiable forward function - just one layer!
[Differentiable]
VOutput singleLayerForward(Storage storage, VInput input, Layer layer)
{
    return layer.eval<Storage>(storage, input);
}

// Compute shader entry point - one thread group per sample
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_single_layer_backward(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    RWStructuredBuffer<half> params,
    StructuredBuffer<half> inputs,
    StructuredBuffer<half> grad_outputs,
    RWStructuredBuffer<half> grad_params,
    RWStructuredBuffer<half> grad_inputs,
    uniform int batch_size)
{
    int sampleIdx = int(gid.x);
    if (sampleIdx >= batch_size)
        return;
    
    let storage = Storage(params);
    let gradStorage = Storage(grad_params);
    let layer = Layer(WEIGHT_OFFSET, BIAS_OFFSET);
    
    // Load input
    half inputArr[INPUT_SIZE];
    int inBaseIdx = sampleIdx * INPUT_SIZE;
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
    {
        inputArr[i] = inputs[inBaseIdx + i];
    }
    let input = VInput(inputArr);
    
    // Load output gradient
    half gradOutArr[OUTPUT_SIZE];
    int gradOutBaseIdx = sampleIdx * OUTPUT_SIZE;
    [ForceUnroll]
    for (int i = 0; i < OUTPUT_SIZE; i++)
    {
        gradOutArr[i] = grad_outputs[gradOutBaseIdx + i];
    }
    let dOutput = VOutput(gradOutArr);
    
    // Backward pass
    var storagePair = DifferentialPtrPair<Storage>(storage, gradStorage);
    var inputPair = diffPair(input);
    
    bwd_diff(singleLayerForward)(storagePair, inputPair, layer, dOutput);
    
    // Write input gradients
    if (gtid == 0)
    {
        int gradInBaseIdx = sampleIdx * INPUT_SIZE;
        [ForceUnroll]
        for (int i = 0; i < INPUT_SIZE; i++)
        {
            grad_inputs[gradInBaseIdx + i] = inputPair.d[i];
        }
    }
}

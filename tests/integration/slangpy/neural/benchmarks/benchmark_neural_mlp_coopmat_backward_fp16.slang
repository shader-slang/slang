// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// CoopMat neural MLP BACKWARD pass benchmark using WaveTangledVector backend.
// FP16 version for fair comparison with Tin2/Tin3 fp16 backward benchmarks.
// Uses autodiff (bwd_diff) for backward pass computation.

import neural;

// Network configuration - use defines or defaults
#ifndef INPUT_SIZE
#define INPUT_SIZE 64
#endif
#ifndef HIDDEN_SIZE
#define HIDDEN_SIZE 64
#endif
#ifndef OUTPUT_SIZE
#define OUTPUT_SIZE 16
#endif

// CoopMat configuration
static const int SubgroupSize = 32;
static const int BatchSize = 32;

// Use half precision storage
typealias Storage = StructuredBufferStorage<half>;

// Parameter layout:
// Layer 1: weights [INPUT_SIZE * HIDDEN_SIZE], biases [HIDDEN_SIZE]
// Layer 2: weights [HIDDEN_SIZE * OUTPUT_SIZE], biases [OUTPUT_SIZE]
static const int LAYER1_WEIGHTS = INPUT_SIZE * HIDDEN_SIZE;
static const int LAYER1_BIASES = HIDDEN_SIZE;
static const int LAYER2_WEIGHTS = HIDDEN_SIZE * OUTPUT_SIZE;
static const int LAYER2_BIASES = OUTPUT_SIZE;

static const int PARAM_COUNT = LAYER1_WEIGHTS + LAYER1_BIASES + LAYER2_WEIGHTS + LAYER2_BIASES;

// Parameter offsets
static const uint LAYER1_WEIGHT_OFFSET = 0u;
static const uint LAYER1_BIAS_OFFSET = uint(LAYER1_WEIGHTS);
static const uint LAYER2_WEIGHT_OFFSET = uint(LAYER1_WEIGHTS + LAYER1_BIASES);
static const uint LAYER2_BIAS_OFFSET = uint(LAYER1_WEIGHTS + LAYER1_BIASES + LAYER2_WEIGHTS);

// Query functions
public int get_input_size() { return INPUT_SIZE; }
public int get_hidden_size() { return HIDDEN_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }
public int get_total_params() { return PARAM_COUNT; }

// Shared memory size calculation for 2-layer network (using half)
// Use Training mode for backward pass
typealias ShMemSize = SharedMemorySize<half, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, BatchSize / SubgroupSize>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<INPUT_SIZE, HIDDEN_SIZE>;
typealias ShMemSizeLayer2 = ShMemSize.OfLayer1<HIDDEN_SIZE, OUTPUT_SIZE>;
typealias ShMemSizeTotal = ShMemSizeLayer1;

// Define vector types globally for use in differentiable function
typealias ShMemPool = SharedMemoryPool<ShMemSizeTotal>;
typealias VInput = WaveTangledVector<half, ShMemPool, INPUT_SIZE, SubgroupSize>;
typealias VHidden = WaveTangledVector<half, ShMemPool, HIDDEN_SIZE, SubgroupSize>;
typealias VOutput = WaveTangledVector<half, ShMemPool, OUTPUT_SIZE, SubgroupSize>;
typealias Act = ReLU<half>;
typealias Layer1 = FFLayer<half, VInput, VHidden, Storage, LinearLayout, Act, true>;
typealias Layer2 = FFLayer<half, VHidden, VOutput, Storage, LinearLayout, Act, true>;

// Differentiable forward function for autodiff
[Differentiable]
VOutput mlpForward(Storage storage, VInput input, Layer1 layer1, Layer2 layer2)
{
    let hidden = layer1.eval<Storage>(storage, input);
    let output = layer2.eval<Storage>(storage, hidden);
    return output;
}

// Compute shader entry point - one thread group per sample
// Backward pass: computes weight gradients via autodiff
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_batch_backward(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    RWStructuredBuffer<half> params,         // Network parameters
    StructuredBuffer<half> inputs,           // Original inputs
    StructuredBuffer<half> grad_outputs,     // Gradient from loss
    RWStructuredBuffer<half> grad_params,    // Output: accumulated parameter gradients
    RWStructuredBuffer<half> grad_inputs,    // Output: input gradients
    uniform int batch_size)
{
    // Each thread group processes one sample
    int sampleIdx = int(gid.x);
    if (sampleIdx >= batch_size)
        return;
    
    // Create storage objects
    let storage = Storage(params);
    let gradStorage = Storage(grad_params);
    
    // Create layers
    let layer1 = Layer1(LAYER1_WEIGHT_OFFSET, LAYER1_BIAS_OFFSET);
    let layer2 = Layer2(LAYER2_WEIGHT_OFFSET, LAYER2_BIAS_OFFSET);
    
    // Load input for this sample
    half inputArr[INPUT_SIZE];
    int inBaseIdx = sampleIdx * INPUT_SIZE;
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
    {
        inputArr[i] = inputs[inBaseIdx + i];
    }
    let input = VInput(inputArr);
    
    // Load gradient from loss
    half gradOutArr[OUTPUT_SIZE];
    int gradOutBaseIdx = sampleIdx * OUTPUT_SIZE;
    [ForceUnroll]
    for (int i = 0; i < OUTPUT_SIZE; i++)
    {
        gradOutArr[i] = grad_outputs[gradOutBaseIdx + i];
    }
    let dOutput = VOutput(gradOutArr);
    
    // Create DifferentialPtrPair for storage (primal, gradient)
    var storagePair = DifferentialPtrPair<Storage>(storage, gradStorage);
    
    // Create diffPair for input
    var inputPair = diffPair(input);
    
    // Compute backward pass using autodiff
    bwd_diff(mlpForward)(storagePair, inputPair, layer1, layer2, dOutput);
    
    // Write input gradients (from inputPair.d)
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

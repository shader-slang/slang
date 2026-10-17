// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// CoopMat neural MLP benchmark using WaveTangledVector backend.
// FP16 version for fair comparison with Tin2/Tin3 fp16 benchmarks.
// Network architecture configurable via preprocessor defines.

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
typealias ShMemSize = SharedMemorySize<half, TargetEnum.CUDA, ExecutionMode.Inference, SubgroupSize, BatchSize / SubgroupSize>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<INPUT_SIZE, HIDDEN_SIZE>;
typealias ShMemSizeLayer2 = ShMemSize.OfLayer1<HIDDEN_SIZE, OUTPUT_SIZE>;
// Use max of both layers for shared memory pool
typealias ShMemSizeTotal = ShMemSizeLayer1;  // Assuming layer1 needs more (larger input)

// Compute shader entry point - one thread group per sample
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_batch_forward(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    RWStructuredBuffer<half> params,
    StructuredBuffer<half> inputs,
    RWStructuredBuffer<half> outputs,
    uniform int batch_size)
{
    // Each thread group processes one sample
    int sampleIdx = int(gid.x);
    if (sampleIdx >= batch_size)
        return;
    
    // Define CoopMat types inside shader function (using half)
    typealias ShMemPool = SharedMemoryPool<ShMemSizeTotal>;
    
    // Layer 1: INPUT -> HIDDEN
    typealias VInput = WaveTangledVector<half, ShMemPool, INPUT_SIZE, SubgroupSize>;
    typealias VHidden = WaveTangledVector<half, ShMemPool, HIDDEN_SIZE, SubgroupSize>;
    typealias Act1 = ReLU<half>;
    typealias Layer1 = FFLayer<half, VInput, VHidden, Storage, LinearLayout, Act1, true>;
    
    // Layer 2: HIDDEN -> OUTPUT
    typealias VOutput = WaveTangledVector<half, ShMemPool, OUTPUT_SIZE, SubgroupSize>;
    typealias Act2 = ReLU<half>;
    typealias Layer2 = FFLayer<half, VHidden, VOutput, Storage, LinearLayout, Act2, true>;
    
    let storage = Storage(params);
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
    
    // Forward pass through 2 layers
    let input = VInput(inputArr);
    let hidden = layer1.eval<Storage>(storage, input);
    let output = layer2.eval<Storage>(storage, hidden);
    
    // Only first thread writes result
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

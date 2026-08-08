// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// CoopVec benchmark using direct coopVecMatMulAdd intrinsics.
// Network architecture configurable via preprocessor defines.
// This mimics the original SlangPy samples (CoopVecLinearLayer).

// Use half precision like original samples
typealias NFloat = half;
static const CoopVecComponentType kComponentType = CoopVecComponentType.Float16;

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

// MLVec wrapper for CoopVec (from original mlp example)
struct MLVec<let N : int>
{
    CoopVec<NFloat, N> data;
    
    static MLVec<N> fromArray(NFloat arr[N])
    {
        MLVec<N> result;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            result.data[i] = arr[i];
        return result;
    }
    
    NFloat[N] toArray()
    {
        NFloat[N] arr;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            arr[i] = data[i];
        return arr;
    }
}

// Feed-forward layer using CoopVec (from original mlp example)
struct FeedForwardLayer<let InputSize : int, let OutputSize : int>
{
    void* weights;
    void* biases;
    
    MLVec<OutputSize> eval(MLVec<InputSize> input)
    {
        // Compute mul(weights, inputVec) + biases
        var output = coopVecMatMulAdd<NFloat, OutputSize>(
            input.data, kComponentType,
            weights, kComponentType,
            biases, kComponentType,
            CoopVecMatrixLayout.RowMajor,
            false,
            InputSize * sizeof(NFloat));
        
        // LeakyReLU activation
        output = max(output, output * NFloat(0.01h));
        return {output};
    }
}

// Network parameters passed from host
public struct NetworkParams
{
    void* layer1_weights;
    void* layer1_biases;
    void* layer2_weights;
    void* layer2_biases;
}

// Query functions
public int get_input_size() { return INPUT_SIZE; }
public int get_hidden_size() { return HIDDEN_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }

// Compute shader entry point - one thread group per sample
[shader("compute")]
[numthreads(32, 1, 1)]
[require(spvCooperativeVectorNV)]
void compute_batch_forward(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform NetworkParams network,
    StructuredBuffer<NFloat> inputs,
    RWStructuredBuffer<NFloat> outputs,
    uniform int batch_size)
{
    // Each thread group processes one sample
    int sampleIdx = int(gid.x);
    if (sampleIdx >= batch_size)
        return;
    
    // Setup layers
    FeedForwardLayer<INPUT_SIZE, HIDDEN_SIZE> layer1;
    layer1.weights = network.layer1_weights;
    layer1.biases = network.layer1_biases;
    
    FeedForwardLayer<HIDDEN_SIZE, OUTPUT_SIZE> layer2;
    layer2.weights = network.layer2_weights;
    layer2.biases = network.layer2_biases;
    
    // Load input into array
    int inBaseIdx = sampleIdx * INPUT_SIZE;
    NFloat inputArr[INPUT_SIZE];
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; i++)
    {
        inputArr[i] = inputs[inBaseIdx + i];
    }
    
    // Forward pass
    let input = MLVec<INPUT_SIZE>.fromArray(inputArr);
    let hidden = layer1.eval(input);
    let output = layer2.eval(hidden);
    
    // Write output (only first thread in group)
    if (gtid == 0)
    {
        let outArr = output.toArray();
        int outBaseIdx = sampleIdx * OUTPUT_SIZE;
        [ForceUnroll]
        for (int i = 0; i < OUTPUT_SIZE; i++)
        {
            outputs[outBaseIdx + i] = outArr[i];
        }
    }
}
